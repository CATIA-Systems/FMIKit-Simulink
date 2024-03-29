cmake_minimum_required (VERSION 3.22)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(MODEL_NAME "" CACHE STRING "Model name")
set(MATLAB_ROOT "C:/Program Files/MATLAB/R2016b" CACHE STRING "MATLAB install directory")
set(RTW_DIR "" CACHE STRING "RTW generated model directory")
set(ARCH "win64" CACHE STRING "System architecture")
set(CUSTOM_INCLUDE "" CACHE STRING "Additional include directories")
set(CUSTOM_SOURCE "" CACHE STRING "Additional source files")
set(CUSTOM_LIBRARY "" CACHE STRING "Additional static libraries")
set(SOURCE_CODE_FMU ON CACHE BOOL "Copy sources to FMU archive")
set(SIMSCAPE OFF CACHE BOOL "Model contains Simscape blocks")
set(FMU_BUILD_DIR FMUArchive)
set(FMI_VERSION 2 CACHE STRING "FMI Version")
set_property(CACHE FMI_VERSION PROPERTY STRINGS 2 3)
set(COMPILER_OPTIMIZATION_LEVEL "Disabled" CACHE STRING "Compiler optimization level")
set_property(CACHE COMPILER_OPTIMIZATION_LEVEL PROPERTY STRINGS "Disabled" "Minimize size" "Maximize speed" "Full optimization" "Custom")
set(COMPILER_OPTIMIZATION_FLAGS "/Ox" CACHE STRING "Custom compiler optimization flags")

# link statically against the Visual C runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

get_filename_component(CURRENT_DIR ${RTW_DIR} DIRECTORY)

project (${MODEL_NAME})

if ("${FMI_VERSION}" STREQUAL "3")

  if (WIN32)
    set(FMI_PLATFORM windows)
  elseif (APPLE)
    set(FMI_PLATFORM darwin)
  else ()
    set(FMI_PLATFORM linux)
  endif ()

  if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set (FMI_PLATFORM x86_64-${FMI_PLATFORM})
  else ()
    set (FMI_PLATFORM i686-${FMI_PLATFORM})
  endif ()

else ()

  if (WIN32)
    set(FMI_PLATFORM win)
  elseif (APPLE)
    set(FMI_PLATFORM darwin)
  else ()
    set(FMI_PLATFORM linux)
  endif ()

  if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set (FMI_PLATFORM ${FMI_PLATFORM}64)
  else ()
    set (FMI_PLATFORM ${FMI_PLATFORM}32)
  endif ()

endif ()

if (MSVC)
  # set configuration types
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING INTERNAL FORCE)

  message("CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE}")

  # set optimization flags
  if (COMPILER_OPTIMIZATION_LEVEL STREQUAL "Disabled")
    STRING(REPLACE "/O2" "/Od" CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
    STRING(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  elseif (COMPILER_OPTIMIZATION_LEVEL STREQUAL "Minimize size")
    STRING(REPLACE "/O2" "/O1" CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
    STRING(REPLACE "/O2" "/O1" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  elseif (COMPILER_OPTIMIZATION_LEVEL STREQUAL "Full optimization")
    STRING(REPLACE "/O2" "/Ox" CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
    STRING(REPLACE "/O2" "/Ox" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  elseif (COMPILER_OPTIMIZATION_LEVEL STREQUAL "Custom")
    STRING(REPLACE "/O2" "${COMPILER_OPTIMIZATION_FLAGS}" CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
    STRING(REPLACE "/O2" "${COMPILER_OPTIMIZATION_FLAGS}" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  endif ()

  message("CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE}")

endif ()

foreach (INCLUDE_DIR ${CUSTOM_INCLUDE})
  FILE(GLOB HEADER_FILES ${INCLUDE_DIR}/*.h)
  set(CUSTOM_HEADERS ${HEADER_FILES} ${CUSTOM_HEADERS})
endforeach (INCLUDE_DIR)

FILE(GLOB HEADERS          "${RTW_DIR}/*.h")
FILE(GLOB SHARED_HEADERS   "${MATLAB_ROOT}/rtw/c/src/*.h")
FILE(GLOB RTW_HEADERS      "${MATLAB_ROOT}/rtw/c/src/*.h")
FILE(GLOB MATLAB_HEADERS   "${MATLAB_ROOT}/extern/include/*.h")
FILE(GLOB SIMULINK_HEADERS "${MATLAB_ROOT}/simulink/include/*.h")

set(HEADERS ${HEADERS} ${CUSTOM_HEADERS})
set(HEADERS ${HEADERS} ${SHARED_HEADERS})
set(HEADERS ${HEADERS} ${RTW_HEADERS})
set(HEADERS ${HEADERS} ${MATLAB_HEADERS})
set(HEADERS ${HEADERS} ${SIMULINK_HEADERS})

FILE(GLOB RTW_SOURCES      "${RTW_DIR}/*.c")
FILE(GLOB SHARED_SOURCES   "${RTW_DIR}/../slprj/grtfmi/*/*.c")

set(SOURCES
  "${MATLAB_ROOT}/rtw/c/src/rt_matrx.c"
  ${RTW_SOURCES}
  ${SHARED_SOURCES}
  ${RTW_DIR}/fmiwrapper.inc
  ${CMAKE_SOURCE_DIR}/fmi${FMI_VERSION}Functions.c
  ${CUSTOM_SOURCE}
)

set(RTW_INCLUDE
  ${MATLAB_ROOT}/simulink/include
  ${MATLAB_ROOT}/extern/include
  ${MATLAB_ROOT}/rtw/c/src
  ${MATLAB_ROOT}/rtw/c/src/ext_mode/common
)

if (${SIMSCAPE})

  file(GLOB EX_SOURCES       "${MATLAB_ROOT}/extern/physmod/${ARCH}/ex/src/*.c")
  file(GLOB MC_SOURCES       "${MATLAB_ROOT}/extern/physmod/${ARCH}/mc/src/*.c")
  file(GLOB PM_SOURCES       "${MATLAB_ROOT}/extern/physmod/${ARCH}/pm/src/*.c")
  file(GLOB SSC_CORE_SOURCES "${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_core/src/*.c")
  file(GLOB SSC_SLI_SOURCES  "${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_sli/src/*.c")
  file(GLOB SSC_ST_SOURCES   "${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_st/src/*.c")

  set(SIMSCAPE_INCLUDE
    ${MATLAB_ROOT}/toolbox/physmod/simscape/engine/sli/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/simscape/engine/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/simscape/compiler/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/simscape/ds/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/simscape/simtypes/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/network_engine/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/math/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/logging/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/lang/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/external/library/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/foundation/core/c/${ARCH}
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/ex/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/lang/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/mc/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/pm/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_core/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_sli/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_st/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/pm_log/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_ds/include
    ${MATLAB_ROOT}/extern/physmod/${ARCH}/ssc_comp/include
  )

  foreach (INCLUDE_PATH ${SIMSCAPE_INCLUDE})
    file(GLOB COMPONENT_HEADERS "${INCLUDE_PATH}/*.h")
    set(SIMSCAPE_HEADERS ${SIMSCAPE_HEADERS} ${COMPONENT_HEADERS})
  endforeach(INCLUDE_PATH)

  set(SIMSCAPE_SOURCES
    ${EX_SOURCES}
    ${MC_SOURCES}
    ${NE_SOURCES}
    ${PM_SOURCES}
    ${SSC_CORE_SOURCES}
    ${SSC_SLI_SOURCES}
    ${SSC_ST_SOURCES}
  )

endif ()

add_library (${MODEL_NAME} SHARED
  ${RTW_DIR}/modelDescription.xml
  ${HEADERS}
  ${SOURCES}
  ${SIMSCAPE_SOURCES}
)

if (${SIMSCAPE})
    source_group("ex"       FILES ${EX_SOURCES})
    source_group("mc"       FILES ${MC_SOURCES})
    source_group("ne"       FILES ${NE_SOURCES})
    source_group("pm"       FILES ${PM_SOURCES})
    source_group("ssc_core" FILES ${SSC_CORE_SOURCES})
    source_group("ssc_sli"  FILES ${SSC_SLI_SOURCES})
    source_group("ssc_st"   FILES ${SSC_ST_SOURCES})
endif ()

set_property(TARGET ${MODEL_NAME} PROPERTY C_STANDARD 99)

target_include_directories (${MODEL_NAME} PUBLIC
  ../include
  ${CUSTOM_INCLUDE}
  ${CMAKE_SOURCE_DIR}
  ${RTW_DIR}
  ${RTW_DIR}/../slprj/grtfmi/_sharedutils
  ${RTW_DIR}/../slprj/grtfmi/${MODEL_NAME}
  ${RTW_INCLUDE}
  ${SIMSCAPE_INCLUDE}
)

# don't add the "lib" prefix to the shared library on Linux
set_target_properties(${MODEL_NAME} PROPERTIES PREFIX "")

target_compile_definitions(${MODEL_NAME} PUBLIC
  _CRT_SECURE_NO_WARNINGS
  RT
  RT_MALLOC
  USE_RTMODEL
  NO_FMI_FUNCTION_PREFIX
  GRTFMI
  ${CUSTOM_DEFINE}
)

target_link_libraries(${MODEL_NAME} ${CUSTOM_LIBRARY})

file(READ ${MATLAB_ROOT}/simulink/include/simstruc.h CONTENTS)
file(WRITE ${RTW_DIR}/Temp/simstruc_rt.h "
/* Add definitions to allow compilation without compiler options */
#define RT
#define USE_RTMODEL
")
file(APPEND ${RTW_DIR}/Temp/simstruc_rt.h "${CONTENTS}")

# modelDescription.xml
add_custom_command(TARGET ${MODEL_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  ${RTW_DIR}/modelDescription.xml
  ${FMU_BUILD_DIR}/modelDescription.xml
)

# buildDescription.xml
if (EXISTS ${RTW_DIR}/buildDescription.xml)
  add_custom_command(TARGET ${MODEL_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    ${RTW_DIR}/buildDescription.xml
    ${FMU_BUILD_DIR}/sources/buildDescription.xml
  )
endif ()

# shared library
add_custom_command(TARGET ${MODEL_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  $<TARGET_FILE:${MODEL_NAME}>
  ${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}/$<TARGET_FILE_NAME:${MODEL_NAME}>
)

# debug info
if (MSVC)
  add_custom_command(TARGET ${MODEL_NAME} POST_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/copypdb.bat"
    "$<TARGET_PDB_FILE:${MODEL_NAME}>"
    "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}/$<TARGET_PDB_FILE_NAME:${MODEL_NAME}>"
  )
endif()

set (ARCHIVE_FILES modelDescription.xml binaries)

# model.png
if (EXISTS ${FMU_BUILD_DIR}/model.png)
  set (ARCHIVE_FILES ${ARCHIVE_FILES} model.png)
endif ()

# documentation
if (EXISTS ${FMU_BUILD_DIR}/documentation)
  set (ARCHIVE_FILES ${ARCHIVE_FILES} documentation)
endif ()

# resources
if (EXISTS ${FMU_BUILD_DIR}/resources)
  set (ARCHIVE_FILES ${ARCHIVE_FILES} resources)
endif ()

if (SOURCE_CODE_FMU)
  # source code
  foreach (SOURCE_FILE ${HEADERS} ${SOURCES} ${SIMSCAPE_HEADERS} ${SIMSCAPE_SOURCES})
    get_filename_component(BASE_NAME ${SOURCE_FILE} NAME)
    add_custom_command(TARGET ${MODEL_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
      ${SOURCE_FILE}
      ${FMU_BUILD_DIR}/sources/${BASE_NAME}
    )
  endforeach(SOURCE_FILE)

  # copy simstruc.h with RT definitions
  add_custom_command(TARGET ${MODEL_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    ${RTW_DIR}/Temp/simstruc_rt.h
    ${FMU_BUILD_DIR}/sources/simstruc.h
  )

  # sources directory
  set (ARCHIVE_FILES ${ARCHIVE_FILES} sources)
endif ()

# create ZIP archive
add_custom_command(TARGET ${MODEL_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${CMAKE_CURRENT_BINARY_DIR}/${MODEL_NAME}.fmu --format=zip
  ${ARCHIVE_FILES}
  WORKING_DIRECTORY ${FMU_BUILD_DIR}
)
