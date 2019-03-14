cmake_minimum_required (VERSION 3.2)

set(MATLAB_ROOT "C:/Program Files/MATLAB/R2016b")
set(MODEL_PATH "C:/Users/tsr2/Downloads/fum_export/f14.slx" CACHE STRING "Path to the Simulink model")

get_filename_component(MODEL_DIR  ${MODEL_PATH} DIRECTORY)
get_filename_component(MODEL_NAME ${MODEL_PATH} NAME_WE)

project (${MODEL_NAME})

# solver
add_library(ode1 STATIC
  "${MATLAB_ROOT}/rtw/c/src/ode1.c"
)

target_compile_definitions(ode1 PRIVATE 
	_CRT_SECURE_NO_WARNINGS
	NRT
	RT_MALLOC
)

target_include_directories(ode1 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${MODEL_DIR}/${MODEL_NAME}_sfcn_rtw_fmi"
  "${MATLAB_ROOT}/extern/include"
  "${MATLAB_ROOT}/simulink/include"
  "${MATLAB_ROOT}/rtw/c/src/matrixmath"
  "${MATLAB_ROOT}/rtw/c/src"
)

# model
add_library(${MODEL_NAME}_sf SHARED
  c/fmi.h
  c/fmi/fmi2Functions.h
  c/fmi/fmi2FunctionTypes.h
  c/fmi/fmi2TypesPlatform.h
  "${MODEL_DIR}/${MODEL_NAME}_sfcn_rtw_fmi/${MODEL_NAME}_sf_data.c"
  "${MODEL_DIR}/${MODEL_NAME}_sfcn_rtw_fmi/rtGetInf.c"
  "${MODEL_DIR}/${MODEL_NAME}_sfcn_rtw_fmi/rtGetNaN.c"
  "c/fmi2Functions.c"
  "c/fmi2.c"
)

target_link_libraries(${MODEL_NAME}_sf
  "${MATLAB_ROOT}/extern/lib/win64/microsoft/libmx.lib"
  "${MATLAB_ROOT}/extern/lib/win64/microsoft/libmex.lib"
  "${MATLAB_ROOT}/extern/lib/win64/microsoft/sf_runtime.lib"
  ode1
)

target_compile_definitions(${MODEL_NAME}_sf PRIVATE 
	_CRT_SECURE_NO_WARNINGS
	FIPXT_SHARED_MODULE
	rt_matrx_h
	RT_MALLOC
	NDEBUG
)

target_include_directories(${MODEL_NAME}_sf PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${MODEL_DIR}/${MODEL_NAME}_sfcn_rtw_fmi"
  "${MATLAB_ROOT}/extern/include"
  "${MATLAB_ROOT}/simulink/include"
  "${MATLAB_ROOT}/rtw/c/src/matrixmath"
  "${MATLAB_ROOT}/rtw/c/src"
)
