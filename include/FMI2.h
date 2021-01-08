#ifndef FMI2_H
#define FMI2_H

/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "FMI.h"

#include <stdbool.h>
#include "fmi1Functions.h"
#include "fmi2Functions.h"

#ifdef _WIN32
#include <Windows.h>

static const char * exceptionCodeToString(DWORD exceptionCode) {
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
	case STATUS_HEAP_CORRUPTION:			return "HEAP_CORRUPTION";
	case STATUS_STACK_BUFFER_OVERRUN:		return "STACK_BUFFER_OVERRUN";
	case STATUS_INVALID_CRUNTIME_PARAMETER: return "INVALID_CRUNTIME_PARAMETER";
	case STATUS_ASSERTION_FAILURE:			return "ASSERTION_FAILURE";
	case STATUS_SXS_EARLY_DEACTIVATION:		return "SXS_EARLY_DEACTIVATION";
	case STATUS_SXS_INVALID_DEACTIVATION:	return "SXS_INVALID_DEACTIVATION";
	default:								return "UNKOWN_EXEPTION_CODE";
	}
}
#endif

typedef enum {
	FMI2RealType,
	FMI2IntegerType,
	FMI2BooleanType,
	FMI2StringType
} FMI2VariableType;

/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers of header files and setting logging status */
const char* FMI2GetTypesPlatform(FMIInstance *instance);

const char* FMI2GetVersion(FMIInstance *instance);

fmi2Status FMI2SetDebugLogging(FMIInstance *instance, fmi2Boolean loggingOn, size_t nCategories, const fmi2String categories[]);

fmi2Status FMI2Instantiate(FMIInstance *instance, const char *fmuResourceLocation, fmi2Type fmuType, fmi2String fmuGUID,
	fmi2Boolean visible, fmi2Boolean loggingOn);

void FMI2FreeInstance(FMIInstance *instance);

/* Enter and exit initialization mode, terminate and reset */
fmi2Status FMI2SetupExperiment(FMIInstance *instance,
	fmi2Boolean toleranceDefined,
	fmi2Real tolerance,
	fmi2Real startTime,
	fmi2Boolean stopTimeDefined,
	fmi2Real stopTime);

fmi2Status FMI2EnterInitializationMode(FMIInstance *instance);

fmi2Status FMI2ExitInitializationMode(FMIInstance *instance);

fmi2Status FMI2Terminate(FMIInstance *instance);

fmi2Status FMI2Reset(FMIInstance *instance);

fmi2Status FMI2SetupExperiment(FMIInstance *instance,
	fmi2Boolean toleranceDefined,
	fmi2Real tolerance,
	fmi2Real startTime,
	fmi2Boolean stopTimeDefined,
	fmi2Real stopTime);

/* Getting and setting variable values */
fmi2Status FMI2GetReal(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]);

fmi2Status FMI2GetInteger(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]);

fmi2Status FMI2GetBoolean(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]);

fmi2Status FMI2GetString(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[]);

fmi2Status FMI2SetReal(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, const fmi2Real    value[]);

fmi2Status FMI2SetInteger(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]);

fmi2Status FMI2SetBoolean(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]);

fmi2Status FMI2SetString(FMIInstance *instance, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[]);

/* Getting and setting the internal FMU state */
fmi2Status FMI2GetFMUstate(FMIInstance *instance, fmi2FMUstate* FMUstate);

fmi2Status FMI2SetFMUstate(FMIInstance *instance, fmi2FMUstate  FMUstate);

fmi2Status FMI2FreeFMUstate(FMIInstance *instance, fmi2FMUstate* FMUstate);

fmi2Status FMI2SerializedFMUstateSize(FMIInstance *instance, fmi2FMUstate  FMUstate, size_t* size);

fmi2Status FMI2SerializeFMUstate(FMIInstance *instance, fmi2FMUstate  FMUstate, fmi2Byte serializedState[], size_t size);

fmi2Status FMI2DeSerializeFMUstate(FMIInstance *instance, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate);

/* Getting partial derivatives */
fmi2Status FMI2GetDirectionalDerivative(FMIInstance *instance,
	const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
	const fmi2ValueReference vKnown_ref[], size_t nKnown,
	const fmi2Real dvKnown[],
	fmi2Real dvUnknown[]);

/***************************************************
Types for Functions for FMI2 for Model Exchange
****************************************************/

/* Enter and exit the different modes */
fmi2Status FMI2EnterEventMode(FMIInstance *instance);

fmi2Status FMI2NewDiscreteStates(FMIInstance *instance, fmi2EventInfo *eventInfo);

fmi2Status FMI2EnterContinuousTimeMode(FMIInstance *instance);

fmi2Status FMI2CompletedIntegratorStep(FMIInstance *instance,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint,
	fmi2Boolean*  enterEventMode,
	fmi2Boolean*  terminateSimulation);

/* Providing independent variables and re-initialization of caching */
fmi2Status FMI2SetTime(FMIInstance *instance, fmi2Real time);

fmi2Status FMI2SetContinuousStates(FMIInstance *instance, const fmi2Real x[], size_t nx);

/* Evaluation of the model equations */
fmi2Status FMI2GetDerivatives(FMIInstance *instance, fmi2Real derivatives[], size_t nx);

fmi2Status FMI2GetEventIndicators(FMIInstance *instance, fmi2Real eventIndicators[], size_t ni);

fmi2Status FMI2GetContinuousStates(FMIInstance *instance, fmi2Real x[], size_t nx);

fmi2Status FMI2GetNominalsOfContinuousStates(FMIInstance *instance, fmi2Real x_nominal[], size_t nx);


/***************************************************
Types for Functions for FMI2 for Co-Simulation
****************************************************/

/* Simulating the slave */
fmi2Status FMI2SetRealInputDerivatives(FMIInstance *instance,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	const fmi2Real value[]);

fmi2Status FMI2GetRealOutputDerivatives(FMIInstance *instance,
	const fmi2ValueReference vr[], size_t nvr,
	const fmi2Integer order[],
	fmi2Real value[]);

fmi2Status FMI2DoStep(FMIInstance *instance,
	fmi2Real      currentCommunicationPoint,
	fmi2Real      communicationStepSize,
	fmi2Boolean   noSetFMUStatePriorToCurrentPoint);

fmi2Status FMI2CancelStep(FMIInstance *instance);

/* Inquire slave status */
fmi2Status FMI2GetStatus(FMIInstance *instance, const fmi2StatusKind s, fmi2Status* value);

fmi2Status FMI2GetRealStatus(FMIInstance *instance, const fmi2StatusKind s, fmi2Real* value);

fmi2Status FMI2GetIntegerStatus(FMIInstance *instance, const fmi2StatusKind s, fmi2Integer* value);

fmi2Status FMI2GetBooleanStatus(FMIInstance *instance, const fmi2StatusKind s, fmi2Boolean* value);

fmi2Status FMI2GetStringStatus(FMIInstance *instance, const fmi2StatusKind s, fmi2String*  value);

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif // FMI2_H
