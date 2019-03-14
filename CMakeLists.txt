cmake_minimum_required (VERSION 3.2)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE TYPE INTERNAL FORCE)

set (CMAKE_CXX_STANDARD 11)

project (FMIKit)

if (MSVC)
  # link statically against the the Visual C runtime
  string(REPLACE "/MD"  "/MT"  CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
  string(REPLACE "/MD"  "/MT"  CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}")
endif ()

if (WIN32)
  if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(MATLAB_DIR "C:/Program Files/MATLAB/R2012b" CACHE STRING "MATLAB install directory")
    set(TARGET_SUFFIX mexw64)
    link_directories(${MATLAB_DIR}/extern/lib/win64/microsoft)
	else ()
    set(MATLAB_DIR "C:/Program Files (x86)/MATLAB/R2012b" CACHE STRING "MATLAB install directory")
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

target_compile_definitions(sfun_fmurun PUBLIC
  _CRT_SECURE_NO_WARNINGS
  "DLL_EXPORT_SYM=__declspec(dllexport)"
  MATLAB_MEX_FILE
)

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

add_custom_command(TARGET sfun_fmurun POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  "$<TARGET_FILE:sfun_fmurun>"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
