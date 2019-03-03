/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

/*
-----------------------------------------------------------
	Header file for declarations of generic
	FMI 2.0 ME implementation.
-----------------------------------------------------------
*/

#ifndef FMI2_MODEL_FUNCTIONS__H
#define FMI2_MODEL_FUNCTIONS__H

/* Common functions */
const char* fmi2GetTypesPlatform_();
const char* fmi2GetVersion_();
fmi2Status fmi2SetDebugLogging_(fmi2Component c, fmi2Boolean loggingOn, size_t nCategories, const fmi2String categories[]);

fmi2Component  fmi2Instantiate_(fmi2String	instanceName,
								fmi2Type	fmuType,
								fmi2String	GUID,
								fmi2String	fmuResourceLocation,
								const fmi2CallbackFunctions* functions,
								fmi2Boolean	visible,
								fmi2Boolean	loggingOn);
void      fmi2FreeInstance_(fmi2Component c);

fmi2Status fmi2SetupExperiment_        (fmi2Component c,
										fmi2Boolean toleranceDefined,
										fmi2Real tolerance,
										fmi2Real startTime,
										fmi2Boolean stopTimeDefined,
										fmi2Real stopTime);
fmi2Status fmi2EnterInitializationMode_(fmi2Component c);
fmi2Status fmi2ExitInitializationMode_ (fmi2Component c);
fmi2Status fmi2Terminate_              (fmi2Component c);
fmi2Status fmi2Reset_                  (fmi2Component c);

fmi2Status fmi2GetReal_   (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real    value[]);
fmi2Status fmi2GetInteger_(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]);
fmi2Status fmi2GetBoolean_(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]);
fmi2Status fmi2GetString_ (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[]);

fmi2Status fmi2SetReal_   (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real    value[]);
fmi2Status fmi2SetInteger_(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]);
fmi2Status fmi2SetBoolean_(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]);
fmi2Status fmi2SetString_ (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[]);

fmi2Status fmi2GetFMUstate_           (fmi2Component c, fmi2FMUstate* FMUstate);
fmi2Status fmi2SetFMUstate_           (fmi2Component c, fmi2FMUstate FMUstate);
fmi2Status fmi2FreeFMUstate_          (fmi2Component c, fmi2FMUstate* FMUstate);
fmi2Status fmi2SerializedFMUstateSize_(fmi2Component c, fmi2FMUstate FMUstate, size_t* size);
fmi2Status fmi2SerializeFMUstate_     (fmi2Component c, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size);
fmi2Status fmi2DeSerializeFMUstate_   (fmi2Component c, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate);

fmi2Status fmi2GetDirectionalDerivative_(fmi2Component c, const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
                                                         const fmi2ValueReference vKnown_ref[], size_t nKnown,
                                                         const fmi2Real dvKnown[],
														       fmi2Real dvUnknown[]);
/* Model Exchange functions */
fmi2Status fmi2EnterEventMode_         (fmi2Component c);
fmi2Status fmi2NewDiscreteStates_      (fmi2Component c, fmi2EventInfo* eventInfo);
fmi2Status fmi2EnterContinuousTimeMode_(fmi2Component c);
fmi2Status fmi2CompletedIntegratorStep_(fmi2Component c, fmi2Boolean noSetFMUStatePriorToCurrentPoint,
										fmi2Boolean* enterEventMode, fmi2Boolean* terminateSimulation);

fmi2Status fmi2SetTime_            (fmi2Component c, fmi2Real time);
fmi2Status fmi2SetContinuousStates_(fmi2Component c, const fmi2Real x[], size_t nx);

fmi2Status fmi2GetDerivatives_               (fmi2Component c, fmi2Real derivatives[], size_t nx);
fmi2Status fmi2GetEventIndicators_           (fmi2Component c, fmi2Real eventIndicators[], size_t ni);
fmi2Status fmi2GetContinuousStates_          (fmi2Component c, fmi2Real x[], size_t nx);
fmi2Status fmi2GetNominalsOfContinuousStates_(fmi2Component c, fmi2Real x_nominal[], size_t nx);

#endif
