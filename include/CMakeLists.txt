cmake_minimum_required (VERSION 3.2)

set(SFUNCTION_SOURCE "E:/Development/FMIKit-Simulink/playground/sfun_Rectifier21xd3.c" CACHE STRING "S-function source file")
set(UNZIP_DIR "E:/Development/FMIKit-Simulink/playground/Rectifier21xd3/sources" CACHE STRING "FMU unzip directory")
set(FMU_SOURCES "E:/Development/FMIKit-Simulink/playground/Rectifier21xd3/sources/all.c" CACHE STRING "FMU source files")

get_filename_component(SFUNCTION_DIR ${SFUNCTION_SOURCE} DIRECTORY)
get_filename_component(SFUNCTION_NAME ${SFUNCTION_SOURCE} NAME_WE)

project (${SFUNCTION_NAME})

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

add_library(${SFUNCTION_NAME} SHARED
  ../include/fmi2Functions.h
  ../include/fmi2FunctionTypes.h
  ../include/fmi2TypesPlatform.h
  ${SFUNCTION_SOURCE}
  ${FMU_SOURCES}
)

SET_TARGET_PROPERTIES(${SFUNCTION_NAME} PROPERTIES PREFIX "")

if (WIN32)
  target_compile_definitions(${SFUNCTION_NAME} PUBLIC
    MATLAB_MEX_FILE
    _CRT_SECURE_NO_WARNINGS
    MW_NEEDS_VERSION_H
    "DLL_EXPORT_SYM=__declspec(dllexport)"
  )
else ()
  target_compile_definitions(${SFUNCTION_NAME} PUBLIC MATLAB_MEX_FILE)
endif ()

target_include_directories(${SFUNCTION_NAME} PUBLIC
	${MATLAB_DIR}/extern/include
	${MATLAB_DIR}/simulink/include
  ${UNZIP_DIR}
  .
)

if (WIN32)
  target_link_libraries(${SFUNCTION_NAME} libmat libmex libmx shlwapi)
elseif (APPLE)
  target_link_libraries(${SFUNCTION_NAME}
    ${MATLAB_DIR}/bin/maci64/libmat.dylib
    ${MATLAB_DIR}/bin/maci64/libmex.dylib
    ${MATLAB_DIR}/bin/maci64/libmx.dylib
  )
endif ()

set_target_properties(${SFUNCTION_NAME} PROPERTIES SUFFIX ".${TARGET_SUFFIX}")

install(TARGETS ${SFUNCTION_NAME} RUNTIME DESTINATION ${SFUNCTION_DIR})
