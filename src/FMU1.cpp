/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#include <iostream>

#include "FMU1.h"

#include <sstream>
#include <iomanip>
#include <stdarg.h>

#ifndef _WIN32
	#include <dlfcn.h>
	#define HMODULE void*
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap) {
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...) {
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#endif

using namespace std;

// ValueReferenc array to debug text
static void appendValueReferences(std::stringstream &ss, const fmikit::ValueReference vr[], size_t nvr) {
	for (size_t i = 0; i < nvr; i++) {
		ss << vr[i];
		if (i < nvr - 1) ss << ", ";
	}
}

static void appendDoubles(std::stringstream &ss, const double values[], size_t nvalues) {
	for (size_t i = 0; i < nvalues; i++) {
		ss << values[i];
		if (i < nvalues - 1) ss << ", ";
	}
}


namespace fmikit {

	// bool to string
	static const char *btoa(bool value) {
		return value ? "true" : "false";
	}

	// double array to debug text
	static void doubleArrayToString(std::string &dst, const double values[], size_t size) {
		for (size_t i = 0; i < size; i++) {
			char buf[128];
            snprintf(buf, 128, "%.16g", values[i]);
			dst.append(buf);
			if (i < size - 1) dst.append(", ");
		}
	}

	static const char *fmi1BooleanToString(fmi1Boolean value) {
		return value ? "true" : "false";
	}
    
    FMU1* FMU1::s_currentInstance = nullptr;

	void FMU1::logFMU1Message(fmi1Component c, fmi1String instanceName, fmi1Status status, fmi1String category, fmi1String message, ...) {
        va_list args;
        va_start(args, message);
        
        auto level = static_cast<LogLevel>(status);
        
        if (level >= s_currentInstance->logLevel()) {
            logFMUMessage(s_currentInstance, level, category, message, args);
        }
        
        va_end(args);
	}

	FMU1::FMU1(const std::string &guid,
               const std::string &modelIdentifier,
               const std::string &unzipDirectory,
               const std::string &instanceName,
               allocateMemoryCallback *allocateMemory,
               freeMemoryCallback *freeMemory) :
		FMU(guid, modelIdentifier, unzipDirectory, instanceName),
		m_component(nullptr) {

        s_currentInstance = this;

        m_fmiVersion = FMI_VERSION_1;

		fmi1GetVersion      = getFunc<fmi1GetVersionTYPE>      ("fmiGetVersion");
		fmi1GetReal         = getFunc<fmi1GetRealTYPE>         ("fmiGetReal");
		fmi1GetInteger      = getFunc<fmi1GetIntegerTYPE>      ("fmiGetInteger");
		fmi1GetBoolean      = getFunc<fmi1GetBooleanTYPE>      ("fmiGetBoolean");
		fmi1GetString       = getFunc<fmi1GetStringTYPE>       ("fmiGetString");
		fmi1SetDebugLogging = getFunc<fmi1SetDebugLoggingTYPE> ("fmiSetDebugLogging");
		fmi1SetReal         = getFunc<fmi1SetRealTYPE>         ("fmiSetReal");
		fmi1SetInteger      = getFunc<fmi1SetIntegerTYPE>      ("fmiSetInteger");
		fmi1SetBoolean      = getFunc<fmi1SetBooleanTYPE>      ("fmiSetBoolean");
		fmi1SetString       = getFunc<fmi1SetStringTYPE>       ("fmiSetString");

		m_callbackFunctions.logger         = logFMU1Message;
		m_callbackFunctions.allocateMemory = allocateMemory ? allocateMemory : calloc;
		m_callbackFunctions.freeMemory     = freeMemory ? freeMemory : free;
		m_callbackFunctions.stepFinished   = nullptr;
	}

	FMU1::~FMU1() {}

	void FMU1::assertNoError(fmi1Status status, const char *message) {
		if (status >= fmi1Error) return;
	}

	double FMU1::getReal(const ValueReference vr) {
        s_currentInstance = this;
		fmi1Real value;
		ASSERT_NO_ERROR(fmi1GetReal(m_component, &vr, 1, &value), "Failed to get Real")
		logDebug("fmi1GetReal(vr=[%d], nvr=1): value=[%.16g]", vr, value);
		return value;
	}

	int FMU1::getInteger(ValueReference vr) {
        s_currentInstance = this;
		fmi1Integer value;
		ASSERT_NO_ERROR(fmi1GetInteger(m_component, &vr, 1, &value), "Failed to get Integer")
		logDebug("fmi1GetInteger(vr=[%d], nvr=1): value=[%d]", vr, value);
		return value;
	}

	bool FMU1::getBoolean(ValueReference vr) {
        s_currentInstance = this;
		fmi1Boolean value;
		ASSERT_NO_ERROR(fmi1GetBoolean(m_component, &vr, 1, &value), "Failed to get Boolean")
		logDebug("fmi1GetBoolean(vr=[%d], nvr=1): value=[%d]", vr, value);
		return value != fmi1False;
	}

	void FMU1::getCString(ValueReference vr, char *value) {
		ASSERT_NO_ERROR(fmi1GetString(m_component, &vr, 1, const_cast<fmi1String *>(&value)), "Failed to get String")
	}

	string FMU1::getString(ValueReference vr) {
        s_currentInstance = this;
		char *value[1] = { nullptr };
		getCString(vr, value[0]);
		logDebug("fmi1GetString(vr=[%d], nvr=1): value=[\"%s\"]", vr, value[0]);
		// TODO: return value[0]
		return "";
	}

	void FMU1::setReal(const ValueReference vr, double value) {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1SetReal(m_component, &vr, 1, &value), "Failed to set Real");
		logDebug("fmi1SetReal(vr=[%d], nvr=1, value=[%.16g])", vr, value);
	}

	void FMU1::setInteger(ValueReference vr, int value) {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1SetInteger(m_component, &vr, 1, &value), "Failed to set Integer value");
		logDebug("fmi1SetInteger(vr=[%d], nvr=1, value=[%d])", vr, value);
	}

	void FMU1::setBoolean(ValueReference vr, bool value) {
        s_currentInstance = this;
		fmi1Boolean v = value ? fmi1True : fmi1False;
		ASSERT_NO_ERROR(fmi1SetBoolean(m_component, &vr, 1, &v), "Failed to set Boolean value");
		logDebug("fmi1SetBoolean(vr=[%d], nvr=1, value=[%d])", vr, v);
	}


	void FMU1::setCString(ValueReference vr, const char *value) {
		ASSERT_NO_ERROR(fmi1SetString(m_component, &vr, 1, &value), "Failed to set String")
	}

	void FMU1::setString(ValueReference vr, string value) {
        s_currentInstance = this;
		fmi1String s = value.c_str();
		setCString(vr, s);
		logDebug("fmi1SetString(vr=[%d], nvr=1, value=[\"%s\"])", vr, s);
	}

	FMU1Slave::FMU1Slave(const std::string &guid,
						const std::string &modelIdentifier,
						const std::string &unzipDirectory,
						const std::string &instanceName,
						allocateMemoryCallback *allocateMemory,
						freeMemoryCallback *freeMemory) :
		FMU1(guid, modelIdentifier, unzipDirectory, instanceName, allocateMemory, freeMemory) {

		m_kind = CO_SIMULATION;

		fmi1GetTypesPlatform         = getFunc<fmi1GetTypesPlatformTYPE>         ("fmiGetTypesPlatform");
		fmi1InstantiateSlave         = getFunc<fmi1InstantiateSlaveTYPE>         ("fmiInstantiateSlave");
		fmi1InitializeSlave          = getFunc<fmi1InitializeSlaveTYPE>          ("fmiInitializeSlave");
		fmi1TerminateSlave           = getFunc<fmi1TerminateSlaveTYPE>           ("fmiTerminateSlave");
		fmi1ResetSlave               = getFunc<fmi1ResetSlaveTYPE>               ("fmiResetSlave");
		fmi1FreeSlaveInstance        = getFunc<fmi1FreeSlaveInstanceTYPE>        ("fmiFreeSlaveInstance");
		fmi1SetRealInputDerivatives  = getFunc<fmi1SetRealInputDerivativesTYPE>  ("fmiSetRealInputDerivatives");
		fmi1GetRealOutputDerivatives = getFunc<fmi1GetRealOutputDerivativesTYPE> ("fmiGetRealOutputDerivatives");
		fmi1CancelStep               = getFunc<fmi1CancelStepTYPE>               ("fmiCancelStep");
		fmi1DoStep                   = getFunc<fmi1DoStepTYPE>                   ("fmiDoStep");
		fmi1GetStatus                = getFunc<fmi1GetStatusTYPE>                ("fmiGetStatus");
		fmi1GetRealStatus            = getFunc<fmi1GetRealStatusTYPE>            ("fmiGetRealStatus");
		fmi1GetIntegerStatus         = getFunc<fmi1GetIntegerStatusTYPE>         ("fmiGetIntegerStatus");
		fmi1GetBooleanStatus         = getFunc<fmi1GetBooleanStatusTYPE>         ("fmiGetBooleanStatus");
		fmi1GetStringStatus          = getFunc<fmi1GetStringStatusTYPE>          ("fmiGetStringStatus");
	}

    void FMU1Slave::instantiateSlave(const std::string &fmuLocation, double timeout, bool loggingOn) {
        instantiateSlave_(instanceName().c_str(), guid().c_str(), fmuLocation.c_str(), "application/x-fmu-sharedlibrary", timeout, fmi1False, fmi1False, m_callbackFunctions, loggingOn);
    }

	void FMU1Slave::instantiateSlave_(fmi1String  instanceName, fmi1String  fmuGUID, fmi1String  fmuLocation, fmi1String  mimeType, fmi1Real timeout, fmi1Boolean visible, fmi1Boolean interactive, fmi1CallbackFunctions functions, fmi1Boolean loggingOn) {
		HANDLE_EXCEPTION(m_component = fmi1InstantiateSlave(instanceName, fmuGUID, fmuLocation, mimeType, timeout, visible, interactive, m_callbackFunctions, loggingOn), "Failed to instantiate slave")
		logDebug("fmi1InstantiateSlave(instanceName=\"%s\", fmuGUID=\"%s\", fmuLocation=\"%s\", mimeType=\"%s\", timeout=%.16g, visible=visible, interactive=interactive, functions=0x%p, loggingOn=%d)",
		instanceName, fmuGUID, fmuLocation, mimeType, timeout, visible, interactive, functions, loggingOn);
        if (!m_component) error("Failed to instantiate slave");
	}

	void FMU1Slave::terminateSlave() {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1TerminateSlave(m_component), "Failed to terminate slave")
		logDebug("fmi1TerminateSlave()");
	}

	void FMU1Slave::freeSlaveInstance() {
        s_currentInstance = this;
		HANDLE_EXCEPTION(fmi1FreeSlaveInstance(m_component), "Failed to terminate slave")
		logDebug("fmi1FreeSlaveInstance()");
	}

	FMU1Slave::~FMU1Slave() {
        s_currentInstance = this;
		terminateSlave();
		freeSlaveInstance();
	}

	void FMU1Slave::initializeSlave(double startTime, bool stopTimeDefined, double stopTime) {
        s_currentInstance = this;
		m_time = startTime;
		this->m_stopTimeDefined = stopTimeDefined;
		this->m_stopTime = stopTime;
		ASSERT_NO_ERROR(fmi1InitializeSlave(m_component, m_time, stopTimeDefined, stopTime), "Failed to initialize slave")
		logDebug("fmi1InitializeSlave(startTime=%.16g, stopTimeDefined=%s, stopTime=%.16g)", startTime, btoa(stopTimeDefined), stopTime);
	}

	void FMU1Slave::doStep(double h) {
        s_currentInstance = this;
		if (m_stopTimeDefined && m_time + h > m_stopTime - h / 1000) {
			h = m_stopTime - m_time;
		}
		ASSERT_NO_ERROR(fmi1DoStep(m_component, m_time, h, fmi1True), "Failed to do step")
		logDebug("fmi1DoStep(currentCommunicationPoint=%.16g, communicationStepSize=%.16g, newStep=fmi1True)", m_time, h);
		m_time += h;
	}

	void FMU1Slave::setRealInputDerivative(ValueReference vr, int order, double value) {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1SetRealInputDerivatives(m_component, &vr, 1, &order, &value), "Failed to set real input derivatives")
		logDebug("fmi1SetRealInputDerivatives(component, vr=[%d], nvr=1, order=[%d], value=[%.16g])", vr, order, value);
	}

	FMU1Model::FMU1Model(const std::string &guid,
		const std::string &modelIdentifier,
		const std::string &unzipDirectory,
		const std::string &instanceName,
		allocateMemoryCallback *allocateMemory,
		freeMemoryCallback *freeMemory) :
		FMU1(guid, modelIdentifier, unzipDirectory, instanceName, allocateMemory, freeMemory) {

		m_kind = MODEL_EXCHANGE;

		m_eventInfo.iterationConverged          = fmi1False;
		m_eventInfo.stateValueReferencesChanged = fmi1False;
		m_eventInfo.stateValuesChanged          = fmi1False;
		m_eventInfo.terminateSimulation         = fmi1False;
		m_eventInfo.upcomingTimeEvent           = fmi1False;
		m_eventInfo.nextEventTime               = 0.0;

		fmi1GetModelTypesPlatform       = getFunc<fmi1GetModelTypesPlatformTYPE>      ("fmiGetModelTypesPlatform");
		fmi1InstantiateModel            = getFunc<fmi1InstantiateModelTYPE>           ("fmiInstantiateModel");
		fmi1FreeModelInstance           = getFunc<fmi1FreeModelInstanceTYPE>          ("fmiFreeModelInstance");
		fmi1SetTime                     = getFunc<fmi1SetTimeTYPE>                    ("fmiSetTime");
		fmi1SetContinuousStates         = getFunc<fmi1SetContinuousStatesTYPE>        ("fmiSetContinuousStates");
		fmi1CompletedIntegratorStep     = getFunc<fmi1CompletedIntegratorStepTYPE>    ("fmiCompletedIntegratorStep");
		fmi1Initialize                  = getFunc<fmi1InitializeTYPE>                 ("fmiInitialize");
		fmi1GetDerivatives              = getFunc<fmi1GetDerivativesTYPE>             ("fmiGetDerivatives");
		fmi1GetEventIndicators          = getFunc<fmi1GetEventIndicatorsTYPE>         ("fmiGetEventIndicators");
		fmi1EventUpdate                 = getFunc<fmi1EventUpdateTYPE>                ("fmiEventUpdate");
		fmi1GetContinuousStates         = getFunc<fmi1GetContinuousStatesTYPE>        ("fmiGetContinuousStates");
		fmi1GetNominalContinuousStates  = getFunc<fmi1GetNominalContinuousStatesTYPE> ("fmiGetNominalContinuousStates");
		fmi1GetStateValueReferences     = getFunc<fmi1GetStateValueReferencesTYPE>    ("fmiGetStateValueReferences");
		fmi1Terminate                   = getFunc<fmi1TerminateTYPE>                  ("fmiTerminate");
	}
    
    void FMU1Model::instantiateModel(bool loggingOn) {
        instantiateModel_(instanceName().c_str(), guid().c_str(), m_callbackFunctions, loggingOn);
    }

	void FMU1Model::instantiateModel_(fmi1String instanceName, fmi1String GUID, fmi1CallbackFunctions functions, fmi1Boolean loggingOn) {
		HANDLE_EXCEPTION(m_component = fmi1InstantiateModel(instanceName, GUID, m_callbackFunctions, loggingOn), "Failed to instantiate model")
		logDebug("fmi1InstantiateModel(instanceName=\"%s\", GUID=\"%s\", loggingOn=%d): component=0x%p", instanceName, GUID, loggingOn, m_component);
        if (!m_component) error("Failed to instantiate model");
	}

	void FMU1Model::terminate() {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1Terminate(m_component), "Failed to terminate");
		logDebug("fmi1Terminate()");
	}

	void FMU1Model::freeModelInstance() {
        s_currentInstance = this;
		HANDLE_EXCEPTION(fmi1FreeModelInstance(m_component), "Failed to free model instance")
		logDebug("fmi1FreeModelInstance()");
	}

	FMU1Model::~FMU1Model() {
        s_currentInstance = this;
		terminate();
		freeModelInstance();
	}

	void FMU1Model::initialize(bool toleranceControlled, double relativeTolerance) {
        s_currentInstance = this;
		logDebug("fmi1Initialize(toleranceControlled=%s, relativeTolerance=%.16g)", btoa(toleranceControlled), relativeTolerance);
		fmi1Initialize(m_component, toleranceControlled, relativeTolerance, &m_eventInfo);
	}

	void FMU1Model::setTime(double time) {
        s_currentInstance = this;
		logDebug("fmi1SetTime(time=%.16g)", time);
		ASSERT_NO_ERROR(fmi1SetTime(m_component, time), "Failed to set time")
		this->m_time = time;
	}

	void FMU1Model::setContinuousStates(const double states[], size_t size) {
        s_currentInstance = this;
		if (size < 1) return; // nothing to do
		logDebug("fmi1SetContinuousStates(states=[...], size=%d)", size);
		ASSERT_NO_ERROR(fmi1SetContinuousStates(m_component, states, size), "Failed to set continuous states")
	}

	void FMU1Model::getContinuousStates(double states[], size_t size) {
        s_currentInstance = this;
		if (size < 1) return; // nothing to do
		ASSERT_NO_ERROR(fmi1GetContinuousStates(m_component, states, size), "Failed to get continuous states")
		logDebug("fmi1GetContinuousStates(size=%d): states=[...]", size);
	}

	void FMU1Model::getNominalContinuousStates(double states[], size_t size) {
        s_currentInstance = this;
		if (size < 1) return; // nothing to do
		ASSERT_NO_ERROR(fmi1GetNominalContinuousStates(m_component, states, size), "Failed to get nominal continuous states")
			logDebug("fmi1GetNominalContinuousStates(size=%d): states=[...]", size);
	}

	void FMU1Model::getDerivatives(double derivatives[], size_t size) {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1GetDerivatives(m_component, derivatives, size), "Failed to get derivatives")
		logDebug("fmi1GetDerivatives(size=%d): derivatives=[...]", size);
	}

	bool FMU1Model::completedIntegratorStep() {
        s_currentInstance = this;
		fmi1Boolean stepEvent;
		ASSERT_NO_ERROR(fmi1CompletedIntegratorStep(m_component, &stepEvent), "Failed to complete integrator step")
		logDebug("fmi1CompletedIntegratorStep(): stepEvent=%s", fmi1BooleanToString(stepEvent));
		return stepEvent != fmi1False;
	}

	void FMU1Model::eventUpdate() {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1EventUpdate(m_component, fmi1False, &m_eventInfo), "Event update failed")
		logDebug("fmi1EventUpdate(intermediateResults=false): "
				"eventInfo.iterationConverged=%s, "
				"eventInfo.stateValueReferencesChanged=%s, "
				"eventInfo.stateValuesChanged=%s, "
				"eventInfo.terminateSimulation=%s, "
				"eventInfo.upcomingTimeEvent=%s, "
				"eventInfo.nextEventTime=%.16g",
				fmi1BooleanToString(m_eventInfo.iterationConverged),
				fmi1BooleanToString(m_eventInfo.stateValueReferencesChanged),
				fmi1BooleanToString(m_eventInfo.stateValuesChanged),
				fmi1BooleanToString(m_eventInfo.terminateSimulation),
				fmi1BooleanToString(m_eventInfo.upcomingTimeEvent),
				m_eventInfo.nextEventTime);
	}

	void FMU1Model::getEventIndicators(double eventIndicators[], size_t size) {
        s_currentInstance = this;
		ASSERT_NO_ERROR(fmi1GetEventIndicators(m_component, eventIndicators, size), "Failed to get event indicators")
		logDebug("fmi1GetEventIndicators(size=%d): eventIndicators=[...]", size);
    }

}
