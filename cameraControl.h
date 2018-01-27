/************************************************************************
//Describe: cameraControl.h: header file for cameraControl.cpp
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/14/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
************************************************************************/
#pragma once
#include "common.h"
//#include "MVCLTen.h"
//#include <opencv2/core.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//#include "imageSaver.h"

//#include "CameraArray.h"

#include "camera_driver/GenCameraDriver.h"

#define MAX_CAMERA_NUM 8
#define MAX_PARAM_NUM 10
#define MAX_PATH_LEN 256
#define MAX_SN_LEN MAX_PATH_LEN
static const int32_t imageWidthMin=1024;
static const int32_t imageWidthMax=4096*4;
static const int32_t imageHeightMin=768;
static const int32_t imageHeightMax=3000*2;
//static const int32_t exposureMin=1;
//static const int32_t exposureMax=1000000;
//static const int32_t gainMin=1;
//static const int32_t gainMax=255;
//static const int32_t brightnessMin=1;
//static const int32_t brightnessMax=96;
//static const int32_t contrastMin=1;
//static const int32_t contrastMax=96;
//static const int32_t bitLutMin=0;
//static const int32_t bitLutMax=8;
//static const int32_t skipNumberMin=0;
//static const int32_t skipNumberMax=255;
//static const int32_t imageResizedFactorMin=1;
//static const int32_t imageResizedFactorMax=10;
//static const int32_t cameraTemperatureMin=-5;
//static const int32_t cameraTemperatureMax=65;
//static const int32_t cameraCoolingTarget=10;
//static const int32_t cameraCoolingErrorMax=2;

struct GenCamInfoStruct 
{
    char sn[MAX_SN_LEN];
    int width;
    int height;
    float fps;
    int autoExposure;
    int bayerPattern;	
    float redGain;
    float greenGain;
    float blueGain;
    bool isWBRaw;
};


struct GenCameraControlData
{
    struct
    {
        int return_val;
    } void_func;
    struct
    {
        int return_val;
        GenCamInfoStruct camInfos[MAX_CAMERA_NUM];
    } caminfo_func;
    struct
    {
        int return_val;
        bool param_bool[MAX_PARAM_NUM];
        int param_enum[MAX_PARAM_NUM];
        int param_int[MAX_PARAM_NUM];
        float param_float[MAX_PARAM_NUM];
    } param_func;
    struct
    {
        int return_val;
        char str[MAX_PATH_LEN];
    } str_func;
};





///存储格式枚举定义
enum Image_Format
{
    Image_Format_None=0,
    Image_Format_Bmp=1,
    Image_Format_Raw=2,
    Image_Format_Bmp_Raw=3,
};


class CameraParametersUnit
{
public:
    CameraParametersUnit();
    ~CameraParametersUnit();

    int32_t id_ = 0;										//对应真实的相机编号，1~35
    int32_t width_ = 2560;									//图像宽度：最大2560
    int32_t height_ = 2160;									//图像高度：最大2160
    int32_t exposure_ = 255;								//相机曝光：1~65535
    int32_t gain_ = 3;										//相机增益：1~255
    int32_t brightness_=32;                                  //相机亮度：1~96 (x0.3125)
    int32_t contrast_=32;                                   //相机对比度：1~96 (x0.3125)
    int32_t bitLut_=8;                                      ///已经改为光源强度 考虑到兼容性暂不修改变量名！   ///输出灰度查找位：0~8 (bit[7+x:x])
    int32_t saveFormat_ = 1;								//保存格式：0:禁用 1:bmp/tiffrush 2:raw 3:bmp+raw
    int32_t skipNumber_ = 0;								//跳帧数：  0:noskip else: 1/(N+1)
    int32_t triggerMode_ = 2;								//触发模式：0：两路独立软触发或硬触发 1:两路同时软触发或硬触发，2：内部自由触发
    std::string savePath_ = "/mnt/dayu/gp/zhu-ty/tmp/";					    //linux下保存路径：~/tmp/		最大长度为256
    std::string saveName_ = "";							    //保存的文件前缀： test			最大长度为128
    //std::string othersInfo_= "";                                    ///Tiff保存的操作者、日期、实验目的信息  固定长度为8192

    void Lock();
    void Unlock();

private:
    PMutex mutex_;
};


//class CameraBoxUnit
//{
//public:
//    CameraBoxUnit();
//    ~CameraBoxUnit();
//	int32_t id_ = 0;										//对应真实的盒子编号，1~18
//	std::string mac_ = "00:11:1c:02:30:8c";					//盒子的mac地址  固定长度为17+1
//	int32_t cameraAmount_ = 2;								//盒子连接的相机数
//	std::vector<CameraParametersUnit> cameraVec_ = std::vector<CameraParametersUnit>(2, CameraParametersUnit());//各相机的配置参数
//
//    void ResizeCameraVec(uint32_t _cameraAmount);
//    void Lock();
//	void Unlock();
//
//private:
//    PMutex mutex_;
//};


// class CameraServerUnit
// {
// public:
//     CameraServerUnit();
//     ~CameraServerUnit();

// 	int32_t id_ = 0;										//对应真实的服务器编号，1~9
// 	bool connectedFlag_ = false;							//连接状态标志
// 	std::string ip_ = "10.0.0.11";					//服务器IP： 192.168.111.000 固定长度为15+1
// 	std::string port_ = "54321";							//服务器端口： 123456
// //	int32_t boxAmount_ = 2;									//服务器下连接的盒子数
// //	std::vector<CameraBoxUnit> boxVec_ = std::vector<CameraBoxUnit>(2, CameraBoxUnit());		//各盒子的配置参数

// //    void ResizeBoxVec(uint32_t _boxAmount);
//     void Lock();
// 	void Unlock();

// private:
//     PMutex mutex_;
// };



///相机控制线程处理各命令所需最长时间
static const int verifyCameraThreadExitMaxMs=500;
static const int verifyCameraOpenBoxMaxMs=5000;
static const int verifyCameraCloseBoxMaxMs=5000;
static const int verifyCameraOpenCameraMaxMs=5000;
static const int verifyCameraCloseCameraMaxMs=5000;
static const int verifyCameraTriggerContinousMaxMs=5000;
static const int verifyCameraTriggerSingleMaxMs=5000;
static const int verifyCameraResetIdMaxMs=5000;
static const int verifyCameraGetTemperatureMaxMs=5000;
static const int verifyCameraGetImageMaxMs=5000;

///相机控制线程动作
enum CameraControl_Action
{
    CameraControl_Thread_Wait=0,
    CameraControl_Thread_Exit=1,
//    CameraControl_Open_Box=2,
//    CameraControl_Close_Box=3,
    CameraControl_Open_Camera=4,
    CameraControl_Close_Camera=5,
//    CameraControl_Trigger_Continous=6,
//    CameraControl_Trigger_Single=7,
//    CameraControl_Reset_Id=8,
//    CameraCOntrol_Get_Temperature=9,
    CameraControl_Get_Image=10,
    CameraControl_Action_Valid=11,
    CameraControl_Action_Invalid=12,
    CameraControl_Action_Overtime=13,
};


///相机控制线程消息
class CameraControlMessage
{
public:
    CameraControlMessage()
    {
        requestor_="";
        action_=CameraControl_Thread_Wait;
    }
    CameraControlMessage(string _requestor,CameraControl_Action _action)
    {
        requestor_=_requestor;
        action_=_action;
    }
    ~CameraControlMessage() {}

    string requestor_="";                                           ///请求者
    CameraControl_Action action_=CameraControl_Action_Invalid;      ///命令，反馈为Ok或Invalid
    int32_t boxIndex_=0;                                            ///欲操作的boxIndex_
    int32_t cameraIndex_=0;                                         ///欲操作的cameraIndex
    int32_t openCameraOperationIndex_=-1;                           ///openCamera时欲操作的参数
    int32_t triggerNumber_=0;                                       ///欲触发的数目，-1：无限，1：1张,只在触发单张的时候起作用
    string saveName_="tmp";                                         ///保存的图片名称，只在触发单张的时候起作用
    int32_t imageType_=2;                                           ///图像类型，1:Mono_8Bit,2:Mono_16Bit,3:Color_8Bit,
    int32_t resizeFactor_=1;                                        ///图片缩放尺寸，用于客户端获取单张时使用
    int32_t resizedWidth_=2560;                                     ///反馈的图像缩放后宽度，
    int32_t resizedHeight_=2160;                                    ///反馈的图像缩放后高度
    char *imageData_=NULL;                                          ///反馈的图像起始地址，由通信线程分配
    int32_t imagelen = 0;                                           ///Varied jpeg image lens.
    int32_t imageamount = 0;                                        ///Camera amount

    string genfunc_="";
    GenCameraControlData gendata_;


    inline void operator=(CameraControlMessage &_value)             ///重载赋值运算符
    {
        requestor_=_value.requestor_;
        action_=_value.action_;
        boxIndex_=_value.boxIndex_;
        cameraIndex_=_value.cameraIndex_;
        triggerNumber_=_value.triggerNumber_;
        saveName_=_value.saveName_;
        imageType_=_value.imageType_;
        resizeFactor_=_value.resizeFactor_;
        resizedWidth_=_value.resizedWidth_;
        resizedHeight_=_value.resizedHeight_;
        imageData_=_value.imageData_;
    }

    inline bool operator==(CameraControlMessage &_rightVal) const   ///重载==运算符,完全比较
    {
        if(this->requestor_==_rightVal.requestor_&&this->action_==_rightVal.action_&&       \
                this->boxIndex_==_rightVal.boxIndex_&&this->cameraIndex_==_rightVal.cameraIndex_)
        {
            return true;
        }
        return false;
    }

    inline bool operator==(string &_requestor) const                ///重载==运算符,请求者名字比较
    {
        return requestor_==_requestor;
    }


    CameraControl_Action VerifyAction(CameraControl_Action _endAction,int _overTime)
    {
        long startTime=GetCurrentTimeMs(),nowTime=GetCurrentTimeMs()+1;
        while(nowTime-startTime<_overTime)
        {
            if(action_==_endAction)
            {
                return CameraControl_Action_Valid;
            }
            else if(action_==CameraControl_Action_Invalid)
            {
                return CameraControl_Action_Invalid;
            }
            else
            {
                SleepMs(2);
            }
            nowTime=GetCurrentTimeMs();
        }
        return CameraControl_Action_Overtime;
    }

};

///相机控制线程消息队列类型定义
typedef ThreadMessageDeque<CameraControlMessage>  CameraControlMessageDeque;


/////相机控制线程有关盒子的类
//class CameraControlThreadBoxUnit
//{
//public:
//    CMVCLTen *ptr_;
//    //std::vector<cv::Mat> imageVec_;
//    std::vector<uint16_t *> imageVec_;
//    std::vector<uint64_t> last_sid;
//    std::vector<bool> imageFlushVec_;
//    //bool imageShowEnable_=false;
//    //std::vector<string> imageShowWindowVec_;
//    //string saveSuffix_=".bmp";
//    //string saveSuffix_=".rushraw";
//    string saveSuffix_=".tiff";
//    string saveSingleSuffix_ = ".tiff";
//    std::vector<int> capturedImageCountVec_;
//    std::vector<ImageSaverThread *> imageSaverVec_;
//    static const int resizedFactorNumMax_=4;  ///resized factor=1,2,4,8
//    //static const int resizedFactorMax_=5;
//    //std::vector<std::vector<cv::Mat>> resizedImagesVec_;
//    std::vector<std::vector<uint16_t *>> resizedImagesVec_;
//    uint16_t * temp_[2][resizedFactorNumMax_]; ///2 cameras in a box
//    uint32_t resizedPixelNum_[resizedFactorNumMax_];
//    void Lock(void)
//    {
//        mutex_.Lock();
//    }
//    void Unlock(void)
//    {
//        mutex_.Unlock();
//    }
//
//private:
//    PMutex mutex_;
//};


///相机控制线程：用于初始化、设置参数、控制拍摄等
class CameraControlThread:public PThreadClass
{
public:
    CameraControlThread(std::shared_ptr<cam::GenCamera> _gencamera, CameraControlMessageDeque *_cameraControlMessageDeque);
    ~CameraControlThread();

    virtual int Run(void);
private:
    string thisName_="cameraControl";

    ///相机参数和消息队列入口
    //CameraServerUnit *cameraServerUnit_;
    CameraControlMessageDeque *cameraControlMessageDeque_;
    std::shared_ptr<cam::GenCamera> gencamera_;
    std::vector<cam::Imagedata> imgdata;
    std::vector<cam::GenCamInfo> camInfos;
    bool opened = false;

    //std::vector<CameraControlThreadBoxUnit> boxVec_;

    //void ResizeBoxVec(void);                ///动态修改连接的盒子数

    ///根据全局参数cameraServerUnit_来控制相机，必须确保cameraServerUnit_参数已被更新且合法
//    bool OpenBox(CameraControlMessage *requestorPtr_);              ///修改boxVec_.size()
//    bool CloseBox(CameraControlMessage *requestorPtr_);             ///修改boxVec_.size()，删除关闭的盒子
    bool OpenCamera(CameraControlMessage *requestorPtr_);           ///只修该所指盒子下的相机总数
    bool CloseCamera(CameraControlMessage *requestorPtr_);          ///只修该所指盒子下的相机总数
//    bool TriggerContinous(CameraControlMessage *requestorPtr_);     ///不修改盒子数和相机数
//    bool TriggerSingle(CameraControlMessage *requestorPtr_);        ///不修改盒子数和相机数
//    bool ResetId(CameraControlMessage *requestorPtr_);              ///不修改盒子数和相机数
//    bool GetTemperature(CameraControlMessage *requestorPtr_);       ///get all camera temperature
    bool GetImage(CameraControlMessage *requestorPtr_);             ///获取最新的图像缓存区数据

//    void CaptureOneFinished(int lDevIndex, CMVImage* lCallBackImg);
//    void CaptureMessage(int lDevIndex, MV_MESSAGE lMsg);
//
//    ///用户捕捉完成数据回调函数
//    static void CaptureCallbackFuncEntry(int lDevIndex, CMVImage* lCallBackImg, void* This){((CameraControlThread *)This)->CaptureOneFinished(lDevIndex,lCallBackImg);}
//    static void MessageCallbackFuncEntry(int lDevIndex, MV_MESSAGE lMsg, void* This){((CameraControlThread *)This)->CaptureMessage(lDevIndex,lMsg); }

    void Resize(uint16_t *imgSrc, uint16_t *imgDst, uint32_t scale, uint32_t widthDst, uint32_t heightDst);
};





















