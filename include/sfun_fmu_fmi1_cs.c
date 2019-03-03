/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/
 
#include <stdarg.h>

#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

#include "fmiFunctions.h"

#define FMI_VERSION 1
#define CO_SIMULATION

#include "sfun_fmu_common.c"


#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S) {

	fmiCallbackFunctions callbacks = { logFMUMessage, calloc, free, 0 };

	COMPONENT = fmiInstantiateSlave(ssGetPath(S), MODEL_GUID, "", "application/x-fmu-sharedlibrary", 0.0, fmiFalse, fmiFalse, callbacks, fmiFalse);

    if (!COMPONENT) {
		ssSetErrorStatus(S, "Failed to instantiate FMU");
		return;
	}

	setStartValues(S);

	assertNoError(S, fmiInitializeSlave(COMPONENT, ssGetT(S), fmiTrue, ssGetTFinal(S)), "Failed to initialize slave");

	FMU_TIME = ssGetT(S);

}
#endif /* MDL_START */


static void mdlOutputs(SimStruct *S, int_T tid) {

	//ssPrintf("mdlOutputs() (t=%.16g, %s)\n", ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	if (ssGetT(S) > FMU_TIME) {

		setInputs(S);

		assertNoError(S, fmiDoStep(COMPONENT, FMU_TIME, ssGetT(S) - FMU_TIME, fmiTrue), "Failed to do step");

		FMU_TIME = ssGetT(S);
	}

	setOutputs(S);
}


static void mdlTerminate(SimStruct *S) {
	fmiTerminateSlave(COMPONENT);
	fmiFreeSlaveInstance(COMPONENT);
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
