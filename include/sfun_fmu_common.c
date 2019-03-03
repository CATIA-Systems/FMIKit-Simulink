/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/
 
#if FMI_VERSION == 1
#define FMI_API(F) fmi ## F
#elif FMI_VERSION == 2
#define FMI_API(F) fmi2 ## F
#endif

typedef enum {
	REAL, INTEGER, BOOLEAN, STRING
} Type;


#define COMPONENT	(ssGetPWork(S)[0])

#define FMU_TIME		(ssGetRWork(S)[0])
#define PRE_U     		(&ssGetRWork(S)[1])

#ifndef CO_SIMULATION
#define EVENT_INFO_PTR 	(ssGetPWork(S)[1])
#define EVENT_INFO  	((FMI_API(EventInfo) *)EVENT_INFO_PTR)
#endif

#define X    ssGetContStates(S)
#define DX   ssGetdX(S)
#define PREZ (&ssGetRWork(S)[1 + N_INPUT_VARIABLES])
#define Z    (&ssGetRWork(S)[1 + N_INPUT_VARIABLES + NZ])

#define UNIZIP_DIRECTORY_PARAM      ssGetSFcnParam(S, 0)
#define UNIZIP_DIRECTORY            mxArrayToString(UNIZIP_DIRECTORY_PARAM)

#define LOG_LEVEL_PARAM             ssGetSFcnParam(S, 1)
#define LOG_LEVEL                   mxGetScalar(LOG_LEVEL_PARAM)

#define RELATIVE_TOLERANCE_PARAM    ssGetSFcnParam(S, 2)
#define RELATIVE_TOLERANCE          mxGetScalar(RELATIVE_TOLERANCE_PARAM)

#define SAMPLE_TIME_PARAM			ssGetSFcnParam(S, 3)
#define SAMPLE_TIME					mxGetScalar(SAMPLE_TIME_PARAM)

#define OFFSET_TIME_PARAM           ssGetSFcnParam(S, 4)
#define OFFSET_TIME                 mxGetScalar(OFFSET_TIME_PARAM)

#define SCALAR_START_TYPES_PARAM    ssGetSFcnParam(S, 5)
#define SCALAR_START_TYPES          ((real_T *) mxGetData(SCALAR_START_TYPES_PARAM))

#define SCALAR_START_VRS_PARAM      ssGetSFcnParam(S, 6)
#define SCALAR_START_VRS            ((real_T *) mxGetData(SCALAR_START_VRS_PARAM))

#define SCALAR_START_VALUES_PARAM   ssGetSFcnParam(S, 7)
#define SCALAR_START_VALUES         ((real_T *) mxGetData(SCALAR_START_VALUES_PARAM))

#define STRING_START_VRS_PARAM      ssGetSFcnParam(S, 8)
#define STRING_START_VRS            ((real_T *) mxGetData(STRING_START_VRS_PARAM))

#define STRING_START_VALUES_PARAM   ssGetSFcnParam(S, 9)

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)


static void assertNoError(SimStruct *S, FMI_API(Status) status, const char *message) {
	if (status >= FMI_API(Error)) ssSetErrorStatus(S, message);
}

#define MAX_MESSAGE_SIZE 4096

static DTypeId simulinkVariableType(Type type) {

	switch(type) {
	case REAL:    return SS_DOUBLE;
	case INTEGER: return SS_INT32;
	case BOOLEAN: return SS_BOOLEAN;
	}

	return -1; // error
}

static void logFMUMessage(FMI_API(Component) c, FMI_API(String) instanceName, FMI_API(Status) status, FMI_API(String) category, FMI_API(String) message, ...) {
	char buf[MAX_MESSAGE_SIZE];
	va_list args;
	va_start(args, message);
	vsnprintf(buf, MAX_MESSAGE_SIZE, message, args);
	strncat(buf, "\n", MAX_MESSAGE_SIZE);
	va_end(args);
	ssPrintf(buf);
}

static void setStartValues(SimStruct *S) {

	int i, j;
	size_t size, m, n;
	FMI_API(ValueReference) vr;
	int type;
	FMI_API(Real) real_value;
	FMI_API(Integer) integer_value;
	FMI_API(Boolean) boolean_value;
	char *buffer, *string_value;

    // scalar start values
    for (i = 0; i < mxGetNumberOfElements(SCALAR_START_VALUES_PARAM); i++) {

		type = SCALAR_START_TYPES[i];
		vr = SCALAR_START_VRS[i];

		switch (((int)SCALAR_START_TYPES[i])) {
        case REAL:
			real_value = SCALAR_START_VALUES[i];
			assertNoError(S, FMI_API(SetReal)   (COMPONENT, &vr, 1, &real_value),    "Failed to set real start value");
			break;
        case INTEGER:
			integer_value = SCALAR_START_VALUES[i];
			assertNoError(S, FMI_API(SetInteger)(COMPONENT, &vr, 1, &integer_value), "Failed to set integer start value");
			break;
        case BOOLEAN:
			boolean_value = SCALAR_START_VALUES[i] != 0;
			assertNoError(S, FMI_API(SetBoolean)(COMPONENT, &vr, 1, &boolean_value), "Failed to set boolean start value");
			break;
        }
    }

    // string start values
    size = mxGetNumberOfElements(STRING_START_VALUES_PARAM) + 1;
    m = mxGetM(STRING_START_VALUES_PARAM);
    n = mxGetN(STRING_START_VALUES_PARAM);
    buffer = (char *)calloc(size, sizeof(char));
    string_value = (char *)calloc(n + 1, sizeof(char));

    if (mxGetString(STRING_START_VALUES_PARAM, buffer, size) != 0) {
        ssSetErrorStatus(S, "Failed to convert string parameters");
        return;
    }

    for (i = 0; i < m; i++) {

        // copy the row
        for (j = 0; j < n; j++) string_value[j] = buffer[j * m + i];

        // remove the trailing blanks
        for (j = n - 1; j >= 0; j--) {
            if (string_value[j] != ' ') break;
            string_value[j] = '\0';
        }

        vr = STRING_START_VRS[i];

        assertNoError(S, FMI_API(SetString)(COMPONENT, &vr, 1, &string_value), "Failed to set string start value");
    }

    free(buffer);
    free(string_value);

}


/* Set the S-function's inputs to the FMU */
static void setInputs(SimStruct *S) {

#if NU > 0

    const int   inport_port_widths[NU] = { INPUT_PORT_WIDTHS };
	const Type  input_port_types[NU]   = { INPUT_PORT_TYPES };
	const int   input_feed_through[NU] = { INPUT_PORT_FEED_THROUGH };
	const FMI_API(ValueReference) input_variable_vrs[N_INPUT_VARIABLES] = { INPUT_VARIABLE_VRS };

    FMI_API(Real) real_value;
    FMI_API(Integer) integer_value, order = 1;
    FMI_API(Boolean) boolean_value;
	int iu, i, j;
    real_T du;
	const void *u;
    time_T h = ssGetT(S) - FMU_TIME;
	Type type;

    iu = 0;
    for (i = 0; i < NU; i++) {

		u = ssGetInputPortSignal(S, i);

        for (j = 0; j < inport_port_widths[i]; j++) {

            FMI_API(ValueReference) vr = input_variable_vrs[iu];

#ifdef DIRECT_INPUT
            switch (input_port_types[i]) {
			case REAL:
				real_value = ((const real_T *)u)[j];
				break;
			case INTEGER:
				real_value = ((const int32_T *)u)[j];
				break;
			case BOOLEAN:
				real_value = ((const boolean_T *)u)[j];
				break;
			}
#else
            real_value = PRE_U[iu];
#endif

            switch (input_port_types[i]) {
            case REAL:
                assertNoError(S, FMI_API(SetReal)(COMPONENT, &vr, 1, &real_value), "Failed to set real input");
#ifdef SET_INPUT_DERIVATIVES
				if (h > 0 && input_feed_through[i]) {
                    du = (((const real_T *)u)[j] - PRE_U[iu]) / h;
                    assertNoError(S, FMI_API(SetRealInputDerivatives)(COMPONENT, &vr, 1, &order, &du), "Failed to set real input derivatives");
                }
#endif
                break;
            case INTEGER:
                integer_value = real_value;
                assertNoError(S, FMI_API(SetInteger)(COMPONENT, &vr, 1, &integer_value), "Failed to set integer input");
                break;
            case BOOLEAN:
                boolean_value = real_value != 0.0;
                assertNoError(S, FMI_API(SetBoolean)(COMPONENT, &vr, 1, &boolean_value), "Failed to set boolean input");
                break;
            }

            iu++;
        }
    }
#endif /* NU > 0 */

}

/* Set the S-function's outputs */
static void setOutputs(SimStruct *S) {

#if NY > 0
    const int outport_widths[NY] = { OUTPUT_PORT_WIDTHS };
	const Type outport_variable_types[NY] = { OUTPUT_PORT_TYPES };
    const FMI_API(ValueReference) outport_variable_vrs[N_OUTPUT_VARIABLES] = { OUTPUT_VARIABLE_VRS };

	FMI_API(Real) real_value;
    FMI_API(Integer) integer_value;
    FMI_API(Boolean) boolean_value;

	int iy, i, j;
	void *y;
	FMI_API(ValueReference) vr;

    iy = 0;
    for (i = 0; i < NY; i++) {

        y = ssGetOutputPortSignal(S, i);

        for (j = 0; j < outport_widths[i]; j++) {

            vr = outport_variable_vrs[iy];

            switch (outport_variable_types[i]) {
            case REAL:
                assertNoError(S, FMI_API(GetReal)(COMPONENT, &vr, 1, &real_value), "Failed to get real output");
                ((real_T *)y)[j] = real_value;
                break;
            case INTEGER:
                assertNoError(S, FMI_API(GetInteger)(COMPONENT, &vr, 1, &integer_value), "Failed to get integer input");
                ((int32_T *)y)[j] = integer_value;
                break;
            case BOOLEAN:
                assertNoError(S, FMI_API(GetBoolean)(COMPONENT, &vr, 1, &boolean_value), "Failed to get boolean input");
				((boolean_T *)y)[j] = boolean_value;
                break;
            }

            iy++;
        }
    }
#endif

}


#define MDL_UPDATE
#if defined(MDL_UPDATE)
static void mdlUpdate(SimStruct *S, int_T tid) {

#if NU > 0
    const int   inport_port_widths[NU] = { INPUT_PORT_WIDTHS };
	const Type  input_port_types[NU]   = { INPUT_PORT_TYPES };

	int iu, i, j;
    const void *u;

	//ssPrintf("mdlUpdate() (t=%.16g, %s)\n", ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	// record the inputs
    iu = 0;
    for (i = 0; i < NU; i++) {

		u = ssGetInputPortSignal(S, i);

		for (j = 0; j < inport_port_widths[i]; j++) {

			switch (input_port_types[i]) {
            case REAL:
				PRE_U[iu] = ((const real_T *)u)[j];
                break;
            case INTEGER:
				PRE_U[iu] = ((const int32_T *)u)[j];
                break;
            case BOOLEAN:
				PRE_U[iu] = ((const boolean_T *)u)[j];
                break;
            }

			iu++;
        }
    }
#endif

}
#endif /* MDL_UPDATE */


#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlCheckParameters(SimStruct *S) {

	int i;
	real_T v;

    if (!mxIsChar(UNIZIP_DIRECTORY_PARAM)) {
        ssSetErrorStatus(S, "Parameter 1 (unzip directory) must be a string");
        return;
    }

    if (!mxIsNumeric(LOG_LEVEL_PARAM) || mxGetNumberOfElements(LOG_LEVEL_PARAM) != 1 || (LOG_LEVEL != 1 && LOG_LEVEL != 2 && LOG_LEVEL != 3)) {
        ssSetErrorStatus(S, "Parameter 2 (log level) must be one of 1 (= DEBUG), 2 (= INFO) or 3 (= WARNING)");
        return;
    }

	if (!mxIsNumeric(RELATIVE_TOLERANCE_PARAM) || mxGetNumberOfElements(RELATIVE_TOLERANCE_PARAM) != 1) {
        ssSetErrorStatus(S, "Parameter 3 (relative tolerance) must be numeric");
        return;
    }

    if (!mxIsNumeric(SAMPLE_TIME_PARAM) || mxGetNumberOfElements(SAMPLE_TIME_PARAM) != 1) {
        ssSetErrorStatus(S, "Parameter 4 (sample time) must be numeric");
        return;
    }

    if (!mxIsNumeric(OFFSET_TIME_PARAM) || mxGetNumberOfElements(OFFSET_TIME_PARAM) != 1) {
        ssSetErrorStatus(S, "Parameter 5 (offset time) must be numeric");
        return;
    }

	if (!mxIsDouble(SCALAR_START_TYPES_PARAM)) {
        ssSetErrorStatus(S, "Parameter 6 (scalar start value types) must be a double array");
        return;
    }

	for (i = 0; i < mxGetNumberOfElements(SCALAR_START_TYPES_PARAM); i++) {
		v = SCALAR_START_TYPES[i];
		if (v != 0 && v != 1 && v != 2) {
			ssSetErrorStatus(S, "The values in parameter 6 (scalar start value types) must be one of 0 (= Real), 1 (= Integer) or 2 (= Boolean)");
			return;
		}
	}

    if (!mxIsDouble(SCALAR_START_VRS_PARAM)) {
        ssSetErrorStatus(S, "Parameter 7 (scalar start value references) must be a double array");
        return;
    }

    if (mxGetNumberOfElements(SCALAR_START_VRS_PARAM) != mxGetNumberOfElements(SCALAR_START_TYPES_PARAM)) {
        ssSetErrorStatus(S, "The number of elements in parameter 7 (scalar start value references) and parameter 6 (scalar start value types) must be equal");
        return;
    }

    if (!mxIsDouble(SCALAR_START_VALUES_PARAM)) {
        ssSetErrorStatus(S, "Parameter 8 (scalar start values) must be a double array");
        return;
    }

    if (mxGetNumberOfElements(SCALAR_START_VALUES_PARAM) != mxGetNumberOfElements(SCALAR_START_TYPES_PARAM)) {
        ssSetErrorStatus(S, "The number of elements in parameter 8 (scalar start values) and parameter 6 (scalar start value types) must be equal");
        return;
    }

    if (!mxIsDouble(STRING_START_VRS_PARAM)) {
        ssSetErrorStatus(S, "Parameter 9 (string start value references) must be a double array");
        return;
    }

    if (!mxIsChar(STRING_START_VALUES_PARAM)) {
        ssSetErrorStatus(S, "Parameter 10 (string start values) must be a char matrix");
        return;
    }

    if (mxGetM(STRING_START_VALUES_PARAM) != mxGetNumberOfElements(STRING_START_VRS_PARAM)) {
        ssSetErrorStatus(S, "The number of rows in parameter 10 (string start values) must be equal to the number of elements in parameter 9 (string start value references)");
        return;
    }

}
#endif /* MDL_CHECK_PARAMETERS */


static void mdlInitializeSizes(SimStruct *S)
{
	int i;

#if NU > 0
	const int input_port_widths[NU]  = { INPUT_PORT_WIDTHS };
	const int input_port_types[NU]   = { INPUT_PORT_TYPES };
	const int input_feed_through[NU] = { INPUT_PORT_FEED_THROUGH };
#endif

#if NY > 0
	const int output_port_widths[NY] = { OUTPUT_PORT_WIDTHS };
	const int output_port_types[NY]  = { OUTPUT_PORT_TYPES };
#endif

	ssSetNumSFcnParams(S, 10); // parameters

#if defined(MATLAB_MEX_FILE)
	if (ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S)) {
		mdlCheckParameters(S);
		if (ssGetErrorStatus(S) != NULL) return;
	} else {
		return; // parameter mismatch will be reported by Simulink
	}
#endif

#ifdef CO_SIMULATION
	ssSetNumContStates(S, 0);
#else
	ssSetNumContStates(S, NX);
#endif

	ssSetNumDiscStates(S, 0);

	if (!ssSetNumInputPorts(S, NU)) return;

#if NU > 0
	for (i = 0; i < NU; i++) {
		ssSetInputPortWidth(S, i, input_port_widths[i]);
		ssSetInputPortDataType(S, i, simulinkVariableType(input_port_types[i]));
		ssSetInputPortRequiredContiguous(S, i, 1); // direct input signal access
		ssSetInputPortDirectFeedThrough(S, i, input_feed_through[i]);
	}
#endif

	if (!ssSetNumOutputPorts(S, NY)) return;

#if NY > 0
	for (i = 0; i < NY; i++) {
		ssSetOutputPortWidth(S, i, output_port_widths[i]);
		ssSetOutputPortDataType(S, i, simulinkVariableType(output_port_types[i]));
	}
#endif

	ssSetNumSampleTimes(S, 1);
	ssSetNumIWork(S, 0);
	ssSetNumModes(S, 0);

#ifdef CO_SIMULATION
	ssSetNumNonsampledZCs(S, 0);
	ssSetNumRWork(S, 1 + N_INPUT_VARIABLES); // [FMU_TIME, PRE_U]
	ssSetNumPWork(S, 1); // [COMPONENT]
#else
	ssSetNumNonsampledZCs(S, NZ + 1);
	ssSetNumRWork(S, 1 + N_INPUT_VARIABLES + 2 * NZ); // [FMU_TIME, PRE_U, PREZ, Z]
	ssSetNumPWork(S, 2);  // [COMPONENT, EVENT_INFO]
#endif

	/* Specify the sim state compliance to be same as a built-in block */
	//ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);

	ssSetOptions(S, 0);
}

static void mdlInitializeSampleTimes(SimStruct *S) {

#ifdef CO_SIMULATION
	ssSetSampleTime(S, 0, SAMPLE_TIME);
#else
	ssSetSampleTime(S, 0, CONTINUOUS_SAMPLE_TIME);
#endif

	ssSetOffsetTime(S, 0, OFFSET_TIME);

}
