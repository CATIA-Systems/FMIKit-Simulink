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
	ModelVariable modelVariables[N_MODEL_VARIABLES];
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
	const fmi3String categories[]) {
    NOT_IMPLEMENTED
}

/* Creation and destruction of FMU instances and setting debug status */
fmi3Instance fmi3InstantiateModelExchange(
	fmi3String                 instanceName,
	fmi3String                 instantiationToken,
	fmi3String                 resourceLocation,
	fmi3Boolean                visible,
	fmi3Boolean                loggingOn,
	fmi3InstanceEnvironment    instanceEnvironment,
	fmi3CallbackLogMessage     logMessage) {

	return NULL; // not supported
}

fmi3Instance fmi3InstantiateCoSimulation(
    fmi3String                     instanceName,
    fmi3String                     instantiationToken,
    fmi3String                     resourceLocation,
    fmi3Boolean                    visible,
    fmi3Boolean                    loggingOn,
    fmi3Boolean                    eventModeUsed,
    fmi3Boolean                    earlyReturnAllowed,
    const fmi3ValueReference       requiredIntermediateVariables[],
    size_t                         nRequiredIntermediateVariables,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3CallbackLogMessage         logMessage,
    fmi3CallbackIntermediateUpdate intermediateUpdate) {

	/* check GUID */
	if (strcmp(instantiationToken, MODEL_GUID) != 0) {
		return NULL;
	}

	ModelInstance *instance = malloc(sizeof(ModelInstance));

	size_t len = strlen(instanceName);
	instance->instanceName = malloc((len + 1) * sizeof(char));
	strncpy((char *)instance->instanceName, instanceName, len + 1);
	instance->logger = logMessage;
	instance->componentEnvironment = instanceEnvironment;

#ifdef REUSABLE_FUNCTION
	instance->S = MODEL();
	MODEL_INITIALIZE(instance->S);
#else
	MODEL_INITIALIZE();

	instance->S = RT_MDL_INSTANCE;
#endif

	initializeModelVariables(instance->S, instance->modelVariables);

	return instance;
}

fmi3Instance fmi3InstantiateScheduledExecution(
	fmi3String                     instanceName,
	fmi3String                     instantiationToken,
	fmi3String                     resourceLocation,
	fmi3Boolean                    visible,
	fmi3Boolean                    loggingOn,
	const fmi3ValueReference       requiredIntermediateVariables[],
	size_t                         nRequiredIntermediateVariables,
	fmi3InstanceEnvironment        instanceEnvironment,
	fmi3CallbackLogMessage         logMessage,
	fmi3CallbackIntermediateUpdate intermediateUpdate,
	fmi3CallbackLockPreemption     lockPreemption,
	fmi3CallbackUnlockPreemption   unlockPreemption) {

	return NULL; // not supported
}

void fmi3FreeInstance(fmi3Instance c) {
	ModelInstance *instance = (ModelInstance *)c;
	free((void *)instance->instanceName);
	free(instance);
}

fmi3Status fmi3EnterInitializationMode(fmi3Instance instance,
									   fmi3Boolean toleranceDefined,
									   fmi3Float64 tolerance,
									   fmi3Float64 startTime,
									   fmi3Boolean stopTimeDefined,
									   fmi3Float64 stopTime) {
	return fmi3OK;
}

fmi3Status fmi3ExitInitializationMode(fmi3Instance c) {
	return fmi3OK;
}

fmi3Status fmi3EnterEventMode(fmi3Instance instance,
                              fmi3Boolean stepEvent,
                              fmi3Boolean stateEvent,
                              const fmi3Int32 rootsFound[],
                              size_t nEventIndicators,
                              fmi3Boolean timeEvent) {

	return fmi3Error; // not supported
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

	size_t i, j, index, copied = 0;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi3Error;
		}

		v = instance->modelVariables[index];

		if (v.dtypeID != datatypeID) {
			return fmi3Error;
		}

		if (copied + v.size > nValues) {
			return fmi3Error;
		}

		if (datatypeID == SS_BOOLEAN) {
			for (j = 0; j < v.size; j++) {
				((fmi3Boolean*)values)[j] = ((boolean_T *)v.address)[j] ? fmi3True : fmi3False;
			}
			values = (char *)values + (v.size * sizeof(fmi3Boolean));
		} else {
			memcpy(values, v.address, typeSize * v.size);
			values = (char *)values + (v.size * typeSize);
		}
		
		copied += v.size;
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

fmi3Status fmi3GetString(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3String values[],
    size_t nValues) { 
    NOT_IMPLEMENTED 
}

fmi3Status fmi3GetBinary(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	size_t size[], fmi3Binary value[], size_t nValues) {
	NOT_IMPLEMENTED
}

fmi3Status fmi3GetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Clock values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

static fmi3Status setVariables(ModelInstance *instance,
	const fmi3ValueReference vr[], size_t nvr,
	const void *values, size_t nValues, BuiltInDTypeId datatypeID, size_t typeSize) {

	size_t i, j, index, copied = 0;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

		index = vr[i] - 1;

		if (index >= N_MODEL_VARIABLES) {
			return fmi3Error;
		}

		v = instance->modelVariables[index];

		if (v.dtypeID != datatypeID) {
			return fmi3Error;
		}

		if (copied + v.size > nValues) {
			return fmi3Error;
		}

		if (datatypeID == SS_BOOLEAN) {
			for (j = 0; j < v.size; j++) {
				((boolean_T *)v.address)[j] = ((fmi3Boolean*)values)[j] != fmi3False;
			}
			values = (char *)values + (v.size * sizeof(fmi3Boolean));
		}
		else {
			memcpy(v.address, values, typeSize * v.size);
			values = (char *)values + (v.size * typeSize);
		}

		copied += v.size;
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
	const fmi3String value[], size_t nValues) { 
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetBinary(fmi3Instance c,
	const fmi3ValueReference vr[], size_t nvr,
	const size_t size[], const fmi3Binary value[], size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Clock values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

/* Getting Variable Dependency Information */
fmi3Status fmi3GetNumberOfVariableDependencies(fmi3Instance instance,
    fmi3ValueReference valueReference,
    size_t* nDependencies) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetVariableDependencies(fmi3Instance instance,
    fmi3ValueReference dependent,
    size_t elementIndicesOfDependent[],
    fmi3ValueReference independents[],
    size_t elementIndicesOfIndependents[],
    fmi3DependencyKind dependencyKinds[],
    size_t nDependencies) {
    NOT_IMPLEMENTED
}

/* Getting and setting the internal FMU state */
fmi3Status fmi3GetFMUState(fmi3Instance instance, fmi3FMUState* FMUState) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetFMUState(fmi3Instance instance, fmi3FMUState  FMUState) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3FreeFMUState(fmi3Instance instance, fmi3FMUState* FMUState) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SerializedFMUStateSize(fmi3Instance instance,
    fmi3FMUState  FMUState,
    size_t* size) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SerializeFMUState(fmi3Instance instance,
    fmi3FMUState  FMUState,
    fmi3Byte serializedState[],
    size_t size) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3DeSerializeFMUState(fmi3Instance instance,
    const fmi3Byte serializedState[],
    size_t size,
    fmi3FMUState* FMUState) {
    NOT_IMPLEMENTED
}

/* Getting partial derivatives */
fmi3Status fmi3GetDirectionalDerivative(fmi3Instance instance,
    const fmi3ValueReference unknowns[],
    size_t nUnknowns,
    const fmi3ValueReference knowns[],
    size_t nKnowns,
    const fmi3Float64 seed[],
    size_t nSeed,
    fmi3Float64 sensitivity[],
    size_t nSensitivity) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetAdjointDerivative(fmi3Instance instance,
    const fmi3ValueReference unknowns[],
    size_t nUnknowns,
    const fmi3ValueReference knowns[],
    size_t nKnowns,
    const fmi3Float64 seed[],
    size_t nSeed,
    fmi3Float64 sensitivity[],
    size_t nSensitivity) {
    NOT_IMPLEMENTED
}

/* Entering and exiting the Configuration or Reconfiguration Mode */
fmi3Status fmi3EnterConfigurationMode(fmi3Instance instance) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3ExitConfigurationMode(fmi3Instance instance) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 interval[],
    fmi3IntervalQualifier qualifier[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 intervalCounter[],
    fmi3UInt64 resolution[],
    fmi3IntervalQualifier qualifier[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetShiftDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 shift[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetShiftFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 shiftCounter[],
    fmi3UInt64 resolution[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float64 interval[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt64 intervalCounter[],
    const fmi3UInt64 resolution[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3UpdateDiscreteStates(fmi3Instance instance,
    fmi3Boolean* discreteStatesNeedUpdate,
    fmi3Boolean* terminateSimulation,
    fmi3Boolean* nominalsOfContinuousStatesChanged,
    fmi3Boolean* valuesOfContinuousStatesChanged,
    fmi3Boolean* nextEventTimeDefined,
    fmi3Float64* nextEventTime) {
    NOT_IMPLEMENTED
}

/***************************************************
Types for Functions for Model Exchange
****************************************************/

fmi3Status fmi3EnterContinuousTimeMode(fmi3Instance c) { 
    NOT_IMPLEMENTED 
}

fmi3Status fmi3CompletedIntegratorStep(fmi3Instance c,
	fmi3Boolean   noSetFMUStatePriorToCurrentPoint,
	fmi3Boolean*  enterEventMode,
	fmi3Boolean*  terminateSimulation) { 
    NOT_IMPLEMENTED 
}

/* Providing independent variables and re-initialization of caching */
fmi3Status fmi3SetTime(fmi3Instance c, fmi3Float64 time) { 
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetContinuousStates(fmi3Instance c, const fmi3Float64 x[], size_t nx) { 
    NOT_IMPLEMENTED 
}

/* Evaluation of the model equations */
fmi3Status fmi3GetDerivatives(fmi3Instance c, fmi3Float64 derivatives[], size_t nx) { 
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetEventIndicators(fmi3Instance c, fmi3Float64 eventIndicators[], size_t ni) {
    NOT_IMPLEMENTED 
}

fmi3Status fmi3GetContinuousStates(fmi3Instance c, fmi3Float64 x[], size_t nx) { 
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetNominalsOfContinuousStates(fmi3Instance c, fmi3Float64 x_nominal[], size_t nx) { 
    NOT_IMPLEMENTED 
}

fmi3Status fmi3GetNumberOfEventIndicators(fmi3Instance instance,
    size_t* nEventIndicators) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetNumberOfContinuousStates(fmi3Instance instance,
    size_t* nContinuousStates) {
    NOT_IMPLEMENTED
}

/***************************************************
Types for Functions for Co-Simulation
****************************************************/

/* Simulating the FMU */

fmi3Status fmi3EnterStepMode(fmi3Instance instance) { 
    NOT_IMPLEMENTED 
}

fmi3Status fmi3GetOutputDerivatives(fmi3Instance c,
	const fmi3ValueReference vr[],
	size_t nvr,
	const fmi3Int32 order[],
	fmi3Float64 value[],
	size_t nValues) { 
    NOT_IMPLEMENTED 
}

fmi3Status fmi3DoStep(fmi3Instance c,
                      fmi3Float64 currentCommunicationPoint,
                      fmi3Float64 communicationStepSize,
                      fmi3Boolean noSetFMUStatePriorToCurrentPoint,
                      fmi3Boolean* eventEncountered,
                      fmi3Boolean* terminateSimulation,
                      fmi3Boolean* earlyReturn,
                      fmi3Float64* lastSuccessfulTime) {

	ModelInstance *instance = (ModelInstance *)c;

	time_T tNext = currentCommunicationPoint + communicationStepSize;

#ifdef rtmGetT
	while (rtmGetT(instance->S) + STEP_SIZE < tNext + DBL_EPSILON)
#endif
	{
        
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

	}

	return fmi3OK;
}

fmi3Status fmi3ActivateModelPartition(fmi3Instance instance,
    fmi3ValueReference clockReference,
    size_t clockElementIndex,
    fmi3Float64 activationTime) { 
    NOT_IMPLEMENTED 
}
