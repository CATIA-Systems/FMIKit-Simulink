#pragma once

/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#ifdef _WIN32
#include <windows.h>
#else
typedef void *HMODULE;
#endif

#include <fstream>

#ifdef _WIN32
#define ASSERT_NO_ERROR(F, M) __try { assertNoError(F, M); } __except (EXCEPTION_EXECUTE_HANDLER) { error("%s. The FMU crashed (exception code: %s).", M, exceptionCodeToString(GetExceptionCode())); }
#else
#define ASSERT_NO_ERROR(F, M) assertNoError(F, M);
#endif

#ifdef _WIN32
#define HANDLE_EXCEPTION(F, M) __try { F; } __except (EXCEPTION_EXECUTE_HANDLER) { error("%s. The FMU crashed (exception code: %s).", M, exceptionCodeToString(GetExceptionCode())); }
#else
#define HANDLE_EXCEPTION(F, M) F;
#endif

namespace fmikit {

	enum Status {
		OK,
		Warning,
		Discard,
		Error,
		Fatal,
		Pending
	};

    enum FMIVersion {
        FMI_VERSION_1 = 1,
        FMI_VERSION_2
    };

    enum Kind {
        MODEL_EXCHANGE,
        CO_SIMULATION
    };

    enum LogLevel {
        LOG_INFO    = 0,
        LOG_WARNING = 1,
        LOG_DISCARD = 2,
        LOG_ERROR   = 3,
        LOG_FATAL   = 4,
        LOG_NONE    = 5,
    };

	enum Type {
		REAL,
		INTEGER,
		BOOLEAN,
		STRING
	};

	class FMU;

	typedef unsigned int ValueReference;

	typedef void MessageLogger(FMU *instance, LogLevel level, const char* category, const char* message);

	typedef void FMICallLogger(FMU *instance, const char* message);

	typedef void * allocateMemoryCallback(size_t count, size_t size);

	typedef void freeMemoryCallback(void *memory);


	class Model {

	public:
		virtual ~Model() {}
		virtual void setContinuousStates(const double states[], size_t size) = 0;
		virtual void getContinuousStates(double states[], size_t size) = 0;
		virtual void getNominalContinuousStates(double states[], size_t size) = 0;
		virtual void getDerivatives(double derivatives[], size_t size) = 0;
		virtual void getEventIndicators(double eventIndicators[], size_t size) = 0;
		virtual void setTime(double time) = 0;
		virtual bool completedIntegratorStep() = 0;
		virtual double nextEventTime() const = 0;

	};


	class Slave {

	public:
		virtual ~Slave() {}
		virtual void doStep(double h) = 0;
		virtual void setRealInputDerivative(ValueReference vr, int order, double value) = 0;

	};


	class FMU {

	public:
		static MessageLogger *m_messageLogger;
		FMICallLogger *m_fmiCallLogger = nullptr;

		static const char *platform();
		LogLevel logLevel() { return m_logLevel; }
		void setLogLevel(LogLevel level) { m_logLevel = level; }

		explicit FMU(const std::string &guid,
			const std::string &modelIdentifier,
			const std::string &unzipDirectory,
			const std::string &instanceName);

		virtual ~FMU();

        void *m_userData = nullptr;

		double getTime() const { return m_time; }

		virtual double getReal(ValueReference vr) = 0;
		virtual int getInteger(ValueReference vr) = 0;
		virtual bool getBoolean(ValueReference vr) = 0;
		virtual std::string getString(ValueReference vr) = 0;

		virtual void setReal(const ValueReference vr, double value) = 0;
		virtual void setInteger(ValueReference vr, int value) = 0;
		virtual void setBoolean(ValueReference vr, bool value) = 0;
		virtual void setString(ValueReference vr, std::string value) = 0;

		Kind kind() const { return m_kind; }
		FMIVersion fmiVersion() const { return m_fmiVersion; }

		const std::string& guid() const { return m_guid; }
		const std::string& modelIdentifier() const { return m_modelIdentifier; }
		const std::string& instanceName() const { return m_instanceName; }
		const std::string& fmuLocation() const { return m_fmuLocation; }

	protected:
        LogLevel m_logLevel;
		HMODULE m_libraryHandle;
		double m_time;
		FMIVersion m_fmiVersion;
		Kind m_kind;
		bool m_stopTimeDefined;
		double m_stopTime;

		void logDebug(const char *message, ...);
		void logInfo(const char *message, ...);
		void error(const char *message, ...);

		static void logFMUMessage(FMU *instance, LogLevel level, const char* category, const char* message, va_list args);

		void logGetReal(const char *functionName, const ValueReference vr[], size_t nvr, const double value[]);
		void logSetReal(const char *functionName, const ValueReference vr[], size_t nvr, const double value[]);

#ifdef _WIN32
		const char * exceptionCodeToString(DWORD exceptionCode) {
			switch (exceptionCode) {
				case STATUS_WAIT_0:						return "WAIT_0";
				case STATUS_ABANDONED_WAIT_0:			return "ABANDONED_WAIT_0";
				case STATUS_USER_APC:					return "USER_APC";
				case STATUS_TIMEOUT:					return "TIMEOUT";
				case STATUS_PENDING:					return "PENDING";
				case DBG_EXCEPTION_HANDLED:				return "EXCEPTION_HANDLED";
				case DBG_CONTINUE:						return "CONTINUE";
				case STATUS_SEGMENT_NOTIFICATION:		return "SEGMENT_NOTIFICATION";
				case STATUS_FATAL_APP_EXIT:				return "FATAL_APP_EXIT";
				case DBG_TERMINATE_THREAD:				return "TERMINATE_THREAD";
				case DBG_TERMINATE_PROCESS:				return "TERMINATE_PROCESS";
				case DBG_CONTROL_C:						return "CONTROL_C";
				case DBG_PRINTEXCEPTION_C:				return "PRINTEXCEPTION_C";
				case DBG_RIPEXCEPTION:					return "RIPEXCEPTION";
				case DBG_CONTROL_BREAK:					return "CONTROL_BREAK";
				case DBG_COMMAND_EXCEPTION:				return "COMMAND_EXCEPTION";
				case STATUS_GUARD_PAGE_VIOLATION:		return "GUARD_PAGE_VIOLATION";
				case STATUS_DATATYPE_MISALIGNMENT:		return "DATATYPE_MISALIGNMENT";
				case STATUS_BREAKPOINT:					return "BREAKPOINT";
				case STATUS_SINGLE_STEP:				return "SINGLE_STEP";
				case STATUS_LONGJUMP:					return "LONGJUMP";
				case STATUS_UNWIND_CONSOLIDATE:			return "UNWIND_CONSOLIDATE";
				case DBG_EXCEPTION_NOT_HANDLED:			return "EXCEPTION_NOT_HANDLED";
				case STATUS_ACCESS_VIOLATION:			return "ACCESS_VIOLATION";
				case STATUS_IN_PAGE_ERROR:				return "IN_PAGE_ERROR";
				case STATUS_INVALID_HANDLE:				return "INVALID_HANDLE";
				case STATUS_INVALID_PARAMETER:			return "INVALID_PARAMETER";
				case STATUS_NO_MEMORY:					return "NO_MEMORY";
				case STATUS_ILLEGAL_INSTRUCTION:		return "ILLEGAL_INSTRUCTION";
				case STATUS_NONCONTINUABLE_EXCEPTION:	return "NONCONTINUABLE_EXCEPTION";
				case STATUS_INVALID_DISPOSITION:		return "INVALID_DISPOSITION";
				case STATUS_ARRAY_BOUNDS_EXCEEDED:		return "ARRAY_BOUNDS_EXCEEDED";
				case STATUS_FLOAT_DENORMAL_OPERAND:		return "FLOAT_DENORMAL_OPERAND";
				case STATUS_FLOAT_DIVIDE_BY_ZERO:		return "FLOAT_DIVIDE_BY_ZERO";
				case STATUS_FLOAT_INEXACT_RESULT:		return "FLOAT_INEXACT_RESULT";
				case STATUS_FLOAT_INVALID_OPERATION:	return "FLOAT_INVALID_OPERATION";
				case STATUS_FLOAT_OVERFLOW:				return "FLOAT_OVERFLOW";
				case STATUS_FLOAT_STACK_CHECK:			return "FLOAT_STACK_CHECK";
				case STATUS_FLOAT_UNDERFLOW:			return "FLOAT_UNDERFLOW";
				case STATUS_INTEGER_DIVIDE_BY_ZERO:		return "INTEGER_DIVIDE_BY_ZERO";
				case STATUS_INTEGER_OVERFLOW:			return "INTEGER_OVERFLOW";
				case STATUS_PRIVILEGED_INSTRUCTION:		return "PRIVILEGED_INSTRUCTION";
				case STATUS_STACK_OVERFLOW:				return "STACK_OVERFLOW";
				case STATUS_DLL_NOT_FOUND:				return "DLL_NOT_FOUND";
				case STATUS_ORDINAL_NOT_FOUND:			return "ORDINAL_NOT_FOUND";
				case STATUS_ENTRYPOINT_NOT_FOUND:		return "ENTRYPOINT_NOT_FOUND";
				case STATUS_CONTROL_C_EXIT:				return "CONTROL_C_EXIT";
				case STATUS_DLL_INIT_FAILED:			return "DLL_INIT_FAILED";
				case STATUS_FLOAT_MULTIPLE_FAULTS:		return "FLOAT_MULTIPLE_FAULTS";
				case STATUS_FLOAT_MULTIPLE_TRAPS:		return "FLOAT_MULTIPLE_TRAPS";
				case STATUS_REG_NAT_CONSUMPTION:		return "REG_NAT_CONSUMPTION";
				//case STATUS_HEAP_CORRUPTION:			return "HEAP_CORRUPTION";
				case STATUS_STACK_BUFFER_OVERRUN:		return "STACK_BUFFER_OVERRUN";
				case STATUS_INVALID_CRUNTIME_PARAMETER: return "INVALID_CRUNTIME_PARAMETER";
				case STATUS_ASSERTION_FAILURE:			return "ASSERTION_FAILURE";
				case STATUS_SXS_EARLY_DEACTIVATION:		return "SXS_EARLY_DEACTIVATION";
				case STATUS_SXS_INVALID_DEACTIVATION:	return "SXS_INVALID_DEACTIVATION";
				default:								return "UNKOWN_EXEPTION_CODE";
			}
		}
#endif

	private:
		std::string m_guid;
		std::string m_modelIdentifier;
		std::string m_unzipDirectory;
		std::string m_instanceName;
		std::string m_fmuLocation;

	};

}
