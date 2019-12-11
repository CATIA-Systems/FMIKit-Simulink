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

#include <stdio.h>
#include <stdarg.h>
#include <string>

extern "C" {
#include "simstruc.h"

#ifdef GRTFMI
extern const char *FMU_RESOURCES_DIR;
#endif
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
	inputPortDirectFeedThroughParam,
	inputPortTypesParam,
	inputPortVariableVRsParam,
	outputPortWidthsParam,
	outputPortTypesParam,
	outputPortVariableVRsParam,
	numParams

};

static string getStringParam(SimStruct *S, int index) {

	const mxArray *pa = ssGetSFcnParam(S, index);

	auto n = mxGetN(pa);
	auto m = mxGetM(pa);

	// TODO: assert m == 1

	if (n < 1) return "";

	auto data = static_cast<const mxChar*>(mxGetData(pa));

	if (!data) return "";

	auto cstr = static_cast<char *>(mxMalloc(n));

	// convert real_T to ASCII char
	for (int i = 0; i < n; i++) {
		// TODO: assert 0 <= data[i] <= 127
		cstr[i] = data[i];
	}

	string cppstr(cstr, n);
	mxFree(cstr);
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

static void setInput(SimStruct *S, bool direct) {

	auto fmu = component<FMU>(S);

	int iu = 0;

	for (int i = 0; i < nu(S); i++) {

		if (direct && !inputPortDirectFeedThrough(S, i)) continue;

		auto type = variableType(S, inputPortTypesParam, i);

		const void *y = ssGetInputPortSignal(S, i);

		for (int j = 0; j < inputPortWidth(S, i); j++) {

			auto vr = valueReference(S, inputPortVariableVRsParam, iu);

			// set the input
			switch (type) {
			case Type::REAL:
				fmu->setReal(vr, static_cast<const real_T*>(y)[j]);
				break;
			case Type::INTEGER:
				fmu->setInteger(vr, static_cast<const int32_T*>(y)[j]);
				break;
			case Type::BOOLEAN:
				fmu->setBoolean(vr, static_cast<const boolean_T*>(y)[j]);
				break;
			default:
				break;
			}

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

	//if (mxGetString(pa, buffer, size) != 0) {
	//	ssSetErrorStatus(S, "Failed to convert string parameters");
	//	return;
	//}

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

static void setErrorStatus(SimStruct *S, const char *message, ...) {
	va_list args;
	va_start(args, message);
	static char msg[1024];
	vsnprintf(msg, 1024, message, args);
	ssSetErrorStatus(S, msg);
	va_end(args);
}

#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
static void mdlCheckParameters(SimStruct *S) {

	logDebug(S, "mdlCheckParameters() called on %s", ssGetPath(S));

	if (!mxIsChar(ssGetSFcnParam(S, fmiVersionParam)) || (fmiVersion(S) != "1.0" && fmiVersion(S) != "2.0")) {
        setErrorStatus(S, "Parameter %d (FMI version) must be one of '1.0' or '2.0'", fmiVersionParam + 1);
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, runAsKindParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, runAsKindParam)) != 1 || (runAsKind(S) != MODEL_EXCHANGE && runAsKind(S) != CO_SIMULATION)) {
        setErrorStatus(S, "Parameter %d (run as kind) must be one of 0 (= MODEL_EXCHANGE) or 1 (= CO_SIMULATION)", runAsKindParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, guidParam))) {
        setErrorStatus(S, "Parameter %d (GUID) must be a string", guidParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, modelIdentifierParam))) {
        setErrorStatus(S, "Parameter %d (model identifier) must be a string", modelIdentifierParam + 1);
        return;
    }

	if (!mxIsChar(ssGetSFcnParam(S, unzipDirectoryParam))) {
		setErrorStatus(S, "Parameter %d (unzip directory) must be a string", unzipDirectoryParam + 1);
		return;
	}

    if (!mxIsNumeric(ssGetSFcnParam(S, debugLoggingParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, debugLoggingParam)) != 1) {
        setErrorStatus(S, "Parameter %d (debug logging) must be a scalar", debugLoggingParam + 1);
        return;
    }

    if (!mxIsNumeric(ssGetSFcnParam(S, logFMICallsParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, logFMICallsParam)) != 1) {
        setErrorStatus(S, "Parameter %d (log FMI calls) must be a scalar", logFMICallsParam + 1);
        return;
    }

    if (!mxIsNumeric(ssGetSFcnParam(S, logLevelParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, logLevelParam)) != 1 ||
		(logLevel(S) != 0 && logLevel(S) != 1 && logLevel(S) != 2 && logLevel(S) != 3 && logLevel(S) != 4 && logLevel(S) != 5)) {
        setErrorStatus(S, "Parameter %d (log level) must be one of 0 (= info), 1 (= warning), 2 (= discard), 3 (= error), 4 (= fatal) or 5 (= none)", logLevelParam + 1);
        return;
    }

    if (!mxIsChar(ssGetSFcnParam(S, logFileParam))) {
        setErrorStatus(S, "Parameter %d (log file) must be a string", logFileParam + 1);
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, relativeToleranceParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, relativeToleranceParam)) != 1) {
        setErrorStatus(S, "Parameter %d (relative tolerance) must be numeric", relativeToleranceParam + 1);
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, sampleTimeParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, sampleTimeParam)) != 1) {
        setErrorStatus(S, "Parameter %d (sample time) must be numeric", sampleTimeParam + 1);
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, offsetTimeParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, offsetTimeParam)) != 1) {
		setErrorStatus(S, "Parameter %d (offset time) must be numeric", offsetTimeParam + 1);
		return;
	}

	if (!mxIsNumeric(ssGetSFcnParam(S, nxParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, nxParam)) != 1) {
        setErrorStatus(S, "Parameter %d (number of continuous states) must be a scalar", nxParam + 1);
        return;
    }

	if (!mxIsNumeric(ssGetSFcnParam(S, nzParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, nzParam)) != 1) {
        setErrorStatus(S, "Parameter %d (number of event indicators) must be a scalar", nzParam + 1);
        return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartTypesParam))) {
    	setErrorStatus(S, "Parameter %d (scalar start value types) must be a double array", scalarStartTypesParam + 1);
    	return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartVRsParam))) {
    	setErrorStatus(S, "Parameter %d (scalar start value references) must be a double array", scalarStartVRsParam + 1);
    	return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartVRsParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartTypesParam))) {
        setErrorStatus(S, "The number of elements in parameter %d (scalar start value references) and parameter %d (scalar start value types) must be equal", scalarStartVRsParam + 1, scalarStartTypesParam + 1);
        return;
    }

	// TODO: check VRS values!

	if (!mxIsDouble(ssGetSFcnParam(S, scalarStartValuesParam))) {
        setErrorStatus(S, "Parameter %d (scalar start values) must be a double array", scalarStartValuesParam + 1);
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartValuesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, scalarStartTypesParam))) {
        setErrorStatus(S, "The number of elements in parameter %d (scalar start values) and parameter %d (scalar start value types) must be equal", scalarStartValuesParam + 1, scalarStartTypesParam + 1);
        return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, stringStartVRsParam))) {
        setErrorStatus(S, "Parameter %d (string start value references) must be a double array", stringStartVRsParam + 1);
        return;
    }

	// TODO: check VRS values!

	if (!mxIsChar(ssGetSFcnParam(S, stringStartValuesParam))) {
        setErrorStatus(S, "Parameter %d (string start values) must be a char matrix", stringStartValuesParam + 1);
        return;
    }

	if (mxGetM(ssGetSFcnParam(S, stringStartValuesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, stringStartVRsParam))) {
        setErrorStatus(S, "The number of rows in parameter %d (string start values) must be equal to the number of elements in parameter %d (string start value references)", stringStartValuesParam + 1, stringStartVRsParam + 1);
        return;
    }

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "Parameter %d (input port widths) must be a double array", inputPortWidthsParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortDirectFeedThroughParam))) {
		setErrorStatus(S, "Parameter %d (input port direct feed through) must be a double array", inputPortDirectFeedThroughParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (input port direct feed through) must be equal to the number of elements in parameter %d (inport port widths)", inputPortDirectFeedThroughParam + 1, inputPortWidthsParam + 1);
		return;
	}

	int nu = 0; // number of input variables

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam)); i++) {
		if (inputPortWidth(S, i) < 1) {
			setErrorStatus(S, "Elements in parameter %d (input port widths) must be >= 1", inputPortWidthsParam + 1);
			return;
		}
		nu += inputPortWidth(S, i);
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) || mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "Parameter %d (input port direct feed through) must be a double array with the same number of elements as parameter %d (input port widths)", inputPortDirectFeedThroughParam + 1, inputPortWidthsParam + 1);
		return;
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortDirectFeedThroughParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (inport port direct feed through) must be equal to the number of elements in parameter %d (inport port widths)", inputPortDirectFeedThroughParam + 1, inputPortWidthsParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortTypesParam))) {
		setErrorStatus(S, "Parameter %d (input port variable types) must be a double array", inputPortTypesParam + 1);
		return;
	}

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, inputPortTypesParam)); i++) {
		auto v = static_cast<real_T *>(mxGetData(ssGetSFcnParam(S, inputPortTypesParam)))[i];
		if (v != 0 && v != 1 && v != 2) {
			setErrorStatus(S, "Elements in parameter %d (input port types) must be one of 0 (= Real), 1 (= Integer) or 2 (= Boolean)", inputPortTypesParam + 1);
			return;
		}
	}

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortTypesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, inputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (inport port types) must be equal to the number of the elements in parameter %d (inport port widths)", inputPortTypesParam + 1, inputPortWidthsParam + 1);
		return;
	}

	if (!mxIsDouble(ssGetSFcnParam(S, inputPortVariableVRsParam))) {
        setErrorStatus(S, "Parameter %d (input port value references) must be a double array", inputPortVariableVRsParam + 1);
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, inputPortVariableVRsParam)) != nu) {
		setErrorStatus(S, "The number of elements in parameter %d (input port value references) must be equal to the sum of the elements in parameter %d (inport port widths)", inputPortVariableVRsParam + 1, inputPortWidthsParam + 1);
		return;
	}

	// TODO: check VRS values!

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortWidthsParam))) {
		setErrorStatus(S, "Parameter %d (output port widths) must be a double array", outputPortWidthsParam + 1);
		return;
	}

	int ny = 0; // number of output variables

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); i++) {
		if (outputPortWidth(S, i) < 1) {
			setErrorStatus(S, "Elements in parameter %d (output port widths) must be >= 1", outputPortWidthsParam + 1);
			return;
		}
		ny += outputPortWidth(S, i);
	}

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortTypesParam))) {
        setErrorStatus(S, "Parameter %d (output port types) must be a double array", outputPortTypesParam + 1);
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, outputPortTypesParam)) != mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam))) {
		setErrorStatus(S, "The number of elements in parameter %d (output port types) must be equal to the number of the elements in parameter %d (output port widths)", outputPortTypesParam + 1, outputPortWidthsParam + 1);
		return;
	}

	for (int i = 0; i < mxGetNumberOfElements(ssGetSFcnParam(S, outputPortWidthsParam)); i++) {
		auto v = variableType(S, outputPortTypesParam, i);
		if (v != 0 && v != 1 && v != 2) { // TODO: check this
			setErrorStatus(S, "Elements in parameter %d (output port types) must be one of 0 (= Real), 1 (= Integer) or 2 (= Boolean)", outputPortTypesParam + 1);
			return;
		}
	}

	if (!mxIsDouble(ssGetSFcnParam(S, outputPortVariableVRsParam))) {
        setErrorStatus(S, "Parameter %d (output variable value references) must be a double array", outputPortVariableVRsParam + 1);
        return;
    }

	if (mxGetNumberOfElements(ssGetSFcnParam(S, outputPortVariableVRsParam)) != ny) {
		setErrorStatus(S, "The number of elements in parameter %d (output variable value references) must be equal to the sum of the elements in parameter %d (output port widths)", outputPortVariableVRsParam + 1, outputPortWidthsParam + 1);
		return;
	}

	// TODO: check VRS values!

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
		ssSetInputPortDirectFeedThrough(S, i, inputPortDirectFeedThrough(S, i)); // direct feed through
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

#ifdef GRTFMI
	auto unzipdir = FMU_RESOURCES_DIR + string("/") + modelIdentifier(S);
#else
	auto unzipdir = unzipDirectory(S);
#endif

	if (fmiVersion(S) == "1.0") {

		if (runAsKind(S) == CO_SIMULATION) {
			auto slave = new FMU1Slave(guid(S), modelIdentifier(S), unzipDirectory(S), instanceName);
            slave->m_userData = S;
            slave->setLogLevel(logLevel(S));
            if (logFMICalls(S)) slave->m_fmiCallLogger = logFMICall;
            slave->instantiateSlave(unzipDirectory(S), 0, loggingOn);
			setStartValues(S, slave);
			slave->initializeSlave(time, true, ssGetTFinal(S));
			p[0] = slave;
		} else {
			auto model = new FMU1Model(guid(S), modelIdentifier(S), unzipDirectory(S), instanceName);
            model->m_userData = S;
            model->setLogLevel(logLevel(S));
            if (logFMICalls(S)) model->m_fmiCallLogger = logFMICall;
            model->instantiateModel(loggingOn);
			setStartValues(S, model);
			model->setTime(time);
			model->initialize(toleranceDefined, relativeTolerance(S));
			if (model->terminateSimulation()) ssSetErrorStatus(S, "Model requested termination at init");
			p[0] = model;
		}

	} else {

		FMU2 *fmu = nullptr;

		if (runAsKind(S) == CO_SIMULATION) {
			fmu = new FMU2Slave(guid(S), modelIdentifier(S), unzipdir, instanceName);
		} else {
			fmu = new FMU2Model(guid(S), modelIdentifier(S), unzipdir, instanceName);
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

			setInput(S, true);

			do {
				model2->newDiscreteStates();
			} while (model2->newDiscreteStatesNeeded() && !model2->terminateSimulation());

			model2->enterContinuousTimeMode();
		}

		if (model2 && model2->getState() != ContinuousTimeModeState) model2->enterContinuousTimeMode();

		model->setTime(ssGetT(S));
		model->setContinuousStates(x, nx(S));

		setInput(S, true);

		if (ssIsMajorTimeStep(S)) {
			update(S);
		}

	} else {
		time_T h = ssGetT(S) - fmu->getTime();
		auto slave = dynamic_cast<Slave *>(fmu);

		if (h > 0) {
			slave->doStep(h);
		}
	}

	setOutput(S, fmu);
}

#define MDL_UPDATE
#if defined(MDL_UPDATE)
static void mdlUpdate(SimStruct *S, int_T tid) {

	logDebug(S, "mdlUpdate() called on %s (t=%.16g, %s)", ssGetPath(S), ssGetT(S), ssIsMajorTimeStep(S) ? "major" : "minor");

	setInput(S, false);
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
