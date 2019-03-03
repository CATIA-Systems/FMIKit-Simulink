/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

 /*
-----------------------------------------------------------
	Header file for declarations of generic
	FMI 2.0 CS implementation.
-----------------------------------------------------------
*/

#ifndef FMI2_FUNCTIONS__H
#define FMI2_FUNCTIONS__H

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
/* Co-Simulation functions */
fmi2Status fmi2SetRealInputDerivatives_(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], const fmi2Real value[]);
fmi2Status fmi2GetRealOutputDerivatives_(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer order[], fmi2Real value[]);

fmi2Status fmi2DoStep_     (fmi2Component c, fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint);
fmi2Status fmi2CancelStep_ (fmi2Component c);

fmi2Status fmi2GetStatus_       (fmi2Component c, const fmi2StatusKind s, fmi2Status* value);
fmi2Status fmi2GetRealStatus_   (fmi2Component c, const fmi2StatusKind s, fmi2Real* value);
fmi2Status fmi2GetIntegerStatus_(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value);
fmi2Status fmi2GetBooleanStatus_(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value);
fmi2Status fmi2GetStringStatus_ (fmi2Component c, const fmi2StatusKind s, fmi2String* value);


#endif
