cmake_minimum_required (VERSION 3.2)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE TYPE INTERNAL FORCE)

set (CMAKE_CXX_STANDARD 11)

project (FMIKit)

set(MATLAB_DIR "C:/Program Files/MATLAB/R2010bSP2" CACHE STRING "MATLAB install directory")

if (WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MTd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    	set(TARGET_SUFFIX mexw64)
	    link_directories(${MATLAB_DIR}/extern/lib/win64/microsoft)
	else ()
		set(TARGET_SUFFIX mexw32)
	    link_directories(${MATLAB_DIR}/extern/lib/win32/microsoft)
	endif ()
elseif (APPLE)
    set(TARGET_SUFFIX mexmaci64)
else ()
    set(TARGET_SUFFIX mexa64)
endif ()

add_library(sfun_fmurun SHARED sfun_fmurun.cpp src/FMU.cpp src/FMU1.cpp src/FMU2.cpp)

SET_TARGET_PROPERTIES(sfun_fmurun PROPERTIES PREFIX "")

target_compile_definitions(sfun_fmurun PUBLIC "EXPORT_EXTERN_C=__declspec(dllexport)")
target_compile_definitions(sfun_fmurun PUBLIC MATLAB_MEX_FILE)

target_include_directories(sfun_fmurun PUBLIC
	${MATLAB_DIR}/extern/include
	${MATLAB_DIR}/simulink/include
	include
	coder
)

if (WIN32)
  target_link_libraries(sfun_fmurun libmx libmat libmex shlwapi)
endif ()

set_target_properties(sfun_fmurun PROPERTIES SUFFIX ".${TARGET_SUFFIX}")

set_target_properties(sfun_fmurun PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY         "${CMAKE_CURRENT_SOURCE_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
)
