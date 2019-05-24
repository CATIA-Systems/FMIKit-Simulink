#include "fmi2Functions.h"

#include "fmiwrapper.inc"

const char *RT_MEMORY_ALLOCATION_ERROR = "memory allocation error";

int rtPrintfNoOp(const char *fmt, ...) {
	return 0;  // do nothing
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
	const fmi2String categories[]) { return fmi2Error; }

/* Creation and destruction of FMU instances and setting debug status */
fmi2Component fmi2Instantiate(fmi2String instanceName,
	fmi2Type fmuType,
	fmi2String fmuGUID,
	fmi2String fmuResourceLocation,
	const fmi2CallbackFunctions* functions,
	fmi2Boolean visible,
	fmi2Boolean loggingOn) {

	RT_MDL_TYPE **c = malloc(sizeof(RT_MDL_TYPE *));

#ifdef REUSABLE_FUNCTION
	RT_MDL_TYPE *S = MODEL();

	const char_T *errmsg = rt_StartDataLogging(rtmGetRTWLogInfo(S),
		rtmGetTFinal(S),
		rtmGetStepSize(S),
		&rtmGetErrorStatus(S));

	MODEL_INITIALIZE(S);

	*c = S;
#else
	MODEL_INITIALIZE();

	*c = RT_MDL_INSTANCE;
#endif

	return c;
}

void fmi2FreeInstance(fmi2Component c) {
	free(c);
}

/* Enter and exit initialization mode, terminate and reset */
fmi2Status fmi2SetupExperiment(fmi2Component c,
	fmi2Boolean toleranceDefined,
	fmi2Real tolerance,
	fmi2Real startTime,
	fmi2Boolean stopTimeDefined,
	fmi2Real stopTime) {
	return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {
	return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {
	return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component c) {
#ifdef REUSABLE_FUNCTION
	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	MODEL_TERMINATE(S);
#else
	MODEL_TERMINATE();
#endif

	*(RT_MDL_TYPE **)c = NULL;

	return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component c) {

#ifdef REUSABLE_FUNCTION
	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	
	MODEL_TERMINATE(S);

	S = MODEL();

	const char_T *errmsg = rt_StartDataLogging(rtmGetRTWLogInfo(S),
		rtmGetTFinal(S),
		rtmGetStepSize(S),
		&rtmGetErrorStatus(S));

	MODEL_INITIALIZE(S);

	*(RT_MDL_TYPE **)c = S;
#else
	MODEL_TERMINATE();
	MODEL_INITIALIZE();
#endif

	return fmi2OK;
}

/* Getting and setting variable values */
fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {

	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	BuiltInDTypeId dtypeID = -1;

	for (size_t i = 0; i < nvr; i++) {

		void *vptr = getScalarVariable(S, vr[i], &dtypeID);

		switch (dtypeID) {
		case SS_DOUBLE:
			value[i] = *(REAL64_T *)vptr;
			break;
		case SS_SINGLE:
			value[i] = *(REAL32_T *)vptr;
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]) {

	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	BuiltInDTypeId dtypeID = -1;

	for (size_t i = 0; i < nvr; i++) {
		void *vptr = getScalarVariable(S, vr[i], &dtypeID);

		switch (dtypeID) {
		case SS_INT8:
			value[i] = *(INT8_T *)vptr;
			break;
		case SS_UINT8:
			value[i] = *(UINT8_T *)vptr;
			break;
		case SS_INT16:
			value[i] = *(INT16_T *)vptr;
			break;
		case SS_UINT16:
			value[i] = *(UINT16_T *)vptr;
			break;
		case SS_INT32:
			value[i] = *(INT32_T *)vptr;
			break;
		case SS_UINT32:
			value[i] = *(UINT32_T *)vptr;
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]) {

	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	BuiltInDTypeId dtypeID = -1;

	for (size_t i = 0; i < nvr; i++) {
		void *vptr = getScalarVariable(S, vr[i], &dtypeID);

		switch (dtypeID) {
		case SS_BOOLEAN:
			value[i] = *(BOOLEAN_T *)vptr;
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[]) { return fmi2Error; }

fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]) { 

	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	BuiltInDTypeId dtypeID = -1;

	for (size_t i = 0; i < nvr; i++) {
		void *vptr = getScalarVariable(S, vr[i], &dtypeID);

		switch (dtypeID) {
		case SS_DOUBLE:
			*((REAL64_T *)vptr) = value[i];
			break;
		case SS_SINGLE:
			*((REAL32_T *)vptr) = value[i];
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]) {

	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	BuiltInDTypeId dtypeID = -1;

	for (size_t i = 0; i < nvr; i++) {
		void *vptr = getScalarVariable(S, vr[i], &dtypeID);

		switch (dtypeID) {
		case SS_INT8:
			*((INT8_T *)vptr) = value[i];
			break;
		case SS_UINT8:
			*((UINT8_T *)vptr) = value[i];
			break;
		case SS_INT16:
			*((INT16_T *)vptr) = value[i];
			break;
		case SS_UINT16:
			*((UINT16_T *)vptr) = value[i];
			break;
		case SS_INT32:
			*((INT32_T *)vptr) = value[i];
			break;
		case SS_UINT32:
			*((UINT32_T *)vptr) = value[i];
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]) {

	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;
	BuiltInDTypeId dtypeID = -1;

	for (size_t i = 0; i < nvr; i++) {
		void *vptr = getScalarVariable(S, vr[i], &dtypeID);

		switch (dtypeID) {
		case SS_BOOLEAN:
			*((BOOLEAN_T *)vptr) = value[i];
			break;
		default:
			return fmi2Error;
		}
	}

	return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[]) { return fmi2Error; }

/* Getting and setting the internal FMU state */
fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) { return fmi2Error; }
fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate  FMUstate) { return fmi2Error; }
fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) { return fmi2Error; }
fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate  FMUstate, size_t* size) { return fmi2Error; }
fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate  FMUstate, fmi2Byte serializedState[], size_t size) { return fmi2Error; }
fmi2Status fmi2DeSerializeFMUstate(fmi2Component c, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate) { return fmi2Error; }

/* Getting partial derivatives */
fmi2Status fmi2GetDirectionalDerivative(fmi2Component c,
	const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
	const fmi2ValueReference vKnown_ref[], size_t nKnown,
	const fmi2Real dvKnown[],
	fmi2Real dvUnknown[]) { return fmi2Error; }

/***************************************************
Types for Functions for FMI2 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component c) { return fmi2Error; }
fmi2Status fmi2NewDiscreteStates(fmi2Component c, fmi2EventInfo* fmi2eventInfo) { return fmi2Error; }
fmi2Status fmi2EnterContinuousTimeMode(fmi2Component c) { return fmi2Error; }
fmi2Status fmi2CompletedIntegratorStep(fmi2Component c,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint,
	fmi2Boolean*  enterEventMode,
	fmi2Boolean*  terminateSimulation) { return fmi2Error; }

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time) { return fmi2Error; }
fmi2Status fmi2SetContinuousStates(fmi2Component c, const fmi2Real x[], size_t nx) { return fmi2Error; }

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component c, fmi2Real derivatives[], size_t nx) { return fmi2Error; }
fmi2Status fmi2GetEventIndicators(fmi2Component c, fmi2Real eventIndicators[], size_t ni) { return fmi2Error; }
fmi2Status fmi2GetContinuousStates(fmi2Component c, fmi2Real x[], size_t nx) { return fmi2Error; }
fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component c, fmi2Real x_nominal[], size_t nx) { return fmi2Error; }


/***************************************************
Types for Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	const fmi2Real value[]) { return fmi2Error; }

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	fmi2Real value[]) { return fmi2Error; }

fmi2Status fmi2DoStep(fmi2Component c,
	fmi2Real      currentCommunicationPoint,
	fmi2Real      communicationStepSize,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint) {

	RT_MDL_TYPE *S = *(RT_MDL_TYPE **)c;

	time_T tNext = currentCommunicationPoint + communicationStepSize;

	while (rtmGetT(S) + rtmGetStepSize(S) < tNext + DBL_EPSILON) {
		MODEL_STEP(S);
	}
	
	return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component c) { return fmi2Error; }

/* Inquire slave status */
fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status*  value) { return fmi2Error; }
fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real*    value) { return fmi2Error; }
fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value) { return fmi2Error; }
fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value) { return fmi2Error; }
fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String*  value) { return fmi2Error; }
