/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

/*
-----------------------------------------------------------
	Common includes and macros for FMI 1.0 and FMI 2.0
	S-function wrappers
-----------------------------------------------------------
*/

#ifndef FMI__H
#define FMI__H

/*
 * Generated header to configure MATLAB release
*/
#include "sfcn_fmi_rel_conf.h"


#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <math.h>

#if defined(_MSC_VER)
#include "windows.h"

#else

#include <sys/stat.h>
/* might need access to non-standard function dladdr */
#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU

#endif

#include <simstruc.h>

#undef SFCN_FMI_VERBOSITY /* Define to add debug logging */

#define SFCN_FMI_MAX_TIME	1e100
#define SFCN_FMI_EPS		2e-13	/* Not supported with discrete sample times smaller than this */

extern char* GUIDString;

/* Model-specific variables and functions */
extern char* SFCN_FMI_MODEL_IDENTIFIER;
extern int_T SFCN_FMI_IS_VARIABLE_STEP_SOLVER;
extern real_T SFCN_FMI_FIXED_STEP_SIZE;
extern int_T SFCN_FMI_EXTRAPOLATION_ORDER;
extern int_T SFCN_FMI_NEWTON_ITER;
extern int_T SFCN_FMI_IS_MT;
extern int_T SFCN_FMI_ZC_LENGTH;
extern int_T SFCN_FMI_NBR_INPUTS;
extern int_T SFCN_FMI_NBR_OUTPUTS;
extern int_T SFCN_FMI_NBR_PARAMS;
extern int_T SFCN_FMI_NBR_BLOCKIO;
extern int_T SFCN_FMI_NBR_DWORK;
extern int_T SFCN_FMI_LOAD_MEX;
extern const char* SFCN_FMI_MATLAB_BIN;
extern int_T SFCN_FMI_NBR_MEX;
extern char* SFCN_FMI_MEX_NAMES[];

extern void  sfcn_fmi_registerMdlCallbacks_(SimStruct* S);
extern void  sfcn_fmi_registerRTModelCallbacks_(SimStruct*S);
extern void  sfcn_fmi_assignInputs_(SimStruct* S, void** inputs);
extern void  sfcn_fmi_assignOutputs_(SimStruct* S, void** outputs);
extern void  sfcn_fmi_assignParameters_(SimStruct* S, void** parameters);
extern void  sfcn_fmi_assignBlockOutputs_(SimStruct* S, void** blockoutputs);
extern void  sfcn_fmi_assignDWork_(SimStruct* S, void** dwork);
extern void* sfcn_fmi_getParametersP_(SimStruct* S);
extern void  sfcn_fmi_copyToSFcnParams_(SimStruct* S);
extern void* sfcn_fmi_allocateBusObject(int_T isInput, int_T portid, int_T width);
extern void  sfcn_fmi_mxGlobalTunable_(SimStruct* S, int_T create, int_T update);

#if defined(_MSC_VER)
#if _MSC_VER > 1350
/* avoid warnings from Visual Studio */
#define strncpy(dest, src, len) strncpy_s(dest, (len) + 1, src, len)
#endif
#endif

/* Variable categories */
typedef enum {
  SFCN_FMI_PARAMETER  = 1,
  SFCN_FMI_STATE      = 2,
  SFCN_FMI_DERIVATIVE = 3,
  SFCN_FMI_OUTPUT     = 4,
  SFCN_FMI_INPUT      = 5,
  SFCN_FMI_BLOCKIO    = 6,
  SFCN_FMI_DWORK	  = 7
} sfcn_fmi_category_T;

/* Create a 32-bit value reference on format: */
/*  category: 31-28  datatype: 27-24  index: 23-0 */
#define SFCN_FMI_VALUE_REFERENCE(category, datatype, index) ((unsigned int)(category) << 28 | \
															 (unsigned int)(datatype) << 24 | \
														     (unsigned int)(index))

#define SFCN_FMI_CATEGORY(valueRef) ((valueRef) >> 28)
#define SFCN_FMI_DATATYPE(valueRef)	(((valueRef) >> 24) & 0xf)
#define SFCN_FMI_INDEX(valueRef)	((valueRef) & 0xffffff)


/* ------------- Macro to free allocated memory -------------- */

#define sfcn_fmi_FREE(ptr, freeFcn)                   \
	if((ptr) != NULL) {\
	   freeFcn((void *)(ptr));\
       (ptr) = NULL;\
    }

/* ----------------------------------------------------------- */

/* --------- Function to copy per-task sample hits ----------- */

void copyPerTaskSampleHits(SimStruct* S)
{
	int_T m, n;

	for (m=1; m<S->sizes.numSampleTimes; m++) {
		for (n=0; n<S->sizes.numSampleTimes; n++) {
			S->mdlInfo->perTaskSampleHits[n + m * (S->sizes.numSampleTimes)] = S->mdlInfo->sampleHits[n];
		}
	}
}

/* ----------------------------------------------------------- */

/* -------- Function for double precision comparison --------- */

int isEqual(double a, double b)
{
	double A, B, largest;
	double diff = fabs(a-b);

	A = fabs(a);
	B = fabs(b);
	largest = (B > A) ? B : A;

	if (diff <= (1.0+largest)*SFCN_FMI_EPS)
		return 1;
	return 0;
}

/* ----------------------------------------------------------- */


#endif
