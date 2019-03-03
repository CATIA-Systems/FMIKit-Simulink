/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/
 
#include <stdarg.h>

#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#include "fmiModelFunctions.h"

#define FMI_VERSION 1

#include "sfun_fmu_common.c"


static void update(SimStruct *S) {

	//const real_T *z = ssGetNonsampledZCs(S);

	fmiBoolean timeEvent, stepEvent, stateEvent;
	int i, rising, falling;

	// Work around for the event handling in Dymola FMUs:
	timeEvent = ssGetT(S) >= EVENT_INFO->nextEventTime;

	assertNoError(S, fmiCompletedIntegratorStep(COMPONENT, &stepEvent), "Completed integrator step failed");

	stateEvent = fmiFalse;

#if NZ > 0
	// check for state events
	for (i = 0; i < NZ; i++) {

		rising  = (PREZ[i] < 0 && Z[i] >= 0) || (PREZ[i] == 0 && Z[i] > 0);
		falling = (PREZ[i] > 0 && Z[i] <= 0) || (PREZ[i] == 0 && Z[i] < 0);

		if (rising || falling) {
			stateEvent = fmiTrue;
			break;
		}
	}
#endif

	if (timeEvent || stepEvent || stateEvent) {

		assertNoError(S, fmiEventUpdate(COMPONENT, fmiFalse, EVENT_INFO), "Failed to do event update");

#if NX > 0
		assertNoError(S, fmiGetContinuousStates(COMPONENT, X, NX), "Failed to get continuous states");
#endif

		ssSetSolverNeedsReset(S);
	}
}


#define MDL_INITIALIZE_CONDITIONS
#if defined(MDL_INITIALIZE_CONDITIONS)
static void mdlInitializeConditions(SimStruct *S) {

#if NX > 0
	// initialize the continuous states
	assertNoError(S, fmiGetContinuousStates(COMPONENT, X, NX), "Failed to get continuous states");
#endif

#if NZ > 0
	// initialize the event indicators
	assertNoError(S, fmiGetEventIndicators(COMPONENT, PREZ, NZ), "Failed to get event indicators");
#endif

}
#endif /* MDL_INITIALIZE_CONDITIONS */


#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S) {

	fmiCallbackFunctions callbacks = { logFMUMessage, calloc, free };

	EVENT_INFO_PTR = calloc(1, sizeof(fmiEventInfo));

	if (!(COMPONENT = fmiInstantiateModel(ssGetPath(S), MODEL_GUID, callbacks, fmiFalse))) {
		ssSetErrorStatus(S, "Failed to instantiate model");
		return;
	}

	setStartValues(S);

	assertNoError(S, fmiSetTime(COMPONENT, ssGetT(S)), "Failed to set time");

	assertNoError(S, fmiInitialize(COMPONENT, fmiFalse, 0.0, EVENT_INFO), "Failed to initialize");
}
#endif /* MDL_START */


static void mdlOutputs(SimStruct *S, int_T tid) {

	//ssPrintf("mdlOutputs() (t=%.16g, %s)\n", ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	setInputs(S);

	assertNoError(S, fmiSetTime(COMPONENT, ssGetT(S)), "Failed to set time");

	if (ssIsMajorTimeStep(S)) update(S);

	assertNoError(S, fmiSetContinuousStates(COMPONENT, X, NX), "Failed to set continuous states");

	setOutputs(S);

}


#define MDL_ZERO_CROSSINGS
#if defined(MDL_ZERO_CROSSINGS) && (defined(MATLAB_MEX_FILE) || defined(NRT))
static void mdlZeroCrossings(SimStruct *S) {

	real_T *z = ssGetNonsampledZCs(S);
	int i;

#if NZ > 0
	// remember the current event indicators
	for (i = 0; i < NZ; i++) PREZ[i] = Z[i];

	assertNoError(S, fmiGetEventIndicators(COMPONENT, z, NZ), "Failed to get event indicators");
	assertNoError(S, fmiGetEventIndicators(COMPONENT, Z, NZ), "Failed to get event indicators");
#endif

	z[NZ] = EVENT_INFO->nextEventTime - ssGetT(S);
}
#endif /* MDL_ZERO_CROSSINGS */


#define MDL_DERIVATIVES
#if defined(MDL_DERIVATIVES)
static void mdlDerivatives(SimStruct *S) {

	//ssPrintf("mdlDerivatives() (t=%.16g, %s)\n", ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	assertNoError(S, fmiGetContinuousStates(COMPONENT, X, NX), "Failed to get continuous states");
	assertNoError(S, fmiGetDerivatives(COMPONENT, DX, NX), "Failed to get derivatives");
}
#endif /* MDL_DERIVATIVES */


static void mdlTerminate(SimStruct *S) {
	fmiTerminate(COMPONENT);
	fmiFreeModelInstance(COMPONENT);
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
