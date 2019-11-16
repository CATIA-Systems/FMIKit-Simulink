cmake_minimum_required (VERSION 3.2)

set(MODEL "" CACHE STRING "Model name")
set(MATLAB_ROOT "C:/Program Files/MATLAB/R2016b" CACHE STRING "MATLAB install directory")
set(RTW_DIR "" CACHE STRING "RTW generated model directory")
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

get_filename_component(CURRENT_DIR ${RTW_DIR} DIRECTORY)

project (${MODEL})

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

# MATLAB architecture
if (WIN32)
  set(ARCH win)
elseif (APPLE)
  set(ARCH maci)
else ()
  set(ARCH glnxa)
endif ()

if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set (ARCH ${ARCH}64)
else ()
  set (ARCH ${ARCH}32)
endif ()

if (MSVC)
  # set configuration types
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING INTERNAL FORCE)

  # link statically against the Visual C runtime
  string(REPLACE "/MD"  "/MT"  CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
  string(REPLACE "/MD"  "/MT"  CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}")

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
FILE(GLOB SHARED_SOURCES   "${RTW_DIR}/../slprj/grtfmi/_sharedutils/*.c")
FILE(GLOB REFMODEL_SOURCES "${RTW_DIR}/../slprj/grtfmi/sldemo_mdlref_counter_bus/*.c")

set(SOURCES
  ${RTW_SOURCES}
  ${SHARED_SOURCES}
  ${REFMODEL_SOURCES}
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

  file(GLOB EX_SOURCES       "${MATLAB_ROOT}/toolbox/physmod/common/external/library/c/${ARCH}/*.c")
  file(GLOB MC_SOURCES       "${MATLAB_ROOT}/toolbox/physmod/common/math/core/c/${ARCH}/*.c")
  file(GLOB NE_SOURCES       "${MATLAB_ROOT}/toolbox/physmod/network_engine/c/${ARCH}/*.c")
  file(GLOB PM_SOURCES       "${MATLAB_ROOT}/toolbox/physmod/common/foundation/core/c/${ARCH}/*.c")
  file(GLOB SSC_CORE_SOURCES "${MATLAB_ROOT}/toolbox/physmod/simscape/engine/core/c/${ARCH}/*.c")
  file(GLOB SSC_SLI_SOURCES  "${MATLAB_ROOT}/toolbox/physmod/simscape/engine/sli/c/${ARCH}/*.c")

  add_library (ex       STATIC ${EX_SOURCES})
  add_library (mc       STATIC ${MC_SOURCES})
  add_library (ne       STATIC ${NE_SOURCES})
  add_library (pm       STATIC ${PM_SOURCES})
  add_library (ssc_core STATIC ${SSC_CORE_SOURCES})
  add_library (ssc_sli  STATIC ${SSC_SLI_SOURCES})

  set(SIMSCAPE_INCLUDE
    ${MATLAB_ROOT}/toolbox/physmod/simscape/engine/sli/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/simscape/engine/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/simscape/compiler/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/network_engine/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/math/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/lang/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/external/library/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/common/foundation/core/c/${ARCH}
    ${MATLAB_ROOT}/toolbox/physmod/simscape/simscapedemos
  )

  target_include_directories (ex       PUBLIC ${RTW_DIR} ${RTW_INCLUDE} ${SIMSCAPE_INCLUDE})
  target_include_directories (mc       PUBLIC ${RTW_DIR} ${RTW_INCLUDE} ${SIMSCAPE_INCLUDE})
  target_include_directories (ne       PUBLIC ${RTW_DIR} ${RTW_INCLUDE} ${SIMSCAPE_INCLUDE})
  target_include_directories (pm       PUBLIC ${RTW_DIR} ${RTW_INCLUDE} ${SIMSCAPE_INCLUDE})
  target_include_directories (ssc_core PUBLIC ${RTW_DIR} ${RTW_INCLUDE} ${SIMSCAPE_INCLUDE})
  target_include_directories (ssc_sli  PUBLIC ${RTW_DIR} ${RTW_INCLUDE} ${SIMSCAPE_INCLUDE})

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
  )

endif ()

add_library (${MODEL} SHARED
  ${RTW_DIR}/modelDescription.xml
  ${HEADERS}
  ${SOURCES}
)

target_include_directories (${MODEL} PUBLIC
  ${CUSTOM_INCLUDE}
  ${CURRENT_DIR}
  ${CMAKE_SOURCE_DIR}
  ${RTW_DIR}
  ${RTW_DIR}/../slprj/grtfmi/_sharedutils
  ${RTW_DIR}/../slprj/grtfmi/${MODEL}
  ${RTW_INCLUDE}
  ${SIMSCAPE_INCLUDE}
)

# don't add the "lib" prefix to the shared library on Linux
set_target_properties(${MODEL} PROPERTIES PREFIX "")

target_compile_definitions(${MODEL} PUBLIC
  _CRT_SECURE_NO_WARNINGS
  RT
  RT_MALLOC
  USE_RTMODEL
  NO_FMI_FUNCTION_PREFIX
  GRTFMI
)

target_link_libraries(${MODEL} ${CUSTOM_LIBRARY})

if (${SIMSCAPE})
  target_link_libraries(${MODEL} ex mc ne pm ssc_core ssc_sli)
endif ()

file(READ ${MATLAB_ROOT}/simulink/include/simstruc.h CONTENTS)
file(WRITE ${RTW_DIR}/Temp/simstruc_rt.h "
/* Add definitions to allow compilation without compiler options */
#define RT
#define USE_RTMODEL
")
file(APPEND ${RTW_DIR}/Temp/simstruc_rt.h "${CONTENTS}")

# modelDescription.xml
add_custom_command(TARGET ${MODEL} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  ${RTW_DIR}/modelDescription.xml
  ${FMU_BUILD_DIR}/modelDescription.xml
)

# shared library
add_custom_command(TARGET ${MODEL} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  $<TARGET_FILE:${MODEL}>
  ${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}/$<TARGET_FILE_NAME:${MODEL}>
)

set (ARCHIVE_FILES modelDescription.xml binaries)

if (EXISTS ${FMU_BUILD_DIR}/resources)
  set (ARCHIVE_FILES ${ARCHIVE_FILES} resources)
endif ()

if (SOURCE_CODE_FMU)
  # source code
  foreach (SOURCE_FILE ${HEADERS} ${SOURCES} ${SIMSCAPE_HEADERS} ${SIMSCAPE_SOURCES})
    get_filename_component(BASE_NAME ${SOURCE_FILE} NAME)
    add_custom_command(TARGET ${MODEL} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
      ${SOURCE_FILE}
      ${FMU_BUILD_DIR}/sources/${BASE_NAME}
    )
  endforeach(SOURCE_FILE)

  # copy simstruc.h with RT definitions
  add_custom_command(TARGET ${MODEL} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    ${RTW_DIR}/Temp/simstruc_rt.h
    ${FMU_BUILD_DIR}/sources/simstruc.h
  )

  # sources directory
  set (ARCHIVE_FILES ${ARCHIVE_FILES} sources)
endif ()

# create ZIP archive
add_custom_command(TARGET ${MODEL} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${CMAKE_CURRENT_BINARY_DIR}/${MODEL}.fmu --format=zip
  ${ARCHIVE_FILES}
  WORKING_DIRECTORY ${FMU_BUILD_DIR}
)
