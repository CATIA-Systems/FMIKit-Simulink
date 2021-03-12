cmake_minimum_required (VERSION 3.2)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING INTERNAL FORCE)

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
  set(MATLAB_DIR "/Applications/MATLAB_R2018b.app" CACHE STRING "MATLAB install directory")
  set(TARGET_SUFFIX mexmaci64)
  set (CMAKE_SHARED_LINKER_FLAGS "-Wl,-exported_symbols_list,${MATLAB_DIR}/extern/lib/maci64/mexFunction.map")
else ()
  set(TARGET_SUFFIX mexa64)
endif ()

add_library(sfun_fmurun SHARED
  include/fmi1Functions.h
  include/fmi2Functions.h
  include/fmi2FunctionTypes.h
  include/fmi2TypesPlatform.h
  include/fmi3Functions.h
  include/fmi3FunctionTypes.h
  include/fmi3PlatformTypes.h
  include/FMI.h
  include/FMI1.h
  include/FMI2.h
  src/sfun_fmurun.c
)

SET_TARGET_PROPERTIES(sfun_fmurun PROPERTIES PREFIX "")

if (WIN32)
  target_compile_definitions(sfun_fmurun PUBLIC
    MATLAB_MEX_FILE
    _CRT_SECURE_NO_WARNINGS
    "DLL_EXPORT_SYM=__declspec(dllexport)"
  )
else ()
  target_compile_definitions(sfun_fmurun PUBLIC MATLAB_MEX_FILE)
endif ()

target_include_directories(sfun_fmurun PUBLIC
	${MATLAB_DIR}/extern/include
	${MATLAB_DIR}/simulink/include
	include
	src
    .
)

if (WIN32)
  target_link_libraries(sfun_fmurun libmat libmex libmx)
elseif (APPLE)
  target_link_libraries(sfun_fmurun
    ${MATLAB_DIR}/bin/maci64/libmat.dylib
    ${MATLAB_DIR}/bin/maci64/libmex.dylib
    ${MATLAB_DIR}/bin/maci64/libmx.dylib
  )
endif ()

set_target_properties(sfun_fmurun PROPERTIES SUFFIX ".${TARGET_SUFFIX}")

add_custom_command(TARGET sfun_fmurun POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  "$<TARGET_FILE:sfun_fmurun>"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

# add_custom_command(TARGET sfun_fmurun POST_BUILD COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/merge_sfun.py)
