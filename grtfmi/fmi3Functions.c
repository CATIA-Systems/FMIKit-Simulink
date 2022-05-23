#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include "fmi3Functions.h"

#include <float.h>  /* for DBL_EPSILON, FLT_MAX */
#include <math.h>   /* for fabs() */
#include <string.h> /* for strdup() */
#include <stdarg.h> /* for va_list */
#include <stdio.h>  /* for vsnprintf(), vprintf() */

#include "fmiwrapper.inc"


const char *RT_MEMORY_ALLOCATION_ERROR = "memory allocation error";

/* Path to the resources directory of the extracted FMU */
const char *FMU_RESOURCES_DIR = NULL;

typedef struct {
    RT_MDL_TYPE *S;
    const char *instanceName;
    fmi3LogMessageCallback logger;
    fmi3InstanceEnvironment componentEnvironment;
    ModelVariable modelVariables[N_MODEL_VARIABLES];
} ModelInstance;

static ModelInstance *s_instance = NULL;

#define ASSERT_INSTANCE \
    if (!instance || instance != s_instance) { \
        return fmi3Error; \
    }

#define NOT_IMPLEMENTED \
    logError("Function is not implemented."); \
	return fmi3Error;

#define CHECK_ERROR_STATUS \
	const char *errorStatus = rtmGetErrorStatus(s_instance->S); \
	if (errorStatus) { \
		logError(errorStatus); \
		return fmi3Error; \
	}

#define UNUSED(x) (void)(x)

int rtPrintfNoOp(const char *fmt, ...) {

    va_list args;
    va_start(args, fmt);

    if (s_instance && s_instance->logger) {
        char message[1024] = "";
        vsnprintf(message, 1024, fmt, args);
        s_instance->logger(s_instance->componentEnvironment, fmi3OK, "info", message);
    } else {
        vprintf(fmt, args);
    }

    va_end(args);

    return 0;
}

static void logError(const char *message) {
    if (s_instance && s_instance->logger) {
        s_instance->logger(s_instance->componentEnvironment, fmi3Error, "error", message);
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

const char* fmi3GetVersion() {
	return fmi3Version;
}

fmi3Status  fmi3SetDebugLogging(fmi3Instance instance,
	fmi3Boolean loggingOn,
	size_t nCategories,
	const fmi3String categories[]) {

    ASSERT_INSTANCE

    if (nCategories == 0) {
        return fmi3OK;
    }

    return fmi3Error;
}

/* Creation and destruction of FMU instances and setting debug status */
fmi3Instance fmi3InstantiateModelExchange(
    fmi3String                 instanceName,
    fmi3String                 instantiationToken,
    fmi3String                 resourcePath,
    fmi3Boolean                visible,
    fmi3Boolean                loggingOn,
    fmi3InstanceEnvironment    instanceEnvironment,
    fmi3LogMessageCallback     logMessage) {

	return NULL; // not supported
}

fmi3Instance fmi3InstantiateCoSimulation(
    fmi3String                     instanceName,
    fmi3String                     instantiationToken,
    fmi3String                     resourcePath,
    fmi3Boolean                    visible,
    fmi3Boolean                    loggingOn,
    fmi3Boolean                    eventModeUsed,
    fmi3Boolean                    earlyReturnAllowed,
    const fmi3ValueReference       requiredIntermediateVariables[],
    size_t                         nRequiredIntermediateVariables,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3LogMessageCallback         logMessage,
    fmi3IntermediateUpdateCallback intermediateUpdate) {

	/* check GUID */
	if (strcmp(instantiationToken, MODEL_GUID) != 0) {
		return NULL;
	}

    /* check if the FMU has already been instantiated */
    if (s_instance) {
        logError("The FMU can only be instantiated once per process.");
        return NULL;
    }

    /* set the path to the resources directory */
    if (!FMU_RESOURCES_DIR && resourcePath) {
        FMU_RESOURCES_DIR = strdup(resourcePath);
    }

	s_instance = malloc(sizeof(ModelInstance));

    s_instance->instanceName = strdup(instanceName);
    s_instance->logger = logMessage;
    s_instance->componentEnvironment = instanceEnvironment;

    s_instance->S = RT_MDL_INSTANCE;

	initializeModelVariables(s_instance->S, s_instance->modelVariables);

	return s_instance;
}

fmi3Instance fmi3InstantiateScheduledExecution(
    fmi3String                     instanceName,
    fmi3String                     instantiationToken,
    fmi3String                     resourcePath,
    fmi3Boolean                    visible,
    fmi3Boolean                    loggingOn,
    fmi3InstanceEnvironment        instanceEnvironment,
    fmi3LogMessageCallback         logMessage,
    fmi3ClockUpdateCallback        clockUpdate,
    fmi3LockPreemptionCallback     lockPreemption,
    fmi3UnlockPreemptionCallback   unlockPreemption) {

	return NULL; // not supported
}

void fmi3FreeInstance(fmi3Instance instance) {

    if (!instance || instance != s_instance) return;

    free((void *)s_instance->instanceName);
	free(s_instance);
    free((void *)FMU_RESOURCES_DIR);

    FMU_RESOURCES_DIR = NULL;
    s_instance = NULL;
}

fmi3Status fmi3EnterInitializationMode(fmi3Instance instance,
									   fmi3Boolean toleranceDefined,
									   fmi3Float64 tolerance,
									   fmi3Float64 startTime,
									   fmi3Boolean stopTimeDefined,
									   fmi3Float64 stopTime) {

    ASSERT_INSTANCE

    if (startTime != 0) {
        logError("startTime != 0.0 is not supported.");
        return fmi3Error;
    }

    if (stopTimeDefined && stopTime <= startTime) {
        logError("stopTime must be greater than startTime.");
        return fmi3Error;
    }

    MODEL_INITIALIZE();

    CHECK_ERROR_STATUS

	return fmi3OK;
}

fmi3Status fmi3ExitInitializationMode(fmi3Instance instance) {
    
    ASSERT_INSTANCE

    doFixedStep(s_instance->S);

    CHECK_ERROR_STATUS

	return fmi3OK;
}

fmi3Status fmi3EnterEventMode(fmi3Instance instance) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3Terminate(fmi3Instance instance) {

    ASSERT_INSTANCE

	MODEL_TERMINATE();

    s_instance->S = NULL;

	return fmi3OK;
}

fmi3Status fmi3Reset(fmi3Instance instance) {

    ASSERT_INSTANCE

    if (s_instance->S) {
        MODEL_TERMINATE();
    }

	return fmi3OK;
}


/* Getting and setting variable values */

static fmi3Status getVariables(ModelInstance *instance,
	const fmi3ValueReference vr[], size_t nvr,
	void *values, size_t nValues, BuiltInDTypeId datatypeID, size_t typeSize) {

	size_t i, j, index, copied = 0;
	ModelVariable v;

	for (i = 0; i < nvr; i++) {

        // independent variable
        if (vr[i] == 0) {
            
            if (datatypeID != SS_DOUBLE || typeSize != sizeof(REAL64_T)) {
                return fmi3Error;
            }
#ifdef rtmGetT
            *((fmi3Float64*)values) = rtmGetT(instance->S);
#else
            *((fmi3Float64*)values) = 0;
#endif
            values = (char *)values + sizeof(REAL64_T);
            continue;
        }

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

fmi3Status fmi3GetFloat32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float32 values[],
    size_t nValues) {

    ASSERT_INSTANCE

	return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_SINGLE, sizeof(REAL32_T));
}

fmi3Status fmi3GetFloat64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_DOUBLE, sizeof(REAL64_T));
}

fmi3Status fmi3GetInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int8 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_INT8, sizeof(INT8_T));
}

fmi3Status fmi3GetUInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt8 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_UINT8, sizeof(UINT8_T));
}

fmi3Status fmi3GetInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int16 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_INT16, sizeof(INT16_T));
}

fmi3Status fmi3GetUInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt16 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_UINT16, sizeof(UINT16_T));
}

fmi3Status fmi3GetInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int32 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_INT32, sizeof(INT32_T));
}

fmi3Status fmi3GetUInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt32 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_UINT32, sizeof(UINT32_T));
}

fmi3Status fmi3GetInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Int64 values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetUInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetBoolean(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Boolean values[],
    size_t nValues) {
    ASSERT_INSTANCE
	return getVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_BOOLEAN, sizeof(BOOLEAN_T));
}

fmi3Status fmi3GetString(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3String values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetBinary(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    size_t valueSizes[],
    fmi3Binary values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Clock values[]) {
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

fmi3Status fmi3SetFloat32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float32 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_SINGLE, sizeof(REAL32_T));
}

fmi3Status fmi3SetFloat64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float64 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_DOUBLE, sizeof(REAL64_T));
}

fmi3Status fmi3SetInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int8 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_INT8, sizeof(INT8_T));
}

fmi3Status fmi3SetUInt8(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt8 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_UINT8, sizeof(UINT8_T));
}

fmi3Status fmi3SetInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int16 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_INT16, sizeof(INT16_T));
}

fmi3Status fmi3SetUInt16(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt16 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_UINT16, sizeof(UINT16_T));
}

fmi3Status fmi3SetInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int32 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_INT32, sizeof(INT32_T));
}

fmi3Status fmi3SetUInt32(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt32 values[],
    size_t nValues) {

    ASSERT_INSTANCE

    return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_UINT32, sizeof(UINT32_T));
}

fmi3Status fmi3SetInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int64 values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetUInt64(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt64 values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetBoolean(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Boolean values[],
    size_t nValues) {
    ASSERT_INSTANCE
	return setVariables(s_instance, valueReferences, nValueReferences, values, nValues, SS_BOOLEAN, sizeof(BOOLEAN_T));
}

fmi3Status fmi3SetString(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3String values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetBinary(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const size_t valueSizes[],
    const fmi3Binary values[],
    size_t nValues) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetClock(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Clock values[]) {
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
    ASSERT_INSTANCE
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

fmi3Status fmi3DeserializeFMUState(fmi3Instance instance,
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
    ASSERT_INSTANCE
    return fmi3OK;
}

fmi3Status fmi3ExitConfigurationMode(fmi3Instance instance) {
    ASSERT_INSTANCE
    return fmi3OK;
}

fmi3Status fmi3GetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 intervals[],
    fmi3IntervalQualifier qualifiers[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 counters[],
    fmi3UInt64 resolutions[],
    fmi3IntervalQualifier qualifiers[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetShiftDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3Float64 shifts[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetShiftFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    fmi3UInt64 counters[],
    fmi3UInt64 resolutions[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetIntervalDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float64 intervals[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetIntervalFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt64 counters[],
    const fmi3UInt64 resolutions[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetShiftDecimal(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Float64 shifts[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetShiftFraction(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3UInt64 counters[],
    const fmi3UInt64 resolutions[]) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3EvaluateDiscreteStates(fmi3Instance instance) {
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

fmi3Status fmi3EnterContinuousTimeMode(fmi3Instance instance) { 
    NOT_IMPLEMENTED 
}

fmi3Status fmi3CompletedIntegratorStep(fmi3Instance instance,
	fmi3Boolean   noSetFMUStatePriorToCurrentPoint,
	fmi3Boolean*  enterEventMode,
	fmi3Boolean*  terminateSimulation) { 
    NOT_IMPLEMENTED 
}

/* Providing independent variables and re-initialization of caching */
fmi3Status fmi3SetTime(fmi3Instance instance, fmi3Float64 time) { 
    NOT_IMPLEMENTED
}

fmi3Status fmi3SetContinuousStates(fmi3Instance instance,
    const fmi3Float64 continuousStates[],
    size_t nContinuousStates) {
    NOT_IMPLEMENTED 
}

/* Evaluation of the model equations */
fmi3Status fmi3GetContinuousStateDerivatives(fmi3Instance instance,
    fmi3Float64 derivatives[],
    size_t nContinuousStates) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetEventIndicators(fmi3Instance instance,
    fmi3Float64 eventIndicators[],
    size_t nEventIndicators) {
    NOT_IMPLEMENTED 
}

fmi3Status fmi3GetContinuousStates(fmi3Instance instance,
    fmi3Float64 continuousStates[],
    size_t nContinuousStates) {
    NOT_IMPLEMENTED
}

fmi3Status fmi3GetNominalsOfContinuousStates(fmi3Instance instance,
    fmi3Float64 nominals[],
    size_t nContinuousStates) {
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

fmi3Status fmi3GetOutputDerivatives(fmi3Instance instance,
    const fmi3ValueReference valueReferences[],
    size_t nValueReferences,
    const fmi3Int32 orders[],
    fmi3Float64 values[],
    size_t nValues) {
    NOT_IMPLEMENTED 
}

fmi3Status fmi3DoStep(fmi3Instance instance,
    fmi3Float64 currentCommunicationPoint,
    fmi3Float64 communicationStepSize,
    fmi3Boolean noSetFMUStatePriorToCurrentPoint,
    fmi3Boolean* eventHandlingNeeded,
    fmi3Boolean* terminateSimulation,
    fmi3Boolean* earlyReturn,
    fmi3Float64* lastSuccessfulTime) {

    ASSERT_INSTANCE

    UNUSED(noSetFMUStatePriorToCurrentPoint);

    time_T tNext = currentCommunicationPoint + communicationStepSize;

#ifdef rtmGetT
    double epsilon = (1.0 + fabs(rtmGetT(s_instance->S))) * 2 * DBL_EPSILON;

    while (rtmGetT(s_instance->S) < tNext + epsilon)
#endif
    {
        doFixedStep(s_instance->S);

        CHECK_ERROR_STATUS
    }

    *eventHandlingNeeded = fmi3False;
    *terminateSimulation = fmi3False;
    *earlyReturn         = fmi3False;

#ifdef rtmGetT
    *lastSuccessfulTime = rtmGetT(s_instance->S);
#else
    *lastSuccessfulTime = tNext;
#endif

	return fmi3OK;
}

/***************************************************
Types for Functions for Scheduled Execution
****************************************************/

fmi3Status fmi3ActivateModelPartition(fmi3Instance instance,
    fmi3ValueReference clockReference,
    fmi3Float64 activationTime) {
    NOT_IMPLEMENTED 
}
