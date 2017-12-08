/************************************************************************
//Describe: communication.h: header file for communication.cpp
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/14/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
************************************************************************/
#pragma once
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <cerrno>
#include <sys/epoll.h>

#include "common.h"
#include "cameraControl.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

/*****************************************以下为与服务器端的通信命令定义******************************************************************************/
//#define COMMUNICATION_COMMAND_DATA_MAX_SIZE  (8192+512)					///通信命令数据包最大长度
#define COMMUNICATION_COMMAND_DATA_MAX_SIZE  512			///通信命令数据包最大长度
#define CAMERA_IMAGE_DATA_MAX_SIZE    (2560*2160*2+1024)	    ///图像数据最大长度

///以下为相机控制的命令与状态定义
enum Communication_Camera_Command
{
    Communication_Camera_Reset_Connection = 0,				    ///复位socket连接,用于本地，反馈为Get_Status
    Communication_Camera_Get_Status = 1,					    ///用于监测连接及设备状态,用于心跳包
    Communication_Camera_Open_Box = 2,						    ///同时设置图像宽度，高度，网络延迟以及路径
    Communication_Camera_Close_Box = 3,						    ///关闭盒子,再次启动需要重新初始化
    Communication_Camera_Open_Camera = 4,					    ///同时设置曝光、增益、存图格式、跳帧、触发模式
    Communication_Camera_Close_Camera = 5,					    ///以盒子为单位关闭相机
    Communication_Camera_Trigger_Continous = 6,				    ///连续触发,内部自由运行或外部触发信号
    Communication_Camera_Trigger_Single = 7,				    ///触发单张,从连续流中捕捉单张
    Communication_Camera_Reset_Id = 8,						    ///复位存图Id
    Communication_Camera_Get_Image = 9						    ///读取拍摄图
};

enum Communication_Camera_Status
{
    Communication_Camera_Get_Status_Ok = 0,						///获取状态命令有效
    Communication_Camera_Get_Status_Invalid = 1,				///获取状态命令无效
    Communication_Camera_Open_Box_Ok = 2,						///打开盒子命令有效
    Communication_Camera_Open_Box_Invalid = 3,					///打开盒子命令无效
    Communication_Camera_Close_Box_Ok = 4,						///关闭盒子命令有效
    Communication_Camera_Close_Box_Invalid=5,					///关闭盒子命令无效
    Communication_Camera_Open_Camera_Ok = 6,					///打开相机命令有效
    Communication_Camera_Open_Camera_Invalid=7,					///打开相机命令无效
    Communication_Camera_Close_Camera_Ok = 8,					///关闭相机命令有效
    Communication_Camera_Close_Camera_Invalid = 9,				///关闭相机命令无效
    Communication_Camera_Trigger_Continous_Ok = 10,				///连续触发命令有效
    Communication_Camera_Trigger_Continous_Invalid = 11,		///连续触发命令无效
    Communication_Camera_Trigger_Single_Ok = 12,				///触发单张命令有效
    Communication_Camera_Trigger_Single_Invalid = 13,			///触发单张命令有效
    Communication_Camera_Reset_Id_Ok = 14,						///复位Id命令有效
    Communication_Camera_Reset_Id_Invalid = 15,					///复位Id命令有效
    Communication_Camera_Get_Image_Ok = 16,						///读取图片命令有效
    Communication_Camera_Get_Image_Invalid = 17,				///读取图片命令有效
    Communication_Camera_Action_Overtime = 18,					///相机控制动作命令超时
    Communication_Camera_Action_Invalid = 19					///相机控制动作命令超时
};


class CameraOpenBoxPackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Open_Box;		///命令
    int32_t boxAmount_ = 2;																///当前服务器下的盒子总数
    int32_t boxIndex_ = 0;																///预操作的盒子号
    char macAddress_[18] = "00:00:00:00:00:00";											///盒子的mac地址 固定长度为18

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Open_Box" << endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "macAddress: " << macAddress_ << endl;
    }
};


class CameraCloseBoxPackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Close_Box;		///命令
    int32_t boxAmount_ = 2;											///当前服务器下的盒子总数
    int32_t boxIndex_ = 0;											///预操作的盒子号
    char macAddress_[18] = "00:00:00:00:00:00";						///盒子的mac地址 固定长度为18

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Close_Box" << endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "macAddress: " << macAddress_ << endl;
    }
};


class CameraOpenCameraPackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Open_Camera;		///命令
    int32_t operationIndex_=-1;                                     ///欲操作参数序号，-1：整个相机参数，其它：从exposure的第几个参数
    int32_t boxAmount_ = 2;											///当前服务器下的盒子总数
    int32_t boxIndex_ = 0;											///预操作的盒子号
    int32_t cameraAmount_ = 2;										///预操作的盒子下的相机总数
    int32_t cameraIndex_ = 0;										///预操作的盒子下的相机编号
    int32_t cameraId_ = 0;											///相机的整体编号1~35
    int32_t width_ = 2560;											///图像宽度：最大2560
    int32_t height_ = 2160;											///图像高度：最大2160
    int32_t exposure_ = 255;										///0:相机曝光：1~65535
    int32_t gain_ = 3;												///1:相机增益：1~255
    int32_t brightness_=32;                                         ///2:相机亮度：1~96 (x0.3125)
    int32_t contrast_=32;                                           ///3:相机对比度：1~96 (x0.3125)
    int32_t bitLut_=8;                                              ///4:输出灰度查找位：0~8 (bit[7+x:x])
    int32_t saveFormat_ = 1;										///5:保存格式：0:禁用 1:bmp 2:raw 3:bmp+raw
    int32_t skipNumber_ = 0;										///6:跳帧数：0:noskip else: 1/(N+1)
    int32_t triggerMode_ = 1;										///7:触发模式：0：两路独立软触发或硬触发 1:两路同时软触发或硬触发，2：内部自由触发，一直采集图像
    char savePath_[256] = "/home/tmp";								///8:linux下保存路径：/home/tmp/		固定长度为256
    char saveName_[128] = "";										///8:保存的文件名： test				固定长度为128
    //char othersInfo_[8192] = "";                                    ///Tiff保存的操作者、日期、实验目的信息  固定长度为8192

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Open_Camera" << endl;
        std::cout << "         " << "operationIndex: "<<operationIndex_<<endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
        std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
        std::cout << "         " << "cameraId: " << cameraId_ << endl;
        std::cout << "         " << "width: " << width_ << endl;
        std::cout << "         " << "height: " << height_ << endl;
        std::cout << "         " << "exposure: " << exposure_ << endl;
        std::cout << "         " << "gain: " << gain_ << endl;
        std::cout << "         " << "brightness:  "<<brightness_<<endl;
        std::cout << "         " << "contrast: "<<contrast_<<endl;
        std::cout << "         " << "bitLut:  "<<bitLut_<<endl;
        std::cout << "         " << "saveFormat: " << saveFormat_ << endl;
        std::cout << "         " << "skipNumber: " << skipNumber_ << endl;
        std::cout << "         " << "triggerMode: " << triggerMode_ << endl;
        std::cout << "         " << "savePath: " << savePath_ << endl;
        std::cout << "         " << "saveName: " << saveName_ << endl;
    }
};



class CameraCloseCameraPackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Close_Camera;	//命令
    int32_t boxAmount_ = 2;											//当前服务器下的盒子总数
    int32_t boxIndex_ = 0;											//预操作的盒子号
    int32_t cameraAmount_ = 2;										//预操作的盒子下的相机总数
    int32_t cameraIndex_ = 0;										//预操作的盒子下的相机编号

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Close_Camera" << endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
        std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
    }
};


class CameraTriggerContinousPackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Trigger_Continous;	//命令
    int32_t boxAmount_ = 2;												//当前服务器下的盒子总数
    int32_t boxIndex_ = 0;												//预操作的盒子号
    int32_t cameraAmount_ = 2;											//预操作的盒子下的相机总数
    int32_t cameraIndex_ = 0;											//预操作的盒子下的相机编号
    int32_t triggerNumber_ = -1;										//采集数量：在triggerMode_为2时无效，将会一直采图，在触发模式为0或1时，设为-1将会一直采图，大于0采集指定的数量
    char saveName_[128] = "";											//保存的文件名：固定长度为128

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Trigger_Continous" << endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
        std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
        std::cout << "         " << "triggerNumber: " << triggerNumber_ << endl;
        std::cout << "         " << "saveName: " << saveName_ << endl;
    }
};


class CameraTriggerSinglePackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Trigger_Single;	//命令
    int32_t boxAmount_ = 2;											//当前服务器下的盒子总数
    int32_t boxIndex_ = 0;											//预操作的盒子号
    int32_t cameraAmount_ = 2;										//预操作的盒子下的相机总数
    int32_t cameraIndex_ = 0;										//预操作的盒子下的相机编号
    int32_t triggerNumber_ = 1;										//采集数量：在triggerMode_为2时无效，将会一直采图，在触发模式为0或1时，设为-1将会一直采图，大于0采集指定的数量
    char saveName_[128] = "";										//保存的文件名：固定长度为128

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Trigger_Single" << endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
        std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
        std::cout << "         " << "triggerNumber: " << triggerNumber_ << endl;
        std::cout << "         " << "saveName: " << saveName_ << endl;
    }
};


class CameraResetIdPackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Reset_Id;		//命令
    int32_t boxAmount_ = 2;											//当前服务器下的盒子总数
    int32_t boxIndex_ = 0;											//预操作的盒子号
    int32_t cameraAmount_ = 2;										//预操作的盒子下的相机总数
    int32_t cameraIndex_ = 0;										//预操作的盒子下的相机编号

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Reset_Id" << endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
        std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
    }
};


class CameraGetImagePackage
{
public:
    const Communication_Camera_Command command_ = Communication_Camera_Get_Image;		//命令
    int32_t boxAmount_ = 2;																///当前服务器下的盒子总数
    int32_t boxIndex_ = 0;																///预操作的盒子号
    int32_t cameraAmount_ = 2;															///预操作的盒子下的相机总数
    int32_t cameraIndex_ = 0;															///预操作的盒子下的相机编号
    int32_t imageType_ = 2;																///图像类型，1:Mono_8Bit,2:Mono_16Bit,3:Color_8Bit,
    int32_t resizeFactor_ = 1;															///图像插值等级,长宽比恒定，resizeFactor=src/dst，最小为1，最大为10
    int32_t resizedWidth_ = 2560;														///插值图像宽度，该值由请求者指定
    int32_t resizedHeight_ = 2160;														///插值图像高度，该值由请求者指定

    void PrintInfo(std::string _str)
    {
        std::cout << _str << endl;
        std::cout << "Command: " << "Communication_Camera_Get_Image" << endl;
        std::cout << "         " << "boxAmount: " << boxAmount_ << endl;
        std::cout << "         " << "boxIndex: " << boxIndex_ << endl;
        std::cout << "         " << "cameraAmount: " << cameraAmount_ << endl;
        std::cout << "         " << "cameraIndex: " << cameraIndex_ << endl;
        std::cout << "         " << "imageType: " << imageType_ << endl;
        std::cout << "         " << "resizeFactor: " << resizeFactor_ << endl;
        std::cout << "         " << "resizedWidth: " << resizedWidth_ << endl;
        std::cout << "         " << "resizedHeight: " << resizedHeight_ << endl;
    }

    inline bool operator==(CameraGetImagePackage &_value)
    {
        if (boxAmount_ != _value.boxAmount_ || boxIndex_ != _value.boxIndex_||cameraAmount_!=_value.cameraAmount_||cameraIndex_!=_value.cameraIndex_)
        {
            return false;
        }
        if (imageType_ != _value.imageType_ || resizeFactor_ != _value.resizeFactor_ || resizedWidth_ != _value.resizedWidth_ || resizedHeight_ != _value.resizedHeight_)
        {
            return false;
        }
        return true;
    }
};


typedef struct
{
    Communication_Camera_Command command_;
    ///只有在命令不为Camera_Command_Get_Status时才调用
    ///第一个字节为command,后面的对应不同的解析方式
    char data_[COMMUNICATION_COMMAND_DATA_MAX_SIZE];

} CommunicationClientSendPackageTypdef;


///释放时要先释放data_指向的内存区域
typedef struct
{
    Communication_Camera_Status status_;
    int dataSize_;          ///说明后面接的缓冲区的内存大小
    char *data_=NULL;       ///图像数据缓冲区，需要动态分配
} CommunicationClientReceivePackageTypeDef;


typedef struct
{
    Communication_Camera_Command command_;
    ///只有在命令不为Camera_Command_Get_Status时才调用
    ///第一个字节为command,后面的对应不同的解析方式
    char data[COMMUNICATION_COMMAND_DATA_MAX_SIZE];

} CommunicationServerReceivePackageTypdef;


///释放时要先释放data指向的内存区域
typedef struct
{
    Communication_Camera_Status status_;
    int dataSize_;          ///说明后面接的缓冲区的内存大小
    char *data_=NULL;       ///图像数据缓冲区，需要动态分配
} CommunicationServerSendPackageTypeDef;

/************************************************************************************************************************************************/

///通信线程处理各命令所需最长时间
static const int verifyCommunicationThreadExitMaxMs=500;


///通信线程动作
enum Communication_Action
{
    Communication_Thread_Wait=0,
    Communication_Thread_Exit=1,
    Communication_Action_Valid=2,
    Communication_Action_Invalid=3,
    Communication_Action_Overtime=4,
};



//通信线程消息
class CommunicationMessage
{
public:
    CommunicationMessage()
    {
        requestor_="";
        action_=Communication_Thread_Wait;
    }
    CommunicationMessage(string _requestor,Communication_Action _action)
    {
        requestor_=_requestor;
        action_=_action;
    }
    ~CommunicationMessage() {}

    string requestor_="";                                           ///请求者
    Communication_Action action_=Communication_Thread_Wait;         ///命令，反馈为Ok或Invalid

    inline void operator=(CommunicationMessage &_value)             ///重载赋值运算符
    {
        requestor_=_value.requestor_;
        action_=_value.action_;
    }

    inline bool operator==(CommunicationMessage &_rightVal) const   ///重载==运算符,完全比较
    {
        if(this->requestor_==_rightVal.requestor_&&this->action_==_rightVal.action_)
        {
            return true;
        }
        return false;
    }

    inline bool operator==(string &_requestor) const                ///重载==运算符,请求者名字比较
    {
        return requestor_==_requestor;
    }

    int VerifyAction(Communication_Action _endAction,int _overTime) ///验证命令
    {
        long startTime=GetCurrentTimeMs(),nowTime=GetCurrentTimeMs()+1;
        while(nowTime-startTime<_overTime)
        {
            if(action_==_endAction)
            {
                return Communication_Action_Valid;
            }
            else if(action_==Communication_Action_Invalid)
            {
                return Communication_Action_Invalid;
            }
            else
            {
                SleepMs(2);
            }
            nowTime=GetCurrentTimeMs();
        }
        return Communication_Action_Overtime;
    }
};


///通信线程消息队列类型定义
typedef ThreadMessageDeque<CommunicationMessage> CommunicationMessageDeque;

class SocketThread;
///通信监听线程，每检测到新的连接就新建立一个SocketThread
class CommunicationThread:public PThreadClass
{
public:
    //CommunicationThread(CommunicationMessageDeque *_communicationMessageDeque,CameraServerUnit *_cameraServerUnit,CameraControlMessageDeque *_cameraControlMessageDeque);
    CommunicationThread(CommunicationMessageDeque *_communicationMessageDeque ,CameraControlMessageDeque *_cameraControlMessageDeque);
    ~CommunicationThread();

    virtual int Run(void);

private:
    static const int maxConnection_=10;
    std::vector<string> localIpVec_;
    int localPort_=54321;
    std::vector<SocketThread *> socketThreadVec_;

    ///相机参数和消息队列入口
    CommunicationMessageDeque *communicationMessageDeque_;
    //CameraServerUnit *cameraServerUnit_;
    CameraControlMessageDeque *cameraControlMessageDeque_;

    int PrintMachineInfo(void);
    int MakeSocketNonBlocking(int _socketFd);
};


///socket控制线程，用于心跳、数据读取和发送
class SocketThread:public PThreadClass
{
public:
    //SocketThread(int _newServerFd,string _clientIp,int16_t _clientPort,
    // CommunicationMessageDeque *_communicationMessageDeque,
    // CameraServerUnit *_cameraServerUnit,CameraControlMessageDeque *_cameraControlMessageDeque);
    SocketThread(int _newServerFd,string _clientIp,int16_t _clientPort,
                 CommunicationMessageDeque *_communicationMessageDeque,
                 CameraControlMessageDeque *_cameraControlMessageDeque);
    ~SocketThread();

    virtual int Run(void);

    int serverFd_=-1;

private:
    string thisName_="";
    string clientIp_="000.000.000.000";
    uint16_t clientPort_=6666;
    uint32_t heartBeatIntervalMs_=5000;

    ///socket通信发送接送缓冲区
    CommunicationServerReceivePackageTypdef receivePackage_;
    CommunicationServerSendPackageTypeDef   sendPackage_;

    ///本线程向其它线程发送的消息
    CameraControlMessage cameraControlMessage_;

    ///相机参数和消息队列入口
    CommunicationMessageDeque *communicationMessageDeque_;
    //CameraServerUnit *cameraServerUnit_;
    CameraControlMessageDeque *cameraControlMessageDeque_;

    int MakeSocketNonBlocking(int _socketFd);

    ///解析接收到的命令并反馈给客户端
    void ParseAndFeedback(void);

    ///以下为检验各命令并超时等待相机控制线程反馈，同时将反馈数据装载到sendPackage_;
    ///该命令将会改变cameraServerUnit_的值
    //bool VerifyOpenBox(CameraOpenBoxPackage &_data);
    //bool VerifyCloseBox(CameraCloseBoxPackage &_data );
    bool VerifyOpenCamera(CameraOpenCameraPackage &_data);
    bool VerifyCloseCamera(CameraCloseCameraPackage &_data);
    //bool VerifyTriggerContinous(CameraTriggerContinousPackage &_data);
    //bool VerifyTriggerSingle(CameraTriggerSinglePackage &_data);
    //bool VerifyResetId(CameraResetIdPackage &_data);
    bool VerifyGetImage(CameraGetImagePackage &_data);
};

























