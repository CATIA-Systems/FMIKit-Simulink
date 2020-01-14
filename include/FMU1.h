#pragma once

/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/
 
#include "fmi1.h"
#include "FMU.h"

#ifndef _WIN32
#include <dlfcn.h>
#endif

namespace fmikit {

	class FMU1 : public FMU {

	public:
		explicit FMU1(const std::string &guid,
			 const std::string &modelIdentifier,
			 const std::string &unzipDirectory,
			 const std::string &instanceName,
			 allocateMemoryCallback *allocateMemory,
			 freeMemoryCallback *freeMemory);

		virtual ~FMU1();

		double getReal(ValueReference vr) override;
		int getInteger(ValueReference vr) override;
		bool getBoolean(ValueReference vr) override;
		std::string getString(ValueReference vr) override;

		void setReal(const ValueReference vr, double value) override;
		void setInteger(ValueReference vr, int value) override;
		void setBoolean(ValueReference vr, bool value) override;
		void setString(ValueReference vr, std::string value) override;

	protected:
        static FMU1 *s_currentInstance;
        
		static void logFMU1Message(fmi1Component c, fmi1String instanceName, fmi1Status status, fmi1String category, fmi1String message, ...);

		fmi1Component m_component ;
		fmi1CallbackFunctions m_callbackFunctions;

		void assertNoError(fmi1Status status, const char *message);

		template<typename T> T* getFunc(const char *functionName, bool required = true) {

			std::string procName = modelIdentifier() + "_" + functionName;

# ifdef _WIN32
			FARPROC fp = GetProcAddress(m_libraryHandle, procName.c_str());
# else
			void *fp = dlsym(m_libraryHandle, procName.c_str());
# endif

			if (required && !fp) {
				error("Function %s not found in shared library", functionName);
			}

			return reinterpret_cast<T *>(fp);
		}

		/* Common functions for FMI 1.0 */
		fmi1GetTypesPlatformTYPE *fmi1GetTypesPlatform;
		fmi1GetVersionTYPE		 *fmi1GetVersion;
		fmi1GetRealTYPE          *fmi1GetReal;
		fmi1GetIntegerTYPE		 *fmi1GetInteger;
		fmi1GetBooleanTYPE		 *fmi1GetBoolean;
		fmi1GetStringTYPE		 *fmi1GetString;
		fmi1SetDebugLoggingTYPE	 *fmi1SetDebugLogging;
		fmi1SetRealTYPE			 *fmi1SetReal;
		fmi1SetIntegerTYPE	     *fmi1SetInteger;
		fmi1SetBooleanTYPE		 *fmi1SetBoolean;
		fmi1SetStringTYPE		 *fmi1SetString;

	private:

		/* Wrapper functions for SEH */
		void getCString(ValueReference vr, char *value);
		void setCString(ValueReference vr, const char *value);

	};


	class FMU1Slave : public FMU1, public Slave {

	public:
		FMU1Slave(const std::string &guid,
				  const std::string &modelIdentifier,
				  const std::string &unzipDirectory,
				  const std::string &instanceName,
				  allocateMemoryCallback *allocateMemory = nullptr,
				  freeMemoryCallback *freeMemory = nullptr);

		~FMU1Slave();

        void instantiateSlave(const std::string &fmuLocation, double timeout, bool loggingOn);
		void initializeSlave(double startTime, bool stopTimeDefined, double stopTime);

		void doStep(double h) override;
		void setRealInputDerivative(ValueReference vr, int order, double value) override;

	private:
		/* Wrapper functions for SEH */
		void instantiateSlave_(fmi1String  instanceName, fmi1String  fmuGUID, fmi1String  fmuLocation, fmi1String  mimeType, fmi1Real timeout, fmi1Boolean visible, fmi1Boolean interactive, fmi1CallbackFunctions functions, fmi1Boolean loggingOn);
		void terminateSlave();
		void freeSlaveInstance();

		/***************************************************
		Functions for FMI 1.0 for Co-Simulation
		****************************************************/
		fmi1InstantiateSlaveTYPE		 *fmi1InstantiateSlave;
		fmi1InitializeSlaveTYPE			 *fmi1InitializeSlave;
		fmi1TerminateSlaveTYPE			 *fmi1TerminateSlave;
		fmi1ResetSlaveTYPE				 *fmi1ResetSlave;
		fmi1FreeSlaveInstanceTYPE		 *fmi1FreeSlaveInstance;
		fmi1GetRealOutputDerivativesTYPE *fmi1GetRealOutputDerivatives;
		fmi1SetRealInputDerivativesTYPE  *fmi1SetRealInputDerivatives;
		fmi1DoStepTYPE					 *fmi1DoStep;
		fmi1CancelStepTYPE				 *fmi1CancelStep;
		fmi1GetStatusTYPE				 *fmi1GetStatus;
		fmi1GetRealStatusTYPE			 *fmi1GetRealStatus;
		fmi1GetIntegerStatusTYPE		 *fmi1GetIntegerStatus;
		fmi1GetBooleanStatusTYPE		 *fmi1GetBooleanStatus;
		fmi1GetStringStatusTYPE			 *fmi1GetStringStatus;

	};

	class FMU1Model : public FMU1, public Model {

	public:
		explicit FMU1Model(	const std::string &guid,
							const std::string &modelIdentifier,
							const std::string &unzipDirectory,
							const std::string &instanceName,
							allocateMemoryCallback *allocateMemory = nullptr,
							freeMemoryCallback *freeMemory = nullptr);

		~FMU1Model();

        void instantiateModel(bool loggingOn);
		void initialize(bool toleranceControlled, double relativeTolerance);

		void getContinuousStates(double states[], size_t size) override;
		void getNominalContinuousStates(double states[], size_t size) override;
		void setContinuousStates(const double states[], size_t size) override;
		void getDerivatives(double derivatives[], size_t size) override;
		void getEventIndicators(double eventIndicators[], size_t size) override;
		void setTime(double time) override;
		bool completedIntegratorStep() override;
		void eventUpdate();

		bool iterationConverged() const { return m_eventInfo.iterationConverged != fmi1False; }
		bool stateValueReferencesChanged() const { return m_eventInfo.stateValueReferencesChanged != fmi1False; }
		bool stateValuesChanged() const { return m_eventInfo.stateValuesChanged != fmi1False; }
		bool terminateSimulation() const { return m_eventInfo.terminateSimulation != fmi1False; }
		bool upcomingTimeEvent() const { return m_eventInfo.upcomingTimeEvent != fmi1False; }
		double nextEventTime() const override { return m_eventInfo.nextEventTime; }

	private:
		fmi1EventInfo m_eventInfo;

		/* Wrapper functions for SEH */
		void instantiateModel_(fmi1String instanceName, fmi1String GUID, fmi1CallbackFunctions functions, fmi1Boolean loggingOn);
		void terminate();
		void freeModelInstance();

		/* Functions for FMI 1.0 for Model Exchange */
		fmi1GetModelTypesPlatformTYPE	   *fmi1GetModelTypesPlatform;
		fmi1InstantiateModelTYPE		   *fmi1InstantiateModel;
		fmi1FreeModelInstanceTYPE		   *fmi1FreeModelInstance;
		fmi1SetTimeTYPE					   *fmi1SetTime;
		fmi1SetContinuousStatesTYPE		   *fmi1SetContinuousStates;
		fmi1CompletedIntegratorStepTYPE    *fmi1CompletedIntegratorStep;
		fmi1InitializeTYPE				   *fmi1Initialize;
		fmi1GetDerivativesTYPE			   *fmi1GetDerivatives;
		fmi1GetEventIndicatorsTYPE		   *fmi1GetEventIndicators;
		fmi1EventUpdateTYPE				   *fmi1EventUpdate;
		fmi1GetContinuousStatesTYPE		   *fmi1GetContinuousStates;
		fmi1GetNominalContinuousStatesTYPE *fmi1GetNominalContinuousStates;
		fmi1GetStateValueReferencesTYPE    *fmi1GetStateValueReferences;
		fmi1TerminateTYPE				   *fmi1Terminate;

	};

}
