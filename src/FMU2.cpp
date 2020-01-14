/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#include <iostream>

#ifndef _WIN32
	#include <stdarg.h>
	#include <dlfcn.h>
#endif

#include "FMU2.h"

using namespace std;

namespace fmikit {

	// bool to fmi2Boolean (== int)
	static fmi2Boolean btoi(bool value) {
		return value ? fmi2True : fmi2False;
	}

	FMU2::FMU2(const std::string &guid, const std::string &modelIdentifier, const std::string &unzipDirectory, const std::string &instanceName, allocateMemoryCallback *allocateMemory, freeMemoryCallback *freeMemory) :
		FMU(guid, modelIdentifier, unzipDirectory, instanceName),
		m_component(nullptr),
		m_state(StartAndEndState) {

		m_fmiVersion = FMI_VERSION_2;

		fmi2GetTypesPlatform			= getFunc<fmi2GetTypesPlatformTYPE>         ("fmi2GetTypesPlatform");
		fmi2GetVersion					= getFunc<fmi2GetVersionTYPE>               ("fmi2GetVersion");
		fmi2SetDebugLogging				= getFunc<fmi2SetDebugLoggingTYPE>          ("fmi2SetDebugLogging");
		fmi2Instantiate					= getFunc<fmi2InstantiateTYPE>              ("fmi2Instantiate");
		fmi2FreeInstance				= getFunc<fmi2FreeInstanceTYPE>             ("fmi2FreeInstance");
		fmi2SetupExperiment				= getFunc<fmi2SetupExperimentTYPE>          ("fmi2SetupExperiment");
		fmi2EnterInitializationMode		= getFunc<fmi2EnterInitializationModeTYPE>  ("fmi2EnterInitializationMode");
		fmi2ExitInitializationMode		= getFunc<fmi2ExitInitializationModeTYPE>   ("fmi2ExitInitializationMode");
		fmi2Terminate					= getFunc<fmi2TerminateTYPE>                ("fmi2Terminate");
		fmi2Reset						= getFunc<fmi2ResetTYPE>                    ("fmi2Reset");
		fmi2GetReal						= getFunc<fmi2GetRealTYPE>                  ("fmi2GetReal");
		fmi2GetInteger					= getFunc<fmi2GetIntegerTYPE>               ("fmi2GetInteger");
		fmi2GetBoolean					= getFunc<fmi2GetBooleanTYPE>               ("fmi2GetBoolean");
		fmi2GetString					= getFunc<fmi2GetStringTYPE>                ("fmi2GetString");
		fmi2SetReal						= getFunc<fmi2SetRealTYPE>                  ("fmi2SetReal");
		fmi2SetInteger					= getFunc<fmi2SetIntegerTYPE>               ("fmi2SetInteger");
		fmi2SetBoolean					= getFunc<fmi2SetBooleanTYPE>               ("fmi2SetBoolean");
		fmi2SetString					= getFunc<fmi2SetStringTYPE>                ("fmi2SetString");

		/* FMU state functions (optional) */
		fmi2GetFMUstate					= getFunc<fmi2GetFMUstateTYPE>				("fmi2GetFMUstate",              false);
		fmi2SetFMUstate					= getFunc<fmi2SetFMUstateTYPE>				("fmi2SetFMUstate",              false);
		fmi2FreeFMUstate				= getFunc<fmi2FreeFMUstateTYPE>				("fmi2FreeFMUstate",             false);
		fmi2SerializedFMUstateSize		= getFunc<fmi2SerializedFMUstateSizeTYPE>	("fmi2SerializedFMUstateSize",   false);
		fmi2SerializeFMUstate			= getFunc<fmi2SerializeFMUstateTYPE>	    ("fmi2SerializeFMUstate",        false);
		fmi2DeSerializeFMUstate			= getFunc<fmi2DeSerializeFMUstateTYPE>		("fmi2DeSerializeFMUstate",      false);
		fmi2GetDirectionalDerivative	= getFunc<fmi2GetDirectionalDerivativeTYPE>	("fmi2GetDirectionalDerivative", false);

		m_callbackFunctions.logger               = logFMU2Message;
		m_callbackFunctions.allocateMemory       = allocateMemory ? allocateMemory : calloc;
		m_callbackFunctions.freeMemory           = freeMemory ? freeMemory : free;
		m_callbackFunctions.componentEnvironment = this;
		m_callbackFunctions.stepFinished         = nullptr;
	}

	FMU2::~FMU2() {
		terminate();
		freeInstance();
	}

	void FMU2::terminate() {
		assertState(EventModeState | ContinuousTimeModeState | StepCompleteState | StepFailedState);
		ASSERT_NO_ERROR(fmi2Terminate(m_component), "Failed to terminate")
		logDebug("fmi2Terminate()");
		m_state = TerminatedState;
	}

	void FMU2::freeInstance() {
		assertState(InstantiatedState | InitializationModeState | EventModeState | ContinuousTimeModeState
			| StepCompleteState | StepFailedState | StepCanceledState | TerminatedState | ErrorState);
		HANDLE_EXCEPTION(fmi2FreeInstance(m_component), "Failed to free instance")
		logDebug("fmi2FreeInstance()");
	}

    void FMU2::instantiate(bool loggingOn) {
		auto callbacks = reinterpret_cast<fmi2CallbackFunctions*>(&m_callbackFunctions);
		auto fmuResourceLocation = fmuLocation();
		fmuResourceLocation += string("/resources");
		instantiate_(instanceName().c_str(), static_cast<fmi2Type>(kind()), guid().c_str(), fmuResourceLocation.c_str(), callbacks, fmi2False, loggingOn);
	}

	void FMU2::instantiate_(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID, fmi2String fmuResourceLocation, const fmi2CallbackFunctions* functions, fmi2Boolean visible, fmi2Boolean loggingOn) {
		assertState(StartAndEndState);
		HANDLE_EXCEPTION(m_component = fmi2Instantiate(instanceName, fmuType, fmuGUID, fmuResourceLocation, functions, visible, loggingOn), "Failed to instantiate FMU")
		logDebug("fmi2Instantiate(instanceName=\"%s\", fmuType=%d, fmuGUID=\"%s\", fmuResourceLocation=\"%s\", visible=%d, loggingOn=%d)",
		instanceName, fmuType, fmuGUID, fmuResourceLocation, visible, loggingOn);
		if (!m_component) error("Failed to instantiate FMU");
		m_state = InstantiatedState;
	}

	void FMU2::setupExperiment(bool toleranceDefined, double tolerance, double startTime, bool stopTimeDefined, double stopTime) {
		assertState(InstantiatedState);

		this->m_stopTimeDefined = stopTimeDefined;
		this->m_stopTime = stopTime;

		m_time = startTime;
		ASSERT_NO_ERROR(fmi2SetupExperiment(m_component, toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime), "Failed to set up experiment")
		logDebug("fmi2SetupExperiment(toleranceDefined=%d, tolerance=%f, startTime=%f, stopTimeDefined=%d, stopTime=%f)",
			toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
	}

	void FMU2::enterInitializationMode() {
		assertState(InstantiatedState);
		logDebug("fmi2EnterInitializationMode()");
		ASSERT_NO_ERROR(fmi2EnterInitializationMode(m_component), "Failed to enter initialization mode")
		m_state = InitializationModeState;
	}

	void FMU2::exitInitializationMode() {
		assertState(InitializationModeState);
		logDebug("fmi2ExitInitializationMode()");
		ASSERT_NO_ERROR(fmi2ExitInitializationMode(m_component), "Failed to exit initialization mode")
		m_state = (m_kind == MODEL_EXCHANGE) ? EventModeState : StepCompleteState;
	}

	void FMU2::logFMU2Message(fmi2ComponentEnvironment environment, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...) {
		va_list args;
		va_start(args, message);

		auto instance = static_cast<FMU *>(environment);
        
        auto level = static_cast<LogLevel>(status);
        
        if (level >= instance->logLevel()) {
            logFMUMessage(instance, level, category, message, args);
        }

		va_end(args);
	}

	void FMU2::assertNoError(fmi2Status status, const char *message) {
		if (status >= fmi2Error) error(message);
	}

	double FMU2::getReal(const ValueReference vr) {
		fmi2Real value;
		assertNoError(fmi2GetReal(m_component, &vr, 1, &value), "Failed to get Real");
		logDebug("fmi2GetReal(vr=[%d], nvr=1): value=[%.16g]", vr, value);
		return value;
	}

	int FMU2::getInteger(ValueReference vr) {
		fmi2Integer value;
		assertNoError(fmi2GetInteger(m_component, &vr, 1, &value), "Failed to get Integer");
		logDebug("fmi2GetInteger(vr=[%d], nvr=1): value=[%d]", vr, value);
		return value;
	}

	bool FMU2::getBoolean(ValueReference vr) {
		fmi2Boolean value;
		assertNoError(fmi2GetBoolean(m_component, &vr, 1, &value), "Failed to get Boolean");
		logDebug("fmi2GetBoolean(vr=[%d], nvr=1): value=[%d]", vr, value);
		return value != fmi2False;
	}

	string FMU2::getString(ValueReference vr) {
		fmi2String value;
		assertNoError(fmi2GetString(m_component, &vr, 1, &value), "Failed to get String");
		logDebug("fmi2GetString(vr=[%d], nvr=1): value=[\"%s\"]", vr, value);
		return value;
	}

	void FMU2::setReal(const ValueReference vr, double value) {
		assertNoError(fmi2SetReal(m_component, &vr, 1, &value), "Failed to set Real");
		logDebug("fmi2SetReal(vr=[%d], nvr=1, value=[%.16g])", vr, value);
	}

	void FMU2::setInteger(ValueReference vr, int value) {
		assertNoError(fmi2SetInteger(m_component, &vr, 1, &value), "Failed to set Integer value");
		logDebug("fmi2SetInteger(vr=[%d], nvr=1, value=[%d])", vr, value);
	}

	void FMU2::setBoolean(ValueReference vr, bool value) {
		fmi2Boolean v = value ? fmi2True : fmi2False;
		assertNoError(fmi2SetBoolean(m_component, &vr, 1, &v), "Failed to set Boolean value");
		logDebug("fmi2SetBoolean(vr=[%d], nvr=1, value=[%d])", vr, v);
	}

	void FMU2::setString(ValueReference vr, string value) {
		fmi2String s = value.c_str();
		assertNoError(fmi2SetString(m_component, &vr, 1, &s), "Failed to set String");
		logDebug("fmi2SetString(vr=[%d], nvr=1, value=[\"%s\"])", vr, s);
	}

	FMU2Slave::FMU2Slave(const std::string &guid, const std::string &modelIdentifier, const std::string &unzipDirectory, const std::string &instanceName, allocateMemoryCallback *allocateMemory, freeMemoryCallback *freeMemory) :
		FMU2(guid, modelIdentifier, unzipDirectory, instanceName, allocateMemory, freeMemory) {

		m_kind = CO_SIMULATION;

		fmi2SetRealInputDerivatives			= getFunc<fmi2SetRealInputDerivativesTYPE>  ("fmi2SetRealInputDerivatives");
		fmi2GetRealOutputDerivatives		= getFunc<fmi2GetRealOutputDerivativesTYPE> ("fmi2GetRealOutputDerivatives");
		fmi2DoStep							= getFunc<fmi2DoStepTYPE>                   ("fmi2DoStep");
		fmi2CancelStep						= getFunc<fmi2CancelStepTYPE>               ("fmi2CancelStep");
		fmi2GetStatus						= getFunc<fmi2GetStatusTYPE>                ("fmi2GetStatus");
		fmi2GetRealStatus					= getFunc<fmi2GetRealStatusTYPE>            ("fmi2GetRealStatus");
		fmi2GetIntegerStatus				= getFunc<fmi2GetIntegerStatusTYPE>         ("fmi2GetIntegerStatus");
		fmi2GetBooleanStatus				= getFunc<fmi2GetBooleanStatusTYPE>         ("fmi2GetBooleanStatus");
		fmi2GetStringStatus					= getFunc<fmi2GetStringStatusTYPE>          ("fmi2GetStringStatus");
	}

	void FMU2Slave::doStep(double h) {
		assertState(StepCompleteState);

		if (m_stopTimeDefined && m_time + h > m_stopTime - h / 1000) {
			h = m_stopTime - m_time;
		}

		fmi2Boolean noSetFMUStatePriorToCurrentPoint = fmi2True;
		ASSERT_NO_ERROR(fmi2DoStep(m_component, m_time, h, noSetFMUStatePriorToCurrentPoint), "Failed to do step")
		logDebug("fmi2DoStep(currentCommunicationPoint=%f, communicationStepSize=%f, noSetFMUStatePriorToCurrentPoint=%d)", m_time, h, noSetFMUStatePriorToCurrentPoint);

		m_time += h;
	}

	void FMU2Slave::setRealInputDerivative(ValueReference vr, int order, double value) {
		ASSERT_NO_ERROR(fmi2SetRealInputDerivatives(m_component, &vr, 1, &order, &value), "Failed to set real input derivatives")
		logDebug("fmi2SetRealInputDerivatives(component, vr=[%d], nvr=1, order=[%d], value=[%.16g])", vr, order, value);
	}

	bool FMU2Slave::terminated() {
		fmi2Boolean status;
		// TODO: logDebug(...)
		ASSERT_NO_ERROR(fmi2GetBooleanStatus(m_component, fmi2Terminated, &status), "Failed to get boolean status")
		return status != fmi2False;
	}

	FMU2Model::FMU2Model(const std::string &guid, const std::string &modelIdentifier, const std::string &unzipDirectory, const std::string &instanceName, allocateMemoryCallback *allocateMemory, freeMemoryCallback *freeMemory) :
		FMU2(guid, modelIdentifier, unzipDirectory, instanceName, allocateMemory, freeMemory) {

		m_kind = MODEL_EXCHANGE;

		m_eventInfo.newDiscreteStatesNeeded           = fmi2False;
		m_eventInfo.terminateSimulation               = fmi2False;
		m_eventInfo.nominalsOfContinuousStatesChanged = fmi2False;
		m_eventInfo.valuesOfContinuousStatesChanged   = fmi2False;
		m_eventInfo.nextEventTimeDefined              = fmi2False;
		m_eventInfo.nextEventTime                     = 0.0;

		fmi2EnterEventMode					= getFunc<fmi2EnterEventModeTYPE>                ("fmi2EnterEventMode");
		fmi2NewDiscreteStates				= getFunc<fmi2NewDiscreteStatesTYPE>             ("fmi2NewDiscreteStates");
		fmi2EnterContinuousTimeMode			= getFunc<fmi2EnterContinuousTimeModeTYPE>       ("fmi2EnterContinuousTimeMode");
		fmi2CompletedIntegratorStep			= getFunc<fmi2CompletedIntegratorStepTYPE>       ("fmi2CompletedIntegratorStep");
		fmi2SetTime							= getFunc<fmi2SetTimeTYPE>                       ("fmi2SetTime");
		fmi2SetContinuousStates				= getFunc<fmi2SetContinuousStatesTYPE>           ("fmi2SetContinuousStates");
		fmi2GetDerivatives					= getFunc<fmi2GetDerivativesTYPE>                ("fmi2GetDerivatives");
		fmi2GetEventIndicators				= getFunc<fmi2GetEventIndicatorsTYPE>            ("fmi2GetEventIndicators");
		fmi2GetContinuousStates				= getFunc<fmi2GetContinuousStatesTYPE>           ("fmi2GetContinuousStates");
		fmi2GetNominalsOfContinuousStates	= getFunc<fmi2GetNominalsOfContinuousStatesTYPE> ("fmi2GetNominalsOfContinuousStates");
	}


	bool FMU2Model::newDiscreteStatesNeeded() const {
		return m_eventInfo.newDiscreteStatesNeeded != fmi2False;
	}

	bool FMU2Model::terminateSimulation() const {
		return m_eventInfo.terminateSimulation != fmi2False;
	}

	bool FMU2Model::nominalsOfContinuousStatesChanged() const {
		return m_eventInfo.nominalsOfContinuousStatesChanged != fmi2False;
	}

	bool FMU2Model::valuesOfContinuousStatesChanged() const {
		return m_eventInfo.valuesOfContinuousStatesChanged != fmi2False;
	}

	bool FMU2Model::nextEventTimeDefined() const {
		return m_eventInfo.nextEventTimeDefined != fmi2False;
	}

	double FMU2Model::nextEventTime() const {
		return m_eventInfo.nextEventTime;
	}

	void FMU2Model::newDiscreteStates() {
		logDebug("fmi2NewDiscreteStates()");
		ASSERT_NO_ERROR(fmi2NewDiscreteStates(m_component, &m_eventInfo), "Failed to calculate new discrete states")
	}

	void FMU2Model::enterContinuousTimeMode() {
		logDebug("fmi2EnterContinuousTimeMode()");
		ASSERT_NO_ERROR(fmi2EnterContinuousTimeMode(m_component), "Failed to enter continuous time mode")
		m_state = ContinuousTimeModeState;
	}

	void FMU2Model::getContinuousStates(double x[], size_t nx) {
		if (nx < 1) return; // nothing to do
		ASSERT_NO_ERROR(fmi2GetContinuousStates(m_component, x, nx), "Failed to get continuous states")
		logDebug("fmi2GetContinuousStates(x=[...], nx=%d)", nx);
	}

	void FMU2Model::getNominalContinuousStates(double x[], size_t nx) {
		if (nx < 1) return; // nothing to do
		ASSERT_NO_ERROR(fmi2GetNominalsOfContinuousStates(m_component, x, nx), "Failed to get nominal continuous states")
			logDebug("fmi2GetNominalsOfContinuousStates(x=[...], nx=%d)", nx);
	}

	void FMU2Model::getDerivatives(double derivatives[], size_t nx) {
		ASSERT_NO_ERROR(fmi2GetDerivatives(m_component, derivatives, nx), "Failed to get derivatives")
		logDebug("fmi2GetDerivatives(derivatives=[...], nx=%d)", nx);
	}

	void FMU2Model::getEventIndicators(double indicators[], size_t ni) {
		ASSERT_NO_ERROR(fmi2GetEventIndicators(m_component, indicators, ni), "Failed to get event indicators")
		logDebug("fmi2GetEventIndicators(indicators=[...], ni=%d)", ni);
	}

	void FMU2Model::setTime(double time) {
		ASSERT_NO_ERROR(fmi2SetTime(m_component, time), "Failed to set time")
		logDebug("fmi2SetTime(time=%.16g)", time);
		this->m_time = time;
	}

	void FMU2Model::setContinuousStates(const double x[], size_t nx) {
		if (nx < 1) return; // nothing to do
		ASSERT_NO_ERROR(fmi2SetContinuousStates(m_component, x, nx), "Failed to set continuous states")
		logDebug("fmi2SetContinuousStates(x=[...], nx=%d)", nx);
	}

	bool FMU2Model::completedIntegratorStep() {
		fmi2Boolean noSetFMUStatePriorToCurrentPoint = fmi2True;
		fmi2Boolean enterEventMode;

		ASSERT_NO_ERROR(fmi2CompletedIntegratorStep(m_component, noSetFMUStatePriorToCurrentPoint, &enterEventMode, &m_eventInfo.terminateSimulation),
			"Failed to complete integrator step")

		logDebug("fmi2CompletedIntegratorStep(noSetFMUStatePriorToCurrentPoint=fmi2True): enterEventMode=%d, terminateSimulation=%d", enterEventMode, m_eventInfo.terminateSimulation);

		return enterEventMode != fmi2False;
	}

	void FMU2Model::enterEventMode() {
		ASSERT_NO_ERROR(fmi2EnterEventMode(m_component), "Failed to enter event mode")
		logDebug("fmi2EnterEventMode()");
		m_state = EventModeState;
	}

}
