/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#include <math.h>

#define FMI_MAX_MESSAGE_LENGTH 4096

#define INTERNET_MAX_URL_LENGTH 2083

#ifdef GRTFMI
extern const char *FMU_RESOURCES_DIR;
#endif

#define FMI_STATIC static

#include "FMI.c"
#include "FMI1.c"
#include "FMI2.c"
#include "FMI3.c"

#ifdef _WIN32

#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#else

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#endif

#ifndef S_FUNCTION_NAME
#define S_FUNCTION_NAME sfun_fmurun
#endif

#define S_FUNCTION_LEVEL 2

#include "simstruc.h"


typedef enum {

	fmiVersionParam,
	runAsKindParam,
	guidParam,
	modelIdentifierParam,
	unzipDirectoryParam,
    debugLoggingParam,
    logFMICallsParam,
    logLevelParam,
    logFileParam,
	relativeToleranceParam,
	sampleTimeParam,
	offsetTimeParam,
	nxParam,
	nzParam,
    resettableParam,
	inputPortWidthsParam,
	inputPortDirectFeedThroughParam,
	inputPortTypesParam,
	inputPortVariableVRsParam,
	outputPortWidthsParam,
	outputPortTypesParam,
	outputPortVariableVRsParam,
    numParams

} Parameter;

static size_t typeSizes[13] = {
    sizeof(real32_T),  //FMIFloat32Type,
    sizeof(real32_T),  //FMIDiscreteFloat32Type,
    sizeof(real_T),    //FMIFloat64Type,
    sizeof(real_T),    //FMIDiscreteFloat64Type,
    sizeof(int8_T),    //FMIInt8Type,
    sizeof(uint8_T),   //FMIUInt8Type,
    sizeof(int16_T),   //FMIInt16Type,
    sizeof(uint16_T),  //FMIUInt16Type,
    sizeof(int32_T),   //FMIInt32Type,
    sizeof(uint32_T),  //FMIUInt32Type,
    sizeof(int32_T),   //FMIInt64Type,
    sizeof(uint32_T),  //FMIUInt64Type,
    sizeof(boolean_T), //FMIBooleanType,
};

static char* getStringParam(SimStruct *S, int parameter, int index) {

	const mxArray *array = ssGetSFcnParam(S, parameter);

	const int m = (int)mxGetM(array);  // number of strings
	const int n = (int)mxGetN(array);  // max length
	const mxChar *data = (mxChar *)mxGetData(array);

	char *cstr = (char *)mxMalloc(n + 1);
	memset(cstr, '\0', n + 1);

	if (index >= m || n < 1 || !data) {
		return cstr;
	}

	// copy the row
	for (int j = 0; j < n; j++) {
		cstr[j] = (char)data[j * m + index];
	}

	// remove the trailing blanks
	for (int j = n - 1; j >= 0; j--) {
		if (cstr[j] != ' ') break;
		cstr[j] = '\0';
	}
	
	return cstr; // must be mxFree()'d
}

static bool isFMI1(SimStruct *S) {
	return mxGetScalar(ssGetSFcnParam(S, fmiVersionParam)) == 1.0;
}

static bool isFMI2(SimStruct *S) {
	return mxGetScalar(ssGetSFcnParam(S, fmiVersionParam)) == 2.0;
}

static bool isFMI3(SimStruct *S) {
	return mxGetScalar(ssGetSFcnParam(S, fmiVersionParam)) == 3.0;
}

static bool isME(SimStruct *S) { 
	return mxGetScalar(ssGetSFcnParam(S, runAsKindParam)) == FMIModelExchange;
}

static bool isCS(SimStruct *S) {
	return mxGetScalar(ssGetSFcnParam(S, runAsKindParam)) == FMICoSimulation;
}

static bool debugLogging(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, debugLoggingParam));
}

static bool logFMICalls(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, logFMICallsParam));
}

static FMIStatus logLevel(SimStruct *S) {
    return (int)mxGetScalar(ssGetSFcnParam(S, logLevelParam));
}

static double relativeTolerance(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, relativeToleranceParam));
}

static double sampleTime(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, sampleTimeParam));
}

static double offsetTime(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, offsetTimeParam));
}

// number of continuous states
static int nx(SimStruct *S) {
    return (int)mxGetScalar(ssGetSFcnParam(S, nxParam));
}

// number of zero-crossings
static int nz(SimStruct *S) {
    return (int)mxGetScalar(ssGetSFcnParam(S, nzParam));
}

static bool resettable(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, resettableParam));
}

static int inputPortWidth(SimStruct *S, int index) {
	const real_T *portWidths = (const real_T *)mxGetData(ssGetSFcnParam(S, inputPortWidthsParam));
	return (int)portWidths[index];
}

static bool inputPortDirectFeedThrough(SimStruct *S, int index) {
	const real_T *directFeedThrough = (const real_T *)mxGetData(ssGetSFcnParam(S, inputPortDirectFeedThroughParam));
	return directFeedThrough[index] != 0;
}

// number of input ports
static int nu(SimStruct *S) {
	return (int)mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam));
}

// number of input variables
static int nuv(SimStruct *S) {
	return (int)mxGetNumberOfElements(ssGetSFcnParam(S, inputPortVariableVRsParam));
}

static int startValueSize(SimStruct *S, Parameter parameter, int index) {
	const real_T *sizes = (const real_T *)mxGetData(ssGetSFcnParam(S, parameter));
	return (int)sizes[index];
}

static FMIValueReference valueReference(SimStruct *S, Parameter parameter, int index) {
	const mxArray *param = ssGetSFcnParam(S, parameter);
	const real_T realValue = ((const real_T *)mxGetData(param))[index];
	return (FMIValueReference)realValue;
}

static FMIVariableType variableType(SimStruct *S, Parameter parameter, int index) {
	const mxArray *param = ssGetSFcnParam(S, parameter);
	const real_T realValue = ((const real_T *)mxGetData(param))[index];
	const int intValue = (int)realValue;
	return (FMIVariableType)intValue;
}

static DTypeId simulinkVariableType(SimStruct *S, Parameter parameter, size_t index) {
	
	const mxArray *param = ssGetSFcnParam(S, parameter);
	const real_T realValue = ((real_T *)mxGetData(param))[index];
	const int intValue = (int)realValue;
	const FMIVariableType type = (FMIVariableType)intValue;
	
	switch (type) {
	case FMIFloat32Type:
	case FMIDiscreteFloat32Type:
		return SS_SINGLE;
	case FMIFloat64Type:
	case FMIDiscreteFloat64Type:
		return SS_DOUBLE;
	case FMIInt8Type:     
		return SS_INT8;
	case FMIUInt8Type:    
		return SS_UINT8;
	case FMIInt16Type:    
		return SS_INT16;
	case FMIUInt16Type:   
		return SS_UINT16;
	case FMIInt32Type:    
		return SS_INT32;
	case FMIUInt32Type:   
		return SS_UINT32;
	case FMIInt64Type:    
		return SS_INT32;
	case FMIUInt64Type:   
		return SS_UINT32;
	case FMIBooleanType:  
		return SS_BOOLEAN;
	default:              
		return -1; // error
	}
}

static real_T scalarValue(SimStruct *S, Parameter parameter, size_t index) {
	const mxArray *param = ssGetSFcnParam(S, parameter);
	return ((real_T *)mxGetData(param))[index];
}

static int outputPortWidth(SimStruct *S, size_t index) {
	const real_T *portWidths = (const real_T *)mxGetData(ssGetSFcnParam(S, outputPortWidthsParam));
	return (int)portWidths[index];
}

static int ny(SimStruct *S) { 
	return (int)mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam));
}

static bool initialized(SimStruct* S) {

	void** p = ssGetPWork(S);

	FMIInstance* instance = (FMIInstance*)p[0];

	switch (instance->state) {
	case FMIEventModeState:
	case FMIContinuousTimeModeState:
	case FMIStepModeState:
		return true;
	default: 
		return false;
	}
}

static void logCall(SimStruct *S, const char* message) {

    FILE *logfile = NULL;

	void **p = ssGetPWork(S);

    if (p) {
        logfile = (FILE *)p[1];
    }

    if (logfile) {
        fputs(message, logfile);
        fputs("\n", logfile);
        fflush(logfile);
    } else {
        ssPrintf(message);
        ssPrintf("\n");
    }
}

static void appendStatus(FMIStatus status, char *message, size_t size) {
	
	const char *ret;

	switch (status) {
	case FMIOK:
		ret = " -> OK";
		break;
	case FMIWarning:
		ret = " -> Warning";
		break;
	case FMIDiscard:
		ret = " -> Discard";
		break;
	case FMIError:
		ret = " -> Error";
		break;
	case FMIFatal:
		ret = " -> Fatal";
		break;
	case FMIPending:
		ret = " -> Pending";
		break;
	default:
		ret = "Illegal status code";
		break;
	}

	strncat(message, ret, size);
}

static void cb_logMessage(FMIInstance *instance, FMIStatus status, const char *category, const char * message) {
	
	SimStruct *S = (SimStruct *)instance->userData;

	if (status < logLevel(S)) {
		return;
	}

	char buf[FMI_MAX_MESSAGE_LENGTH];

	size_t len = snprintf(buf, FMI_MAX_MESSAGE_LENGTH, "[%s] ", instance->name);

	strncat(buf, message, FMI_MAX_MESSAGE_LENGTH);

	appendStatus(status, buf, FMI_MAX_MESSAGE_LENGTH);
	
	logCall(S, buf);
}

static void cb_logFunctionCall(FMIInstance *instance, FMIStatus status, const char *message) {
	
	char buf[FMI_MAX_MESSAGE_LENGTH];

	snprintf(buf, FMI_MAX_MESSAGE_LENGTH, "[%s] %s", instance->name, message);

	appendStatus(status, buf, FMI_MAX_MESSAGE_LENGTH);

	SimStruct *S = (SimStruct *)instance->userData;

	logCall(S, buf);
}

#ifdef _WIN32
const char * exceptionCodeToString(DWORD exceptionCode) {
	switch (exceptionCode) {
	case STATUS_WAIT_0:						return "WAIT_0";
	case STATUS_ABANDONED_WAIT_0:			return "ABANDONED_WAIT_0";
	case STATUS_USER_APC:					return "USER_APC";
	case STATUS_TIMEOUT:					return "TIMEOUT";
	case STATUS_PENDING:					return "PENDING";
	case DBG_EXCEPTION_HANDLED:				return "EXCEPTION_HANDLED";
	case DBG_CONTINUE:						return "CONTINUE";
	case STATUS_SEGMENT_NOTIFICATION:		return "SEGMENT_NOTIFICATION";
	case STATUS_FATAL_APP_EXIT:				return "FATAL_APP_EXIT";
	case DBG_TERMINATE_THREAD:				return "TERMINATE_THREAD";
	case DBG_TERMINATE_PROCESS:				return "TERMINATE_PROCESS";
	case DBG_CONTROL_C:						return "CONTROL_C";
	case DBG_PRINTEXCEPTION_C:				return "PRINTEXCEPTION_C";
	case DBG_RIPEXCEPTION:					return "RIPEXCEPTION";
	case DBG_CONTROL_BREAK:					return "CONTROL_BREAK";
	case DBG_COMMAND_EXCEPTION:				return "COMMAND_EXCEPTION";
	case STATUS_GUARD_PAGE_VIOLATION:		return "GUARD_PAGE_VIOLATION";
	case STATUS_DATATYPE_MISALIGNMENT:		return "DATATYPE_MISALIGNMENT";
	case STATUS_BREAKPOINT:					return "BREAKPOINT";
	case STATUS_SINGLE_STEP:				return "SINGLE_STEP";
	case STATUS_LONGJUMP:					return "LONGJUMP";
	case STATUS_UNWIND_CONSOLIDATE:			return "UNWIND_CONSOLIDATE";
	case DBG_EXCEPTION_NOT_HANDLED:			return "EXCEPTION_NOT_HANDLED";
	case STATUS_ACCESS_VIOLATION:			return "ACCESS_VIOLATION";
	case STATUS_IN_PAGE_ERROR:				return "IN_PAGE_ERROR";
	case STATUS_INVALID_HANDLE:				return "INVALID_HANDLE";
	case STATUS_INVALID_PARAMETER:			return "INVALID_PARAMETER";
	case STATUS_NO_MEMORY:					return "NO_MEMORY";
	case STATUS_ILLEGAL_INSTRUCTION:		return "ILLEGAL_INSTRUCTION";
	case STATUS_NONCONTINUABLE_EXCEPTION:	return "NONCONTINUABLE_EXCEPTION";
	case STATUS_INVALID_DISPOSITION:		return "INVALID_DISPOSITION";
	case STATUS_ARRAY_BOUNDS_EXCEEDED:		return "ARRAY_BOUNDS_EXCEEDED";
	case STATUS_FLOAT_DENORMAL_OPERAND:		return "FLOAT_DENORMAL_OPERAND";
	case STATUS_FLOAT_DIVIDE_BY_ZERO:		return "FLOAT_DIVIDE_BY_ZERO";
	case STATUS_FLOAT_INEXACT_RESULT:		return "FLOAT_INEXACT_RESULT";
	case STATUS_FLOAT_INVALID_OPERATION:	return "FLOAT_INVALID_OPERATION";
	case STATUS_FLOAT_OVERFLOW:				return "FLOAT_OVERFLOW";
	case STATUS_FLOAT_STACK_CHECK:			return "FLOAT_STACK_CHECK";
	case STATUS_FLOAT_UNDERFLOW:			return "FLOAT_UNDERFLOW";
	case STATUS_INTEGER_DIVIDE_BY_ZERO:		return "INTEGER_DIVIDE_BY_ZERO";
	case STATUS_INTEGER_OVERFLOW:			return "INTEGER_OVERFLOW";
	case STATUS_PRIVILEGED_INSTRUCTION:		return "PRIVILEGED_INSTRUCTION";
	case STATUS_STACK_OVERFLOW:				return "STACK_OVERFLOW";
	case STATUS_DLL_NOT_FOUND:				return "DLL_NOT_FOUND";
	case STATUS_ORDINAL_NOT_FOUND:			return "ORDINAL_NOT_FOUND";
	case STATUS_ENTRYPOINT_NOT_FOUND:		return "ENTRYPOINT_NOT_FOUND";
	case STATUS_CONTROL_C_EXIT:				return "CONTROL_C_EXIT";
	case STATUS_DLL_INIT_FAILED:			return "DLL_INIT_FAILED";
	case STATUS_FLOAT_MULTIPLE_FAULTS:		return "FLOAT_MULTIPLE_FAULTS";
	case STATUS_FLOAT_MULTIPLE_TRAPS:		return "FLOAT_MULTIPLE_TRAPS";
	case STATUS_REG_NAT_CONSUMPTION:		return "REG_NAT_CONSUMPTION";
	//case STATUS_HEAP_CORRUPTION:			return "HEAP_CORRUPTION";
	case STATUS_STACK_BUFFER_OVERRUN:		return "STACK_BUFFER_OVERRUN";
	case STATUS_INVALID_CRUNTIME_PARAMETER: return "INVALID_CRUNTIME_PARAMETER";
	case STATUS_ASSERTION_FAILURE:			return "ASSERTION_FAILURE";
	case STATUS_SXS_EARLY_DEACTIVATION:		return "SXS_EARLY_DEACTIVATION";
	case STATUS_SXS_INVALID_DEACTIVATION:	return "SXS_INVALID_DEACTIVATION";
	default:								return "UNKOWN_EXEPTION_CODE";
	}
}

#define CHECK_STATUS(s) \
	__try { \
		if (s > fmi2Warning) { \
			setErrorStatus(S, "The FMU reported an error."); \
			return; \
		} \
	} __except (EXCEPTION_EXECUTE_HANDLER) { \
		setErrorStatus(S, "The FMU crashed (exception code 0x%lx (%s)).", GetExceptionCode(), exceptionCodeToString(GetExceptionCode())); \
		return; \
	}
#else
#define CHECK_STATUS(s) if (s > fmi2Warning) { ssSetErrorStatus(S, "The FMU encountered an error."); return; }
#endif


#define CHECK_ERROR(f) f; if (ssGetErrorStatus(S)) return;


/* log mdl*() and fmi*() calls */
static void logDebug(SimStruct *S, const char* message, ...) {

    if (logFMICalls(S)) {
		
		char buf[FMI_MAX_MESSAGE_LENGTH];
		
		size_t len = snprintf(buf, FMI_MAX_MESSAGE_LENGTH, "[%s] ", ssGetPath(S));
		
		va_list args;
		va_start(args, message);
        vsnprintf(&buf[len], FMI_MAX_MESSAGE_LENGTH - len, message, args);
        va_end(args);

        logCall(S, buf);
    }
}

static void setErrorStatus(SimStruct *S, const char *message, ...) {
	va_list args;
	va_start(args, message);
	static char msg[1024];
	vsnprintf(msg, 1024, message, args);
	ssSetErrorStatus(S, msg);
	va_end(args);
}

static void setInput(SimStruct *S, bool direct, bool discrete, bool *inputEvent) {

    *inputEvent = false;

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];
    const char *preU = p[3];

	int iu  = 0;  // input port index
    int ipu = 0;  // previous input index

	for (int i = 0; i < nu(S); i++) {

		const int w = inputPortWidth(S, i);
        
        FMIVariableType type = variableType(S, inputPortTypesParam, i);
        const size_t typeSize = typeSizes[type];
        const bool discreteVariable = type != FMIFloat32Type && type != FMIFloat64Type;

		if (direct && !inputPortDirectFeedThrough(S, i)) {
			iu += w;
            ipu += w * typeSize;
			continue;
		}

		const void *y = ssGetInputPortSignal(S, i);

		if (isFMI1(S) || isFMI2(S)) {

			for (int j = 0; j < w; j++) {

				const FMIValueReference vr = valueReference(S, inputPortVariableVRsParam, iu);

                const char *value = &((const char *)y)[j * typeSize];
                char *preValue = &preU[ipu];

                ipu += typeSize;

                if (memcmp(value, preValue, typeSize)) {
                    if (!discreteVariable || (discreteVariable && discrete)) {
                        memcpy(preValue, value, typeSize);
                    }
                    *inputEvent |= discreteVariable;
                } else {
                    iu++;
                    continue;
                }

				// set the input
				if (isFMI1(S)) {

					switch (type) {
					case FMIRealType:
						CHECK_STATUS(FMI1SetReal(instance, &vr, 1, &((const real_T *)y)[j]));
						break;
					case FMIIntegerType:
						CHECK_STATUS(FMI1SetInteger(instance, &vr, 1, &((const int32_T *)y)[j]));
						break;
					case FMIBooleanType:
						CHECK_STATUS(FMI1SetBoolean(instance, &vr, 1, &((const boolean_T *)y)[j]));
						break;
					default:
						setErrorStatus(S, "Unsupported type id for FMI 1.0: %d", type);
						return;
					}

				} else {

                    if (type == FMIRealType || (type == FMIDiscreteRealType && discrete)) {
                        CHECK_STATUS(FMI2SetReal(instance, &vr, 1, (const fmi2Real *)value));
                    } else if (type == FMIIntegerType && discrete) {
                        CHECK_STATUS(FMI2SetInteger(instance, &vr, 1, (const int32_T *)value));
                    } else if (type == FMIBooleanType && discrete) {
                        const fmi2Boolean booleanValue = *value;
                        CHECK_STATUS(FMI2SetBoolean(instance, &vr, 1, &booleanValue));
                    } else {
                        setErrorStatus(S, "Unsupported type id for FMI 2.0: %d", type);
                        return;
                    }

				}

				iu++;
			}

		} else {

            const size_t nValues = inputPortWidth(S, i);
            const FMIValueReference vr = valueReference(S, inputPortVariableVRsParam, i);

            char *preValue = &preU[ipu];

            ipu += nValues * typeSize;

            if (memcmp(y, preValue, nValues * typeSize)) {
                if (!discreteVariable || (discreteVariable && discrete)) {
                    memcpy(preValue, y, nValues * typeSize);
                }
                *inputEvent |= discreteVariable;
            } else {
                continue;
            }

			switch (type) {
			case FMIFloat32Type:
				CHECK_STATUS(FMI3SetFloat32(instance, &vr, 1, (const real32_T *)y, nValues));
				break;
			case FMIFloat64Type:
				CHECK_STATUS(FMI3SetFloat64(instance, &vr, 1, (const real_T *)y, nValues));
				break;
			case FMIInt8Type:
				CHECK_STATUS(FMI3SetInt8(instance, &vr, 1, (const int8_T *)y, nValues));
				break;
			case FMIUInt8Type:
				CHECK_STATUS(FMI3SetUInt8(instance, &vr, 1, (const uint8_T *)y, nValues));
				break;
			case FMIInt16Type:
				CHECK_STATUS(FMI3SetInt16(instance, &vr, 1, (const int16_T *)y, nValues));
				break;
			case FMIUInt16Type:
				CHECK_STATUS(FMI3SetUInt16(instance, &vr, 1, (const uint16_T *)y, nValues));
				break;
			case FMIInt32Type:
				CHECK_STATUS(FMI3SetInt32(instance, &vr, 1, (const int32_T *)y, nValues));
				break;
			case FMIUInt32Type:
				CHECK_STATUS(FMI3SetUInt32(instance, &vr, 1, (const uint32_T *)y, nValues));
				break;
			case FMIInt64Type: {
				fmi3Int64* values = (fmi3Int64*)calloc(nValues, sizeof(fmi3Int64));
				for (int j = 0; j < nValues; j++) {
					values[j] = ((const int32_T*)y)[j];
				}
				CHECK_STATUS(FMI3SetInt64(instance, &vr, 1, values, nValues));
				free(values);
				break;
			}
			case FMIUInt64Type: {
				fmi3UInt64* values = (fmi3UInt64*)calloc(nValues, sizeof(fmi3UInt64));
				for (int j = 0; j < nValues; j++) {
					values[j] = ((const uint32_T*)y)[j];
				}
				CHECK_STATUS(FMI3SetUInt64(instance, &vr, 1, values, nValues));
				free(values);
				break;
			}
			case FMIBooleanType: {
				fmi3Boolean *values = (fmi3Boolean *)calloc(nValues, sizeof(fmi3Boolean));
				for (int j = 0; j < nValues; j++) {
					 values[j] = ((const boolean_T*)y)[j];
				}
				CHECK_STATUS(FMI3SetBoolean(instance, &vr, 1, values, nValues));
				free(values);
				break;
			}
			default:
				setErrorStatus(S, "Unsupported type id for FMI 3.0: %d", type);
				return;
			}

			iu++;
		}
	}
}

static void getOutput(SimStruct *S) {

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

	int iy = 0;  // output port variable index

	for (int i = 0; i < ny(S); i++) {

		const FMIVariableType type = variableType(S, outputPortTypesParam, i);

		void *y = ssGetOutputPortSignal(S, i);

        if (isFMI1(S)) {

            for (int j = 0; j < outputPortWidth(S, i); j++) {

                const FMIValueReference vr = valueReference(S, outputPortVariableVRsParam, iy);

                switch (type) {
                case FMIRealType:
                case FMIDiscreteRealType:
                    CHECK_STATUS(FMI1GetReal(instance, &vr, 1, &((real_T *)y)[j]));
                    break;
                case FMIIntegerType:
                    CHECK_STATUS(FMI1GetInteger(instance, &vr, 1, &((int32_T *)y)[j]));
                    break;
                case FMIBooleanType:
                    CHECK_STATUS(FMI1GetBoolean(instance, &vr, 1, &((boolean_T *)y)[j]));
                    break;
                default:
                    setErrorStatus(S, "Unsupported type id for FMI 1.0: %d", type);
                    return;
                }

                iy++;
            }

        } else if (isFMI2(S)) {

            for (int j = 0; j < outputPortWidth(S, i); j++) {

                const FMIValueReference vr = valueReference(S, outputPortVariableVRsParam, iy);

                switch (type) {
                case FMIRealType:
                case FMIDiscreteRealType:
                    CHECK_STATUS(FMI2GetReal(instance, &vr, 1, &((real_T *)y)[j]));
                    break;
                case FMIIntegerType:
                    CHECK_STATUS(FMI2GetInteger(instance, &vr, 1, &((int32_T *)y)[j]));
                    break;
                case FMIBooleanType: {
                    fmi2Boolean booleanValue;
                    CHECK_STATUS(FMI2GetBoolean(instance, &vr, 1, &booleanValue));
                    ((boolean_T *)y)[j] = booleanValue;
                    break;
                }
                default:
                    setErrorStatus(S, "Unsupported type id for FMI 2.0: %d", type);
                    return;
                }
            
                iy++;
            }

		} else {

			const size_t nValues = outputPortWidth(S, i);
			const FMIValueReference vr = valueReference(S, outputPortVariableVRsParam, i);

			switch (type) {
            case FMIFloat32Type:
            case FMIDiscreteFloat32Type:
                CHECK_STATUS(FMI3GetFloat32(instance, &vr, 1, (real32_T *)y, nValues));
				break;
            case FMIFloat64Type:
            case FMIDiscreteFloat64Type:
                CHECK_STATUS(FMI3GetFloat64(instance, &vr, 1, (real_T *)y, nValues));
				break;
			case FMIInt8Type:
				CHECK_STATUS(FMI3GetInt8(instance, &vr, 1, (int8_T *)y, nValues));
				break;
			case FMIUInt8Type:
				CHECK_STATUS(FMI3GetUInt8(instance, &vr, 1, (uint8_T *)y, nValues));
				break;
			case FMIInt16Type:
				CHECK_STATUS(FMI3GetInt16(instance, &vr, 1, (int16_T *)y, nValues));
				break;
			case FMIUInt16Type:
				CHECK_STATUS(FMI3GetUInt16(instance, &vr, 1, (uint16_T *)y, nValues));
				break;
			case FMIInt32Type:
				CHECK_STATUS(FMI3GetInt32(instance, &vr, 1, (int32_T *)y, nValues));
				break;
			case FMIUInt32Type:
				CHECK_STATUS(FMI3GetUInt32(instance, &vr, 1, (uint32_T *)y, nValues));
				break;
			case FMIInt64Type: {
				fmi3Int64* values = (fmi3Int64*)calloc(nValues, sizeof(fmi3Int64));
				CHECK_STATUS(FMI3GetInt64(instance, &vr, 1, values, nValues));
				for (int j = 0; j < nValues; j++) {
					((int32_T*)y)[j] = values[j];
				}
				free(values);
				break;
			}
			case FMIUInt64Type: {
				fmi3UInt64* values = (fmi3UInt64*)calloc(nValues, sizeof(fmi3UInt64));
				CHECK_STATUS(FMI3GetUInt64(instance, &vr, 1, values, nValues));
				for (int j = 0; j < nValues; j++) {
					((uint32_T*)y)[j] = values[j];
				}
				free(values);
				break;
			}
			case FMIBooleanType: {
				fmi3Boolean *values = (fmi3Boolean *)calloc(nValues, sizeof(fmi3Boolean));
				CHECK_STATUS(FMI3GetBoolean(instance, &vr, 1, values, nValues));
				for (int j = 0; j < nValues; j++) {
					((boolean_T*)y)[j] = values[j];
				}
				free(values);
				break;
			}
			default:
				setErrorStatus(S, "Unsupported type id for FMI 3.0: %d", type);
				return;
			}
		}
	}
}


// all, only structural, only tunable
static void setParameters(SimStruct *S, bool structuralOnly, bool tunableOnly) {

    void **p = ssGetPWork(S);

    FMIInstance *instance = (FMIInstance *)p[0];
    FMIStatus status = FMIOK;

    int nSFcnParams = ssGetSFcnParamsCount(S);

    for (int i = numParams; i < nSFcnParams; i += 5) {

        const bool strucural = (bool)mxGetScalar(ssGetSFcnParam(S, i));
        const bool tunable = (bool)mxGetScalar(ssGetSFcnParam(S, i + 1));
        const FMIVariableType type = (FMIVariableType)mxGetScalar(ssGetSFcnParam(S, i + 2));
        const FMIValueReference vr = (FMIValueReference)mxGetScalar(ssGetSFcnParam(S, i + 3));

        if (structuralOnly && !strucural) continue;

        if (tunableOnly && !tunable) continue;

        if (isFMI2(S)) {

            if (instance->state == FMIContinuousTimeModeState) {
                CHECK_STATUS(FMI2EnterEventMode(instance));
            }

            // TODO: iterate over array

            const mxArray *pa = ssGetSFcnParam(S, i + 4);

            switch (type) {
            case FMIRealType:
            case FMIDiscreteRealType: {
                const fmi2Real value = (fmi2Real)mxGetScalar(pa);
                CHECK_STATUS(FMI2SetReal(instance, &vr, 1, &value));
                break;
            }
            case FMIIntegerType: {
                const fmi2Integer value = (fmi2Integer)mxGetScalar(pa);
                CHECK_STATUS(FMI2SetInteger(instance, &vr, 1, &value));
                break;
            }
            case FMIBooleanType: {
                const fmi2Boolean value = (fmi2Boolean)mxGetScalar(pa);
                CHECK_STATUS(FMI2SetBoolean(instance, &vr, 1, &value));
                break;
            }
            case FMIStringType: {
                const fmi2String value = (fmi2String)getStringParam(S, i + 4, 0);
               	CHECK_STATUS(FMI2SetString(instance, &vr, 1, (const fmi2String*)&value));
                mxFree((void*)value);
                break;
            }
            default:
                setErrorStatus(S, "Unsupported type id for FMI 2.0: %d", type);
                return;
            }

            if (instance->state == FMIEventModeState) {
                CHECK_STATUS(FMI2EnterContinuousTimeMode(instance));
            }
        
		} else if (isFMI3(S)) {

			if (instance->state == FMIContinuousTimeModeState) {
				CHECK_STATUS(FMI3EnterEventMode(instance));
			}

			// TODO: iterate over array

			const mxArray* pa = ssGetSFcnParam(S, i + 4);

			switch (type) {
			case FMIFloat32Type:
			case FMIDiscreteFloat32Type: {
				const fmi3Float32 value = (fmi3Float32)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetFloat32(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIFloat64Type:
			case FMIDiscreteFloat64Type: {
				const fmi3Float64 value = (fmi3Float64)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetFloat64(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIInt8Type: {
				const fmi3Int8 value = (fmi3Int8)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetInt8(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIUInt8Type: {
				const fmi3UInt8 value = (fmi3UInt8)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetUInt8(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIInt16Type: {
				const fmi3Int16 value = (fmi3Int16)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetInt16(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIUInt16Type: {
				const fmi3UInt16 value = (fmi3UInt16)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetUInt16(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIInt32Type: {
				const fmi3Int32 value = (fmi3Int32)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetInt32(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIUInt32Type: {
				const fmi3UInt32 value = (fmi3UInt32)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetUInt32(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIInt64Type: {
				const fmi3Int64 value = (fmi3Int64)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetInt64(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIUInt64Type: {
				const fmi3UInt64 value = (fmi3UInt64)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetUInt64(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIBooleanType: {
				const fmi3Boolean value = (fmi3Boolean)mxGetScalar(pa);
				CHECK_STATUS(FMI3SetBoolean(instance, &vr, 1, &value, 1));
				break;
			}
			case FMIStringType: {
				fmi3String value = (fmi3String)getStringParam(S, i + 4, 0);
				CHECK_STATUS(FMI3SetString(instance, &vr, 1, (const fmi3String*)&value, 1));
				mxFree((void*)value);
				break;
			}
			default:
				setErrorStatus(S, "Unsupported type id for FMI 3.0: %d", type);
				return;
			}

			if (instance->state == FMIEventModeState) {
				CHECK_STATUS(FMI3EnterContinuousTimeMode(instance));
			}

		}

    }

}

#define SET_VALUES(t) \
	{ \
		fmi3 ## t *values = calloc(s, sizeof(fmi3 ## t)); \
		for (int j = 0; j < nValues; j++) { \
			values[j] = (fmi3 ## t)realValues[iv + j]; \
		} \
		CHECK_STATUS(FMI3Set ## t(instance, &vr, 1, values, nValues)); \
		free(values); \
	}

static void update(SimStruct *S, bool inputEvent) {

	if (isCS(S)) {
		return;  // nothing to do
	}

	FMIInstance *instance = (FMIInstance *)ssGetPWork(S)[0];

	const double time = instance->time;

	real_T* nextEventTime = &(ssGetRWork(S)[2 * nz(S) + nuv(S) + 1]);

	// Work around for the event handling in Dymola FMUs:
	bool timeEvent = time >= *nextEventTime;

	if (timeEvent) {
		logDebug(S, "Time event at t=%.16g", time);
	}

	bool stepEvent = false;

	if (isFMI1(S)) {

		fmi1Boolean callEventUpdate = fmi1False;
		CHECK_STATUS(FMI1CompletedIntegratorStep(instance, &callEventUpdate))
		stepEvent = callEventUpdate;
	
	} else if (isFMI2(S)) {

		fmi2Boolean enterEventMode = fmi2False;
		fmi2Boolean terminateSimulation = fmi2False;
		CHECK_STATUS(FMI2CompletedIntegratorStep(instance, fmi2True, &enterEventMode, &terminateSimulation))
		if (terminateSimulation) {
			setErrorStatus(S, "The FMU requested to terminate the simulation.");
			return;
		}
		stepEvent = enterEventMode;
	
	} else {
	
		fmi3Boolean enterEventMode = fmi3False;
		fmi3Boolean terminateSimulation = fmi3False;
		CHECK_STATUS(FMI3CompletedIntegratorStep(instance, fmi3True, &enterEventMode, &terminateSimulation))
		if (terminateSimulation) {
			setErrorStatus(S, "The FMU requested to terminate the simulation.");
			return;
		}
		stepEvent = enterEventMode;
	}

	if (stepEvent) {
		logDebug(S, "Step event at t=%.16g\n", time);
	}

	bool stateEvent = false;

	void **p = ssGetPWork(S);

	fmi3Int32 *rootsFound = (fmi3Int32 *)p[2];

	if (nz(S) > 0) {

		real_T *prez = ssGetRWork(S);
		real_T *z = prez + nz(S);

		if (isFMI1(S)) {
			CHECK_STATUS(FMI1GetEventIndicators(instance, z, nz(S)));
		} else if (isFMI2(S)) {
			CHECK_STATUS(FMI2GetEventIndicators(instance, z, nz(S)));
		} else {
			CHECK_STATUS(FMI3GetEventIndicators(instance, z, nz(S)));
		}

		// check for state events
		for (int i = 0; i < nz(S); i++) {

			const bool rising  = (prez[i] < 0 && z[i] >= 0) || (prez[i] == 0 && z[i] > 0);
			const bool falling = (prez[i] > 0 && z[i] <= 0) || (prez[i] == 0 && z[i] < 0);

			if (rising || falling) {
				logDebug(S, "State event %s z[%d] at t=%.16g", rising ? "-\\+" : "+/-", i, instance->time);
				stateEvent = true;
				rootsFound[i] = rising ? 1 : -1;
			} else {
				rootsFound[i] = 0;
			}
		}

		// remember the current event indicators
		for (int i = 0; i < nz(S); i++) prez[i] = z[i];
	}

	if (inputEvent || timeEvent || stepEvent || stateEvent) {

		if (isFMI1(S)) {

			CHECK_ERROR(setInput(S, true, true, &inputEvent));

			fmi1EventInfo eventInfo = { 0 };
                
            CHECK_STATUS(FMI1EventUpdate(instance, fmi1False, &eventInfo));
		
		} else if (isFMI2(S)) {

			CHECK_STATUS(FMI2EnterEventMode(instance));

            CHECK_ERROR(setInput(S, true, true, &inputEvent));

			fmi2EventInfo eventInfo = { 0 };

			do {
				CHECK_STATUS(FMI2NewDiscreteStates(instance, &eventInfo));
				if (eventInfo.terminateSimulation) {
					setErrorStatus(S, "The FMU requested to terminate the simulation.");
					return;
				}
			} while (eventInfo.newDiscreteStatesNeeded);

			*nextEventTime = eventInfo.nextEventTimeDefined ? eventInfo.nextEventTime : INFINITY;

			CHECK_STATUS(FMI2EnterContinuousTimeMode(instance));
		
		} else {

            CHECK_STATUS(FMI3EnterEventMode(instance));

            CHECK_ERROR(setInput(S, true, true, &inputEvent));

			fmi3Boolean discreteStatesNeedUpdate          = fmi3False;
			fmi3Boolean terminateSimulation               = fmi3False;
			fmi3Boolean nominalsOfContinuousStatesChanged = fmi3False;
			fmi3Boolean valuesOfContinuousStatesChanged   = fmi3False;
			fmi3Boolean nextEventTimeDefined              = fmi3False;

			do {
				CHECK_STATUS(FMI3UpdateDiscreteStates(instance,
					&discreteStatesNeedUpdate,
					&terminateSimulation,
					&nominalsOfContinuousStatesChanged,
					&valuesOfContinuousStatesChanged,
					&nextEventTimeDefined,
					nextEventTime));

				if (terminateSimulation) {
					setErrorStatus(S, "The FMU requested to terminate the simulation.");
					return;
				}
			} while (discreteStatesNeedUpdate);

			CHECK_STATUS(FMI3EnterContinuousTimeMode(instance));
		}

		if (nx(S) > 0) {

			real_T *x = ssGetContStates(S);

			if (isFMI1(S)) {
				CHECK_STATUS(FMI1GetContinuousStates(instance, x, nx(S)));
			} else if (isFMI2(S)) {
				CHECK_STATUS(FMI2GetContinuousStates(instance, x, nx(S)));
			} else {
				CHECK_STATUS(FMI3GetContinuousStates(instance, x, nx(S)));
			}
		}

		if (nz(S) > 0) {

			real_T *prez = ssGetRWork(S);

			if (isFMI1(S)) {
				CHECK_STATUS(FMI1GetEventIndicators(instance, prez, nz(S)));
			} else if (isFMI2(S)) {
				CHECK_STATUS(FMI2GetEventIndicators(instance, prez, nz(S)));
			} else {
				CHECK_STATUS(FMI3GetEventIndicators(instance, prez, nz(S)));
			}
		}

		ssSetSolverNeedsReset(S);
	}
}

static bool isScalar(SimStruct *S, Parameter param) {
	const mxArray *array = ssGetSFcnParam(S, param);
	return mxIsNumeric(array) && mxGetNumberOfElements(array) == 1;
}

static bool isValidVariableType(FMIVariableType type) {
	switch (type) {
    case FMIFloat32Type:
    case FMIDiscreteFloat32Type:
    case FMIFloat64Type:
    case FMIDiscreteFloat64Type:
    case FMIInt8Type:
	case FMIUInt8Type:
	case FMIInt16Type:
	case FMIUInt16Type:
	case FMIInt32Type:
	case FMIUInt32Type:
	case FMIInt64Type:
	case FMIUInt64Type:
	case FMIBooleanType:
	case FMIStringType:
	// case FMIBinaryType:
	// case FMIClockType:
		return true;
	default:
		return false;
	}
}

static void initialize(SimStruct *S) {

    void **p = ssGetPWork(S);

    FMIInstance *instance = p[0];
    const time_T time = ssGetT(S);
    const time_T stopTime = ssGetTFinal(S);  // can be -1
    const bool toleranceDefined = relativeTolerance(S) > 0;

    if (isFMI1(S)) {

        CHECK_ERROR(setParameters(S, false, false));

        if (isCS(S)) {
            CHECK_STATUS(FMI1InitializeSlave(instance, time, stopTime > time, stopTime));
        } else {
            CHECK_STATUS(FMI1SetTime(instance, time));
			fmi1EventInfo eventInfo;
            CHECK_STATUS(FMI1Initialize(instance, toleranceDefined, relativeTolerance(S), &eventInfo));
            if (eventInfo.terminateSimulation) {
                setErrorStatus(S, "Model requested termination at init");
                return;
            }
        }

    } else if (isFMI2(S)) {

        CHECK_ERROR(setParameters(S, false, false));
        CHECK_STATUS(FMI2SetupExperiment(instance, toleranceDefined, relativeTolerance(S), time, stopTime > time, stopTime));
        CHECK_STATUS(FMI2EnterInitializationMode(instance));

    } else {

        if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam)) > 0) {
            CHECK_STATUS(FMI3EnterConfigurationMode(instance));
            CHECK_ERROR(setParameters(S, true, false));
            CHECK_STATUS(FMI3ExitConfigurationMode(instance));
        }

        CHECK_ERROR(setParameters(S, false, false));

        CHECK_STATUS(FMI3EnterInitializationMode(instance, toleranceDefined, relativeTolerance(S), time, stopTime > time, stopTime));

    }

    if (isME(S)) {

		if (isFMI2(S)) {
			CHECK_STATUS(FMI2ExitInitializationMode(instance));
		} else {
			CHECK_STATUS(FMI3ExitInitializationMode(instance));
		}

        // initialize the continuous states
        real_T *x = ssGetContStates(S);

        if (nx(S) > 0) {

            if (isFMI1(S)) {
                CHECK_STATUS(FMI1GetContinuousStates(instance, x, nx(S)));
            } else if (isFMI2(S)) {
                CHECK_STATUS(FMI2GetContinuousStates(instance, x, nx(S)));
            } else {
                CHECK_STATUS(FMI3GetContinuousStates(instance, x, nx(S)));
            }
        }

        // initialize the event indicators
        if (nz(S) > 0) {

            real_T *prez = ssGetRWork(S);

            if (isFMI1(S)) {
                CHECK_STATUS(FMI1GetEventIndicators(instance, prez, nz(S)));
            } else if (isFMI2(S)) {
                CHECK_STATUS(FMI2GetEventIndicators(instance, prez, nz(S)));
            } else {
                CHECK_STATUS(FMI3GetEventIndicators(instance, prez, nz(S)));
            }

            real_T *z = prez + nz(S);

            memcpy(z, prez, nz(S) * sizeof(real_T));
        }
    }
}

#define MDL_ENABLE
static void mdlEnable(SimStruct *S) {
    
    logDebug(S, "mdlEnable()");

    void **p = ssGetPWork(S);

    FMIInstance *instance = p[0];

    if (instance) {

        if (isCS(S)) {

            if (isFMI1(S)) {
                CHECK_STATUS(FMI1ResetSlave(instance));
            } else if (isFMI2(S)) {
                CHECK_STATUS(FMI2Reset(instance));
            } else {
                CHECK_STATUS(FMI3Reset(instance));
            }

            CHECK_ERROR(initialize(S));
        }

        return;
    }

    if (nz(S) > 0) {
        p[2] = calloc(nz(S), sizeof(fmi3Int32)); // rootsFound
    }

    const char_T* instanceName = ssGetPath(S);

    const bool loggingOn = debugLogging(S);

    const char *modelIdentifier = getStringParam(S, modelIdentifierParam, 0);

#ifdef GRTFMI
    char *unzipdir = (char *)mxMalloc(MAX_PATH);
    strncpy(unzipdir, FMU_RESOURCES_DIR, MAX_PATH);
    strncat(unzipdir, "/", MAX_PATH);
    strncat(unzipdir, modelIdentifier, MAX_PATH);
#else
    char *unzipdir = getStringParam(S, unzipDirectoryParam, 0);
#endif

#ifdef _WIN32
    char libraryPath[MAX_PATH];
    strncpy(libraryPath, unzipdir, MAX_PATH);
    PathAppend(libraryPath, "binaries");
    if (isFMI1(S) || isFMI2(S)) {
#ifdef _WIN64
        PathAppend(libraryPath, "win64");
#else
        PathAppend(libraryPath, "win32");
#endif
    } else {
#ifdef _WIN64
        PathAppend(libraryPath, "x86_64-windows");
#else
        PathAppend(libraryPath, "i686-windows");
#endif
    }
    PathAppend(libraryPath, modelIdentifier);
    strncat(libraryPath, ".dll", MAX_PATH);
#elif defined(__APPLE__)
    char libraryPath[PATH_MAX];
    strncpy(libraryPath, unzipdir, PATH_MAX);
    strncat(libraryPath, "/binaries/darwin64/", PATH_MAX);
    strncat(libraryPath, modelIdentifier, PATH_MAX);
    strncat(libraryPath, ".dylib", PATH_MAX);
#else
    char libraryPath[PATH_MAX];
    strncpy(libraryPath, unzipdir, PATH_MAX);
    strncat(libraryPath, "/binaries/linux64/", PATH_MAX);
    strncat(libraryPath, modelIdentifier, PATH_MAX);
    strncat(libraryPath, ".so", PATH_MAX);
#endif

    instance = FMICreateInstance(instanceName, cb_logMessage, logFMICalls(S) ? cb_logFunctionCall : NULL);

    if (!instance) {
        setErrorStatus(S, "Failed to allocate FMIInstance.");
        return;
    }

#if !defined(FMI2_FUNCTION_PREFIX)
	if (FMILoadPlatformBinary(instance, libraryPath) != FMIOK) {
		setErrorStatus(S, "Failed to load platform binary %s.", libraryPath);
		return;
	}
#endif

    instance->userData = S;

    p[0] = instance;

    const char *guid = getStringParam(S, guidParam, 0);

    char fmuResourceLocation[INTERNET_MAX_URL_LENGTH];

    if (isFMI3(S)) {
        strncpy(fmuResourceLocation, unzipdir, INTERNET_MAX_URL_LENGTH);
    } else {
#ifdef _WIN32
        DWORD fmuLocationLength = INTERNET_MAX_URL_LENGTH;
        if (UrlCreateFromPath(unzipdir, fmuResourceLocation, &fmuLocationLength, 0) != S_OK) {
            setErrorStatus(S, "Failed to create fmuResourceLocation.");
            return;
        }
#else
        strcpy(fmuResourceLocation, "file://");
        strcat(fmuResourceLocation, unzipdir);
#endif
    }

    if (!isFMI1(S)) {
        strcat(fmuResourceLocation, "/resources");
    }
    
    // instantiate the FMU
    if (isFMI1(S)) {

        if (isCS(S)) {
            CHECK_STATUS(FMI1InstantiateSlave(instance, modelIdentifier, guid, fmuResourceLocation, "application/x-fmu-sharedlibrary", 0, fmi1False, fmi1False, loggingOn));
        } else {
            CHECK_STATUS(FMI1InstantiateModel(instance, modelIdentifier, guid, loggingOn));
        }

    } else if (isFMI2(S)) {

        CHECK_STATUS(FMI2Instantiate(instance, fmuResourceLocation, isCS(S) ? fmi2CoSimulation : fmi2ModelExchange, guid, fmi2False, loggingOn));

    } else {

        if (isME(S)) {
            CHECK_STATUS(FMI3InstantiateModelExchange(instance, guid, fmuResourceLocation, fmi3False, loggingOn));
        } else {
            CHECK_STATUS(FMI3InstantiateCoSimulation(instance, guid, fmuResourceLocation, fmi3False, loggingOn, fmi3False, fmi3False, NULL, 0, NULL));
        }

    }

    // initialize the FMU instance
    CHECK_ERROR(initialize(S));

    // free string parameters
    mxFree((void *)modelIdentifier);
    mxFree((void *)unzipdir);
    mxFree((void *)guid);
}

#define MDL_DISABLE
static void mdlDisable(SimStruct *S) {
    logDebug(S, "mdlDisable()");
}

#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlCheckParameters(SimStruct *S) {

	logDebug(S, "mdlCheckParameters()");

	if (!isScalar(S, fmiVersionParam) || !(isFMI1(S) || isFMI2(S) || isFMI3(S))) {
		setErrorStatus(S, "Parameter %d (FMI version) must be one of 1.0, 2.0 or 3.0.", fmiVersionParam + 1);
		return;
	}

	if (!isScalar(S, runAsKindParam) || (!isME(S) && !isCS(S))) {
        setErrorStatus(S, "Parameter %d (run as kind) must be one of 0 (= Model Exchange) or 1 (= Co-Simulation).", runAsKindParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, guidParam))) {
        setErrorStatus(S, "Parameter %d (GUID) must be a string.", guidParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, modelIdentifierParam))) {
        setErrorStatus(S, "Parameter %d (model identifier) must be a string.", modelIdentifierParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, unzipDirectoryParam))) {
		setErrorStatus(S, "Parameter %d (unzip directory) must be a string.", unzipDirectoryParam + 1);
		return;
	}

    if (!isScalar(S, debugLoggingParam)) {
        setErrorStatus(S, "Parameter %d (debug logging) must be a scalar.", debugLoggingParam + 1);
        return;
    }
	
	if (!isScalar(S, logFMICallsParam)) {
		setErrorStatus(S, "Parameter %d (log FMI calls) must be a scalar.", logFMICallsParam + 1);
		return;
	}

	if (!isScalar(S, logLevelParam) ||
		(logLevel(S) != 0 && logLevel(S) != 1 && logLevel(S) != 2 && logLevel(S) != 3 && logLevel(S) != 4 && logLevel(S) != 5)) {
		setErrorStatus(S, "Parameter %d (log level) must be one of 0 (= info), 1 (= warning), 2 (= discard), 3 (= error), 4 (= fatal) or 5 (= none).", logLevelParam + 1);
		return;
	}

	if (!mxIsChar(ssGetSFcnParam(S, logFileParam))) {
		setErrorStatus(S, "Parameter %d (log file) must be a string.", logFileParam + 1);
		return;
	}

	if (!isScalar(S, relativeToleranceParam)) {
		setErrorStatus(S, "Parameter %d (relative tolerance) must be a scalar.", relativeToleranceParam + 1);
		return;
	}

	if (!isScalar(S, sampleTimeParam)) {
		setErrorStatus(S, "Parameter %d (sample time) must be a scalar.", sampleTimeParam + 1);
		return;
	}

	if (!isScalar(S, offsetTimeParam)) {
		setErrorStatus(S, "Parameter %d (offset time) must be a scalar.", offsetTimeParam + 1);
		return;
	}

	if (!isScalar(S, nxParam)) {
		setErrorStatus(S, "Parameter %d (number of continuous states) must be a scalar.", nxParam + 1);
		return;
	}

	if (!isScalar(S, nzParam)) {
		setErrorStatus(S, "Parameter %d (number of event indicators) must be a scalar.", nzParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "Parameter %d (input port widths) must be a double array", inputPortWidthsParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortDirectFeedThroughParam))) {
		setErrorStatus(S, "Parameter %d (input port direct feed through) must be a double array", inputPortDirectFeedThroughParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (input port direct feed through) must be equal to the number of elements in parameter %d (inport port widths)", inputPortDirectFeedThroughParam + 1, inputPortWidthsParam + 1);
		return;
	}

	int nu = 0; // number of input variables

	if (isFMI1(S) || isFMI2(S)) {
		for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam)); i++) {
			if (inputPortWidth(S, i) < 1) {
				setErrorStatus(S, "Elements in parameter %d (input port widths) must be >= 1", inputPortWidthsParam + 1);
				return;
			}
			nu += inputPortWidth(S, i);
		}
	} else {
		nu = (int)mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam));
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "Parameter %d (input port direct feed through) must be a double array with the same number of elements as parameter %d (input port widths)", inputPortDirectFeedThroughParam + 1, inputPortWidthsParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (inport port direct feed through) must be equal to the number of elements in parameter %d (inport port widths)", inputPortDirectFeedThroughParam + 1, inputPortWidthsParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortTypesParam))) {
		setErrorStatus(S, "Parameter %d (input port variable types) must be a double array", inputPortTypesParam + 1);
		return;
	}

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, inputPortTypesParam)); i++) {
		FMIVariableType t = variableType(S, inputPortTypesParam, i);
		if (!isValidVariableType(t)) {
			setErrorStatus(S, "Elements in parameter %d (input port types) must be valid variable types", inputPortTypesParam + 1);
			return;
		}
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortTypesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (inport port types) must be equal to the number of the elements in parameter %d (inport port widths)", inputPortTypesParam + 1, inputPortWidthsParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortVariableVRsParam))) {
		setErrorStatus(S, "Parameter %d (input port value references) must be a double array", inputPortVariableVRsParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortVariableVRsParam)) != nu) {
		setErrorStatus(S, "The number of elements in parameter %d (input port value references) must be equal to the sum of the elements in parameter %d (inport port widths)", inputPortVariableVRsParam + 1, inputPortWidthsParam + 1);
		return;
	}

	// TODO: check VRS values!

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortWidthsParam))) {
		setErrorStatus(S, "Parameter %d (output port widths) must be a double array", outputPortWidthsParam + 1);
		return;
	}

	int ny = 0; // number of output variables

	if (isFMI1(S) || isFMI2(S)) {
		for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); i++) {
			if (outputPortWidth(S, i) < 1) {
				setErrorStatus(S, "Elements in parameter %d (output port widths) must be >= 1", outputPortWidthsParam + 1);
				return;
			}
			ny += outputPortWidth(S, i);
		}
	} else {
		ny = (int)mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam));
	}

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortTypesParam))) {
		setErrorStatus(S, "Parameter %d (output port types) must be a double array", outputPortTypesParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, outputPortTypesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (output port types) must be equal to the number of the elements in parameter %d (output port widths)", outputPortTypesParam + 1, outputPortWidthsParam + 1);
		return;
	}

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); i++) {
		FMIVariableType t = variableType(S, outputPortTypesParam, i);
		if (!isValidVariableType(t)) {
			setErrorStatus(S, "Elements in parameter %d (output port types) must be valid variable types", outputPortTypesParam + 1);
			return;
		}
	}

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortVariableVRsParam))) {
		setErrorStatus(S, "Parameter %d (output variable value references) must be a double array", outputPortVariableVRsParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, outputPortVariableVRsParam)) != ny) {
		setErrorStatus(S, "The number of elements in parameter %d (output variable value references) must be equal to the sum of the elements in parameter %d (output port widths)", outputPortVariableVRsParam + 1, outputPortWidthsParam + 1);
		return;
	}

	// TODO: check VRS values!

}
#endif /* MDL_CHECK_PARAMETERS */


static int inputSize(SimStruct *S) {

    size_t s = 0;

    for (int i = 0; i < nu(S); i++) {
        const size_t nValues = inputPortWidth(S, i);
        FMIVariableType type = variableType(S, inputPortTypesParam, i);        
        s += nValues * typeSizes[type];
    }

    return s;
}


#define MDL_PROCESS_PARAMETERS   /* Change to #undef to remove function */
#if defined(MDL_PROCESS_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlProcessParameters(SimStruct *S) {

    logDebug(S, "mdlProcessParameters()");

    CHECK_ERROR(setParameters(S, false, true));
}
#endif /* MDL_PROCESS_PARAMETERS */


static void mdlInitializeSizes(SimStruct *S) {

	logDebug(S, "mdlInitializeSizes()");

    const int nSFcnParams = ssGetSFcnParamsCount(S);

    if ((nSFcnParams - numParams) % 5) {
        setErrorStatus(S, "Wrong number of arguments.");
        return;
    }

	ssSetNumSFcnParams(S, nSFcnParams);

    for (int i = 0; i < numParams; i++) {
        ssSetSFcnParamTunable(S, i, false);
    }

    for (int i = numParams; i < nSFcnParams; i += 5) {
        const double paramTunable = mxGetScalar(ssGetSFcnParam(S, i + 1));
        ssSetSFcnParamTunable(S, i, paramTunable != 0);
    }

#if defined(MATLAB_MEX_FILE)
	if (ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S)) {
		mdlCheckParameters(S);
		if (ssGetErrorStatus(S) != NULL) {
			return;
		}
	} else {
		return; // parameter mismatch will be reported by Simulink
	}
#endif

	ssSetNumContStates(S, isME(S) ? nx(S) : 0);
	ssSetNumDiscStates(S, 0);
    
    const int_T numInputPorts = nu(S) + (resettable(S) ? 1 : 0);

	if (!ssSetNumInputPorts(S, numInputPorts)) return;

	for (int i = 0; i < nu(S); i++) {
		ssSetInputPortWidth(S, i, inputPortWidth(S, i));
		ssSetInputPortRequiredContiguous(S, i, 1); // direct input signal access
		const DTypeId type = simulinkVariableType(S, inputPortTypesParam, i);
		if (type < 0) {
			setErrorStatus(S, "Unexpected type id for input port %d.", i);
			return;
		}
		ssSetInputPortDataType(S, i, type);
		bool dirFeed = inputPortDirectFeedThrough(S, i);
		ssSetInputPortDirectFeedThrough(S, i, dirFeed); // direct feed through
		logDebug(S, "ssSetInputPortDirectFeedThrough(port=%d, dirFeed=%d)", i, dirFeed);
	}

    if (resettable(S)) {
        ssSetInputPortWidth(S, nu(S), 1);
        ssSetInputPortRequiredContiguous(S, nu(S), true); // direct input signal access
        ssSetInputPortDataType(S, nu(S), SS_DOUBLE);
        ssSetInputPortDirectFeedThrough(S, nu(S), true);
    }

	if (!ssSetNumOutputPorts(S, ny(S))) return;

	for (int i = 0; i < ny(S); i++) {
		ssSetOutputPortWidth(S, i, outputPortWidth(S, i));
		DTypeId type = simulinkVariableType(S, outputPortTypesParam, i);
		if (type < 0) {
			setErrorStatus(S, "Unexpected type id for output port %d.", i);
			return;
		}
		ssSetOutputPortDataType(S, i, type);
	}

	ssSetNumSampleTimes(S, 1);
	ssSetNumRWork(S, 2 * nz(S) + nuv(S) + 1 + 1); // [pre(z), z, pre(u), pre(reset), nextEventTime]
    ssSetNumPWork(S, 4); // [FMU, logfile, rootsFound, preInput]
    ssSetNumModes(S, 3); // [stateEvent, timeEvent, stepEvent]
	ssSetNumNonsampledZCs(S, (isME(S)) ? nz(S) + 1 : 0);

	// specify the sim state compliance to be same as a built-in block
	//ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);

	ssSetOptions(S, 0);
}


static void mdlInitializeSampleTimes(SimStruct *S) {

	logDebug(S, "mdlInitializeSampleTimes()");

	ssSetSampleTime(S, 0, sampleTime(S));
	ssSetOffsetTime(S, 0, offsetTime(S));
}


#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S) {

    void **p = ssGetPWork(S);

    if (p[1]) {
        fclose((FILE *)p[1]);
        p[1] = NULL;
    }

	const char *logFile = getStringParam(S, logFileParam, 0);

	if (strlen(logFile) > 0) {
	    p[1] = fopen(logFile, "w");
	}

    mxFree((void *)logFile);
	if (nz(S) > 0) {
		p[2] = calloc(nz(S), sizeof(fmi3Int32));
	} else {
		p[2] = NULL;
	}

    const size_t s = inputSize(S);
    
    if (s > 0) {
        p[3] = malloc(s);
        memset(p[3], 0xFF, s);
    }

	logDebug(S, "mdlStart()");
}
#endif /* MDL_START */


static void mdlOutputs(SimStruct *S, int_T tid) {

    const time_T time = ssGetT(S);

	logDebug(S, "mdlOutputs(tid=%d, time=%.16g, majorTimeStep=%d)", tid, time, ssIsMajorTimeStep(S));

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

    if (resettable(S)) {
        real_T *preReset = &(ssGetRWork(S)[2 * nz(S) + nuv(S)]);
        const real_T reset = *ssGetInputPortRealSignal(S, nu(S));

        if (*preReset == 0 && reset > 0) {
            
            if (isFMI1(S) && isCS(S)) {
                CHECK_STATUS(FMI1ResetSlave(instance));
            } else if (isFMI2(S)) {
                CHECK_STATUS(FMI2Reset(instance));
            } else if (isFMI3(S)) {
                CHECK_STATUS(FMI3Reset(instance));
            }

            CHECK_ERROR(initialize(S));
		}

        *preReset = reset;
    }

	if (isME(S)) {

		const real_T *x = ssGetContStates(S);

		real_T* nextEventTime = &(ssGetRWork(S)[2 * nz(S) + nuv(S) + 1]);

		if (isFMI1(S)) {

			CHECK_STATUS(FMI1SetTime(instance, time));

			if (nx(S) > 0) {
				CHECK_STATUS(FMI1SetContinuousStates(instance, x, nx(S)));
			}

		} else if (isFMI2(S)) {

			if (instance->state == FMIEventModeState) {

                bool inputEvent;

				CHECK_ERROR(setInput(S, true, true, &inputEvent));

				fmi2EventInfo eventInfo = { 0 };

				do {
					CHECK_STATUS(FMI2NewDiscreteStates(instance, &eventInfo));
					if (eventInfo.terminateSimulation) {
						setErrorStatus(S, "The FMU requested to terminate the simulation.");
						return;
					}
				} while (eventInfo.newDiscreteStatesNeeded);

				*nextEventTime = eventInfo.nextEventTimeDefined ? eventInfo.nextEventTime : INFINITY;

				CHECK_STATUS(FMI2EnterContinuousTimeMode(instance));
			}

			if (instance->state != FMIContinuousTimeModeState) {
				CHECK_STATUS(FMI2EnterContinuousTimeMode(instance));
			}

			CHECK_STATUS(FMI2SetTime(instance, time));

			if (nx(S) > 0) {
				CHECK_STATUS(FMI2SetContinuousStates(instance, x, nx(S)));
			}

		} else {
			
			if (instance->state == FMIEventModeState) {

                bool inputEvent;

                CHECK_ERROR(setInput(S, true, true, &inputEvent));

				fmi3Boolean discreteStatesNeedUpdate          = fmi3False;
				fmi3Boolean terminateSimulation               = fmi3False;
				fmi3Boolean nominalsOfContinuousStatesChanged = fmi3False;
				fmi3Boolean valuesOfContinuousStatesChanged   = fmi3False;
				fmi3Boolean nextEventTimeDefined              = fmi3False;

				do {
					CHECK_STATUS(FMI3UpdateDiscreteStates(instance,
						&discreteStatesNeedUpdate,
						&terminateSimulation,
						&nominalsOfContinuousStatesChanged,
						&valuesOfContinuousStatesChanged,
						&nextEventTimeDefined,
						nextEventTime));

					if (terminateSimulation) {
						setErrorStatus(S, "The FMU requested to terminate the simulation.");
						return;
					}
				} while (discreteStatesNeedUpdate);

				*nextEventTime = nextEventTimeDefined ? *nextEventTime : INFINITY;

				CHECK_STATUS(FMI3EnterContinuousTimeMode(instance));
			}

			if (instance->state != FMIContinuousTimeModeState) {
				CHECK_STATUS(FMI3EnterContinuousTimeMode(instance));
			}

			CHECK_STATUS(FMI3SetTime(instance, time));

			if (nx(S) > 0) {
				CHECK_STATUS(FMI3SetContinuousStates(instance, x, nx(S)));
			}
		}

        bool inputEvent;
			   		
		CHECK_ERROR(setInput(S, true, false, &inputEvent));

		if (ssIsMajorTimeStep(S)) {
			CHECK_ERROR(update(S, inputEvent));
		}

	} else {

		const time_T h = time - instance->time;

		if (h > 0) {

			if (!initialized(S)) {
				if (isFMI2(S)) {
					CHECK_STATUS(FMI2ExitInitializationMode(instance));
				} else if (isFMI3(S)) {
					CHECK_STATUS(FMI3ExitInitializationMode(instance));
				}
			}

			if (isFMI1(S)) {
				CHECK_STATUS(FMI1DoStep(instance, instance->time, h, fmi1True));
			} else if (isFMI2(S)) {
				CHECK_STATUS(FMI2DoStep(instance, instance->time, h, fmi2True));
			} else {
				fmi3Boolean eventEncountered;
				fmi3Boolean terminateSimulation;
				fmi3Boolean earlyReturn;
				fmi3Float64 lastSuccessfulTime;
				CHECK_STATUS(FMI3DoStep(instance, instance->time, h, fmi3True, &eventEncountered, &terminateSimulation, &earlyReturn, &lastSuccessfulTime));
                // TODO: handle terminateSimulation == true
			}
		}
	}

	CHECK_ERROR(getOutput(S));
}


#define MDL_UPDATE
#if defined(MDL_UPDATE)
static void mdlUpdate(SimStruct *S, int_T tid) {

	void** p = ssGetPWork(S);

	FMIInstance* instance = (FMIInstance*)p[0];

	logDebug(S, "mdlUpdate(tid=%d, time=%.16g, majorTimeStep=%d)", tid, ssGetT(S), ssIsMajorTimeStep(S));

    bool inputEvent;

    CHECK_ERROR(setInput(S, false, isCS(S), &inputEvent));

    if (isME(S) && inputEvent) {
        ssSetErrorStatus(S, "Unexpected input event in mdlUpdate().");
    }
}
#endif // MDL_UPDATE


#define MDL_ZERO_CROSSINGS
#if defined(MDL_ZERO_CROSSINGS) && (defined(MATLAB_MEX_FILE) || defined(NRT))
static void mdlZeroCrossings(SimStruct *S) {

	logDebug(S, "mdlZeroCrossings(time=%.16g, majorTimeStep=%d)", ssGetT(S), ssIsMajorTimeStep(S));

	if (isME(S)) {

		bool inputEvent;

        CHECK_ERROR(setInput(S, true, false, &inputEvent));

        if (inputEvent) {
            ssSetErrorStatus(S, "Unexpected input event in mdlZeroCrossings().");
            return;
        }

		real_T *z = ssGetNonsampledZCs(S);

		void **p = ssGetPWork(S);

		FMIInstance *instance = (FMIInstance *)p[0];

		if (nz(S) > 0) {
			if (isFMI1(S)) {
				CHECK_STATUS(FMI1GetEventIndicators(instance, z, nz(S)));
			} else if (isFMI2(S)) {
				CHECK_STATUS(FMI2GetEventIndicators(instance, z, nz(S)));
			} else {
				CHECK_STATUS(FMI3GetEventIndicators(instance, z, nz(S)));
			}
		}

		const real_T* nextEventTime = &(ssGetRWork(S)[2 * nz(S) + nuv(S) + 1]);

		z[nz(S)] = *nextEventTime - ssGetT(S);
	}
}
#endif


#define MDL_DERIVATIVES
#if defined(MDL_DERIVATIVES)
static void mdlDerivatives(SimStruct *S) {

	logDebug(S, "mdlDerivatives(time=%.16g, majorTimeStep=%d)", ssGetT(S), ssIsMajorTimeStep(S));

	if (isCS(S)) {
		return;  // nothing to do
	}
		
	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

    bool inputEvent;

	CHECK_ERROR(setInput(S, true, false, &inputEvent));

    if (isME(S) && inputEvent) {
        ssSetErrorStatus(S, "Unexpected input event in mdlDerivatives().");
    }

	real_T *x = ssGetContStates(S);
	real_T *dx = ssGetdX(S);

	if (isFMI1(S)) {
        CHECK_STATUS(FMI1GetContinuousStates(instance, x, nx(S)));
        CHECK_STATUS(FMI1GetDerivatives(instance, dx, nx(S)));
	} else if (isFMI2(S)) {
        CHECK_STATUS(FMI2GetContinuousStates(instance, x, nx(S)));
        CHECK_STATUS(FMI2GetDerivatives(instance, dx, nx(S)));
	} else {
        CHECK_STATUS(FMI3GetContinuousStates(instance, x, nx(S)));
        CHECK_STATUS(FMI3GetContinuousStateDerivatives(instance, dx, nx(S)));
	}
}
#endif


static void mdlTerminate(SimStruct *S) {

	logDebug(S, "mdlTerminate()");

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

    if (instance) {

	    if (!ssGetErrorStatus(S)) {

		    if (isFMI1(S)) {
		
			    if (isME(S)) {
					if (initialized(S)) {
						CHECK_STATUS(FMI1Terminate(instance));
					}
				    FMI1FreeModelInstance(instance);
			    } else {
					if (initialized(S)) {
						CHECK_STATUS(FMI1TerminateSlave(instance));
					}
				    FMI1FreeSlaveInstance(instance);
			    }

		    } else if (isFMI2(S)) {
				
				if (initialized(S)) {
					CHECK_STATUS(FMI2Terminate(instance));
				}
			    FMI2FreeInstance(instance);
		
		    } else {

				if (initialized(S)) {
					CHECK_STATUS(FMI3Terminate(instance));
				}
			    FMI3FreeInstance(instance);
		
		    }
	    }

	    FMIFreeInstance(instance);
    }

    char *preU = p[3];

    if (preU) {
        free(preU);
    }

	FILE *logFile = (FILE *)p[1];

		if (logFile) {
			fclose(logFile);
			p[1] = NULL;
		}
	}

/*=============================*
* Required S-function trailer *
*=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
