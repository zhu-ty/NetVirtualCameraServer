/************************************************************************
//Describe: communication.cpp: used for communicating with server
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/14/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
************************************************************************/
#include "communication.h"

CommunicationThread::CommunicationThread(CommunicationMessageDeque *_communicationMessageDeque ,CameraControlMessageDeque *_cameraControlMessageDeque)
{
    if(_communicationMessageDeque!=NULL&&_cameraControlMessageDeque!=NULL)
    {
        communicationMessageDeque_=_communicationMessageDeque;
        cameraControlMessageDeque_=_cameraControlMessageDeque;
        if(!StartInternalThread())
        {
            cout << Colormod::red << "[INFO] CommunicationThread failed to start!" << Colormod::def << endl;
        }
    }
    else
    {
        cout << Colormod::red << "[INFO] CommunicationThread failed to start!" << Colormod::def << endl;
    }
}


CommunicationThread::~CommunicationThread()
{

}


int CommunicationThread::PrintMachineInfo(void)
{
    localIpVec_.clear();
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        printf("getifaddrs");
        return -1;
    }

    /* Walk through linked list, maintaining head pointUserCommuniThreader so we can free list later */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic form of the latter for the common families) */
        /* For an AF_INET* interface address, display the address */
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr,(family == AF_INET) ? sizeof(struct sockaddr_in) :sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                return -1;
            }
            std::string strTmp=host;
            localIpVec_.push_back(strTmp);
            printf("\tIp address: <%s>\n", host);
        }
    }
    printf("\tIp port: <%d>\n\n", localPort_);
    freeifaddrs(ifaddr);
    return 0;
}


int CommunicationThread::MakeSocketNonBlocking(int _socketFd)
{
    int flags=fcntl(_socketFd,F_GETFL,0);
    if(flags==-1)
    {
        printf("make socket non blocking failed! errno: %d\n",errno);
        return -1;
    }
    if(fcntl(_socketFd,F_SETFL,flags|O_NONBLOCK)==-1)
    {
        printf("make socket non blocking failed! errno: %d\n",errno);
        return -1;
    }
    return 0;
}


int CommunicationThread::Run()
{
    cout << "[INFO] CommunicationThread started!" << endl;
    PrintMachineInfo();

    ///设置本地套接字
    int listenFd;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    if ((listenFd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        cout << Colormod::red << "Create socket failed!" << Colormod::def << endl;
    }

    ///设置端口可复用，否则上次程序结束后再重启会导致绑定不成功
    int flag=1;
    if(setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))<0)
    {
        cout << Colormod::red << "[INFO] CommunicationThread: set socket reuse address failed! port: " << localPort_ << Colormod::def << endl;
    }
    if(setsockopt(listenFd,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(flag))<0)
    {
        cout << Colormod::red << "[INFO] CommunicationThread: set socket no delay failed! port: " << localPort_ << Colormod::def << endl;
    }

    ///绑定套接字
    my_addr.sin_family      = AF_INET;
    my_addr.sin_port        = htons(localPort_);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);
    if (bind(listenFd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        cout << Colormod::red << "[INFO] CommunicationThread: socket bind error!" << Colormod::def << endl;
    }

    ///监听连接
    if (listen(listenFd, maxConnection_) == -1)
    {
        cout << Colormod::red << "[INFO] CommunicationThread: socket listen error!" << Colormod::def << endl;
    }

    ///设置监听socket为非阻塞
    MakeSocketNonBlocking(listenFd);

    socketThreadVec_.clear();

    CommunicationMessage communicationMessageThis("",Communication_Thread_Wait);

    ///从消息队列里弹出的临时消息指针
    CommunicationMessage *communicationMessageTmpPtr=NULL;

    ///通信线程运行体
    while(communicationMessageThis.action_!=Communication_Thread_Exit)
    {
//       cout<<"This is CommunicationThread!"<<endl<<endl;
        if(!communicationMessageDeque_->Empty())
        {
            communicationMessageTmpPtr=communicationMessageDeque_->PopFront();
            communicationMessageThis=*communicationMessageTmpPtr;
        }
        if(communicationMessageThis.action_==Communication_Thread_Exit)
        {
            if(communicationMessageTmpPtr!=NULL)
            {
                communicationMessageTmpPtr->action_=Communication_Action_Valid;
            }
            break;
        }
        else if(communicationMessageThis.action_==Communication_Thread_Wait)
        {
            SleepMs(1);
        }
        ///等待连接
        socklen_t sin_size = sizeof(struct sockaddr_in);
        int serverFd_=-1;

        if ((serverFd_ = accept(listenFd, (struct sockaddr*)&remote_addr,&sin_size))>0)
        {
            SocketThread *newSocketThread=new SocketThread(serverFd_,
                inet_ntoa(remote_addr.sin_addr),remote_addr.sin_port,communicationMessageDeque_,cameraControlMessageDeque_);
            socketThreadVec_.push_back(newSocketThread);
        }
        ///检测已退出的线程，释放申请的空间
        for(std::vector<SocketThread *>::iterator itr=socketThreadVec_.begin(); itr!=socketThreadVec_.end();)
        {
            if((*itr)->serverFd_==-1)
            {
                (*itr)->WaitForInternalThreadToExit();
                delete (*itr);
                itr=socketThreadVec_.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
    }
    ///退出时要删除所开辟的所有新线程
    for(std::vector<SocketThread *>::iterator itr=socketThreadVec_.begin(); itr!=socketThreadVec_.end();)
    {
        close((*itr)->serverFd_);
        (*itr)->serverFd_=-1;
        (*itr)->WaitForInternalThreadToExit();
        delete (*itr);
        itr=socketThreadVec_.erase(itr);
    }
    cout << "[INFO] CommunicationThread ended!" << endl;
    return 0;
}




SocketThread::SocketThread(int _newServerFd,string _clientIp,int16_t _clientPort,
                           CommunicationMessageDeque *_communicationMessageDeque,
                           CameraControlMessageDeque *_cameraControlMessageDeque)
{

    if(_newServerFd>0&&_communicationMessageDeque!=NULL&&_cameraControlMessageDeque!=NULL)
    {
        serverFd_=_newServerFd;
        clientIp_=_clientIp;
        clientPort_=_clientPort;
        std::ostringstream strTmp("");
        strTmp<<_clientIp<<":"<<clientPort_;
        thisName_=strTmp.str();
        //cameraControlMessage_.requestor_=thisName_;
        communicationMessageDeque_=_communicationMessageDeque;
        cameraControlMessageDeque_=_cameraControlMessageDeque;
        sendPackage_.data_=new char[CAMERA_IMAGE_DATA_MAX_SIZE];
        if(!StartInternalThread())
        {
            cout << Colormod::red << "[INFO] SocketThread: "<<thisName_<<"failed to start!" << Colormod::def << endl;
        }
    }
}


SocketThread::~SocketThread()
{
    if(sendPackage_.data_!=NULL)
    {
        delete []sendPackage_.data_;
    }
}


int SocketThread::MakeSocketNonBlocking(int _socketFd)
{
    int flags=fcntl(_socketFd,F_GETFL,0);
    if(flags==-1)
    {
        printf("make socket non blocking failed! errno: %d\n",errno);
        return -1;
    }
    if(fcntl(_socketFd,F_SETFL,flags|O_NONBLOCK)==-1)
    {
        printf("make socket non blocking failed! errno: %d\n",errno);
        return -1;
    }
    return 0;
}


int SocketThread::Run(void)
{
    cout << "[INFO] SocketThread: "<<thisName_<<" started!" << endl;
    long lastCheckTime=GetCurrentTimeMs();
    long currentCheckTime=GetCurrentTimeMs()+1;

    ///设置当前socket为非阻塞
    MakeSocketNonBlocking(serverFd_);

    ///该线程运行依赖于serverFd_,serverFd_会在CommunicationThread退出时修改为-1
    while(serverFd_!=-1)
    {
        SleepUs(200);
        currentCheckTime=GetCurrentTimeMs();
        int readByteSize=-1;
        if((readByteSize=read(serverFd_,&receivePackage_,sizeof(receivePackage_)))>0)
        {
            if(readByteSize >= 4)
            {
                if(receivePackage_.command_ == Communication_Camera_Get_Status)
                {
                    
                }
                else if(receivePackage_.command_ == Communication_Camera_Open_Camera)
                {
                    while(readByteSize < sizeof(CameraOpenCameraPackage))
                    {
                        readByteSize += read(serverFd_,&receivePackage_ + readByteSize,sizeof(CameraOpenCameraPackage) - readByteSize);
                    }
                }
                else if(receivePackage_.command_ == Communication_Camera_Close_Camera)
                {
                    while(readByteSize < sizeof(CameraCloseCameraPackage))
                    {
                        readByteSize += read(serverFd_,&receivePackage_ + readByteSize,sizeof(CameraCloseCameraPackage) - readByteSize);
                    }
                }
                else if(receivePackage_.command_ == Communication_Camera_Get_Image)
                {
                    while(readByteSize < sizeof(CameraGetImagePackage))
                    {
                        readByteSize += read(serverFd_,&receivePackage_ + readByteSize,sizeof(CameraGetImagePackage) - readByteSize);
                    }
                }
                ParseAndFeedback();
                lastCheckTime=GetCurrentTimeMs();
            }
        }
        else if((currentCheckTime-lastCheckTime)>heartBeatIntervalMs_)
        {
            cout << Colormod::red << "[INFO] SocketThread: "<<thisName_<<" time out and ended!" << Colormod::def << endl;
            close(serverFd_);
            serverFd_=-1;
            break;
        }
    }
    cout << Colormod::red << "[INFO] SocketThread: "<<thisName_<<" ended!" << Colormod::def << endl;
    return 0;
}


void SocketThread::ParseAndFeedback(void)
{
    switch(receivePackage_.command_)
    {
    case Communication_Camera_Get_Status:
    {
        sendPackage_.status_=Communication_Camera_Get_Status_Ok;
        int writtenByteSize=-1;
        if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0)
        {
            ///记录socket通信状态
        }
        break;
    }
    case Communication_Camera_Open_Camera:
    {
        CameraOpenCameraPackage dataTmp;
        memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
        dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyOpenCamera(dataTmp);
        int writtenByteSize=-1;
        if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)+sizeof(sendPackage_.dataSize_)))>0)
        //if((writtenByteSize=write(serverFd_,&sendPackage_,&sendPackage_.dataSize_ - &sendPackage_.status_ + 4)))>0)
        {
            int sum=0;
            long startTime=GetCurrentTimeMs();
            if(sendPackage_.status_==Communication_Camera_Open_Camera_Ok)
            {
                while(sum<sendPackage_.dataSize_)
                {
                    //cout << Colormod::magenta<<"[SHADOWK]"<<Colormod::def<<"written byte size: " << sum << endl;

                    if((writtenByteSize=write(serverFd_,sendPackage_.data_+sum,sendPackage_.dataSize_-sum))>0)
                    {
                        sum+=writtenByteSize;
                    }
                }
            }
            cout<<Colormod::magenta<<"[SHADOWK]"<<Colormod::def<<"[INFO] SocketThread: "<<thisName_<< 
                "Opencamera reply send "<< Colormod::blue << sum << Colormod::def <<" bytes, " << 
                "consume: " << Colormod::blue << GetCurrentTimeMs()-startTime << Colormod::def << " ms. "<<
                endl;
        }
        break;
    }
    case Communication_Camera_Close_Camera:
    {
        CameraCloseCameraPackage dataTmp;
        memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
        dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyCloseCamera(dataTmp);
        int writtenByteSize=-1;
        if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0)
        {
            ///记录socket通信状态
        }
        break;
    }
    case Communication_Camera_Get_Image:
    {
        CameraGetImagePackage dataTmp;
        memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
//		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyGetImage(dataTmp);
        int writtenByteSize=-1;
        if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)+sizeof(sendPackage_.dataSize_)))>0)
        {
            ///成功时返回图像数据
            if(sendPackage_.status_==Communication_Camera_Get_Image_Ok)
            {
                int sum=0;
                long startTime=GetCurrentTimeMs();
                while(sum<sendPackage_.dataSize_)
                {
                    if((writtenByteSize=write(serverFd_,sendPackage_.data_+sum,sendPackage_.dataSize_-sum))>0)
                    {
                        sum+=writtenByteSize;
                    }
                }
                cout<<"[INFO] SocketThread: "<<thisName_<< 
                " send "<< Colormod::blue << sum << Colormod::def <<" bytes, " << 
                "consume: " << Colormod::blue << GetCurrentTimeMs()-startTime << Colormod::def << " ms, "<<
                "count: " << Colormod::blue << getimagecount << Colormod::def << " , "<<
                endl;
                getimagecount++;
            }
        }
        break;
    }
    }
}

bool SocketThread::VerifyOpenCamera(CameraOpenCameraPackage &_data)
{
    ///添加至消息队列并超时等待反馈
    CameraControlMessage cameraControlMessage_;
    cameraControlMessage_.action_=CameraControl_Open_Camera;
    cameraControlMessage_.boxIndex_=_data.boxIndex_;
    cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
    cameraControlMessage_.openCameraOperationIndex_=_data.operationIndex_;
    std::string tmp_str(_data.genfunc_c);
    cameraControlMessage_.genfunc_ = tmp_str;
    cameraControlMessage_.gendata_ = _data.gendata_c;
    
    sendPackage_.dataSize_=sizeof(CameraOpenCameraPackage);

    cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

    switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid, verifyCameraOpenCameraMaxMs * _data.cameraAmount_))
    {
    case CameraControl_Action_Invalid:
    {
        _data.gendata_c = cameraControlMessage_.gendata_;
        memcpy(sendPackage_.data_,&_data,sizeof(CameraOpenCameraPackage));
        cout << Colormod::red << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Camera invalid!" << Colormod::def <<endl;
        sendPackage_.status_=Communication_Camera_Open_Camera_Invalid;
        return false;
    }
    case CameraControl_Action_Valid:
    {
        _data.gendata_c = cameraControlMessage_.gendata_;
        memcpy(sendPackage_.data_,&_data,sizeof(CameraOpenCameraPackage));
        cout << Colormod::green << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Camera("<<cameraControlMessage_.genfunc_ <<") valid!" << Colormod::def <<endl;
        sendPackage_.status_=Communication_Camera_Open_Camera_Ok;
        return true;
    }
    case CameraControl_Action_Overtime:
    {
        _data.gendata_c = cameraControlMessage_.gendata_;
        memcpy(sendPackage_.data_,&_data,sizeof(CameraOpenCameraPackage));
        cout << Colormod::red << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Camera overtime!" << Colormod::def <<endl;
        cameraControlMessageDeque_->Erase(&cameraControlMessage_);
        sendPackage_.status_=Communication_Camera_Action_Overtime;
        return false;
    }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyCloseCamera(CameraCloseCameraPackage &_data)
{
    CameraControlMessage cameraControlMessage_;
    cameraControlMessage_.action_=CameraControl_Close_Camera;
    cameraControlMessage_.boxIndex_=_data.boxIndex_;
    cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
    cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

    switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraCloseCameraMaxMs * _data.cameraAmount_))
    {
    case CameraControl_Action_Invalid:
    {
        cout << Colormod::red  << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Camera invalid!" << Colormod::def <<endl;
        sendPackage_.status_=Communication_Camera_Close_Camera_Invalid;
        return false;
    }
    case CameraControl_Action_Valid:
    {
        cout << Colormod::green  << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Camera valid!" << Colormod::def <<endl;
        sendPackage_.status_=Communication_Camera_Close_Camera_Ok;
        return true;
    }
    case CameraControl_Action_Overtime:
    {
        cout << Colormod::red  << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Camera overtime!" << Colormod::def <<endl;
        cameraControlMessageDeque_->Erase(&cameraControlMessage_);
        sendPackage_.status_=Communication_Camera_Action_Overtime;
        return false;
    }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyGetImage(CameraGetImagePackage &_data)
{
    ///添加至消息队列并超时等待反馈
    CameraControlMessage cameraControlMessage_;
    cameraControlMessage_.action_=CameraControl_Get_Image;
    cameraControlMessage_.boxIndex_=_data.boxIndex_;
    cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
    cameraControlMessage_.imageType_=_data.imageType_;
    cameraControlMessage_.resizeFactor_=_data.resizeFactor_;
    cameraControlMessage_.resizedWidth_=_data.resizedWidth_;
    cameraControlMessage_.resizedHeight_=_data.resizedHeight_;
    ///为发送缓冲区申请地址空间

    //JPEG size will vary.

    // sendPackage_.dataSize_=sizeof(CameraGetImagePackage) +
    // cameraControlMessage_.imageType_*cameraControlMessage_.resizedWidth_*cameraControlMessage_.resizedHeight_;

    //memcpy(sendPackage_.data_,&_data,sizeof(CameraGetImagePackage));

    cameraControlMessage_.imageData_=sendPackage_.data_+sizeof(CameraGetImagePackage);
    cameraControlMessageDeque_->PushBack(&cameraControlMessage_);
    switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraGetImageMaxMs))
    {
    case CameraControl_Action_Invalid:
    {
        _data.cameraAmount_ = 0;
        memcpy(sendPackage_.data_,&_data,sizeof(CameraGetImagePackage));
        cout << Colormod::red  << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Get_Image invalid!" << Colormod::def <<endl;
        sendPackage_.status_=Communication_Camera_Get_Image_Invalid;
        return false;
    }
    case CameraControl_Action_Valid:
    {
        _data.cameraAmount_ = cameraControlMessage_.imageamount;
        memcpy(sendPackage_.data_,&_data,sizeof(CameraGetImagePackage));
        sendPackage_.dataSize_=sizeof(CameraGetImagePackage) +
                                cameraControlMessage_.imagelen;
        cout << "[INFO] SocketThread: " << thisName_ << " CameraControl_Get_Image valid!" << endl;
        //cout << "[INFO] CameraIndex: " << Colormod::blue << _data.cameraIndex_ << Colormod::def << endl;
        sendPackage_.status_=Communication_Camera_Get_Image_Ok;
        return true;
    }
    case CameraControl_Action_Overtime:
    {
        _data.cameraAmount_ = 0;
        memcpy(sendPackage_.data_,&_data,sizeof(CameraGetImagePackage));
        cout << Colormod::red << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Get_Image overtime!" << Colormod::def <<endl;
        cameraControlMessageDeque_->Erase(&cameraControlMessage_);
        sendPackage_.status_=Communication_Camera_Action_Overtime;
        return false;
    }
    }
    _data.cameraAmount_ = 0;
    memcpy(sendPackage_.data_,&_data,sizeof(CameraGetImagePackage));
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}

