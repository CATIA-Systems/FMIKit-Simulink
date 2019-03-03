/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/
 
#include <stdarg.h>

#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#include "fmi2Functions.h"

#define FMI_VERSION 2

#include "sfun_fmu_common.c"


#define MDL_INITIALIZE_CONDITIONS
#if defined(MDL_INITIALIZE_CONDITIONS)
static void mdlInitializeConditions(SimStruct *S) {

#if NX > 0
	// initialize the continuous states
	assertNoError(S, fmi2GetContinuousStates(COMPONENT, X, NX), "Failed to get continuous states");
#endif

#if NZ > 0
	// initialize the event indicators
	assertNoError(S, fmi2GetEventIndicators(COMPONENT, PREZ, NZ), "Failed to get event indicators");
	assertNoError(S, fmi2GetEventIndicators(COMPONENT, Z, NZ), "Failed to get event indicators");
#endif

}
#endif /* MDL_INITIALIZE_CONDITIONS */


#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S) {

	static fmi2CallbackFunctions callbacks = { logFMUMessage, calloc, free, NULL, NULL };

	EVENT_INFO_PTR = calloc(1, sizeof(fmi2EventInfo));

	COMPONENT = fmi2Instantiate(ssGetPath(S), fmi2ModelExchange, MODEL_GUID, "", &callbacks, fmi2False, fmi2False);

	if (!COMPONENT) {
		ssSetErrorStatus(S, "Failed to instantiate FMU");
		return;
	}

	setStartValues(S);

	assertNoError(S, fmi2SetupExperiment(COMPONENT, fmi2False, 0, ssGetT(S), fmi2True, ssGetTFinal(S)), "Failed to set up experiment");

	assertNoError(S, fmi2EnterInitializationMode(COMPONENT), "Failed to enter initialization mode");
	assertNoError(S, fmi2ExitInitializationMode(COMPONENT), "Failed to exit initialization mode");

	// event iteration
	EVENT_INFO->newDiscreteStatesNeeded = fmi2True;
	EVENT_INFO->terminateSimulation = fmi2False;

	while (EVENT_INFO->newDiscreteStatesNeeded && !EVENT_INFO->terminateSimulation) {
		assertNoError(S, fmi2NewDiscreteStates(COMPONENT, EVENT_INFO), "Event update failed");
	}

	// TODO: handle EVENT_INFO->terminateSimulation

	assertNoError(S, fmi2EnterContinuousTimeMode(COMPONENT), "Failed to enter continuous time mode");

}
#endif /* MDL_START */


static void update(SimStruct *S) {

	fmi2Boolean timeEvent, stateEvent;
	fmi2Boolean enterEventMode, terminateSimulation;
	int i;
	fmi2EventInfo e;

	// Work around for the event handling in Dymola FMUs:
	timeEvent = ssGetT(S) >= EVENT_INFO->nextEventTime;

	assertNoError(S, fmi2CompletedIntegratorStep(COMPONENT, fmi2True, &enterEventMode, &terminateSimulation), "Completed integrator step failed");

	if (terminateSimulation) {
		ssSetErrorStatus(S, "FMU requested termination");
		return;
	}

	stateEvent = fmi2False;

#if NZ > 0
	assertNoError(S, fmi2GetEventIndicators(COMPONENT, Z, NZ), "Failed to get event indicators");

	// check for state events
	for (i = 0; i < NZ; i++) {
		stateEvent = stateEvent || (PREZ[i] * Z[i] < 0);
	}

	// remember the current event indicators
	for (i = 0; i < NZ; i++) {
		PREZ[i] = Z[i];
	}
#endif

	// handle events
	if (timeEvent || stateEvent || enterEventMode) {

		assertNoError(S, fmi2EnterEventMode(COMPONENT), "Failed to enter event time mode");

		EVENT_INFO->newDiscreteStatesNeeded = fmi2True;

		while (EVENT_INFO->newDiscreteStatesNeeded && !EVENT_INFO->terminateSimulation) {
			// update discrete states
			assertNoError(S, fmi2NewDiscreteStates(COMPONENT, EVENT_INFO), "New discrete states failed");
		}

		if (EVENT_INFO->terminateSimulation) {
			ssSetErrorStatus(S, "FMU requested termination");
			return;
		}

		assertNoError(S, fmi2EnterContinuousTimeMode(COMPONENT), "Failed to enter continuous time mode");

#if NX > 0
		assertNoError(S, fmi2GetContinuousStates(COMPONENT, X, NX), "Failed to get continuous states");
#endif

		ssSetSolverNeedsReset(S);
	}
}


static void mdlOutputs(SimStruct *S, int_T tid) {

	//ssPrintf("mdlOutputs() (t=%.16g, %s)\n", ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	setInputs(S);

	assertNoError(S, fmi2SetTime(COMPONENT, ssGetT(S)), "Failed to set time");

	assertNoError(S, fmi2SetContinuousStates(COMPONENT, X, NX), "Failed to set continuous states");

	if (ssIsMajorTimeStep(S)) update(S);

	setOutputs(S);

}


#define MDL_ZERO_CROSSINGS
#if defined(MDL_ZERO_CROSSINGS) && (defined(MATLAB_MEX_FILE) || defined(NRT))
static void mdlZeroCrossings(SimStruct *S) {

#if NZ > 0
	assertNoError(S, fmi2GetEventIndicators(COMPONENT, Z, NZ), "Failed to get event indicators");
#endif

}
#endif /* MDL_ZERO_CROSSINGS */


#define MDL_DERIVATIVES
#if defined(MDL_DERIVATIVES)
static void mdlDerivatives(SimStruct *S) {

	//ssPrintf("mdlDerivatives() (t=%.16g, %s)\n", ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	assertNoError(S, fmi2GetContinuousStates(COMPONENT, X, NX), "Failed to get continuous states");
	assertNoError(S, fmi2GetDerivatives(COMPONENT, DX, NX), "Failed to get derivatives");
}
#endif /* MDL_DERIVATIVES */


static void mdlTerminate(SimStruct *S) {

	fmi2Terminate(COMPONENT);
	fmi2FreeInstance(COMPONENT);

	free(EVENT_INFO);
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
