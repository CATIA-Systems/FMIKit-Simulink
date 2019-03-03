/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

/*
-----------------------------------------------------------
	Header file for declarations of generic
	FMI 1.0 ME implementation.
-----------------------------------------------------------
*/

#ifndef FMI1_MODEL_FUNCTIONS__H
#define FMI1_MODEL_FUNCTIONS__H

const char* fmiGetModelTypesPlatform_();
const char* fmiGetVersion_();

fmiStatus fmiSetDebugLogging_(fmiComponent c, fmiBoolean loggingOn);

fmiStatus fmiSetReal_                (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal    value[]);
fmiStatus fmiSetInteger_             (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[]);
fmiStatus fmiSetBoolean_             (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[]);
fmiStatus fmiSetString_              (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString  value[]);

fmiStatus fmiGetReal_   (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal    value[]);
fmiStatus fmiGetInteger_(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[]);
fmiStatus fmiGetBoolean_(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[]);
fmiStatus fmiGetString_ (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[]);

fmiComponent  fmiInstantiateModel_(fmiString            instanceName,
								   fmiString            GUID,
								   fmiBoolean           loggingOn,
								   fmiCallbackFunctions functions);
void      fmiFreeModelInstance_(fmiComponent c);

fmiStatus fmiInitialize_(fmiComponent c, fmiBoolean toleranceControlled, fmiReal relativeTolerance,
						 fmiEventInfo* eventInfo);

fmiStatus fmiTerminate_                 (fmiComponent c);

fmiStatus fmiSetTime_                (fmiComponent c, fmiReal time);
fmiStatus fmiSetContinuousStates_    (fmiComponent c, const fmiReal x[], size_t nx);
fmiStatus fmiCompletedIntegratorStep_(fmiComponent c, fmiBoolean* callEventUpdate);

fmiStatus fmiGetDerivatives_    (fmiComponent c, fmiReal derivatives[]    , size_t nx);
fmiStatus fmiGetEventIndicators_(fmiComponent c, fmiReal eventIndicators[], size_t ni);

fmiStatus fmiEventUpdate_               (fmiComponent c, fmiBoolean intermediateResults, fmiEventInfo* eventInfo);
fmiStatus fmiGetContinuousStates_       (fmiComponent c, fmiReal states[], size_t nx);
fmiStatus fmiGetNominalContinuousStates_(fmiComponent c, fmiReal x_nominal[], size_t nx);
fmiStatus fmiGetStateValueReferences_   (fmiComponent c, fmiValueReference vrx[], size_t nx);


#endif
