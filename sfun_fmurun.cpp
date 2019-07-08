/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#define S_FUNCTION_NAME  sfun_fmurun
#define S_FUNCTION_LEVEL 2

#ifdef _WIN32
#include "shlwapi.h"
#include <wininet.h>
#endif

#include <string>

extern "C" {
#include "simstruc.h"
}

#include "FMU1.h"
#include "FMU2.h"

using namespace std;
using namespace fmikit;

#define MAX_MESSAGE_SIZE 4096

enum Parameter {

	fmiVersionParam,
	runAsKindParam,
	guidParam,
	modelIdentifierParam,
	unzipDirectoryParam,
    debugLoggingParam,
    logFMICallsParam,
    logLevelParam,
    logFileParam,
	relativeToleranceParam,
	sampleTimeParam,
	offsetTimeParam,
	nxParam,
	nzParam,
	scalarStartTypesParam,
	scalarStartVRsParam,
	scalarStartValuesParam,
	stringStartVRsParam,
	stringStartValuesParam,
	inputPortWidthsParam,
	directInputParam,
	inputPortDirectFeedThroughParam,
	inputPortTypesParam,
	inputPortVariableVRsParam,
	canInterpolateInputsParam,
	outputPortWidthsParam,
	outputPortTypesParam,
	outputPortVariableVRsParam,
	numParams

};

static string getStringParam(SimStruct *S, int index) {
	auto pa = ssGetSFcnParam(S, index);
	size_t buflen = mxGetN(pa) * sizeof(mxChar) + 1;
	auto str = (char *)mxMalloc(buflen);
	mxGetString(pa, str, buflen);
	string cppstr(str);
	mxFree(str);
	return cppstr;
}

static string fmiVersion(SimStruct *S) {
	return getStringParam(S, fmiVersionParam);
}

static fmikit::Kind runAsKind(SimStruct *S) { return static_cast<fmikit::Kind>(static_cast<int>( mxGetScalar( ssGetSFcnParam(S, runAsKindParam) ) ) ); }

static string guid(SimStruct *S) {
	return getStringParam(S, guidParam);
}

static string modelIdentifier(SimStruct *S) {
	return getStringParam(S, modelIdentifierParam);
}

static string unzipDirectory(SimStruct *S) {
	return getStringParam(S, unzipDirectoryParam);
}

static bool debugLogging(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, debugLoggingParam));
}

static bool logFMICalls(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, logFMICallsParam));
}

static fmikit::LogLevel logLevel(SimStruct *S) {
    int level = static_cast<int>(mxGetScalar(ssGetSFcnParam(S, logLevelParam)));
    return static_cast<fmikit::LogLevel>(level);
}

static string logFile(SimStruct *S) {
    return getStringParam(S, logFileParam);
}

static double relativeTolerance(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, relativeToleranceParam));
}

static double sampleTime(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, sampleTimeParam));
}

static double offsetTime(SimStruct *S) {
    return mxGetScalar(ssGetSFcnParam(S, offsetTimeParam));
}

// number of continuous states
static int nx(SimStruct *S) {
    return static_cast<int>(mxGetScalar(ssGetSFcnParam(S, nxParam)));
}

// number of zero-crossings
static int nz(SimStruct *S) {
    return static_cast<int>(mxGetScalar(ssGetSFcnParam(S, nzParam)));
}

static int nScalarStartValues(SimStruct *S) {
    return mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartVRsParam));
}

static int inputPortWidth(SimStruct *S, int index) {
	auto portWidths = static_cast<real_T *>(mxGetData(ssGetSFcnParam(S, inputPortWidthsParam)));
	return static_cast<int>(portWidths[index]);
}

static bool inputPortDirectFeedThrough(SimStruct *S, int index) {
	return static_cast<real_T *>(mxGetData(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)))[index] != 0;
}

static bool canInterpolateInputs(SimStruct *S) {
	return static_cast<real_T *>(mxGetData(ssGetSFcnParam(S, canInterpolateInputsParam)))[0] != 0;
}

// number of input ports
inline size_t nu(SimStruct *S) { return mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam)); }

// number of input variables
inline size_t nuv(SimStruct *S) { return mxGetNumberOfElements(ssGetSFcnParam(S, inputPortVariableVRsParam)); }

inline ValueReference valueReference(SimStruct *S, Parameter parameter, int index) {
	auto param = ssGetSFcnParam(S, parameter);
	auto realValue = static_cast<real_T *>(mxGetData(param))[index];
	return static_cast<fmikit::ValueReference>(realValue);
}

inline Type variableType(SimStruct *S, Parameter parameter, int index) {
	auto param = ssGetSFcnParam(S, parameter);
	auto realValue = static_cast<real_T *>(mxGetData(param))[index];
	auto intValue = static_cast<int>(realValue);
	return static_cast<fmikit::Type>(intValue);
}

inline DTypeId simulinkVariableType(SimStruct *S, Parameter parameter, int index) {

	auto param = ssGetSFcnParam(S, parameter);
	auto realValue = static_cast<real_T *>(mxGetData(param))[index];
	auto intValue = static_cast<int>(realValue);
	auto type = static_cast<Type>(intValue);

	switch(type) {
	case fmikit::REAL:    return SS_DOUBLE;
	case fmikit::INTEGER: return SS_INT32;
	case fmikit::BOOLEAN: return SS_BOOLEAN;
	default: return -1; // error
	}
}

inline real_T scalarValue(SimStruct *S, Parameter parameter, int index) {
	auto param = ssGetSFcnParam(S, parameter);
	return static_cast<real_T *>(mxGetData(param))[index];
}

static bool directInput(SimStruct *S) {
	return static_cast<real_T *>(mxGetData(ssGetSFcnParam(S, directInputParam)))[0] != 0;
}

static int outputPortWidth(SimStruct *S, int index) {
	auto portWidths = static_cast<real_T *>(mxGetData(ssGetSFcnParam(S, outputPortWidthsParam)));
	return static_cast<int>(portWidths[index]);
}

inline size_t ny(SimStruct *S) { return mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); }

template<typename T> T *component(SimStruct *S) {
	auto fmu = static_cast<FMU *>(ssGetPWork(S)[0]);
	return dynamic_cast<T *>(fmu);
}

static void logCall(SimStruct *S, const char* message) {
    
    FILE *logfile = nullptr;
    
	void **p = ssGetPWork(S);
    
    if (p) {
        logfile = static_cast<FILE *>(p[1]);
    }
    
    if (logfile) {
        fputs(message, logfile);
        fputs("\n", logfile);
        fflush(logfile);
    } else {
        ssPrintf(message);
        ssPrintf("\n");
    }
}

static void logFMUMessage(FMU *instance, LogLevel level, const char* category, const char* message) {
    
    if (instance && instance->m_userData) {
        SimStruct *S = static_cast<SimStruct *>(instance->m_userData);
        logCall(S, message);
    }
}

static void logFMICall(FMU *instance, const char* message) {

	if (instance && instance->m_userData) {
		SimStruct *S = static_cast<SimStruct *>(instance->m_userData);
		logCall(S, message);
	}
}

/* log mdl*() and fmi*() calls */
static void logDebug(SimStruct *S, const char* message, ...) {
    
    if (logFMICalls(S)) {
        va_list args;
        va_start(args, message);
        char buf[MAX_MESSAGE_SIZE];
        vsnprintf(buf, MAX_MESSAGE_SIZE, message, args);
        va_end(args);

        logCall(S, buf);
    }
}

static void setInput(SimStruct *S) {

	auto fmu = component<FMU>(S);

	const auto feedThrough = directInput(S);

	// don't apply the delayed input at the first step
	if (!feedThrough && ssGetT(S) == ssGetTStart(S)) {
		return;
	}

	auto preu = ssGetRWork(S) + 2 * nz(S); // previous inputs

	int iu = 0;

	for (int i = 0; i < nu(S); i++) {

		auto type  = variableType(S, inputPortTypesParam, i);

		const void *y = feedThrough ? ssGetInputPortSignal(S, i) : nullptr;

		for (int j = 0; j < inputPortWidth(S, i); j++) {

			auto vr = valueReference(S, inputPortVariableVRsParam, iu);

			// set the input
			switch (type) {
			case Type::REAL:
				fmu->setReal(vr, feedThrough ? static_cast<const real_T*>(y)[j] : preu[iu]);
				break;
			case Type::INTEGER:
				fmu->setInteger(vr, feedThrough ? static_cast<const int32_T*>(y)[j] : preu[iu]);
				break;
			case Type::BOOLEAN:
				fmu->setBoolean(vr, feedThrough ? static_cast<const boolean_T*>(y)[j] : preu[iu]);
				break;
			default:
				break;
			}

			iu++;
		}
	}

}

/* Set the input derivatives for all real input ports with direct feed-through */
static void setInputDerivatives(SimStruct *S, double h) {

	//if (h <= 0) {
	//	return;
	//}

	auto slave = component<Slave>(S);

	const real_T *preu = ssGetRWork(S) + 2 * nz(S);

	int iu = 0;

	for (int i = 0; i < nu(S); i++) {

		if (!ssGetInputPortDirectFeedThrough(S, i)) {
			continue;
		}

		auto type  = variableType(S, inputPortTypesParam, i);

		if (type != fmikit::REAL) {
			continue;
		}

		const real_T *u = ssGetInputPortRealSignal(S, i);

		for (int j = 0; j < inputPortWidth(S, i); j++) {

			auto vr = valueReference(S, inputPortVariableVRsParam, iu);

			auto du = (u[j] - preu[iu]) / h;

			slave->setRealInputDerivative(vr, 1, du);

			iu++;
		}

	}

}

static void setOutput(SimStruct *S, FMU *fmu) {

	int iy = 0;

	for (int i = 0; i < ny(S); i++) {

		auto type = variableType(S, outputPortTypesParam, i);

		void *y = ssGetOutputPortSignal(S, i);

		for (int j = 0; j < outputPortWidth(S, i); j++) {

			auto vr = valueReference(S, outputPortVariableVRsParam, iy);

			switch (type) {
			case Type::REAL:
				static_cast<real_T *>(y)[j] = fmu->getReal(vr);
				break;
			case Type::INTEGER:
				static_cast<int32_T *>(y)[j] = fmu->getInteger(vr);
				break;
			case Type::BOOLEAN:
				static_cast<boolean_T *>(y)[j] = fmu->getBoolean(vr);
				break;
			default:
				break;
			}

			iy++;
		}
	}

}

static void getLibraryPath(SimStruct *S, char *path) {

#ifdef _WIN32
	strcpy(path, unzipDirectory(S).c_str());
	PathAppend(path, "binaries");

#ifdef _WIN64
	PathAppend(path, "win64");
#else
	PathAppend(path, "win32");
#endif

	PathAppend(path, modelIdentifier(S).c_str());
	PathAddExtension(path, ".dll");
#else
	// TODO
#endif
}

static void setStartValues(SimStruct *S, FMU *fmu) {

    // scalar start values
	for (int i = 0; i < nScalarStartValues(S); i++) {
		auto vr    = valueReference(S, scalarStartVRsParam, i);
		auto type  = variableType(S, scalarStartTypesParam, i);
		auto value = scalarValue(S, scalarStartValuesParam, i);

        switch (type) {
		case Type::REAL:    fmu->setReal(vr, value); break;
		case Type::INTEGER: fmu->setInteger(vr, static_cast<int>(value)); break;
		case Type::BOOLEAN: fmu->setBoolean(vr, value != 0.0); break;
		default: break;
        }
    }

	// string start values
	auto pa     = ssGetSFcnParam(S, stringStartValuesParam);
	auto size   = mxGetNumberOfElements(pa) + 1;
	auto m      = mxGetM(pa);
	auto n      = mxGetN(pa);
	auto buffer = static_cast<char *>(calloc(size, sizeof(char)));
	auto value  = static_cast<char *>(calloc(n + 1, sizeof(char)));

	if (mxGetString(pa, buffer, size) != 0) {
		ssSetErrorStatus(S, "Failed to convert string parameters");
		return;
	}

	for (int i = 0; i < m; i++) {

		// copy the row
		for (int j = 0; j < n; j++) value[j] = buffer[j * m + i];

		// remove the trailing blanks
		for (int j = n - 1; j >= 0; j--) {
			if (value[j] != ' ') break;
			value[j] = '\0';
		}

		auto vr = valueReference(S, stringStartVRsParam, i);

		fmu->setString(vr, value);
	}

	free(buffer);
	free(value);
}

static void update(SimStruct *S) {

	auto fmu = component<FMU>(S);
	auto model = component<Model>(S);

	if (model) {

		double time = fmu->getTime();
		bool upcomingTimeEvent;
		double nextEventTime;

		auto model1 = component<FMU1Model>(S);
		auto model2 = component<FMU2Model>(S);

		if (model1) {
			upcomingTimeEvent = model1->upcomingTimeEvent();
		} else {
			upcomingTimeEvent = model2->nextEventTimeDefined();
		}

		nextEventTime = model->nextEventTime();

		// Work around for the event handling in Dymola FMUs:
		bool timeEvent = time >= nextEventTime;

		if (timeEvent/* && logLevel(S) <= DEBUG*/) {
			logDebug(S, "Time event at t=%.16g", time);
			//ssPrintf("Time event at t=%.16g\n", time);
		}

		bool stepEvent = model->completedIntegratorStep();

		if (stepEvent/* && logLevel(S) <= DEBUG*/) {
			logDebug(S, "Step event at t=%.16g\n", time);
			//ssPrintf("Step event at t=%.16g\n", time);
		}

		bool stateEvent = false;

		if (nz(S) > 0) {
			real_T *prez = ssGetRWork(S);
			real_T *z = prez + nz(S);

			model->getEventIndicators(z, nz(S));

			// check for state events
			for (int i = 0; i < nz(S); i++) {

				bool rising  = (prez[i] < 0 && z[i] >= 0) || (prez[i] == 0 && z[i] > 0);
				bool falling = (prez[i] > 0 && z[i] <= 0) || (prez[i] == 0 && z[i] < 0);

				if (rising || falling) {
					logDebug(S, "State event %s z[%d] at t=%.16g\n", rising ? "-\\+" : "+/-", i, fmu->getTime());
					stateEvent = true;
					// TODO: break?
				}
			}

			// remember the current event indicators
			for (int i = 0; i < nz(S); i++) prez[i] = z[i];
		}

		if (timeEvent || stepEvent || stateEvent) {

			if (model1) {
				model1->eventUpdate();
			} else {
				model2->enterEventMode();

				do {
					model2->newDiscreteStates();
				} while (model2->newDiscreteStatesNeeded() && !model2->terminateSimulation());

				model2->enterContinuousTimeMode();
			}

			if (nx(S) > 0) {
				auto x = ssGetContStates(S);
				model->getContinuousStates(x, nx(S));
			}

			if (nz(S) > 0) {
				auto prez = ssGetRWork(S);
				model->getEventIndicators(prez, nz(S));
			}

			ssSetSolverNeedsReset(S);
		}
	}

}


#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlCheckParameters(SimStruct *S) {

	logDebug(S, "mdlCheckParameters() called on %s", ssGetPath(S));

	if (!mxIsChar(ssGetSFcnParam(S, fmiVersionParam)) || (fmiVersion(S) != "1.0" && fmiVersion(S) != "2.0")) {
        ssSetErrorStatus(S, "Parameter 1 (FMI version) must be one of '1.0' or '2.0'");
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, runAsKindParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, runAsKindParam)) != 1 || (runAsKind(S) != MODEL_EXCHANGE && runAsKind(S) != CO_SIMULATION)) {
        ssSetErrorStatus(S, "Parameter 2 (run as kind) must be one of 0 (= MODEL_EXCHANGE) or 1 (= CO_SIMULATION)");
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, guidParam))) {
        ssSetErrorStatus(S, "Parameter 3 (GUID) must be a string");
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, modelIdentifierParam))) {
        ssSetErrorStatus(S, "Parameter 4 (model identifier) must be a string");
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, unzipDirectoryParam))) {
		ssSetErrorStatus(S, "Parameter 5 (unzip directory) must be a string");
		return;
	}
    
    if (!mxIsNumeric(ssGetSFcnParam(S, debugLoggingParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, logLevelParam)) != 1) {
        ssSetErrorStatus(S, "Parameter 6 (debug logging) must be a scalar");
        return;
    }

    if (!mxIsNumeric(ssGetSFcnParam(S, logFMICallsParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, logLevelParam)) != 1) {
        ssSetErrorStatus(S, "Parameter 7 (log FMI calls) must be a scalar");
        return;
    }
    
    if (!mxIsNumeric(ssGetSFcnParam(S, logLevelParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, logLevelParam)) != 1 || (logLevel(S) != 0 &&logLevel(S) != 1 && logLevel(S) != 2 && logLevel(S) != 3)) {
        ssSetErrorStatus(S, "Parameter 8 (log level) must be one of 0 (= info), 1 (= warning), 2 (= error) or 3 (= none)");
        return;
    }
    
    if (!mxIsChar(ssGetSFcnParam(S, logFileParam))) {
        ssSetErrorStatus(S, "Parameter 9 (log file) must be a string");
        return;
    }

//    if (!mxIsChar(ssGetSFcnParam(S, errorDiagnosticsParam)) || (errorDiagnostics(S) != "ignore" && errorDiagnostics(S) != "warning" && errorDiagnostics(S) != "error")) {
//        ssSetErrorStatus(S, "Parameter 10 (error diagnostics) must be one of 'ignore', 'warning' or 'error'");
//        return;
//    }

	if (!mxIsNumeric(ssGetSFcnParam(S, relativeToleranceParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, relativeToleranceParam)) != 1) {
        ssSetErrorStatus(S, "Parameter 11 (relative tolerance) must be numeric");
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, sampleTimeParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, sampleTimeParam)) != 1) {
        ssSetErrorStatus(S, "Parameter 12 (sample time) must be numeric");
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, offsetTimeParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, offsetTimeParam)) != 1) {
		ssSetErrorStatus(S, "Parameter 13 (offset time) must be numeric");
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, nxParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, nxParam)) != 1) {
        ssSetErrorStatus(S, "Parameter 14 (number of continuous states) must be a scalar");
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, nzParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, nzParam)) != 1) {
        ssSetErrorStatus(S, "Parameter 15 (number of event indicators) must be a scalar");
        return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartTypesParam))) {
    	ssSetErrorStatus(S, "Parameter 16 (scalar start value types) must be a double array");
    	return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartVRsParam))) {
    	ssSetErrorStatus(S, "Parameter 17 (scalar start value references) must be a double array");
    	return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartVRsParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartTypesParam))) {
        ssSetErrorStatus(S, "The number of elements in parameter 13 (scalar start value references) and parameter 17 (scalar start value types) must be equal");
        return;
    }

	// TODO: check VRS values!

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartValuesParam))) {
        ssSetErrorStatus(S, "Parameter 18 (scalar start values) must be a double array");
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartTypesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartValuesParam))) {
        ssSetErrorStatus(S, "The number of elements in parameter 18 (scalar start values) and parameter 12 (scalar start value types) must be equal");
        return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, stringStartVRsParam))) {
        ssSetErrorStatus(S, "Parameter 16 (string start value references) must be a double array");
        return;
    }

	// TODO: check VRS values!

	if (!mxIsChar(ssGetSFcnParam(S, stringStartValuesParam))) {
        ssSetErrorStatus(S, "Parameter 17 (string start values) must be a char matrix");
        return;
    }

	if (mxGetM(ssGetSFcnParam(S, stringStartValuesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, stringStartVRsParam))) {
        ssSetErrorStatus(S, "The number of rows in parameter 17 (string start values) must be equal to the number of elements in parameter 15 (string start value references)");
        return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortWidthsParam))) {
		ssSetErrorStatus(S, "Parameter 18 (input port widths) must be a double array");
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortDirectFeedThroughParam))) {
		ssSetErrorStatus(S, "Parameter 20 (input port direct feed through) must be a double array");
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		ssSetErrorStatus(S, "The number of elements in parameter XX (input port direct feed through) must be equal to the number of elements in parameter XX (inport port widths)");
		return;
	}

	int nu = 0; // number of input variables

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam)); i++) {
		if (inputPortWidth(S, i) < 1) {
			ssSetErrorStatus(S, "Elements in parameter 18 (input port widths) must be >= 1");
			return;
		}
		nu += inputPortWidth(S, i);
	}

	if (!mxIsDouble(ssGetSFcnParam(S, directInputParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, directInputParam)) != 1) {
		ssSetErrorStatus(S, "Parameter 19 (direct input) must be a double scalar");
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		ssSetErrorStatus(S, "Parameter 19 (direct input) must be a doulbe array with the same number of elements as parameter 18 (input port widths)");
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		ssSetErrorStatus(S, "The number of elements in parameter 20 (inport port direct feed through) must be equal to the number of elements in parameter 18 (inport port widths)");
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortTypesParam))) {
		ssSetErrorStatus(S, "Parameter 19 (inport variable types) must be a double array");
		return;
	}

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, inputPortTypesParam)); i++) {
		auto v = static_cast<real_T *>(mxGetData(ssGetSFcnParam(S, inputPortTypesParam)))[i];
		if (v != 0 && v != 1 && v != 2) {
			ssSetErrorStatus(S, "Elements in parameter 19 (input port types) must be one of 0 (= Real), 1 (= Integer) or 2 (= Boolean)");
			return;
		}
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortTypesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		ssSetErrorStatus(S, "The number of elements in parameter 19 (inport port types) must be equal to the number of the elements in parameter 18 (inport port widths)");
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortVariableVRsParam))) {
        ssSetErrorStatus(S, "Parameter 20 (inport variable value references) must be a double array");
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortVariableVRsParam)) != nu) {
		ssSetErrorStatus(S, "The number of elements in parameter 20 (inport variable value references) must be equal to the sum of the elements in parameter 18 (inport port widths)");
		return;
	}

	// TODO: check VRS values!

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortWidthsParam))) {
		ssSetErrorStatus(S, "Parameter 21 (output port widths) must be a double array");
		return;
	}

	int ny = 0; // number of output variables

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); i++) {
		if (outputPortWidth(S, i) < 1) {
			ssSetErrorStatus(S, "Elements in parameter 21 (output port widths) must be >= 1");
			return;
		}
		ny += outputPortWidth(S, i);
	}

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortTypesParam))) {
        ssSetErrorStatus(S, "Parameter 22 (output variable types) must be a double array");
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, outputPortTypesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam))) {
		ssSetErrorStatus(S, "The number of elements in parameter 22 (output port types) must be equal to the number of the elements in parameter 21 (output port widths)");
		return;
	}

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); i++) {
		auto v = variableType(S, outputPortTypesParam, i);
		if (v != 0 && v != 1 && v != 2) { // TODO: check this
			ssSetErrorStatus(S, "Elements in parameter 22 (output variable types) must be one of 0 (= Real), 1 (= Integer) or 2 (= Boolean)");
			return;
		}
	}

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortVariableVRsParam))) {
        ssSetErrorStatus(S, "Parameter 23 (output variable value references) must be a double array");
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, outputPortVariableVRsParam)) != ny) {
		ssSetErrorStatus(S, "The number of elements in parameter 23 (output variable value references) must be equal to the sum of the elements in parameter 21 (output port widths)");
		return;
	}

	// TODO: check VRS values!

	//if (!mxIsLogical(ssGetSFcnParam(S, directInputParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, directInputParam)) != 1) {
	//	ssSetErrorStatus(S, "Parameter 24 (direct input) must be a logical scalar");
	//	return;
	//}

	if (!mxIsDouble(ssGetSFcnParam(S, canInterpolateInputsParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, canInterpolateInputsParam)) != 1) {
		ssSetErrorStatus(S, "Parameter 25 (can interpolate inputs) must be a double scalar");
		return;
	}

}
#endif /* MDL_CHECK_PARAMETERS */


static void mdlInitializeSizes(SimStruct *S) {

	logDebug(S, "mdlInitializeSizes() called on %s", ssGetPath(S));

	ssSetNumSFcnParams(S, numParams);

#if defined(MATLAB_MEX_FILE)
	if (ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S)) {
		mdlCheckParameters(S);
		if (ssGetErrorStatus(S) != NULL) {
			return;
		}
	} else {
		return; // parameter mismatch will be reported by Simulink
	}
#endif

	ssSetNumContStates(S, (runAsKind(S) == MODEL_EXCHANGE) ? nx(S) : 0);
	ssSetNumDiscStates(S, 0);

	if (!ssSetNumInputPorts(S, nu(S))) return;

	for (int i = 0; i < nu(S); i++) {
		ssSetInputPortWidth(S, i, inputPortWidth(S, i));
		ssSetInputPortRequiredContiguous(S, i, 1); // direct input signal access
		DTypeId type = simulinkVariableType(S, inputPortTypesParam, i);
		ssSetInputPortDataType(S, i, type);
		ssSetInputPortDirectFeedThrough(S, i, directInput(S) || inputPortDirectFeedThrough(S, i)); // direct feed through
		logDebug(S, "ssSetInputPortDirectFeedThrough(S, %d, %d) called on %s", i, 1, ssGetPath(S));
	}

	if (!ssSetNumOutputPorts(S, ny(S))) return;

	for (int i = 0; i < ny(S); i++) {
		ssSetOutputPortWidth(S, i, outputPortWidth(S, i));
		DTypeId type = simulinkVariableType(S, outputPortTypesParam, i);
		ssSetOutputPortDataType(S, i, type);
	}

	ssSetNumSampleTimes(S, 1);
	ssSetNumRWork(S, 2 * nz(S) + nuv(S)); // prez & z, preu
	ssSetNumIWork(S, 0);
	ssSetNumPWork(S, 2); // [FMU, logfile]
	ssSetNumModes(S, 3); // [stateEvent, timeEvent, stepEvent]
	ssSetNumNonsampledZCs(S, (runAsKind(S) == MODEL_EXCHANGE) ? nz(S) + 1 : 0);

	// specify the sim state compliance to be same as a built-in block
	//ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);

	ssSetOptions(S, 0);
}


static void mdlInitializeSampleTimes(SimStruct *S) {

	logDebug(S, "mdlInitializeSampleTimes() called on %s", ssGetPath(S));

	if (runAsKind(S) == CO_SIMULATION) {
		ssSetSampleTime(S, 0, sampleTime(S));
		ssSetOffsetTime(S, 0, offsetTime(S));
	} else {
		ssSetSampleTime(S, 0, CONTINUOUS_SAMPLE_TIME);
		ssSetOffsetTime(S, 0, offsetTime(S));
	}

}


#define MDL_START
#if defined(MDL_START)
static void mdlStart(SimStruct *S) {

    void **p = ssGetPWork(S);
    
    if (p[1]) {
        fclose(static_cast<FILE *>(p[1]));
        p[1] = nullptr;
    }
    
    auto logfile = logFile(S);
    
    if (!logfile.empty()) {
        p[1] = fopen(logfile.c_str(), "w");
    }
    
	logDebug(S, "mdlStart() called on %s", ssGetPath(S));

	auto instanceName = ssGetPath(S);
	auto time = ssGetT(S);

	FMU::m_messageLogger = logFMUMessage;

	char libraryFile[1000];
	getLibraryPath(S, libraryFile);

#ifdef _WIN32
	if (!PathFileExists(libraryFile)) {
#ifdef _WIN64
		ssSetErrorStatus(S, "The current platform (Windows 64-bit) is not supported by the FMU");
#else
		ssSetErrorStatus(S, "The current platform (Windows 32-bit) is not supported by the FMU");
#endif
		return;
	}
#endif

	bool toleranceDefined = relativeTolerance(S) > 0;

    bool loggingOn = debugLogging(S);

	if (fmiVersion(S) == "1.0") {

		if (runAsKind(S) == CO_SIMULATION) {
			auto slave = new FMU1Slave(guid(S), modelIdentifier(S), unzipDirectory(S), instanceName, 0.0, loggingOn, calloc, free);
            slave->m_userData = S;
            slave->setLogLevel(logLevel(S));
            if (logFMICalls(S)) slave->m_fmiCallLogger = logFMICall;
			setStartValues(S, slave);
			slave->initializeSlave(time, true, ssGetTFinal(S));
			p[0] = slave;
		} else {
			auto model = new FMU1Model(guid(S), modelIdentifier(S), unzipDirectory(S), instanceName, loggingOn, calloc, free);
            model->m_userData = S;
            model->setLogLevel(logLevel(S));
            if (logFMICalls(S)) model->m_fmiCallLogger = logFMICall;
			setStartValues(S, model);
			model->setTime(time);
			model->initialize(toleranceDefined, relativeTolerance(S));
			if (model->terminateSimulation()) ssSetErrorStatus(S, "Model requested termination at init");
			p[0] = model;
		}

	} else {

		FMU2 *fmu = nullptr;

		if (runAsKind(S) == CO_SIMULATION) {
			fmu = new FMU2Slave(guid(S), modelIdentifier(S), unzipDirectory(S), instanceName, calloc, free);
		} else {
			fmu = new FMU2Model(guid(S), modelIdentifier(S), unzipDirectory(S), instanceName, calloc, free);
		}

        fmu->m_userData = S;
        fmu->setLogLevel(logLevel(S));
		if (logFMICalls(S)) fmu->m_fmiCallLogger = logFMICall;

		fmu->instantiate(loggingOn);
		setStartValues(S, fmu);
		fmu->setupExperiment(toleranceDefined, relativeTolerance(S), time, true, ssGetTFinal(S));
		fmu->enterInitializationMode();
		fmu->exitInitializationMode();

		p[0] = fmu;
	}

}
#endif /* MDL_START */


#define MDL_INITIALIZE_CONDITIONS
#if defined(MDL_INITIALIZE_CONDITIONS)
static void mdlInitializeConditions(SimStruct *S) {

	logDebug(S, "mdlInitializeConditions() called on %s", ssGetPath(S));

	auto model = component<Model>(S);

	if (model) {

		// initialize the continuous states
		auto x = ssGetContStates(S);

		model->getContinuousStates(x, nx(S));
		model->getContinuousStates(x, nx(S));

		// initialize the event indicators
		if (nz(S) > 0) {
			auto prez = ssGetRWork(S);
			auto z = prez + nz(S);

			model->getEventIndicators(prez, nz(S));
			model->getEventIndicators(z, nz(S));
		}
	}
}
#endif


static void mdlOutputs(SimStruct *S, int_T tid) {

	logDebug(S, "mdlOutputs() called on %s (t=%.16g, %s)", ssGetPath(S), ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	auto fmu = component<FMU>(S);

	auto model = component<Model>(S);

	if (model) {

		auto x = ssGetContStates(S);
		auto model2 = component<FMU2Model>(S);

		if (model2 && model2->getState() == EventModeState) {

			if (directInput(S)) {
				setInput(S);
			}

			do {
				model2->newDiscreteStates();
			} while (model2->newDiscreteStatesNeeded() && !model2->terminateSimulation());

			model2->enterContinuousTimeMode();
		}

		if (model2 && model2->getState() != ContinuousTimeModeState) model2->enterContinuousTimeMode();

		model->setTime(ssGetT(S));
		model->setContinuousStates(x, nx(S));

		setInput(S);

		if (ssIsMajorTimeStep(S)) {
			update(S);
		}

	} else {
		time_T h = ssGetT(S) - fmu->getTime();
		auto slave = dynamic_cast<Slave *>(fmu);

		if (h > 0) {

			//setRecordedInput(S);
			setInput(S);

			if (!directInput(S) && canInterpolateInputs(S)) {
				setInputDerivatives(S, h);
			}

			slave->doStep(h);
		}
	}

	setOutput(S, fmu);
}

#define MDL_UPDATE
#if defined(MDL_UPDATE)
static void mdlUpdate(SimStruct *S, int_T tid) {

	logDebug(S, "mdlUpdate() called on %s (t=%.16g, %s)", ssGetPath(S), ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	// record the inputs
	real_T *preu = ssGetRWork(S) + 2 * nz(S);

	int iu = 0;

	for (int i = 0; i < nu(S); i++) {

		auto type = variableType(S, inputPortTypesParam, i);

		const void *u = ssGetInputPortSignal(S, i);

		for (int j = 0; j < inputPortWidth(S, i); j++) {

			auto vr = valueReference(S, inputPortVariableVRsParam, iu);

			switch (type) {
			case Type::REAL:
				preu[iu] = static_cast<const real_T*>(u)[j];
				break;
			case Type::INTEGER:
				preu[iu] = static_cast<const int32_T*>(u)[j];
				break;
			case Type::BOOLEAN:
				preu[iu] = static_cast<const boolean_T*>(u)[j];
				break;
			default:
				break;
			}

			iu++;
		}
	}

}
#endif // MDL_UPDATE


#define MDL_ZERO_CROSSINGS
#if defined(MDL_ZERO_CROSSINGS) && (defined(MATLAB_MEX_FILE) || defined(NRT))
static void mdlZeroCrossings(SimStruct *S) {

	logDebug(S, "mdlZeroCrossings() called on %s (t=%.16g, %s)", ssGetPath(S), ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	auto model = component<Model>(S);

	if (model) {

		auto z = ssGetNonsampledZCs(S);

		if (nz(S) > 0) {
			model->getEventIndicators(z, nz(S));
		}

		z[nz(S)] = model->nextEventTime() - ssGetT(S);
	}
}
#endif


#define MDL_DERIVATIVES
#if defined(MDL_DERIVATIVES)
static void mdlDerivatives(SimStruct *S) {

	logDebug(S, "mdlDerivatives() called on %s (t=%.16g, %s)", ssGetPath(S), ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	auto model = component<Model>(S);

	if (model) {
		auto x = ssGetContStates(S);
		auto dx = ssGetdX(S);

		model->getContinuousStates(x, nx(S));
		model->getDerivatives(dx, nx(S));
	}
}
#endif


static void mdlTerminate(SimStruct *S) {

	logDebug(S, "mdlTerminate() called on %s", ssGetPath(S));

	delete component<FMU>(S);
}

/*=============================*
* Required S-function trailer *
*=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
