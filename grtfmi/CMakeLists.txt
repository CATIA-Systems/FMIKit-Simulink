cmake_minimum_required (VERSION 3.2)

set(MODEL "" CACHE STRING "Model name")
set(MATLAB_ROOT "C:/Program Files/MATLAB/R2016b" CACHE STRING "MATLAB install directory")
set(RTW_DIR "" CACHE STRING "RTW generated model directory")
set(CUSTOM_INCLUDE "" CACHE STRING "Additional include directories")
set(CUSTOM_SOURCE "" CACHE STRING "Additional source files")
set(SOURCE_CODE_FMU ON CACHE BOOL "Copy sources to FMU archive")
set(FMU_BUILD_DIR FMUArchive)
set(FMI_VERSION 2 CACHE STRING "FMI Version")
set_property(CACHE FMI_VERSION PROPERTY STRINGS 2 3)

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

if (MSVC)
  # link statically against the Visual C runtime
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE TYPE INTERNAL FORCE)

  string(REPLACE "/MD"  "/MT"  CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
  string(REPLACE "/MD"  "/MT"  CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

  string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}")
  string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}")

  # disable optimization
  STRING(REPLACE "/O2"  "/Od"  CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}")
  STRING(REPLACE "/O2"  "/Od"  CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
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

FILE(GLOB SOURCES "${RTW_DIR}/*.c")

FILE(GLOB SHARED_SOURCES   "${RTW_DIR}/../slprj/grtfmi/_sharedutils/*.c")
FILE(GLOB REFMODEL_SOURCES "${RTW_DIR}/../slprj/grtfmi/sldemo_mdlref_counter_bus/*.c")

set(SOURCES
  ${SOURCES}
  ${SHARED_SOURCES}
  ${REFMODEL_SOURCES}
  ${RTW_DIR}/fmiwrapper.inc
  ${CMAKE_SOURCE_DIR}/fmi${FMI_VERSION}Functions.c
  ${CUSTOM_SOURCE}
)

add_library (${MODEL} SHARED
  ${RTW_DIR}/modelDescription.xml
  ${HEADERS}
  ${SOURCES}
)

target_include_directories (${MODEL} PUBLIC
  ${CUSTOM_INCLUDE}
  ${CMAKE_SOURCE_DIR}
  ${RTW_DIR}
  ${RTW_DIR}/../slprj/grtfmi/_sharedutils
  ${RTW_DIR}/../slprj/grtfmi/sldemo_mdlref_counter_bus
  ${MATLAB_ROOT}/extern/include
  ${MATLAB_ROOT}/simulink/include
  ${MATLAB_ROOT}/rtw/c/src
  ${MATLAB_ROOT}/rtw/c/src/ext_mode/common
)

# don't add the "lib" prefix to the shared library on Linux
set_target_properties(${MODEL} PROPERTIES PREFIX "")

target_compile_definitions(${MODEL} PUBLIC
  _CRT_SECURE_NO_WARNINGS
  RT
  USE_RTMODEL
)

file(READ ${MATLAB_ROOT}/simulink/include/simstruc.h CONTENTS)
file(WRITE ${RTW_DIR}/Temp/simstruc_rt.h "
/* Add definitions to allow compilation without compiler options */
#define RT
#define USE_RTMODEL
")
file(APPEND ${RTW_DIR}/Temp/simstruc_rt.h "${CONTENTS}")

# remove FMU_BUILD_DIR from previous build
add_custom_command(TARGET ${MODEL} POST_BUILD COMMAND ${CMAKE_COMMAND} -E remove_directory
  ${FMU_BUILD_DIR}
)

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

if (SOURCE_CODE_FMU)
  # source code
  foreach (SOURCE_FILE ${HEADERS} ${SOURCES})
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
