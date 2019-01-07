# CMake entry point
# Note that this CMakeLists.txt in fully independent from the one in the parent directory
# (since that's the goal of this tutorial !)

# For an introduction to CMake, see
# http://www.cmake.org/cmake/help/cmake_tutorial.html (at least steps 1 and 2)

message(
"
       
           ┌─┐       ┌─┐
        ┌──┘ ┴───────┘ ┴──┐
        │                 │
        │       ───       │
        │  ─┬┘       └┬─  │
        │                 │
        │       ─┴─       │
        │                 │
        └───┐         ┌───┘
            │         │
            │         │
            │         │
            │         └──────────────┐
            │                        │
            │                        ├─┐
            │                        ┌─┘    
            │                        │
            └─┐  ┐  ┌───────┬──┐  ┌──┘         
              │ ─┤ ─┤       │ ─┤ ─┤         
              └──┴──┘       └──┴──┘ 
                  神兽  保佑 
                  代码无BUG! 
  
")

cmake_minimum_required (VERSION 3.10)
set(name "NetVirtualCameraServer")
project ( ${name} )

#SET(CMAKE_C_COMPILER gcc)
#SET(CMAKE_CXX_COMPILER g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11")

set(CMAKE_CXX_STANDARD 11)

#opencv 
if ( WIN32 )
	set(OpenCV_DIR "E:/libs/OpenCV/x64/vc14/lib")
	set(OpenCV_CONFIG_PATH "E:/libs/OpenCV")
	set(WIN_HEADER_PATH "${PROJECT_SOURCE_DIR}/winheadfile")
    include_directories(${WIN_HEADER_PATH})
endif()

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


# pointgrey camera sdk
set(USE_PTGREY_CAMERA on)
if (USE_PTGREY_CAMERA)
    set(PTGREY_SDK_INCLUDE_DIR "~/zhu-ty/spinnaker/include")
    set(PTGREY_SDK_INCLUDE_DIR ${PTGREY_SDK_INCLUDE_DIR} "/usr/include/spinnaker/")
    if(EXISTS "/home/zhu-ty/zhu-ty/spinnaker/lib/libSpinnaker.so")
        set(PTGREY_SDK_LIBRARY_DEBUG debug "~/zhu-ty/spinnaker/lib/libSpinnaker.so")
        set(PTGREY_SDK_LIBRARY_RELEASE optimized "~/zhu-ty/spinnaker/lib/libSpinnaker.so")
    else()
    	set(PTGREY_SDK_LIBRARY_DEBUG debug "libSpinnaker.so")
    	set(PTGREY_SDK_LIBRARY_RELEASE optimized "libSpinnaker.so")
    endif()
endif()
include_directories(${PTGREY_SDK_INCLUDE_DIR})

#XIEMA
set(XIMEA_SDK_INCLUDE_DIR "/opt/XIMEA/include")
set(XIMEA_SDK_LIBRARY_DEBUG "/usr/lib/libm3api.so")
set(XIMEA_SDK_LIBRARY_RELEASE "/usr/lib/libm3api.so")
include_directories(${XIMEA_SDK_INCLUDE_DIR})

#cuda
find_package(CUDA)
set(CUDA_NVCC_FLAGS
    ${CUDA_NVCC_FLAGS}
#    -O3 
	-gencode=arch=compute_61,code=\"sm_61,compute_61\"
    )

set(MY_HEADER_FILES
    ${MY_HEADER_FILES}
	# C++ header file
	#cuda/CameraUtilKernel.h
	#cuda/NPPJpegCoderKernel.h
	#CameraUtil.h
	#CameraArray.h
	#NPPJpegCoder.h
	#helper_cuda.h
	#helper_string.h
    # Communication
    communication.h
    common.h
    colormod.h
    cameraControl.h
)

set(MY_SOURCE_FILES 
    ${MY_SOURCE_FILES}
	# C++ source file
	#CameraUtil.cpp
	#CameraArray.cpp
	main.cpp
	#NPPJpegCoder.cpp
    # communication
    communication.cpp
    cameraControl.cpp
	# cuda source file
	#cuda/CameraUtilKernel.cu
	#cuda/NPPJpegCoderKernel.cu
)

set(CAMERA_DRIVER_DIR ${PROJECT_SOURCE_DIR}/camera_driver)
set(CAMERA_DRIVER_HEADER_FILER
    ${CAMERA_DRIVER_DIR}/XIMEA/XIMEACamera.h
    ${CAMERA_DRIVER_DIR}/PointGrey/PointGreyCamera.h
    ${CAMERA_DRIVER_DIR}/GenCameraDriver.h
    ${CAMERA_DRIVER_DIR}/RealCameraDriver.h
    ${CAMERA_DRIVER_DIR}/NPPJpegCoder.h
    ${CAMERA_DRIVER_DIR}/FileCamera/FileCamera.h
    ${CAMERA_DRIVER_DIR}/Stereo/StereoCamera.h
    ${CAMERA_DRIVER_DIR}/Stereo/INIReader.h
    ${CAMERA_DRIVER_DIR}/Stereo/StereoRectify.h
    ${CAMERA_DRIVER_DIR}/Stereo/ExposureFusion.h
)
set(CAMERA_DRIVER_SOURCE_FILER
    ${CAMERA_DRIVER_DIR}/XIMEA/XIMEACamera.cpp
    ${CAMERA_DRIVER_DIR}/PointGrey/PointGreyCamera.cpp
    ${CAMERA_DRIVER_DIR}/GenCameraDriver.cpp
    ${CAMERA_DRIVER_DIR}/GenCameraDriverImpl.cpp
    ${CAMERA_DRIVER_DIR}/RealCameraDriver.cpp
    ${CAMERA_DRIVER_DIR}/NPPJpegCoder.cpp
    ${CAMERA_DRIVER_DIR}/FileCamera/FileCamera.cpp
    ${CAMERA_DRIVER_DIR}/Stereo/StereoCamera.cpp
    ${CAMERA_DRIVER_DIR}/Stereo/StereoRectify.cpp
    ${CAMERA_DRIVER_DIR}/Stereo/ExposureFusion.cpp
    ${CAMERA_DRIVER_DIR}/Stereo/ExposureFusion.cu
    ${CAMERA_DRIVER_DIR}/Stereo/DisparityProcessor.cu
)

#Stereo RealtimeDepth
if ( NOT WIN32 )
    FIND_PACKAGE(Eigen3 REQUIRED) 
    include_directories(${EIGEN3_INCLUDE_DIR})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unknown-pragmas -std=c++11 -msse4")  #-Wall
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wno-unknown-pragmas -msse4")  #-Wall
    include_directories(${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth)
    set(MY_HEADER_FILES
        ${MY_HEADER_FILES}
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/FDF1.0.1/image.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/FDF1.0.1/opticalflow_aux.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/FDF1.0.1/solver.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/depthmap.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/GhostElemer.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/patchgrid.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/refine_variational.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/DepthMapUpdater.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/oflow.h
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/patch.h
    )
    set(MY_SOURCE_FILES 
        ${MY_SOURCE_FILES}
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/FDF1.0.1/image.c
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/FDF1.0.1/opticalflow_aux.c
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/FDF1.0.1/solver.c
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/depthmap.cpp
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/GhostElemer.cpp
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/patch.cpp
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/refine_variational.cpp
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/DepthMapUpdater.cpp
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/oflow.cpp
        ${CAMERA_DRIVER_DIR}/Stereo/RealTimeDepth/patchgrid.cpp 
    )
    
endif()

include_directories(
	${PROJECT_SOURCE_DIR}	
	${CAMERA_DRIVER_DIR}
    ${CAMERA_DRIVER_DIR}/cuda  
    ${CAMERA_DRIVER_DIR}/XIMEA 
    ${CAMERA_DRIVER_DIR}/PointGrey
    ${CAMERA_DRIVER_DIR}/FileCamera
    ${CAMERA_DRIVER_DIR}/Stereo
)

cuda_add_executable(${name}
	${MY_HEADER_FILES}
	${MY_SOURCE_FILES}
    ${CAMERA_DRIVER_HEADER_FILER}
    ${CAMERA_DRIVER_SOURCE_FILER}
)

if(NOT WIN32)
    set_target_properties (${name} PROPERTIES COMPILE_DEFINITIONS "SELECTMODE=2")
    set_target_properties (${name} PROPERTIES COMPILE_DEFINITIONS "SELECTCHANNEL=1")
endif()

target_link_libraries(${name}
	${OpenCV_LIBS}
	${CUDA_LIBRARY}  
    ${CUDA_npp_LIBRARY} 
    ${CUDA_nppc_LIBRARY} 
    ${CUDA_nppicc_LIBRARY}
    ${CUDA_nppicom_LIBRARY}
    ${CUDA_nppidei_LIBRARY}
    ${CUDA_nppif_LIBRARY}
    ${CUDA_nppig_LIBRARY}
    ${CUDA_nppim_LIBRARY}
    ${CUDA_nppist_LIBRARY}
    ${CUDA_nppisu_LIBRARY}
    ${CUDA_nppitc_LIBRARY}
    ${CUDA_npps_LIBRARY} 
    debug ${PTGREY_SDK_LIBRARY_DEBUG} ${XIMEA_SDK_LIBRARY_DEBUG}
	optimized ${PTGREY_SDK_LIBRARY_RELEASE} ${XIMEA_SDK_LIBRARY_RELEASE}
)
