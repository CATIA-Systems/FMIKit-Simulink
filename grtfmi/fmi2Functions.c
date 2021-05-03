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

ModelInstance *currentInstance = NULL;

#define GET_INSTANCE \
	if (!c) return fmi2Error; \
	ModelInstance *instance = (ModelInstance *)c; \
	RT_MDL_TYPE *S = instance->S; \
	currentInstance = instance;

#define NOT_IMPLEMENTED \
	return fmi2Error;

#define CHECK_ERROR_STATUS \
	const char *errorStatus = rtmGetErrorStatus(S); \
	if (errorStatus) { \
		instance->logger(instance->componentEnvironment, instance->instanceName, fmi2Error, "error", errorStatus); \
		return fmi2Error; \
	}

int rtPrintfNoOp(const char *fmt, ...) {

	va_list args;
	va_start(args, fmt);

	if (currentInstance && currentInstance->logger) {
		char message[1024] = "";
		size_t len = vsnprintf(message, 1024, fmt, args);
		currentInstance->logger(currentInstance->componentEnvironment, currentInstance->instanceName, fmi2OK, "info", message);
	} else {
		vprintf(fmt, args);
	}

	va_end(args);

	return 0;
}

static void setResourcePath(const char *uri) {

	const char *scheme1 = "file:///";
	const char *scheme2 = "file:/";
	char *path;

	if (!uri || FMU_RESOURCES_DIR) return;

	if (strncmp(uri, scheme1, strlen(scheme1)) == 0) {
        path = strdup(&uri[strlen(scheme1) - 1]);
	} else if (strncmp(uri, scheme2, strlen(scheme2)) == 0) {
        path = strdup(&uri[strlen(scheme2) - 1]);
    } else {
        return;
    }

#ifdef _WIN32
	// strip any leading slashes
	while (path[0] == '/') {
		strcpy(path, &path[1]);
	}
#endif

	FMU_RESOURCES_DIR = path;
}

static void doFixedStep(RT_MDL_TYPE *S) {

#if NUM_TASKS > 1 // multitasking

#ifdef REUSABLE_FUNCTION
	// step the model for the base sample time
	MODEL_STEP(S, 0);

	// step the model for any other sample times (subrates)
	for (int i = FIRST_TASK_ID + 1; i < NUM_SAMPLE_TIMES; i++) {
		if (rtmStepTask(S, i)) {
			MODEL_STEP(S, i);
		}
		if (++rtmTaskCounter(S, i) == rtmCounterLimit(S, i)) {
			rtmTaskCounter(S, i) = 0;
		}
	}
#else
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
#endif

#else // multitasking

#ifdef REUSABLE_FUNCTION
	MODEL_STEP(S);
#else
	MODEL_STEP();
#endif

#endif // multitasking
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

	ModelInstance *instance;
	size_t len;

	/* check GUID */
	if (strcmp(fmuGUID, MODEL_GUID) != 0) {
		return NULL;
	}

	/* set the path to the resources directory */
	setResourcePath(fmuResourceLocation);

	instance = malloc(sizeof(ModelInstance));

	len = strlen(instanceName);
	instance->instanceName = malloc((len + 1) * sizeof(char));
	strncpy((char *)instance->instanceName, instanceName, len + 1);
	instance->logger = functions->logger;
	instance->componentEnvironment = functions->componentEnvironment;

	currentInstance = instance;

#ifdef REUSABLE_FUNCTION
	instance->S = MODEL();
#else
	instance->S = RT_MDL_INSTANCE;
#endif

	initializeModelVariables(instance->S, instance->modelVariables);

	return instance;
}

void fmi2FreeInstance(fmi2Component c) {

	if (!c) return;
	
	ModelInstance *instance = (ModelInstance *)c;
	
	free((void *)instance->instanceName);
	free(instance);
    free((void *)FMU_RESOURCES_DIR);
	FMU_RESOURCES_DIR = NULL;
}

/* Enter and exit initialization mode, terminate and reset */
fmi2Status fmi2SetupExperiment(fmi2Component c,
	fmi2Boolean toleranceDefined,
	fmi2Real tolerance,
	fmi2Real startTime,
	fmi2Boolean stopTimeDefined,
	fmi2Real stopTime) {

	GET_INSTANCE

	if (stopTimeDefined && stopTime <= startTime) {
		return fmi2Error;
	}

	return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {

	GET_INSTANCE

#ifdef REUSABLE_FUNCTION
	MODEL_INITIALIZE(instance->S);
#else
	MODEL_INITIALIZE();
#endif

	CHECK_ERROR_STATUS

	return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {

	GET_INSTANCE

	doFixedStep(S);

	CHECK_ERROR_STATUS

	return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component c) {

	GET_INSTANCE

#ifdef REUSABLE_FUNCTION
	MODEL_TERMINATE(instance->S);
#else
	MODEL_TERMINATE();
#endif

	instance->S = NULL;

	return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component c) {

	GET_INSTANCE

#ifdef REUSABLE_FUNCTION
    if (instance->S) {
        MODEL_TERMINATE(instance->S);
    }

	instance->S = MODEL();
#else
    if (instance->S) {
        MODEL_TERMINATE();
    }
#endif

	initializeModelVariables(instance->S, instance->modelVariables);

	return fmi2OK;
}

/* Getting and setting variable values */
fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {

	GET_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {
		
		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = instance->modelVariables[index];

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

	GET_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = instance->modelVariables[index];

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

	GET_INSTANCE

	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = instance->modelVariables[index];

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

fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[]) { NOT_IMPLEMENTED }

fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]) { 

	GET_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = instance->modelVariables[index];

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

	GET_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_INT8:
			*((INT8_T *)v.address) = value[i];
			break;
		case SS_UINT8:
			*((UINT8_T *)v.address) = value[i];
			break;
		case SS_INT16:
			*((INT16_T *)v.address) = value[i];
			break;
		case SS_UINT16:
			*((UINT16_T *)v.address) = value[i];
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

	GET_INSTANCE
		
	size_t i, index;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi2Error;
		}

		v = instance->modelVariables[index];

		switch (v.dtypeID) {
		case SS_BOOLEAN:
			*((BOOLEAN_T *)v.address) = value[i];
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[]) { NOT_IMPLEMENTED }

/* Getting and setting the internal FMU state */
fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) { NOT_IMPLEMENTED }
fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate  FMUstate) { NOT_IMPLEMENTED }
fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) { NOT_IMPLEMENTED }
fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate  FMUstate, size_t* size) { NOT_IMPLEMENTED }
fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate  FMUstate, fmi2Byte serializedState[], size_t size) { NOT_IMPLEMENTED }
fmi2Status fmi2DeSerializeFMUstate(fmi2Component c, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate) { NOT_IMPLEMENTED }

/* Getting partial derivatives */
fmi2Status fmi2GetDirectionalDerivative(fmi2Component c,
	const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
	const fmi2ValueReference vKnown_ref[], size_t nKnown,
	const fmi2Real dvKnown[],
	fmi2Real dvUnknown[]) { NOT_IMPLEMENTED }

/***************************************************
Types for Functions for FMI2 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component c) { NOT_IMPLEMENTED }
fmi2Status fmi2NewDiscreteStates(fmi2Component c, fmi2EventInfo* fmi2eventInfo) { NOT_IMPLEMENTED }
fmi2Status fmi2EnterContinuousTimeMode(fmi2Component c) { NOT_IMPLEMENTED }
fmi2Status fmi2CompletedIntegratorStep(fmi2Component c,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint,
	fmi2Boolean*  enterEventMode,
	fmi2Boolean*  terminateSimulation) { NOT_IMPLEMENTED }

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time) { NOT_IMPLEMENTED }
fmi2Status fmi2SetContinuousStates(fmi2Component c, const fmi2Real x[], size_t nx) { NOT_IMPLEMENTED }

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component c, fmi2Real derivatives[], size_t nx) { NOT_IMPLEMENTED }
fmi2Status fmi2GetEventIndicators(fmi2Component c, fmi2Real eventIndicators[], size_t ni) { NOT_IMPLEMENTED }
fmi2Status fmi2GetContinuousStates(fmi2Component c, fmi2Real x[], size_t nx) { NOT_IMPLEMENTED }
fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component c, fmi2Real x_nominal[], size_t nx) { NOT_IMPLEMENTED }


/***************************************************
Types for Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	const fmi2Real value[]) { NOT_IMPLEMENTED }

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	fmi2Real value[]) { NOT_IMPLEMENTED }

fmi2Status fmi2DoStep(fmi2Component c,
	fmi2Real      currentCommunicationPoint,
	fmi2Real      communicationStepSize,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint) {

	GET_INSTANCE
		
#ifdef rtmGetT
	time_T tNext = currentCommunicationPoint + communicationStepSize;
	double epsilon = (1.0 + fabs(rtmGetT(S))) * 2 * DBL_EPSILON;
	
    while (rtmGetT(S) < tNext + epsilon)
#endif
	{
		doFixedStep(S);

		CHECK_ERROR_STATUS
	}

	return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component c) { NOT_IMPLEMENTED }

/* Inquire slave status */
fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status*  value) { NOT_IMPLEMENTED }
fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real*    value) { NOT_IMPLEMENTED }
fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value) { NOT_IMPLEMENTED }
fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value) { NOT_IMPLEMENTED }
fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String*  value) { NOT_IMPLEMENTED }
