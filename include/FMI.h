#ifndef FMI_H
#define FMI_H

/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdbool.h>

#include "fmi1Functions.h"
#include "fmi2Functions.h"

#ifndef FMI_MAX_MESSAGE_LENGTH
#define FMI_MAX_MESSAGE_LENGTH 4096
#endif

typedef enum {
	FMIOK,
	FMIWarning,
	FMIDiscard,
	FMIError,
	FMIFatal,
	FMIPending
} FMIStatus;

typedef enum {
	FMIRealType,
	FMIIntegerType,
	FMIBooleanType,
	FMIStringType
} FMIVariableType;

typedef enum {
	FMIVersion1,
	FMIVersion2
} FMIVersion;

typedef enum {
	FMIModelExchange,
	FMICoSimulation
} FMIInterfaceType;

typedef enum {
	FMI2StartAndEndState = 1 << 0,
	FMI2InstantiatedState = 1 << 1,
	FMI2InitializationModeState = 1 << 2,

	// model exchange states
	FMI2EventModeState = 1 << 3,
	FMI2ContinuousTimeModeState = 1 << 4,

	// co-simulation states
	FMI2StepCompleteState = 1 << 5,
	FMI2StepInProgressState = 1 << 6,
	FMI2StepFailedState = 1 << 7,
	FMI2StepCanceledState = 1 << 8,

	FMI2TerminatedState = 1 << 9,
	FMI2ErrorState = 1 << 10,
	FMI2FatalState = 1 << 11,
} FMI2State;

typedef unsigned int FMIValueReference;

typedef struct FMIInstance_ FMIInstance;

typedef void FMILogFunctionCall(FMIInstance *instance, FMIStatus status, const char *message, ...);

typedef void FMILogMessage(FMIInstance *instance, FMIStatus status, const char *category, const char *message);

struct FMIInstance_ {

	/***************************************************
	 Common Functions for FMI 1.0
	****************************************************/

	fmi1CallbackFunctions functions1;
	fmi1EventInfo eventInfo1;

	fmi1SetRealTYPE         *fmi1SetReal;
	fmi1SetIntegerTYPE      *fmi1SetInteger;
	fmi1SetBooleanTYPE      *fmi1SetBoolean;
	fmi1SetStringTYPE       *fmi1SetString;
	fmi1GetRealTYPE         *fmi1GetReal;
	fmi1GetIntegerTYPE      *fmi1GetInteger;
	fmi1GetBooleanTYPE      *fmi1GetBoolean;
	fmi1GetStringTYPE       *fmi1GetString;
	fmi1SetDebugLoggingTYPE *fmi1SetDebugLogging;

	/***************************************************
	 FMI 1.0 for Model Exchange Functions
	****************************************************/

	fmi1GetModelTypesPlatformTYPE      *fmi1GetModelTypesPlatform;
	fmi1GetVersionTYPE                 *fmi1GetVersion;
	fmi1InstantiateModelTYPE           *fmi1InstantiateModel;
	fmi1FreeModelInstanceTYPE          *fmi1FreeModelInstance;
	fmi1SetTimeTYPE                    *fmi1SetTime;
	fmi1SetContinuousStatesTYPE        *fmi1SetContinuousStates;
	fmi1CompletedIntegratorStepTYPE    *fmi1CompletedIntegratorStep;
	fmi1InitializeTYPE                 *fmi1Initialize;
	fmi1GetDerivativesTYPE             *fmi1GetDerivatives;
	fmi1GetEventIndicatorsTYPE         *fmi1GetEventIndicators;
	fmi1EventUpdateTYPE                *fmi1EventUpdate;
	fmi1GetContinuousStatesTYPE        *fmi1GetContinuousStates;
	fmi1GetNominalContinuousStatesTYPE *fmi1GetNominalContinuousStates;
	fmi1GetStateValueReferencesTYPE    *fmi1GetStateValueReferences;
	fmi1TerminateTYPE                  *fmi1Terminate;

	/***************************************************
	 FMI 1.0 for Co-Simulation Functions
	****************************************************/
	fmi1GetTypesPlatformTYPE         *fmi1GetTypesPlatform;
	fmi1InstantiateSlaveTYPE         *fmi1InstantiateSlave;
	fmi1InitializeSlaveTYPE          *fmi1InitializeSlave;
	fmi1TerminateSlaveTYPE           *fmi1TerminateSlave;
	fmi1ResetSlaveTYPE               *fmi1ResetSlave;
	fmi1FreeSlaveInstanceTYPE        *fmi1FreeSlaveInstance;
	fmi1SetRealInputDerivativesTYPE  *fmi1SetRealInputDerivatives;
	fmi1GetRealOutputDerivativesTYPE *fmi1GetRealOutputDerivatives;
	fmi1CancelStepTYPE               *fmi1CancelStep;
	fmi1DoStepTYPE                   *fmi1DoStep;
	fmi1GetStatusTYPE                *fmi1GetStatus;
	fmi1GetRealStatusTYPE            *fmi1GetRealStatus;
	fmi1GetIntegerStatusTYPE         *fmi1GetIntegerStatus;
	fmi1GetBooleanStatusTYPE         *fmi1GetBooleanStatus;
	fmi1GetStringStatusTYPE          *fmi1GetStringStatus;

	/***************************************************
	Common Functions for FMI 2.0
	****************************************************/

	fmi2CallbackFunctions functions2;
	fmi2EventInfo eventInfo2;

	/* required functions */
	fmi2GetTypesPlatformTYPE         *fmi2GetTypesPlatform;
	fmi2GetVersionTYPE               *fmi2GetVersion;
	fmi2SetDebugLoggingTYPE          *fmi2SetDebugLogging;
	fmi2InstantiateTYPE              *fmi2Instantiate;
	fmi2FreeInstanceTYPE             *fmi2FreeInstance;
	fmi2SetupExperimentTYPE          *fmi2SetupExperiment;
	fmi2EnterInitializationModeTYPE  *fmi2EnterInitializationMode;
	fmi2ExitInitializationModeTYPE   *fmi2ExitInitializationMode;
	fmi2TerminateTYPE                *fmi2Terminate;
	fmi2ResetTYPE                    *fmi2Reset;
	fmi2GetRealTYPE                  *fmi2GetReal;
	fmi2GetIntegerTYPE               *fmi2GetInteger;
	fmi2GetBooleanTYPE               *fmi2GetBoolean;
	fmi2GetStringTYPE                *fmi2GetString;
	fmi2SetRealTYPE                  *fmi2SetReal;
	fmi2SetIntegerTYPE               *fmi2SetInteger;
	fmi2SetBooleanTYPE               *fmi2SetBoolean;
	fmi2SetStringTYPE                *fmi2SetString;

	/* optional functions */
	fmi2GetFMUstateTYPE              *fmi2GetFMUstate;
	fmi2SetFMUstateTYPE              *fmi2SetFMUstate;
	fmi2FreeFMUstateTYPE             *fmi2FreeFMUstate;
	fmi2SerializedFMUstateSizeTYPE   *fmi2SerializedFMUstateSize;
	fmi2SerializeFMUstateTYPE        *fmi2SerializeFMUstate;
	fmi2DeSerializeFMUstateTYPE      *fmi2DeSerializeFMUstate;
	fmi2GetDirectionalDerivativeTYPE *fmi2GetDirectionalDerivative;

	/***************************************************
	Functions for FMI 2.0 for Model Exchange
	****************************************************/

	fmi2EnterEventModeTYPE                *fmi2EnterEventMode;
	fmi2NewDiscreteStatesTYPE             *fmi2NewDiscreteStates;
	fmi2EnterContinuousTimeModeTYPE       *fmi2EnterContinuousTimeMode;
	fmi2CompletedIntegratorStepTYPE       *fmi2CompletedIntegratorStep;
	fmi2SetTimeTYPE                       *fmi2SetTime;
	fmi2SetContinuousStatesTYPE           *fmi2SetContinuousStates;
	fmi2GetDerivativesTYPE                *fmi2GetDerivatives;
	fmi2GetEventIndicatorsTYPE            *fmi2GetEventIndicators;
	fmi2GetContinuousStatesTYPE           *fmi2GetContinuousStates;
	fmi2GetNominalsOfContinuousStatesTYPE *fmi2GetNominalsOfContinuousStates;

	/***************************************************
	Functions for FMI 2.0 for Co-Simulation
	****************************************************/

	fmi2SetRealInputDerivativesTYPE  *fmi2SetRealInputDerivatives;
	fmi2GetRealOutputDerivativesTYPE *fmi2GetRealOutputDerivatives;
	fmi2DoStepTYPE                   *fmi2DoStep;
	fmi2CancelStepTYPE               *fmi2CancelStep;
	fmi2GetStatusTYPE                *fmi2GetStatus;
	fmi2GetRealStatusTYPE            *fmi2GetRealStatus;
	fmi2GetIntegerStatusTYPE         *fmi2GetIntegerStatus;
	fmi2GetBooleanStatusTYPE         *fmi2GetBooleanStatus;
	fmi2GetStringStatusTYPE			 *fmi2GetStringStatus;

#ifdef _WIN32
	HMODULE libraryHandle;
#else
	void *libraryHandle;
#endif

	void *userData;

	FMILogMessage      *logMessage;
	FMILogFunctionCall *logFunctionCall;

	fmi2Real time;

	char *buf1;
	char *buf2;

	size_t bufsize1;
	size_t bufsize2;

	void *component;

	fmi2String name;

	bool logFMICalls;

	FMI2State state;

	FMIStatus status;

	FMIVersion fmiVersion;

	FMIInterfaceType interfaceType;

};

FMIInstance *FMICreateInstance(const char *instanceName, const char *libraryPath, FMILogMessage *logMessage, FMILogFunctionCall *logFunctionCall);

void FMIFreeInstance(FMIInstance *instance);

const char* FMIValueReferencesToString(FMIInstance *instance, const FMIValueReference vr[], size_t nvr);

const char* FMIValuesToString(FMIInstance *instance, size_t nvr, const void *value, FMIVariableType variableType);

#endif // FMI_H
