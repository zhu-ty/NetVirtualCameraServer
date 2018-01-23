# NetVirtualCameraServer

##introduction
Generic Industry Camera Driver Network ServerVersion, support capturing (software synchronization), JPEG compression, Saving to video

should be used with zhu-ty/NetVirtualCamera

Now two kinds of cameras are support 

1. PointGrey (only for color cameras, tested with FL3-U3-120S3C)
2. XIMEA (tested with MC031CG-SY-UB)

## Libraries
1. OpenCV 3.4.0 with CUDA 9.1
2. CUDA 9.1
3. CMake >= 3.10 (FindCUDA in low version can not work for CUDA 9.1 since NPP library has changed a lot in CUDA 9)
4. Spinnaker SDK for PointGrey cameras
5. XIMEA SDK for XIMEA cameras

## Notice
Tested with Ubuntu 16.04

There may be other bugs. #^_^#

Xiaoyun YUAN & Tianyi Zhu

