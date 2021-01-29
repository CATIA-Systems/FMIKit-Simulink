/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#define FMI_MAX_MESSAGE_LENGTH 4096

#define INTERNET_MAX_URL_LENGTH 2083

#ifdef GRTFMI
extern const char *FMU_RESOURCES_DIR;
#endif

#include "FMI.c"
#include "FMI1.c"
#include "FMI2.c"

#ifndef S_FUNCTION_NAME
#define S_FUNCTION_NAME sfun_fmurun
#endif

#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#ifdef _WIN32
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")
#elif defined(__APPLE__)
#else
#include <linux/limits.h>
#endif



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
	scalarStartTypesParam,
	scalarStartVRsParam,
	scalarStartValuesParam,
	stringStartVRsParam,
	stringStartValuesParam,
	inputPortWidthsParam,
	inputPortDirectFeedThroughParam,
	inputPortTypesParam,
	inputPortVariableVRsParam,
	outputPortWidthsParam,
	outputPortTypesParam,
	outputPortVariableVRsParam,
	numParams

} Parameter;

static char *mxCharToChar(const mxChar *src, char *dst, size_t len) {
	for (size_t i = 0; i < len; i++) {
		dst[i] = (char)src[i];
	}
	dst[len] = '\0';
	return dst;
}

static char* getStringParam(SimStruct *S, int index) {

	const mxArray *pa = ssGetSFcnParam(S, index);

	size_t n = mxGetN(pa);

	if (n < 1) return "";

	const mxChar *data = (const mxChar *)mxGetData(pa);

	if (!data) return "";

	char *cstr = (char *)mxMalloc(n + 1);

	return mxCharToChar(data, cstr, n);
}

static bool isFMI1(SimStruct *S) {
	const mxArray *pa = ssGetSFcnParam(S, fmiVersionParam);
	const mxChar *data = (const mxChar*)mxGetData(pa);
	char cstr[4];
	mxCharToChar(data, cstr, 3);
	return mxGetN(pa) == 3 && strncmp(cstr, "1.0", 3) == 0;
}

static bool isFMI2(SimStruct *S) {
	const mxArray *pa = ssGetSFcnParam(S, fmiVersionParam);
	const mxChar *data = (const mxChar*)mxGetData(pa);
	char cstr[4];
	mxCharToChar(data, cstr, 3);
	return mxGetN(pa) == 3 && strncmp(cstr, "2.0", 3) == 0;
}

static bool isME(SimStruct *S) { 
	return mxGetScalar(ssGetSFcnParam(S, runAsKindParam)) == fmi2ModelExchange;
}

static bool isCS(SimStruct *S) {
	return mxGetScalar(ssGetSFcnParam(S, runAsKindParam)) == fmi2CoSimulation;
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

static int nScalarStartValues(SimStruct *S) {
    return (int)mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartVRsParam));
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
	FMIVariableType type = (FMIVariableType)intValue;
	switch (type) {
	case FMIRealType:    return SS_DOUBLE;
	case FMIIntegerType: return SS_INT32;
	case FMIBooleanType: return SS_BOOLEAN;
	default:      return -1; // error
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

static void cb_logFunctionCall(FMIInstance *instance, FMIStatus status, const char *message, ...) {
	
	char buf[FMI_MAX_MESSAGE_LENGTH];

	size_t len = snprintf(buf, FMI_MAX_MESSAGE_LENGTH, "[%s] ", instance->name);

	va_list args;

	va_start(args, message);
	len += vsnprintf(&buf[len], FMI_MAX_MESSAGE_LENGTH - len, message, args);
	va_end(args);

	appendStatus(status, buf, FMI_MAX_MESSAGE_LENGTH);

	SimStruct *S = (SimStruct *)instance->userData;

	logCall(S, buf);
}


#define CHECK_STATUS(s) if (s > fmi2Warning) { ssSetErrorStatus(S, "The FMU encountered an error."); return; }

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

static void setInput(SimStruct *S, bool direct) {

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

	int iu = 0;

	for (int i = 0; i < nu(S); i++) {

		const int w = inputPortWidth(S, i);

		if (direct && !inputPortDirectFeedThrough(S, i)) {
			iu += w;
			continue;
		}

		FMIVariableType type = variableType(S, inputPortTypesParam, i);

		const void *y = ssGetInputPortSignal(S, i);

		for (int j = 0; j < w; j++) {

			const FMIValueReference vr = valueReference(S, inputPortVariableVRsParam, iu);

			// set the input
			if (isFMI1(S)) {

				switch (type) {
				case FMIRealType:
					CHECK_STATUS(FMI1SetReal(instance, &vr, 1, &((const real_T *)y)[j]))
					break;
				case FMIIntegerType:
					CHECK_STATUS(FMI1SetInteger(instance, &vr, 1, &((const int32_T *)y)[j]))
					break;
				case FMIBooleanType:
					CHECK_STATUS(FMI1SetBoolean(instance, &vr, 1, &((const boolean_T *)y)[j]))
					break;
				default:
					break;
				}

			} else {
				
				switch (type) {
				case FMIRealType:
					CHECK_STATUS(FMI2SetReal(instance, &vr, 1, &((const real_T *)y)[j]))
					break;
				case FMIIntegerType:
					CHECK_STATUS(FMI2SetInteger(instance, &vr, 1, &((const int32_T *)y)[j]))
					break;
				case FMIBooleanType: {
					const fmi2Boolean booleanValue = ((const boolean_T *)y)[j];
					CHECK_STATUS(FMI2SetBoolean(instance, &vr, 1, &booleanValue))
					break;
				}
				default:
					break;
				}
			}

			iu++;
		}
	}
}

static void setOutput(SimStruct *S) {

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

	int iy = 0;

	for (int i = 0; i < ny(S); i++) {

		FMIVariableType type = variableType(S, outputPortTypesParam, i);

		void *y = ssGetOutputPortSignal(S, i);

		for (int j = 0; j < outputPortWidth(S, i); j++) {

			FMIValueReference vr = valueReference(S, outputPortVariableVRsParam, iy);

			if (isFMI1(S)) {

				switch (type) {
				case FMIRealType:
					CHECK_STATUS(FMI1GetReal(instance, &vr, 1, &((real_T *)y)[j]))
					break;
				case FMIIntegerType:
					CHECK_STATUS(FMI1GetInteger(instance, &vr, 1, &((int32_T *)y)[j]))
					break;
				case FMIBooleanType:
					CHECK_STATUS(FMI1GetBoolean(instance, &vr, 1, &((boolean_T *)y)[j]))
					break;
				default:
					break;
				}

			} else {

				switch (type) {
				case FMIRealType:
					CHECK_STATUS(FMI2GetReal(instance, &vr, 1, &((real_T *)y)[j]))
					break;
				case FMIIntegerType:
					CHECK_STATUS(FMI2GetInteger(instance, &vr, 1, &((int32_T *)y)[j]))
					break;
				case FMIBooleanType: {
					fmi2Boolean booleanValue;
					CHECK_STATUS(FMI2GetBoolean(instance, &vr, 1, &booleanValue))
					((boolean_T *)y)[j] = booleanValue;
					break;
				}
				default:
					break;
				}

			}

			iy++;
		}
	}
}

static void setStartValues(SimStruct *S) {

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

    // scalar start values
	for (int i = 0; i < nScalarStartValues(S); i++) {
		FMIValueReference vr = valueReference(S, scalarStartVRsParam, i);
		FMIVariableType type = variableType(S, scalarStartTypesParam, i);
		real_T realValue = scalarValue(S, scalarStartValuesParam, i);
				
		if (isFMI1(S)) {

			fmi1Integer intValue  = (fmi1Integer)realValue;
			fmi1Boolean boolValue = (fmi1Boolean)realValue;

			switch (type) {
			case FMIRealType:    FMI1SetReal    (instance, &vr, 1, &realValue); break;
			case FMIIntegerType: FMI1SetInteger (instance, &vr, 1, &intValue);  break;
			case FMIBooleanType: FMI1SetBoolean (instance, &vr, 1, &boolValue); break;
			default: break;
			}

		} else {

			fmi2Integer intValue  = (fmi2Integer)realValue;
			fmi2Boolean boolValue = (fmi2Boolean)realValue;

			switch (type) {
			case FMIRealType:    FMI2SetReal    (instance, &vr, 1, &realValue); break;
			case FMIIntegerType: FMI2SetInteger (instance, &vr, 1, &intValue);  break;
			case FMIBooleanType: FMI2SetBoolean (instance, &vr, 1, &boolValue); break;
			default: break;
			}
		}
    }

	// string start values
	const mxArray *pa = ssGetSFcnParam(S, stringStartValuesParam);
	const int size    = (int)mxGetNumberOfElements(pa) + 1;
	const int m       = (int)mxGetM(pa);
	const int n       = (int)mxGetN(pa);
	char *buffer      = (char *)calloc(size, sizeof(char));
	char *value       = (char *)calloc(n + 1, sizeof(char));

	//if (mxGetString(pa, buffer, size) != 0) {
	//	ssSetErrorStatus(S, "Failed to convert string parameters");
	//	return;
	//}

	for (int i = 0; i < m; i++) {

		// copy the row
		for (int j = 0; j < n; j++) value[j] = buffer[j * m + i];

		// remove the trailing blanks
		for (int j = n - 1; j >= 0; j--) {
			if (value[j] != ' ') break;
			value[j] = '\0';
		}

		FMIValueReference vr = valueReference(S, stringStartVRsParam, i);

		if (isFMI1(S)) {
			FMI1SetString(instance, &vr, 1, (const fmi1String *)&value);
		} else {
			FMI2SetString(instance, &vr, 1, (const fmi2String *)&value);
		}
	}

	free(buffer);
	free(value);
}

static void update(SimStruct *S) {

	if (isCS(S)) {
		return;  // nothing to do
	}

	FMIInstance *instance = (FMIInstance *)ssGetPWork(S)[0];

	double time = instance->time;
	bool upcomingTimeEvent;
	double nextEventTime;

	if (isFMI1(S)) {
		upcomingTimeEvent = instance->eventInfo1.upcomingTimeEvent;
		nextEventTime = instance->eventInfo1.nextEventTime;
	} else {
		upcomingTimeEvent = instance->eventInfo2.nextEventTimeDefined;
		nextEventTime = instance->eventInfo2.nextEventTime;
	}

	// Work around for the event handling in Dymola FMUs:
	bool timeEvent = upcomingTimeEvent && time >= nextEventTime;

	if (timeEvent) {
		logDebug(S, "Time event at t=%.16g", time);
	}

	bool stepEvent;

	if (isFMI1(S)) {
		fmi1Boolean callEventUpdate = fmi1False;
		CHECK_STATUS(FMI1CompletedIntegratorStep(instance, &callEventUpdate))
		stepEvent = callEventUpdate;
	} else {
		fmi2Boolean enterEventMode = fmi2False;
		fmi2Boolean terminateSimulation = fmi2False;
		CHECK_STATUS(FMI2CompletedIntegratorStep(instance, fmi2True, &enterEventMode, &terminateSimulation))
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

	if (nz(S) > 0) {

		real_T *prez = ssGetRWork(S);
		real_T *z = prez + nz(S);

		if (isFMI1(S)) {
			CHECK_STATUS(FMI1GetEventIndicators(instance, z, nz(S)))
		} else {
			CHECK_STATUS(FMI2GetEventIndicators(instance, z, nz(S)))
		}

		// check for state events
		for (int i = 0; i < nz(S); i++) {

			bool rising  = (prez[i] < 0 && z[i] >= 0) || (prez[i] == 0 && z[i] > 0);
			bool falling = (prez[i] > 0 && z[i] <= 0) || (prez[i] == 0 && z[i] < 0);

			if (rising || falling) {
				logDebug(S, "State event %s z[%d] at t=%.16g\n", rising ? "-\\+" : "+/-", i, instance->time);
				stateEvent = true;
				// TODO: break?
			}
		}

		// remember the current event indicators
		for (int i = 0; i < nz(S); i++) prez[i] = z[i];
	}

	if (timeEvent || stepEvent || stateEvent) {

		if (isFMI1(S)) {

			CHECK_STATUS(FMI1EventUpdate(instance, fmi1False, &instance->eventInfo1));
		
		} else {

			CHECK_STATUS(FMI2EnterEventMode(instance))

			do {
				CHECK_STATUS(FMI2NewDiscreteStates(instance, &instance->eventInfo2))
				if (instance->eventInfo2.terminateSimulation) {
					setErrorStatus(S, "The FMU requested to terminate the simulation.");
					return;
				}
			} while (instance->eventInfo2.newDiscreteStatesNeeded);

			CHECK_STATUS(FMI2EnterContinuousTimeMode(instance))
		}

		if (nx(S) > 0) {

			real_T *x = ssGetContStates(S);

			if (isFMI1(S)) {
				CHECK_STATUS(FMI1GetContinuousStates(instance, x, nx(S)))
			} else {
				CHECK_STATUS(FMI2GetContinuousStates(instance, x, nx(S)))
			}
		}

		if (nz(S) > 0) {

			real_T *prez = ssGetRWork(S);

			if (isFMI1(S)) {
				CHECK_STATUS(FMI1GetEventIndicators(instance, prez, nz(S)))
			} else {
				CHECK_STATUS(FMI2GetEventIndicators(instance, prez, nz(S)))
			}
		}

		ssSetSolverNeedsReset(S);
	}
}

#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlCheckParameters(SimStruct *S) {

	logDebug(S, "mdlCheckParameters()");

	if (!mxIsChar(ssGetSFcnParam(S, fmiVersionParam)) || (!isFMI1(S) && !isFMI2(S))) {
        setErrorStatus(S, "Parameter %d (FMI version) must be one of '1.0' or '2.0'", fmiVersionParam + 1);
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, runAsKindParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, runAsKindParam)) != 1 || (!isME(S) && !isCS(S))) {
        setErrorStatus(S, "Parameter %d (run as kind) must be one of 0 (= MODEL_EXCHANGE) or 1 (= CO_SIMULATION)", runAsKindParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, guidParam))) {
        setErrorStatus(S, "Parameter %d (GUID) must be a string", guidParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, modelIdentifierParam))) {
        setErrorStatus(S, "Parameter %d (model identifier) must be a string", modelIdentifierParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, unzipDirectoryParam))) {
		setErrorStatus(S, "Parameter %d (unzip directory) must be a string", unzipDirectoryParam + 1);
		return;
	}

    if (!mxIsNumeric(ssGetSFcnParam(S, debugLoggingParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, debugLoggingParam)) != 1) {
        setErrorStatus(S, "Parameter %d (debug logging) must be a scalar", debugLoggingParam + 1);
        return;
    }
	
	if (!mxIsNumeric(ssGetSFcnParam(S, logFMICallsParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, logFMICallsParam)) != 1) {
		setErrorStatus(S, "Parameter %d (log FMI calls) must be a scalar", logFMICallsParam + 1);
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, logLevelParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, logLevelParam)) != 1 ||
		(logLevel(S) != 0 && logLevel(S) != 1 && logLevel(S) != 2 && logLevel(S) != 3 && logLevel(S) != 4 && logLevel(S) != 5)) {
		setErrorStatus(S, "Parameter %d (log level) must be one of 0 (= info), 1 (= warning), 2 (= discard), 3 (= error), 4 (= fatal) or 5 (= none)", logLevelParam + 1);
		return;
	}

	if (!mxIsChar(ssGetSFcnParam(S, logFileParam))) {
		setErrorStatus(S, "Parameter %d (log file) must be a string", logFileParam + 1);
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, relativeToleranceParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, relativeToleranceParam)) != 1) {
		setErrorStatus(S, "Parameter %d (relative tolerance) must be numeric", relativeToleranceParam + 1);
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, sampleTimeParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, sampleTimeParam)) != 1) {
		setErrorStatus(S, "Parameter %d (sample time) must be numeric", sampleTimeParam + 1);
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, offsetTimeParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, offsetTimeParam)) != 1) {
		setErrorStatus(S, "Parameter %d (offset time) must be numeric", offsetTimeParam + 1);
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, nxParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, nxParam)) != 1) {
		setErrorStatus(S, "Parameter %d (number of continuous states) must be a scalar", nxParam + 1);
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, nzParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, nzParam)) != 1) {
		setErrorStatus(S, "Parameter %d (number of event indicators) must be a scalar", nzParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartTypesParam))) {
		setErrorStatus(S, "Parameter %d (scalar start value types) must be a double array", scalarStartTypesParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartVRsParam))) {
		setErrorStatus(S, "Parameter %d (scalar start value references) must be a double array", scalarStartVRsParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartVRsParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartTypesParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (scalar start value references) and parameter %d (scalar start value types) must be equal", scalarStartVRsParam + 1, scalarStartTypesParam + 1);
		return;
	}

	// TODO: check VRS values!

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartValuesParam))) {
		setErrorStatus(S, "Parameter %d (scalar start values) must be a double array", scalarStartValuesParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartValuesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartTypesParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (scalar start values) and parameter %d (scalar start value types) must be equal", scalarStartValuesParam + 1, scalarStartTypesParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, stringStartVRsParam))) {
		setErrorStatus(S, "Parameter %d (string start value references) must be a double array", stringStartVRsParam + 1);
		return;
	}

	// TODO: check VRS values!

	if (!mxIsChar(ssGetSFcnParam(S, stringStartValuesParam))) {
		setErrorStatus(S, "Parameter %d (string start values) must be a char matrix", stringStartValuesParam + 1);
		return;
	}

	if (mxGetM(ssGetSFcnParam(S, stringStartValuesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, stringStartVRsParam))) {
		setErrorStatus(S, "The number of rows in parameter %d (string start values) must be equal to the number of elements in parameter %d (string start value references)", stringStartValuesParam + 1, stringStartVRsParam + 1);
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

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam)); i++) {
		if (inputPortWidth(S, i) < 1) {
			setErrorStatus(S, "Elements in parameter %d (input port widths) must be >= 1", inputPortWidthsParam + 1);
			return;
		}
		nu += inputPortWidth(S, i);
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
		if (t != FMIRealType && t != FMIIntegerType && t != FMIBooleanType) {
			setErrorStatus(S, "Elements in parameter %d (input port types) must be one of 0 (= Real), 1 (= Integer) or 2 (= Boolean)", inputPortTypesParam + 1);
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

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); i++) {
		if (outputPortWidth(S, i) < 1) {
			setErrorStatus(S, "Elements in parameter %d (output port widths) must be >= 1", outputPortWidthsParam + 1);
			return;
		}
		ny += outputPortWidth(S, i);
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
		if (t != FMIRealType && t != FMIIntegerType && t != FMIBooleanType) {
			setErrorStatus(S, "Elements in parameter %d (output port types) must be one of 0 (= Real), 1 (= Integer) or 2 (= Boolean)", outputPortTypesParam + 1);
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


static void mdlInitializeSizes(SimStruct *S) {

	logDebug(S, "mdlInitializeSizes()");

	ssSetNumSFcnParams(S, numParams);

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

	if (!ssSetNumInputPorts(S, nu(S))) return;

	for (int i = 0; i < nu(S); i++) {
		ssSetInputPortWidth(S, i, inputPortWidth(S, i));
		ssSetInputPortRequiredContiguous(S, i, 1); // direct input signal access
		DTypeId type = simulinkVariableType(S, inputPortTypesParam, i);
		ssSetInputPortDataType(S, i, type);
		bool dirFeed = inputPortDirectFeedThrough(S, i);
		ssSetInputPortDirectFeedThrough(S, i, dirFeed); // direct feed through
		logDebug(S, "ssSetInputPortDirectFeedThrough(port=%d, dirFeed=%d)", i, dirFeed);
	}

	if (!ssSetNumOutputPorts(S, ny(S))) return;

	for (int i = 0; i < ny(S); i++) {
		ssSetOutputPortWidth(S, i, outputPortWidth(S, i));
		DTypeId type = simulinkVariableType(S, outputPortTypesParam, i);
		ssSetOutputPortDataType(S, i, type);
	}

	ssSetNumSampleTimes(S, 1);
	ssSetNumRWork(S, 2 * nz(S) + nuv(S)); // prez & z, preu
	ssSetNumIWork(S, 0);
	ssSetNumPWork(S, 2); // [FMU, logfile]
	ssSetNumModes(S, 3); // [stateEvent, timeEvent, stepEvent]
	ssSetNumNonsampledZCs(S, (isME(S)) ? nz(S) + 1 : 0);

	// specify the sim state compliance to be same as a built-in block
	//ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);

	ssSetOptions(S, 0);
}


static void mdlInitializeSampleTimes(SimStruct *S) {

	logDebug(S, "mdlInitializeSampleTimes()");

	if (isCS(S)) {
		ssSetSampleTime(S, 0, sampleTime(S));
		ssSetOffsetTime(S, 0, offsetTime(S));
	} else {
		ssSetSampleTime(S, 0, CONTINUOUS_SAMPLE_TIME);
		ssSetOffsetTime(S, 0, offsetTime(S));
	}

}


#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S) {

    void **p = ssGetPWork(S);

    if (p[1]) {
        fclose((FILE *)p[1]);
        p[1] = NULL;
    }

	const char *logFile = getStringParam(S, logFileParam);

	if (strlen(logFile) > 0) {
	    p[1] = fopen(logFile, "w");
	}

	logDebug(S, "mdlStart()");

	const char_T* instanceName = ssGetPath(S);
	time_T time = ssGetT(S);

	//FMU::m_messageLogger = logFMUMessage;

	//char libraryFile[1000];
	//getLibraryPath(S, libraryFile);
//
//#ifdef _WIN32
//	if (!PathFileExists(libraryFile)) {
//		static char errorMessage[1024];
//		snprintf(errorMessage, 1024, "Cannot find the FMU's platform binary %s for %s.", libraryFile, instanceName);
//		ssSetErrorStatus(S, errorMessage);
//		return;
//	}
//#endif

	bool toleranceDefined = relativeTolerance(S) > 0;

    bool loggingOn = debugLogging(S);

	const char *modelIdentifier = getStringParam(S, modelIdentifierParam);

#ifdef GRTFMI
	char *unzipdir = calloc(MAX_PATH, sizeof(char));
	strncpy(unzipdir, FMU_RESOURCES_DIR, MAX_PATH);
	strncat(unzipdir, "/", MAX_PATH);
	strncat(unzipdir, modelIdentifier, MAX_PATH);
#else
	char *unzipdir = getStringParam(S, unzipDirectoryParam);
#endif

#ifdef _WIN32
	char libraryPath[MAX_PATH];
	strncpy(libraryPath, unzipdir, MAX_PATH);
	PathAppend(libraryPath, "binaries");
#ifdef _WIN64
	PathAppend(libraryPath, "win64");
#else
	PathAppend(libraryPath, "win32");
#endif
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

	FMIInstance *instance = FMICreateInstance(instanceName, libraryPath, cb_logMessage, logFMICalls(S) ? cb_logFunctionCall : NULL);

	instance->userData = S;

	p[0] = instance;

	const char *guid = getStringParam(S, guidParam);

	char fmuResourceLocation[INTERNET_MAX_URL_LENGTH];

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

	if (isFMI2(S)) {
		strcat(fmuResourceLocation, "/resources");
	}

	time_T stopTime = ssGetTFinal(S);  // can be -1

	if (isFMI1(S)) {

		if (isCS(S)) {
			CHECK_STATUS(FMI1InstantiateSlave(instance, modelIdentifier, guid, fmuResourceLocation, "application/x-fmu-sharedlibrary", 0, fmi1False, fmi1False, loggingOn))
			setStartValues(S);
			CHECK_STATUS(FMI1InitializeSlave(instance, time, stopTime > time, stopTime))
		} else {
			CHECK_STATUS(FMI1InstantiateModel(instance, modelIdentifier, guid, loggingOn))
			setStartValues(S);
			CHECK_STATUS(FMI1SetTime(instance, time))
			CHECK_STATUS(FMI1Initialize(instance, toleranceDefined, relativeTolerance(S)))
			if (instance->eventInfo1.terminateSimulation) {
				setErrorStatus(S, "Model requested termination at init");
				return;
			}
		}

	} else {

		CHECK_STATUS(FMI2Instantiate(instance, fmuResourceLocation, isCS(S) ? fmi2CoSimulation : fmi2ModelExchange, guid, fmi2False, loggingOn))

		if (!instance) {
			ssSetErrorStatus(S, "Failed to instantiate FMU.");
			return;
		}

		setStartValues(S);

		if (ssGetErrorStatus(S)) return;

		CHECK_STATUS(FMI2SetupExperiment(instance, toleranceDefined, relativeTolerance(S), time, stopTime > time, stopTime))

		CHECK_STATUS(FMI2EnterInitializationMode(instance))
		CHECK_STATUS(FMI2ExitInitializationMode(instance))
	}

	mxFree((void *)modelIdentifier);
	mxFree((void *)guid);
}
#endif /* MDL_START */


#define MDL_INITIALIZE_CONDITIONS
#if defined(MDL_INITIALIZE_CONDITIONS)
static void mdlInitializeConditions(SimStruct *S) {

	logDebug(S, "mdlInitializeConditions()");

	if (isCS(S)) {
		return;  // nothing to do
	}


	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

	// initialize the continuous states
	real_T *x = ssGetContStates(S);

	if (isFMI1(S)) {
		CHECK_STATUS(FMI1GetContinuousStates(instance, x, nx(S)))
	} else {
		CHECK_STATUS(FMI2GetContinuousStates(instance, x, nx(S)))
	}

	// initialize the event indicators
	if (nz(S) > 0) {

		real_T *prez = ssGetRWork(S);
		real_T *z = prez + nz(S);

		if (isFMI1(S)) {
			CHECK_STATUS(FMI1GetEventIndicators(instance, prez, nz(S)))
			CHECK_STATUS(FMI1GetEventIndicators(instance, z, nz(S)))
		}
		else {
			CHECK_STATUS(FMI2GetEventIndicators(instance, prez, nz(S)))
			CHECK_STATUS(FMI2GetEventIndicators(instance, z, nz(S)))
		}
	}
}
#endif


static void mdlOutputs(SimStruct *S, int_T tid) {

	logDebug(S, "mdlOutputs(tid=%d, time=%.16g, majorTimeStep=%d)", tid, ssGetT(S), ssIsMajorTimeStep(S));

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

	if (isME(S)) {

		real_T *x = ssGetContStates(S);

		if (isFMI2(S)) {

			if (instance->state == FMI2EventModeState) {

				CHECK_ERROR(setInput(S, true))

				do {
					CHECK_STATUS(FMI2NewDiscreteStates(instance, &instance->eventInfo2))
					if (instance->eventInfo2.terminateSimulation) {
						setErrorStatus(S, "The FMU requested to terminate the simulation.");
						return;
					}
				} while (instance->eventInfo2.newDiscreteStatesNeeded);

				CHECK_STATUS(FMI2EnterContinuousTimeMode(instance))
			}

			if (instance->state != FMI2ContinuousTimeModeState) {
				CHECK_STATUS(FMI2EnterContinuousTimeMode(instance))
			}

			CHECK_STATUS(FMI2SetTime(instance, ssGetT(S)))
			CHECK_STATUS(FMI2SetContinuousStates(instance, x, nx(S)))

		} else {
			
			CHECK_STATUS(FMI1SetTime(instance, ssGetT(S)))
			CHECK_STATUS(FMI1SetContinuousStates(instance, x, nx(S)))
		
		}
			   		
		CHECK_ERROR(setInput(S, true))

		if (ssIsMajorTimeStep(S)) {
			CHECK_ERROR(update(S))
		}

	} else {

		time_T h = ssGetT(S) - instance->time;

		if (h > 0) {
			if (isFMI1(S)) {
				CHECK_STATUS(FMI1DoStep(instance, instance->time, h, fmi1True))
			} else {
				CHECK_STATUS(FMI2DoStep(instance, instance->time, h, fmi2False))
			}
		}
	}

	CHECK_ERROR(setOutput(S))
}

#define MDL_UPDATE
#if defined(MDL_UPDATE)
static void mdlUpdate(SimStruct *S, int_T tid) {

	logDebug(S, "mdlUpdate(tid=%d, time=%.16g, majorTimeStep=%d)", tid, ssGetT(S), ssIsMajorTimeStep(S));

	CHECK_ERROR(setInput(S, false))
}
#endif // MDL_UPDATE


#define MDL_ZERO_CROSSINGS
#if defined(MDL_ZERO_CROSSINGS) && (defined(MATLAB_MEX_FILE) || defined(NRT))
static void mdlZeroCrossings(SimStruct *S) {

	logDebug(S, "mdlZeroCrossings(time=%.16g, majorTimeStep=%d)", ssGetT(S), ssIsMajorTimeStep(S));

	if (isME(S)) {

		CHECK_ERROR(setInput(S, true))

		real_T *z = ssGetNonsampledZCs(S);

		void **p = ssGetPWork(S);

		FMIInstance *instance = (FMIInstance *)p[0];

		if (nz(S) > 0) {
			if (isFMI1(S)) {
				CHECK_STATUS(FMI1GetEventIndicators(instance, z, nz(S)))
			} else {
				CHECK_STATUS(FMI2GetEventIndicators(instance, z, nz(S)))
			}
		}

		real_T nextEventTime = instance->eventInfo1.nextEventTime ? isFMI1(S) : instance->eventInfo2.nextEventTime;

		z[nz(S)] = nextEventTime - ssGetT(S);
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

	CHECK_ERROR(setInput(S, true))

	real_T *x = ssGetContStates(S);
	real_T *dx = ssGetdX(S);

	if (isFMI1(S)) {
		FMI1GetContinuousStates(instance, x, nx(S));
		FMI1GetDerivatives(instance, dx, nx(S));
	} else {
		FMI2GetContinuousStates(instance, x, nx(S));
		FMI2GetDerivatives(instance, dx, nx(S));
	}
}
#endif


static void mdlTerminate(SimStruct *S) {

	logDebug(S, "mdlTerminate()");

	void **p = ssGetPWork(S);

	FMIInstance *instance = (FMIInstance *)p[0];

	if (!ssGetErrorStatus(S)) {
		if (isFMI1(S)) {
			if (isME(S)) {
				CHECK_STATUS(FMI1Terminate(instance))
				FMI1FreeModelInstance(instance);
			} else {
				CHECK_STATUS(FMI1TerminateSlave(instance))
				FMI1FreeSlaveInstance(instance);
			}
		} else {
			CHECK_STATUS(FMI2Terminate(instance))
			FMI2FreeInstance(instance);
		}
	}

	FMIFreeInstance(instance);

	FILE *logFile = NULL;

	if (p) {
		logFile = (FILE *)p[1];
		if (logFile) {
			fclose(logFile);
			void **p = ssGetPWork(S);
			p[1] = NULL;
		}
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
