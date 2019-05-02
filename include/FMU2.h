#pragma once

/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/
 
#include "fmi2Functions.h"
#include "FMU.h"


namespace fmikit {

	typedef struct {
		fmi2CallbackLogger         logger;
		fmi2CallbackAllocateMemory allocateMemory;
		fmi2CallbackFreeMemory     freeMemory;
		fmi2StepFinished           stepFinished;
		fmi2ComponentEnvironment   componentEnvironment;
	} fmi2CallbackFunctionsNonConst;

	typedef enum {
		StartAndEndState = 1 << 0,
		InstantiatedState = 1 << 1,
		InitializationModeState = 1 << 2,

		// model exchange states
		EventModeState = 1 << 3,
		ContinuousTimeModeState = 1 << 4,

		// co-simulation states
		StepCompleteState = 1 << 5,
		StepInProgressState = 1 << 6,
		StepFailedState = 1 << 7,
		StepCanceledState = 1 << 8,

		TerminatedState = 1 << 9,
		ErrorState = 1 << 10,
		FatalState = 1 << 11,
	} State;

	const int fmi2GetXMask = InitializationModeState
		| EventModeState
		| ContinuousTimeModeState
		| StepCompleteState
		| StepFailedState
		| StepCanceledState
		| TerminatedState
		| ErrorState;

	const int fmi2SetBISMask = InstantiatedState
		| InitializationModeState
		| EventModeState
		| StepCompleteState;

	const int fmi2GetBIRSStatusMask = StepCompleteState | StepInProgressState | StepFailedState | TerminatedState;

	class FMU2 : public FMU {

	public:
		explicit FMU2(const std::string &guid, const std::string &modelIdentifier, const std::string &unzipDirectory, const std::string &instanceName, allocateMemoryCallback *allocateMemory, freeMemoryCallback *freeMemory);
		virtual ~FMU2();

        void instantiate(bool loggingOn);
		void setupExperiment(bool toleranceDefined, double tolerance, double startTime, bool stopTimeDefined, double stopTime);
		void enterInitializationMode();
		void exitInitializationMode();

		double getReal(ValueReference vr) override;
		int getInteger(ValueReference vr) override;
		bool getBoolean(ValueReference vr) override;
		std::string getString(ValueReference vr) override;

		void setReal(const ValueReference vr, double value) override;
		void setInteger(ValueReference vr, int value) override;
		void setBoolean(ValueReference vr, bool value) override;
		void setString(ValueReference vr, std::string value) override;

		State getState() const { return m_state; }

	protected:
		fmi2Component m_component;
		fmi2CallbackFunctionsNonConst m_callbackFunctions;
		State m_state;

		void assertState(int statesExpected) {
			if (!(m_state & statesExpected)) error("Illegal state");
		}

		static void logFMU2Message(fmi2ComponentEnvironment environment, fmi2String instanceName, fmi2Status status, fmi2String category, fmi2String message, ...);

		void assertNoError(fmi2Status status, const char *message);

		template<typename T> T* getFunc(const char *functionName, bool required = true) {

# ifdef _WIN32
			auto *fp = GetProcAddress(m_libraryHandle, functionName);
# else
			auto *fp = dlsym(m_libraryHandle, functionName);
# endif

			if (required && !fp) {
				error("Function %s not found in shared library", functionName);
			}

			return reinterpret_cast<T *>(fp);
		}

		/***************************************************
		Common Functions for FMI 2.0
		****************************************************/

		/* required functions */
		fmi2GetTypesPlatformTYPE         *fmi2GetTypesPlatform;
		fmi2GetVersionTYPE               *fmi2GetVersion;
		fmi2SetDebugLoggingTYPE          *fmi2SetDebugLogging;
		fmi2InstantiateTYPE              *fmi2Instantiate;
		fmi2FreeInstanceTYPE             *fmi2FreeInstance;
		fmi2SetupExperimentTYPE          *fmi2SetupExperiment;
		fmi2EnterInitializationModeTYPE  *fmi2EnterInitializationMode;
		fmi2ExitInitializationModeTYPE   *fmi2ExitInitializationMode;
		fmi2TerminateTYPE                *fmi2Terminate;
		fmi2ResetTYPE                    *fmi2Reset;
		fmi2GetRealTYPE                  *fmi2GetReal;
		fmi2GetIntegerTYPE               *fmi2GetInteger;
		fmi2GetBooleanTYPE               *fmi2GetBoolean;
		fmi2GetStringTYPE                *fmi2GetString;
		fmi2SetRealTYPE                  *fmi2SetReal;
		fmi2SetIntegerTYPE               *fmi2SetInteger;
		fmi2SetBooleanTYPE               *fmi2SetBoolean;
		fmi2SetStringTYPE                *fmi2SetString;

		/* optional functions */
		fmi2GetFMUstateTYPE              *fmi2GetFMUstate;
		fmi2SetFMUstateTYPE              *fmi2SetFMUstate;
		fmi2FreeFMUstateTYPE             *fmi2FreeFMUstate;
		fmi2SerializedFMUstateSizeTYPE   *fmi2SerializedFMUstateSize;
		fmi2SerializeFMUstateTYPE        *fmi2SerializeFMUstate;
		fmi2DeSerializeFMUstateTYPE      *fmi2DeSerializeFMUstate;
		fmi2GetDirectionalDerivativeTYPE *fmi2GetDirectionalDerivative;

	private:

		/* Wrapper functions for SEH */
		void instantiate_(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID, fmi2String fmuResourceLocation, const fmi2CallbackFunctions* functions, fmi2Boolean visible, fmi2Boolean loggingOn);
		void terminate();
		void freeInstance();
	};


	class FMU2Slave : public FMU2, public Slave {

	public:
		explicit FMU2Slave(const std::string &guid, const std::string &modelIdentifier, const std::string &unzipDirectory, const std::string &instanceName, allocateMemoryCallback *allocateMemory = nullptr, freeMemoryCallback *freeMemory = nullptr);
		~FMU2Slave() {}

		void doStep(double h) override;
		void setRealInputDerivative(ValueReference vr, int order, double value) override;

		bool terminated();

	private:

		/***************************************************
		Functions for FMI 2.0 for Co-Simulation
		****************************************************/
		fmi2SetRealInputDerivativesTYPE  *fmi2SetRealInputDerivatives;
		fmi2GetRealOutputDerivativesTYPE *fmi2GetRealOutputDerivatives;
		fmi2DoStepTYPE                   *fmi2DoStep;
		fmi2CancelStepTYPE               *fmi2CancelStep;
		fmi2GetStatusTYPE                *fmi2GetStatus;
		fmi2GetRealStatusTYPE            *fmi2GetRealStatus;
		fmi2GetIntegerStatusTYPE         *fmi2GetIntegerStatus;
		fmi2GetBooleanStatusTYPE         *fmi2GetBooleanStatus;
		fmi2GetStringStatusTYPE			 *fmi2GetStringStatus;

	};


	class FMU2Model : public FMU2, public Model {

	public:
		explicit FMU2Model(const std::string &guid, const std::string &modelIdentifier, const std::string &unzipDirectory, const std::string &instanceName, allocateMemoryCallback *allocateMemory = nullptr, freeMemoryCallback *freeMemory = nullptr);
		~FMU2Model() {}

		// event info
		bool newDiscreteStatesNeeded() const;
		bool terminateSimulation() const;
		bool nominalsOfContinuousStatesChanged() const;
		bool valuesOfContinuousStatesChanged() const;
		bool nextEventTimeDefined() const;
		double nextEventTime() const override;

		void newDiscreteStates();
		void enterContinuousTimeMode();

		void getContinuousStates(double x[], size_t nx) override;
		void getNominalContinuousStates(double x[], size_t nx) override;
		void setContinuousStates(const double x[], size_t nx) override;
		void getDerivatives(double derivatives[], size_t nx) override;
		void getEventIndicators(double eventIndicators[], size_t ni) override;
		void setTime(double time) override;
		bool completedIntegratorStep() override;

		void enterEventMode();

	private:
		fmi2EventInfo m_eventInfo;

		/***************************************************
		Functions for FMI 2.0 for Model Exchange
		****************************************************/
		fmi2EnterEventModeTYPE                *fmi2EnterEventMode;
		fmi2NewDiscreteStatesTYPE             *fmi2NewDiscreteStates;
		fmi2EnterContinuousTimeModeTYPE       *fmi2EnterContinuousTimeMode;
		fmi2CompletedIntegratorStepTYPE       *fmi2CompletedIntegratorStep;
		fmi2SetTimeTYPE                       *fmi2SetTime;
		fmi2SetContinuousStatesTYPE           *fmi2SetContinuousStates;
		fmi2GetDerivativesTYPE                *fmi2GetDerivatives;
		fmi2GetEventIndicatorsTYPE            *fmi2GetEventIndicators;
		fmi2GetContinuousStatesTYPE           *fmi2GetContinuousStates;
		fmi2GetNominalsOfContinuousStatesTYPE *fmi2GetNominalsOfContinuousStates;

	};

}
