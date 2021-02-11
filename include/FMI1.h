#ifndef FMI1_H
#define FMI1_H

/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#ifdef _WIN32
#include <Windows.h>
#endif

#include "FMI2.h"


/***************************************************
 Common Functions for FMI 1.0
****************************************************/
fmi1Status    FMI1SetReal         (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr, const fmi1Real    value[]);
fmi1Status    FMI1SetInteger      (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr, const fmi1Integer value[]);
fmi1Status    FMI1SetBoolean      (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr, const fmi1Boolean value[]);
fmi1Status    FMI1SetString       (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr, const fmi1String  value[]);
fmi1Status    FMI1GetReal         (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr,       fmi1Real    value[]);
fmi1Status    FMI1GetInteger      (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr,       fmi1Integer value[]);
fmi1Status    FMI1GetBoolean      (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr,       fmi1Boolean value[]);
fmi1Status    FMI1GetString       (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr,       fmi1String  value[]);
fmi1Status    FMI1SetDebugLogging (FMIInstance *instance, fmi1Boolean loggingOn);

/***************************************************
 FMI 1.0 for Model Exchange Functions
****************************************************/
const char*   FMI1GetModelTypesPlatform      (FMIInstance *instance);
const char*   FMI1GetVersion                 (FMIInstance *instance);
fmi1Status    FMI1InstantiateModel           (FMIInstance *instance, fmi1String modelIdentifier, fmi1String GUID, fmi1Boolean loggingOn);
void          FMI1FreeModelInstance          (FMIInstance *instance);
fmi1Status    FMI1SetTime                    (FMIInstance *instance, fmi1Real time);
fmi1Status    FMI1SetContinuousStates        (FMIInstance *instance, const fmi1Real x[], size_t nx);
fmi1Status    FMI1CompletedIntegratorStep    (FMIInstance *instance, fmi1Boolean* callEventUpdate);
fmi1Status    FMI1Initialize                 (FMIInstance *instance, fmi1Boolean toleranceControlled, fmi1Real relativeTolerance);
fmi1Status    FMI1GetDerivatives             (FMIInstance *instance, fmi1Real derivatives[], size_t nx);
fmi1Status    FMI1GetEventIndicators         (FMIInstance *instance, fmi1Real eventIndicators[], size_t ni);
fmi1Status    FMI1EventUpdate                (FMIInstance *instance, fmi1Boolean intermediateResults, fmi1EventInfo* eventInfo);
fmi1Status    FMI1GetContinuousStates        (FMIInstance *instance, fmi1Real states[], size_t nx);
fmi1Status    FMI1GetNominalContinuousStates (FMIInstance *instance, fmi1Real x_nominal[], size_t nx);
fmi1Status    FMI1GetStateValueReferences    (FMIInstance *instance, fmi1ValueReference vrx[], size_t nx);
fmi1Status    FMI1Terminate                  (FMIInstance *instance);

/***************************************************
 FMI 1.0 for Co-Simulation Functions
****************************************************/
const char*   FMI1GetTypesPlatform         (FMIInstance *instance);
fmi1Status    FMI1InstantiateSlave(FMIInstance *instance, fmi1String modelIdentifier, fmi1String fmuGUID, fmi1String fmuLocation, fmi1String  mimeType, fmi1Real timeout, fmi1Boolean visible, fmi1Boolean interactive, fmi1Boolean loggingOn);
fmi1Status    FMI1InitializeSlave(FMIInstance *instance, fmi1Real tStart, fmi1Boolean StopTimeDefined, fmi1Real tStop);
fmi1Status    FMI1TerminateSlave           (FMIInstance *instance);
fmi1Status    FMI1ResetSlave               (FMIInstance *instance);
void          FMI1FreeSlaveInstance        (FMIInstance *instance);
fmi1Status    FMI1SetRealInputDerivatives  (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr, const fmi1Integer order[], const fmi1Real value[]);
fmi1Status    FMI1GetRealOutputDerivatives (FMIInstance *instance, const fmi1ValueReference vr[], size_t nvr, const fmi1Integer order[],       fmi1Real value[]);
fmi1Status    FMI1CancelStep               (FMIInstance *instance);
fmi1Status    FMI1DoStep                   (FMIInstance *instance, fmi1Real currentCommunicationPoint, fmi1Real communicationStepSize, fmi1Boolean newStep);
fmi1Status    FMI1GetStatus                (FMIInstance *instance, const fmi1StatusKind s, fmi1Status*  value);
fmi1Status    FMI1GetRealStatus            (FMIInstance *instance, const fmi1StatusKind s, fmi1Real*    value);
fmi1Status    FMI1GetIntegerStatus         (FMIInstance *instance, const fmi1StatusKind s, fmi1Integer* value);
fmi1Status    FMI1GetBooleanStatus         (FMIInstance *instance, const fmi1StatusKind s, fmi1Boolean* value);
fmi1Status    FMI1GetStringStatus          (FMIInstance *instance, const fmi1StatusKind s, fmi1String*  value);

#endif // FMI1_H
