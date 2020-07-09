/*****************************************************************
 *  Copyright (c) Dassault Systemes. All rights reserved.        *
 *  This file is part of FMIKit. See LICENSE.txt in the project  *
 *  root for license information.                                *
 *****************************************************************/

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sstream> // stringstream
#include <iomanip>
#include <stdexcept> // for runtime_error

#ifdef _WIN32
#include <direct.h>
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")
#else
#include <stdarg.h>
#include <dlfcn.h>
#endif

#include "FMU.h"

using namespace std;
using namespace fmikit;


#define MAX_MESSAGE_SIZE 4096 // arbitrary
#define INTERNET_MAX_URL_LENGTH 2083 // from wininet.h


MessageLogger * FMU::m_messageLogger = nullptr;

const char* FMU::platform() {
#ifdef _WIN32

#ifdef _WIN64
	return "win64";
#else
	return "win32";
#endif

#else
    return "linux64";
#endif
}

static std::string lastSystemErrorMessage() {

#ifdef _WIN32
	auto error = GetLastError();

	if (error) {
		LPVOID lpMsgBuf;
		auto bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPTSTR>(&lpMsgBuf),
			0, nullptr);

		if (bufLen) {
			auto lpMsgStr = static_cast<LPCSTR>(lpMsgBuf);
			string result(lpMsgStr, lpMsgStr + bufLen);
			LocalFree(lpMsgBuf);
			return result + " (System Error " + to_string(error) + ")";
		}
	}
#endif

	return string();
}


FMU::FMU(const std::string &guid, const std::string &modelIdentifier, const std::string &unzipDirectory, const std::string &instanceName) :
	m_libraryHandle(nullptr),
	m_time(0.0),
	m_stopTimeDefined(false),
	m_stopTime(0.0),
	m_guid(guid),
	m_modelIdentifier(modelIdentifier),
	m_unzipDirectory(unzipDirectory),
	m_instanceName(instanceName) {

#ifdef _WIN32
	TCHAR fmuLocation[INTERNET_MAX_URL_LENGTH];
	DWORD fmuLocationLength = INTERNET_MAX_URL_LENGTH;
	HRESULT result = UrlCreateFromPath(m_unzipDirectory.c_str(), fmuLocation, &fmuLocationLength, 0);
    m_fmuLocation = fmuLocation;
	// TODO: check result
#else
    m_fmuLocation = "file://" + m_unzipDirectory;
#endif


#ifdef _WIN32
    // directory that contains the binaries for the current platform
	const auto libraryDir = unzipDirectory + "\\binaries\\" + platform();
	// path to the model DLL
	const auto libraryPath = libraryDir + "\\" + modelIdentifier + ".dll";
#elif __APPLE__
    const auto libraryPath = unzipDirectory + "/binaries/darwin64/" + modelIdentifier + ".dylib";
#else
    const auto libraryPath = unzipDirectory + "/binaries/linux64/" + modelIdentifier + ".so";
#endif

	logDebug("Loading shared library: \"%s\"", libraryPath.c_str());

	// load the shared library

# ifdef _WIN32
	// DLL directory as wstring
	const wstring dllDirectory(libraryDir.begin(), libraryDir.end());

	// add the binaries directory temporarily to the DLL path to allow discovery of dependencies
	auto dllDirectoryCookie = AddDllDirectory(dllDirectory.c_str());

	m_libraryHandle = LoadLibraryEx(libraryPath.c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

	// remove the binaries directory from the DLL path
	RemoveDllDirectory(dllDirectoryCookie);
# else
	m_libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);
# endif

	if (!m_libraryHandle) {

		auto message = lastSystemErrorMessage();

		// replace "%1" with the library path (this works for most messages)
		const string search = "%1";

		for (size_t pos = 0;; pos += libraryPath.length()) {
			pos = message.find(search, pos);
			if (pos == string::npos) break;
			message.erase(pos, search.length());
			message.insert(pos, libraryPath);
		}

		error("Could not load shared library. %s", message.c_str());
	}
}

FMU::~FMU() {
	// unload the shared library
	if (m_libraryHandle) {
# ifdef _WIN32
		FreeLibrary(m_libraryHandle);
# else
		dlclose(m_libraryHandle);
# endif
		m_libraryHandle = nullptr;
	}
}

void FMU::logDebug(const char *message, ...) {
	if (m_fmiCallLogger) {
		va_list args;
		va_start(args, message);
		char buf[MAX_MESSAGE_SIZE];
		vsnprintf(buf, MAX_MESSAGE_SIZE, message, args);
		m_fmiCallLogger(this, buf);
		va_end(args);
	}
}

void FMU::error(const char *message, ...) {
	va_list args;
	va_start(args, message);
	char buf[MAX_MESSAGE_SIZE];
	vsnprintf(buf, MAX_MESSAGE_SIZE, message, args);
	cout << buf << endl;
	logFMUMessage(this, LOG_ERROR, nullptr, message, args);
	va_end(args);
	throw runtime_error(buf);
}

void FMU::logFMUMessage(FMU *instance, LogLevel level, const char* category, const char* message, va_list args) {
	if (level >= instance->logLevel() && m_messageLogger) {
		char buf[MAX_MESSAGE_SIZE];
		vsnprintf(buf, MAX_MESSAGE_SIZE, message, args);
		m_messageLogger(instance, level, category, buf);
	}
}

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

void FMU::logGetReal(const char *functionName, const ValueReference vr[], size_t nvr, const double value[]) {
    if (m_fmiCallLogger) {
        std::stringstream ss;
        ss << std::setprecision(16);
        ss << functionName << "(vr=["; appendValueReferences(ss, vr, nvr); ss << "], nvr=" << nvr << "): value=["; appendDoubles(ss, value, nvr); ss << "]";
        logDebug(ss.str().c_str());
    }
}

void FMU::logSetReal(const char *functionName, const ValueReference vr[], size_t nvr, const double value[]) {
    if (m_fmiCallLogger) {
        std::stringstream ss;
        ss << std::setprecision(16);
        ss << functionName << "(vr=["; appendValueReferences(ss, vr, nvr); ss << "], nvr=" << nvr << ", value=["; appendDoubles(ss, value, nvr); ss << "])";
        logDebug(ss.str().c_str());
    }
}
