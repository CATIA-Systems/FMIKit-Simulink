/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

/*
-----------------------------------------------------------
	Header file for declarations of generic
	FMI 1.0 CS implementation.
-----------------------------------------------------------
*/

#ifndef FMI1_FUNCTIONS__H
#define FMI1_FUNCTIONS__H

const char* fmiGetTypesPlatform_();
const char* fmiGetVersion_();

fmiStatus fmiSetDebugLogging_(fmiComponent m, fmiBoolean loggingOn);

fmiStatus fmiSetReal_   (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal    value[]);
fmiStatus fmiSetInteger_(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
fmiStatus fmiSetBoolean_(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
fmiStatus fmiSetString_ (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString  value[]);

fmiStatus fmiGetReal_   (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal    value[]);
fmiStatus fmiGetInteger_(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
fmiStatus fmiGetBoolean_(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
fmiStatus fmiGetString_ (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[]);

fmiComponent fmiInstantiateSlave_(fmiString  instanceName,
                                  fmiString  fmuGUID,
                                  fmiString  fmuLocation,
                                  fmiString  mimeType,
                                  fmiReal    timeout,
                                  fmiBoolean visible,
                                  fmiBoolean interactive,
                                  fmiCallbackFunctions functions,
                                  fmiBoolean loggingOn);
void      fmiFreeSlaveInstance_(fmiComponent c);

fmiStatus fmiInitializeSlave_(fmiComponent c,
                              fmiReal      tStart,
                              fmiBoolean   StopTimeDefined,
                              fmiReal      tStop);

fmiStatus fmiTerminateSlave_   (fmiComponent c);
fmiStatus fmiResetSlave_	   (fmiComponent c);

fmiStatus fmiSetRealInputDerivatives_(fmiComponent c,
                                      const  fmiValueReference vr[],
                                      size_t nvr,
                                      const  fmiInteger order[],
                                      const  fmiReal value[]);

fmiStatus fmiGetRealOutputDerivatives_(fmiComponent c,
                                       const   fmiValueReference vr[],
                                       size_t  nvr,
                                       const   fmiInteger order[],
                                       fmiReal value[]);

fmiStatus fmiCancelStep_(fmiComponent c);
fmiStatus fmiDoStep_    (fmiComponent c,
                         fmiReal      currentCommunicationPoint,
                         fmiReal      communicationStepSize,
                         fmiBoolean   newStep);

fmiStatus fmiGetStatus_       (fmiComponent c, const fmiStatusKind s, fmiStatus*  value);
fmiStatus fmiGetRealStatus_   (fmiComponent c, const fmiStatusKind s, fmiReal*    value);
fmiStatus fmiGetIntegerStatus_(fmiComponent c, const fmiStatusKind s, fmiInteger* value);
fmiStatus fmiGetBooleanStatus_(fmiComponent c, const fmiStatusKind s, fmiBoolean* value);
fmiStatus fmiGetStringStatus_ (fmiComponent c, const fmiStatusKind s, fmiString*  value);

#endif
