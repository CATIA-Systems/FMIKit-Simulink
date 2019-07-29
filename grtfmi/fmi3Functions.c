#include <float.h>  /* for DBL_EPSILON */

#include "fmiwrapper.inc"

#include "fmi3Functions.h"

const char *RT_MEMORY_ALLOCATION_ERROR = "memory allocation error";

int rtPrintfNoOp(const char *fmt, ...) {
	return 0;  /* do nothing */
}

typedef struct {
	RT_MDL_TYPE *S;
	const char *instanceName;
	fmi3CallbackLogMessage logger;
	fmi3InstanceEnvironment componentEnvironment;
} ModelInstance;

#define NOT_IMPLEMENTED return fmi3Error;

/***************************************************
Types for Common Functions
****************************************************/

const char* fmi3GetVersion() {
	return fmi3Version;
}

fmi3Status  fmi3SetDebugLogging(fmi3Instance c,
	fmi3Boolean loggingOn,
	size_t nCategories,
	const fmi3String categories[]) { return fmi3Error; }

/* Creation and destruction of FMU instances and setting debug status */
fmi3Instance fmi3Instantiate(fmi3String instanceName,
	fmi3InterfaceType fmuType,
	fmi3String fmuGUID,
	fmi3String fmuResourceLocation,
	const fmi3CallbackFunctions* functions,
	fmi3Boolean visible,
	fmi3Boolean loggingOn) {

	/* check GUID */
	if (strcmp(fmuGUID, MODEL_GUID) != 0) {
		return NULL;
	}

	ModelInstance *instance = malloc(sizeof(ModelInstance));

	size_t len = strlen(instanceName);
	instance->instanceName = malloc((len + 1) * sizeof(char));
	strncpy((char *)instance->instanceName, instanceName, len + 1);
	instance->logger = functions->logMessage;
	instance->componentEnvironment = functions->instanceEnvironment;

#ifdef REUSABLE_FUNCTION
	instance->S = MODEL();
	MODEL_INITIALIZE(instance->S);
#else
	MODEL_INITIALIZE();

	instance->S = RT_MDL_INSTANCE;
#endif

	return instance;
}

void fmi3FreeInstance(fmi3Instance c) {
	ModelInstance *instance = (ModelInstance *)c;
	free((void *)instance->instanceName);
	free(instance);
}

/* Enter and exit initialization mode, terminate and reset */
fmi3Status fmi3SetupExperiment(fmi3Instance c,
	fmi3Boolean toleranceDefined,
	fmi3Float64 tolerance,
	fmi3Float64 startTime,
	fmi3Boolean stopTimeDefined,
	fmi3Float64 stopTime) {
	return fmi3OK;
}

fmi3Status fmi3EnterInitializationMode(fmi3Instance c) {
	return fmi3OK;
}

fmi3Status fmi3ExitInitializationMode(fmi3Instance c) {
	return fmi3OK;
}

fmi3Status fmi3Terminate(fmi3Instance c) {

	ModelInstance *instance = (ModelInstance *)c;

#ifdef REUSABLE_FUNCTION
	MODEL_TERMINATE(instance->S);
#else
	MODEL_TERMINATE();
#endif

	instance->S = NULL;

	return fmi3OK;
}

fmi3Status fmi3Reset(fmi3Instance c) {

    ModelInstance *instance = (ModelInstance *)c;
    
#ifdef REUSABLE_FUNCTION
    if (instance->S) {
        MODEL_TERMINATE(instance->S);
    }
    
    instance->S = MODEL();
    MODEL_INITIALIZE(instance->S);
#else
    if (instance->S) {
        MODEL_TERMINATE();
    }
    
    MODEL_INITIALIZE();
#endif

	return fmi3OK;
}



/* Getting and setting variable values */

static fmi3Status getVariables(ModelInstance *instance,
	const fmi3ValueReference vr[], size_t nvr,
	void *values, size_t nValues, BuiltInDTypeId datatypeID, size_t typeSize) {

	BuiltInDTypeId dtypeID = -1;
	size_t i, size, copied = 0;

	for (i = 0; i < nvr; i++) {

		const void *vptr = getScalarVariable(instance->S, vr[i], &dtypeID, &size);

		if (vptr == NULL) {
			return fmi3Error;
		}

		if (dtypeID != datatypeID) {
			return fmi3Error;
		}

		if (copied + size > nValues) {
			return fmi3Error;
		}

		memcpy(values, vptr, typeSize * size);

		copied += size;
		values = (char *)values + (size * typeSize);
	}

	return fmi3OK;
}

fmi3Status fmi3GetFloat32(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3Float32 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_SINGLE, sizeof(REAL32_T));
}

fmi3Status fmi3GetFloat64(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3Float64 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_DOUBLE, sizeof(REAL64_T));
}

fmi3Status fmi3GetInt8(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3Int8 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_INT8, sizeof(INT8_T));
}

fmi3Status fmi3GetUInt8(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3UInt8 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_UINT8, sizeof(UINT8_T));
}

fmi3Status fmi3GetInt16(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3Int16 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_INT16, sizeof(INT16_T));
}

fmi3Status fmi3GetUInt16(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3UInt16 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_UINT16, sizeof(UINT16_T));
}

fmi3Status fmi3GetInt32(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3Int32 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_INT32, sizeof(INT32_T));
}

fmi3Status fmi3GetUInt32(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3UInt32 value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_UINT32, sizeof(UINT32_T));
}

fmi3Status fmi3GetInt64(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3Int64 value[], size_t nValues) {
	NOT_IMPLEMENTED
}

fmi3Status fmi3GetUInt64(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3UInt64 value[], size_t nValues) {
	NOT_IMPLEMENTED
}

fmi3Status fmi3GetBoolean(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	fmi3Boolean value[], size_t nValues) {
	return getVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_BOOLEAN, sizeof(BOOLEAN_T));
}

fmi3Status fmi3GetBinary(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	size_t size[], fmi3Binary value[], size_t nValues) {
	NOT_IMPLEMENTED
}

fmi3Status fmi3GetString(fmi3Instance c, const fmi3ValueReference vr[], size_t nvr, fmi3String  value[], size_t nValues) { NOT_IMPLEMENTED }

static fmi3Status setVariables(ModelInstance *instance,
	const fmi3ValueReference vr[], size_t nvr,
	const void *values, size_t nValues, BuiltInDTypeId datatypeID, size_t typeSize) {

	BuiltInDTypeId dtypeID = -1;
	size_t i, size, copied = 0;

	for (i = 0; i < nvr; i++) {

		void *vptr = getScalarVariable(instance->S, vr[i], &dtypeID, &size);

		if (vptr == NULL) {
			return fmi3Error;
		}

		if (dtypeID != datatypeID) {
			return fmi3Error;
		}

		if (copied + size > nValues) {
			return fmi3Error;
		}

		memcpy(vptr, values, typeSize * size);

		copied += size;
		values = (char *)values + (size * typeSize);
	}

	return fmi3OK;
}

fmi3Status fmi3SetFloat32(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3Float32 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_SINGLE, sizeof(REAL32_T));
}

fmi3Status fmi3SetFloat64(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3Float64 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_DOUBLE, sizeof(REAL64_T));
}

fmi3Status fmi3SetInt8(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3Int8 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_INT8, sizeof(INT8_T));
}

fmi3Status fmi3SetUInt8(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3UInt8 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_UINT8, sizeof(UINT8_T));
}

fmi3Status fmi3SetInt16(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3Int16 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_INT16, sizeof(INT16_T));
}

fmi3Status fmi3SetUInt16(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3UInt16 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_UINT16, sizeof(UINT16_T));
}

fmi3Status fmi3SetInt32(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3Int32 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_INT32, sizeof(INT32_T));
}

fmi3Status fmi3SetUInt32(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3UInt32 value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_UINT32, sizeof(UINT32_T));
}

fmi3Status fmi3SetInt64(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3Int64 value[], size_t nValues) {
	NOT_IMPLEMENTED
}

fmi3Status fmi3SetUInt64(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3UInt64 value[], size_t nValues) {
	NOT_IMPLEMENTED
}

fmi3Status fmi3SetBoolean(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3Boolean value[], size_t nValues) {
	return setVariables((ModelInstance *)c, vr, nvr, value, nValues, SS_BOOLEAN, sizeof(BOOLEAN_T));
}

fmi3Status fmi3SetString(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const fmi3String value[], size_t nValues) { NOT_IMPLEMENTED }

fmi3Status fmi3SetBinary(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const size_t size[], const fmi3Binary value[], size_t nValues) { NOT_IMPLEMENTED }

/* Getting and setting the internal FMU state */
fmi3Status fmi3GetFMUstate(fmi3Instance c, fmi3FMUState* FMUState) { NOT_IMPLEMENTED }
fmi3Status fmi3SetFMUstate(fmi3Instance c, fmi3FMUState  FMUState) { NOT_IMPLEMENTED }
fmi3Status fmi3FreeFMUstate(fmi3Instance c, fmi3FMUState* FMUState) { NOT_IMPLEMENTED }
fmi3Status fmi3SerializedFMUstateSize(fmi3Instance c, fmi3FMUState  FMUstate, size_t* size) { NOT_IMPLEMENTED }
fmi3Status fmi3SerializeFMUstate(fmi3Instance c, fmi3FMUState  FMUstate, fmi3Byte serializedState[], size_t size) { NOT_IMPLEMENTED }
fmi3Status fmi3DeSerializeFMUstate(fmi3Instance c, const fmi3Byte serializedState[], size_t size, fmi3FMUState* FMUState) { NOT_IMPLEMENTED }

/* Getting partial derivatives */
fmi3Status fmi3GetDirectionalDerivative(fmi3Instance c,
                                        const fmi3ValueReference vrUnknown[],
                                        size_t nUnknown,
                                        const fmi3ValueReference vrKnown[],
                                        size_t nKnown,
                                        const fmi3Float64 dvKnown[],
                                        size_t nDvKnown,
                                        fmi3Float64 dvUnknown[],
                                        size_t nDvUnknown) { NOT_IMPLEMENTED }

/***************************************************
Types for Functions for FMI3 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
fmi3Status fmi3EnterEventMode(fmi3Instance c) { NOT_IMPLEMENTED }
fmi3Status fmi3NewDiscreteStates(fmi3Instance c, fmi3EventInfo* fmi3eventInfo) { NOT_IMPLEMENTED }
fmi3Status fmi3EnterContinuousTimeMode(fmi3Instance c) { NOT_IMPLEMENTED }
fmi3Status fmi3CompletedIntegratorStep(fmi3Instance c,
	fmi3Boolean   noSetFMUStatePriorToCurrentPoint,
	fmi3Boolean*  enterEventMode,
	fmi3Boolean*  terminateSimulation) { NOT_IMPLEMENTED }

/* Providing independent variables and re-initialization of caching */
fmi3Status fmi3SetTime(fmi3Instance c, fmi3Float64 time) { NOT_IMPLEMENTED }
fmi3Status fmi3SetContinuousStates(fmi3Instance c, const fmi3Float64 x[], size_t nx) { NOT_IMPLEMENTED }

/* Evaluation of the model equations */
fmi3Status fmi3GetDerivatives(fmi3Instance c, fmi3Float64 derivatives[], size_t nx) { NOT_IMPLEMENTED }
fmi3Status fmi3GetEventIndicators(fmi3Instance c, fmi3Float64 eventIndicators[], size_t ni) { NOT_IMPLEMENTED }
fmi3Status fmi3GetContinuousStates(fmi3Instance c, fmi3Float64 x[], size_t nx) { NOT_IMPLEMENTED }
fmi3Status fmi3GetNominalsOfContinuousStates(fmi3Instance c, fmi3Float64 x_nominal[], size_t nx) { NOT_IMPLEMENTED }


/***************************************************
Types for Functions for FMI3 for Co-Simulation
****************************************************/

/* Simulating the slave */
fmi3Status fmi3SetInputDerivatives(fmi3Instance c,
	const fmi3ValueReference vr[],
	size_t nvr,
	const fmi3Int32 order[],
	const fmi3Float64 value[],
	size_t nValues) { NOT_IMPLEMENTED }

fmi3Status fmi3GetOutputDerivatives(fmi3Instance c,
	const fmi3ValueReference vr[],
	size_t nvr,
	const fmi3Int32 order[],
	fmi3Float64 value[],
	size_t nValues) { NOT_IMPLEMENTED }

fmi3Status fmi3DoStep(fmi3Instance c,
	fmi3Float64      currentCommunicationPoint,
	fmi3Float64      communicationStepSize,
	fmi3Boolean   noSetFMUStatePriorToCurrentPoint) {

	ModelInstance *instance = (ModelInstance *)c;

#ifndef DISCRETE
	time_T tNext = currentCommunicationPoint + communicationStepSize;

	while (rtmGetT(instance->S) + STEP_SIZE < tNext + DBL_EPSILON) {
#endif
        
#ifdef REUSABLE_FUNCTION
		MODEL_STEP(instance->S);
#else
        MODEL_STEP();
#endif
		const char *errorStatus = rtmGetErrorStatus(instance->S);
		if (errorStatus) {
			instance->logger(instance->componentEnvironment, instance->instanceName, fmi3Error, "error", errorStatus);
			return fmi3Error;
		}
#ifndef DISCRETE
	}
#endif

	return fmi3OK;
}

fmi3Status fmi3CancelStep(fmi3Instance c) { NOT_IMPLEMENTED }

fmi3Status fmi3GetDoStepPendingStatus(fmi3Instance c,
	fmi3Status* status,
	fmi3String* message) { NOT_IMPLEMENTED }

fmi3Status fmi3GetDoStepDiscardedStatus(fmi3Instance c,
		fmi3Boolean* terminate,
		fmi3Float64* lastSuccessfulTime) { NOT_IMPLEMENTED }
