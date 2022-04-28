#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include <float.h>  /* for DBL_EPSILON, FLT_MAX */
#include <math.h>   /* for fabs() */
#include <string.h> /* for strcpy(), strncmp() */
#include <stdarg.h> /* for va_list */
#include <stdio.h>  /* for vsnprintf(), vprintf() */

#include "fmiwrapper.inc"

#include "fmi2Functions.h"

const char *RT_MEMORY_ALLOCATION_ERROR = "memory allocation error";

/* Path to the resources directory of the extracted FMU */
const char *FMU_RESOURCES_DIR = NULL;

typedef struct {
	RT_MDL_TYPE *S;
	const char *instanceName;
	fmi2CallbackLogger logger;
	fmi2ComponentEnvironment componentEnvironment;
	ModelVariable modelVariables[N_MODEL_VARIABLES];
} ModelInstance;

static ModelInstance *s_instance = NULL;

#define ASSERT_INSTANCE \
    if (!c || c != s_instance) { \
        return fmi2Error; \
    }

#define NOT_IMPLEMENTED \
    logError("Function is not implemented."); \
	return fmi2Error;

#define CHECK_ERROR_STATUS \
	const char *errorStatus = rtmGetErrorStatus(s_instance->S); \
	if (errorStatus) { \
		logError(errorStatus); \
		return fmi2Error; \
	}

#define UNUSED(x) (void)(x)

int rtPrintfNoOp(const char *fmt, ...) {

	va_list args;
	va_start(args, fmt);

	if (s_instance && s_instance->logger) {
		char message[1024] = "";
		vsnprintf(message, 1024, fmt, args);
        s_instance->logger(s_instance->componentEnvironment, s_instance->instanceName, fmi2OK, "info", message);
	} else {
		vprintf(fmt, args);
	}

	va_end(args);

	return 0;
}

static void logError(const char *message) {
    if (s_instance && s_instance->logger) {
        s_instance->logger(s_instance->componentEnvironment, s_instance->instanceName, fmi2Error, "error", message);
    }
}

static void doFixedStep(RT_MDL_TYPE *S) {

#if NUM_TASKS > 1 // multitasking

	// step the model for the base sample time
	MODEL_STEP(0);

	// step the model for any other sample times (subrates)
	for (int i = FIRST_TASK_ID + 1; i < NUM_SAMPLE_TIMES; i++) {
		if (rtmStepTask(S, i)) {
			MODEL_STEP(i);
		}
		if (++rtmTaskCounter(S, i) == rtmCounterLimit(S, i)) {
			rtmTaskCounter(S, i) = 0;
		}
	}

#else // singletasking

	MODEL_STEP();

#endif
}

/***************************************************
Types for Common Functions
****************************************************/

/* Inquire version numbers of header files and setting logging status */
const char* fmi2GetTypesPlatform() {
	return fmi2TypesPlatform;
}

const char* fmi2GetVersion() {
	return fmi2Version;
}

fmi2Status  fmi2SetDebugLogging(fmi2Component c,
	fmi2Boolean loggingOn,
	size_t nCategories,
	const fmi2String categories[]) {

    UNUSED(c);
    UNUSED(loggingOn);
    UNUSED(categories);
    
    if (nCategories == 0) {
        return fmi2OK;
    }

    return fmi2Error;
}

/* Creation and destruction of FMU instances and setting debug status */
fmi2Component fmi2Instantiate(fmi2String instanceName,
	fmi2Type fmuType,
	fmi2String fmuGUID,
	fmi2String fmuResourceLocation,
	const fmi2CallbackFunctions* functions,
	fmi2Boolean visible,
	fmi2Boolean loggingOn) {

    UNUSED(fmuType);
    UNUSED(visible);
    UNUSED(loggingOn);

	size_t len;

    /* check interface type */
    if (fmuType != fmi2CoSimulation) {
        return NULL;
    }

	/* check GUID */
	if (strcmp(fmuGUID, MODEL_GUID) != 0) {
		return NULL;
	}

    /* check if the FMU has already been instantiated */
    if (s_instance) {
        logError("The FMU can only be instantiated once per process.");
        return NULL;
    }

	/* set the path to the resources directory */
    if (fmuResourceLocation && !FMU_RESOURCES_DIR) {
        const char *scheme1 = "file:///";
        const char *scheme2 = "file:/";
        char *path = NULL;

        if (strncmp(fmuResourceLocation, scheme1, strlen(scheme1)) == 0) {
            path = strdup(&fmuResourceLocation[strlen(scheme1) - 1]);
        } else if (strncmp(fmuResourceLocation, scheme2, strlen(scheme2)) == 0) {
            path = strdup(&fmuResourceLocation[strlen(scheme2) - 1]);
        }

        if (path) {
#ifdef _WIN32
            // strip any leading slashes
            while (path[0] == '/') {
                strcpy(path, &path[1]);
            }
#endif
            FMU_RESOURCES_DIR = path;
        }
    }

    s_instance = malloc(sizeof(ModelInstance));

	len = strlen(instanceName);
    s_instance->instanceName = malloc((len + 1) * sizeof(char));
	strncpy((char *)s_instance->instanceName, instanceName, len + 1);
    s_instance->logger = functions->logger;
    s_instance->componentEnvironment = functions->componentEnvironment;

    s_instance->S = RT_MDL_INSTANCE;

	initializeModelVariables(s_instance->S, s_instance->modelVariables);

	return s_instance;
}

void fmi2FreeInstance(fmi2Component c) {

    if (!c || c != s_instance) return;
		
	free((void *)s_instance->instanceName);
	free(s_instance);
    free((void *)FMU_RESOURCES_DIR);

	FMU_RESOURCES_DIR = NULL;
    s_instance = NULL;
}

/* Enter and exit initialization mode, terminate and reset */
fmi2Status fmi2SetupExperiment(fmi2Component c,
	fmi2Boolean toleranceDefined,
	fmi2Real tolerance,
	fmi2Real startTime,
	fmi2Boolean stopTimeDefined,
	fmi2Real stopTime) {

    UNUSED(toleranceDefined);
    UNUSED(tolerance);

	ASSERT_INSTANCE

    if (startTime != 0) {
        logError("startTime != 0.0 is not supported.");
        return fmi2Error;
    }

	if (stopTimeDefined && stopTime <= startTime) {
        logError("stopTime must be greater than startTime.");
		return fmi2Error;
	}

	return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {

	ASSERT_INSTANCE

	MODEL_INITIALIZE();

	CHECK_ERROR_STATUS

	return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {

	ASSERT_INSTANCE

	doFixedStep(s_instance->S);

	CHECK_ERROR_STATUS

	return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component c) {

	ASSERT_INSTANCE

	MODEL_TERMINATE();

	s_instance->S = NULL;

	return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component c) {

	ASSERT_INSTANCE

    if (s_instance->S) {
        MODEL_TERMINATE();
    }

	initializeModelVariables(s_instance->S, s_instance->modelVariables);

	return fmi2OK;
}

/* Getting and setting variable values */
fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {

	ASSERT_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {
		
		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = s_instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_DOUBLE:
			value[i] = *(REAL64_T *)v.address;
			break;
		case SS_SINGLE:
			value[i] = *(REAL32_T *)v.address;
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]) {

	ASSERT_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = s_instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_INT8:
			value[i] = *(INT8_T *)v.address;
			break;
		case SS_UINT8:
			value[i] = *(UINT8_T *)v.address;
			break;
		case SS_INT16:
			value[i] = *(INT16_T *)v.address;
			break;
		case SS_UINT16:
			value[i] = *(UINT16_T *)v.address;
			break;
		case SS_INT32:
			value[i] = *(INT32_T *)v.address;
			break;
		case SS_UINT32:
			value[i] = *(UINT32_T *)v.address;
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]) {

	ASSERT_INSTANCE

	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = s_instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_BOOLEAN:
			value[i] = *(BOOLEAN_T *)v.address;
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[]) { 

    UNUSED(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);

    NOT_IMPLEMENTED
}

fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]) { 

	ASSERT_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = s_instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_DOUBLE:
			*((REAL64_T *)v.address) = value[i];
			break;
		case SS_SINGLE:
			if (value[i] < -FLT_MAX || value[i] > FLT_MAX) {
				// TODO: log this
				return fmi2Error;
			}
			*((REAL32_T *)v.address) = (REAL32_T)value[i];
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]) {

	ASSERT_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = s_instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_INT8:
			*((INT8_T *)v.address) = (INT8_T)value[i];
			break;
		case SS_UINT8:
			*((UINT8_T *)v.address) = (UINT8_T)value[i];
			break;
		case SS_INT16:
			*((INT16_T *)v.address) = (INT16_T)value[i];
			break;
		case SS_UINT16:
			*((UINT16_T *)v.address) = (UINT16_T)value[i];
			break;
		case SS_INT32:
			*((INT32_T *)v.address) = value[i];
			break;
		case SS_UINT32:
			*((UINT32_T *)v.address) = value[i];
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]) {

	ASSERT_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = s_instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_BOOLEAN:
			*((BOOLEAN_T *)v.address) = (BOOLEAN_T)value[i];
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[]) {

    UNUSED(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(value);

    NOT_IMPLEMENTED
}

/* Getting and setting the internal FMU state */
fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) {

    UNUSED(c);
    UNUSED(FMUstate);

    NOT_IMPLEMENTED
}

fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate  FMUstate) {

    UNUSED(c);
    UNUSED(FMUstate);

    NOT_IMPLEMENTED
}

fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) {

    UNUSED(c);
    UNUSED(FMUstate);

    NOT_IMPLEMENTED
}

fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate  FMUstate, size_t* size) {

    UNUSED(c);
    UNUSED(FMUstate);
    UNUSED(size);

    NOT_IMPLEMENTED
}

fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate  FMUstate, fmi2Byte serializedState[], size_t size) {

    UNUSED(c);
    UNUSED(FMUstate);
    UNUSED(serializedState);
    UNUSED(size);

    NOT_IMPLEMENTED
}

fmi2Status fmi2DeSerializeFMUstate(fmi2Component c, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate) {

    UNUSED(c);
    UNUSED(serializedState);
    UNUSED(size);
    UNUSED(FMUstate);

    NOT_IMPLEMENTED
}

/* Getting partial derivatives */
fmi2Status fmi2GetDirectionalDerivative(fmi2Component c,
	const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
	const fmi2ValueReference vKnown_ref[], size_t nKnown,
	const fmi2Real dvKnown[],
	fmi2Real dvUnknown[]) {

    UNUSED(c);
    UNUSED(vUnknown_ref);
    UNUSED(nUnknown);
    UNUSED(vKnown_ref);
    UNUSED(nKnown);
    UNUSED(dvKnown);
    UNUSED(dvUnknown);

    NOT_IMPLEMENTED
}

/***************************************************
Types for Functions for FMI2 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component c) { 

    UNUSED(c);

    NOT_IMPLEMENTED
}

fmi2Status fmi2NewDiscreteStates(fmi2Component c, fmi2EventInfo* fmi2eventInfo) {

    UNUSED(c);
    UNUSED(fmi2eventInfo);

    NOT_IMPLEMENTED
}


fmi2Status fmi2EnterContinuousTimeMode(fmi2Component c) {

    UNUSED(c);

    NOT_IMPLEMENTED
}

fmi2Status fmi2CompletedIntegratorStep(fmi2Component c,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint,
	fmi2Boolean*  enterEventMode,
	fmi2Boolean*  terminateSimulation) {

    UNUSED(c);
    UNUSED(noSetFMUStatePriorToCurrentPoint);
    UNUSED(enterEventMode);
    UNUSED(terminateSimulation);

    NOT_IMPLEMENTED
}

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time) {

    UNUSED(c);
    UNUSED(time);

    NOT_IMPLEMENTED
}

fmi2Status fmi2SetContinuousStates(fmi2Component c, const fmi2Real x[], size_t nx) {

    UNUSED(c);
    UNUSED(x);
    UNUSED(nx);

    NOT_IMPLEMENTED
}

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component c, fmi2Real derivatives[], size_t nx) {

    UNUSED(c);
    UNUSED(derivatives);
    UNUSED(nx);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetEventIndicators(fmi2Component c, fmi2Real eventIndicators[], size_t ni) {

    UNUSED(c);
    UNUSED(eventIndicators);
    UNUSED(ni);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetContinuousStates(fmi2Component c, fmi2Real x[], size_t nx) {

    UNUSED(c);
    UNUSED(x);
    UNUSED(nx);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component c, fmi2Real x_nominal[], size_t nx) {

    UNUSED(c);
    UNUSED(x_nominal);
    UNUSED(nx);

    NOT_IMPLEMENTED
}


/***************************************************
Types for Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	const fmi2Real value[]) {

    UNUSED(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	fmi2Real value[]) {

    UNUSED(c);
    UNUSED(vr);
    UNUSED(nvr);
    UNUSED(order);
    UNUSED(value);

    NOT_IMPLEMENTED
}

fmi2Status fmi2DoStep(fmi2Component c,
	fmi2Real      currentCommunicationPoint,
	fmi2Real      communicationStepSize,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint) {

    UNUSED(noSetFMUStatePriorToCurrentPoint);

	ASSERT_INSTANCE
		
#ifdef rtmGetT
	time_T tNext = currentCommunicationPoint + communicationStepSize;
	double epsilon = (1.0 + fabs(rtmGetT(s_instance->S))) * 2 * DBL_EPSILON;
	
    while (rtmGetT(s_instance->S) < tNext + epsilon)
#endif
	{
		doFixedStep(s_instance->S);

		CHECK_ERROR_STATUS
	}

	return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component c) {

    UNUSED(c);

    NOT_IMPLEMENTED
}

/* Inquire slave status */
fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status*  value) {

    UNUSED(c);
    UNUSED(s);
    UNUSED(value);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real*    value) {

    UNUSED(c);
    UNUSED(s);
    UNUSED(value);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value) {

    UNUSED(c);
    UNUSED(s);
    UNUSED(value);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value) {

    UNUSED(c);
    UNUSED(s);
    UNUSED(value);

    NOT_IMPLEMENTED
}

fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String*  value) {

    UNUSED(c);
    UNUSED(s);
    UNUSED(value);

    NOT_IMPLEMENTED
}
