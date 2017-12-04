/************************************************************************
//Describe: communication.cpp: used for communicating with server
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/14/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
************************************************************************/
#include "communication.h"

CommunicationThread::CommunicationThread(CommunicationMessageDeque *_communicationMessageDeque,CameraServerUnit *_cameraServerUnit,CameraControlMessageDeque *_cameraControlMessageDeque)
{
    if(_communicationMessageDeque!=NULL&&_cameraServerUnit!=NULL&&_cameraControlMessageDeque!=NULL){
        communicationMessageDeque_=_communicationMessageDeque;
        cameraServerUnit_=_cameraServerUnit;
        cameraControlMessageDeque_=_cameraControlMessageDeque;
        if(!StartInternalThread()){
            cout << "[INFO] CommunicationThread failed to start!" << endl;
        }
    }
    else{
        cout << "[INFO] CommunicationThread failed to start!" << endl;
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

   if (getifaddrs(&ifaddr) == -1) {
       printf("getifaddrs");
       return -1;
   }

   /* Walk through linked list, maintaining head pointUserCommuniThreader so we can free list later */
   for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
       if (ifa->ifa_addr == NULL)
           continue;

       family = ifa->ifa_addr->sa_family;

       /* Display interface name and family (including symbolic form of the latter for the common families) */
       /* For an AF_INET* interface address, display the address */
       if (family == AF_INET) {
           s = getnameinfo(ifa->ifa_addr,(family == AF_INET) ? sizeof(struct sockaddr_in) :sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
           if (s != 0) {
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
    if(flags==-1){
        printf("make socket non blocking failed! errno: %d\n",errno);
        return -1;
    }
    if(fcntl(_socketFd,F_SETFL,flags|O_NONBLOCK)==-1){
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
	if ((listenFd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		cout<<"Create socket failed!"<<endl;
	}

	///设置端口可复用，否则上次程序结束后再重启会导致绑定不成功
	int flag=1;
	if(setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))<0){
		cout<<"[INFO] CommunicationThread: set socket reuse address failed! port: "<<localPort_<<endl;
	}
	if(setsockopt(listenFd,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(flag))<0){
		cout<<"[INFO] CommunicationThread: set socket no delay failed! port: "<<localPort_<<endl;
	}

    ///绑定套接字
	my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(localPort_);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if (bind(listenFd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr)) == -1) {
		cout<<"[INFO] CommunicationThread: socket bind error!"<<endl;
	}

    ///监听连接
    if (listen(listenFd, maxConnection_) == -1) {
        cout<<"[INFO] CommunicationThread: socket listen error!"<<endl;
    }

    ///设置监听socket为非阻塞
    MakeSocketNonBlocking(listenFd);

    socketThreadVec_.clear();

    CommunicationMessage communicationMessageThis("",Communication_Thread_Wait);

    ///从消息队列里弹出的临时消息指针
    CommunicationMessage *communicationMessageTmpPtr=NULL;

	///通信线程运行体
	while(communicationMessageThis.action_!=Communication_Thread_Exit){
//       cout<<"This is CommunicationThread!"<<endl<<endl;
		if(!communicationMessageDeque_->Empty()){
            communicationMessageTmpPtr=communicationMessageDeque_->PopFront();
			communicationMessageThis=*communicationMessageTmpPtr;
		}
		if(communicationMessageThis.action_==Communication_Thread_Exit){
			if(communicationMessageTmpPtr!=NULL){
				communicationMessageTmpPtr->action_=Communication_Action_Valid;
			}
			break;
		}
		else if(communicationMessageThis.action_==Communication_Thread_Wait){
			SleepMs(1);
		}
        ///等待连接
        socklen_t sin_size = sizeof(struct sockaddr_in);
        int serverFd_=-1;

        if ((serverFd_ = accept(listenFd, (struct sockaddr*)&remote_addr,&sin_size))>0) {
            SocketThread *newSocketThread=new SocketThread(serverFd_,inet_ntoa(remote_addr.sin_addr),remote_addr.sin_port,communicationMessageDeque_,cameraServerUnit_,cameraControlMessageDeque_);
            socketThreadVec_.push_back(newSocketThread);
        }
        ///检测已退出的线程，释放申请的空间
        for(std::vector<SocketThread *>::iterator itr=socketThreadVec_.begin();itr!=socketThreadVec_.end();){
            if((*itr)->serverFd_==-1){
                (*itr)->WaitForInternalThreadToExit();
                delete (*itr);
                itr=socketThreadVec_.erase(itr);
            }
            else{
                ++itr;
            }
        }
	}
	///退出时要删除所开辟的所有新线程
    for(std::vector<SocketThread *>::iterator itr=socketThreadVec_.begin();itr!=socketThreadVec_.end();){
    	close((*itr)->serverFd_);
    	(*itr)->serverFd_=-1;
        (*itr)->WaitForInternalThreadToExit();
        delete (*itr);
        itr=socketThreadVec_.erase(itr);
    }
    cout << "[INFO] CommunicationThread ended!" << endl;
    return 0;
}




SocketThread::SocketThread(int _newServerFd,string _clientIp,int16_t _clientPort,CommunicationMessageDeque *_communicationMessageDeque,CameraServerUnit *_cameraServerUnit,CameraControlMessageDeque *_cameraControlMessageDeque)
{

    if(_newServerFd>0&&_communicationMessageDeque!=NULL&&_cameraServerUnit!=NULL&&_cameraControlMessageDeque!=NULL){
        serverFd_=_newServerFd;
        clientIp_=_clientIp;
        clientPort_=_clientPort;
        std::ostringstream strTmp("");
        strTmp<<_clientIp<<":"<<clientPort_;
        thisName_=strTmp.str();
        cameraControlMessage_.requestor_=thisName_;
        communicationMessageDeque_=_communicationMessageDeque;
        cameraServerUnit_=_cameraServerUnit;
        cameraControlMessageDeque_=_cameraControlMessageDeque;
        sendPackage_.data_=new char[CAMERA_IMAGE_DATA_MAX_SIZE];
        if(!StartInternalThread()){
            cout << "[INFO] SocketThread: "<<thisName_<<"failed to start!" << endl;
        }
    }
}


SocketThread::~SocketThread()
{
    if(sendPackage_.data_!=NULL){
        delete []sendPackage_.data_;
    }
}


int SocketThread::MakeSocketNonBlocking(int _socketFd)
{
    int flags=fcntl(_socketFd,F_GETFL,0);
    if(flags==-1){
        printf("make socket non blocking failed! errno: %d\n",errno);
        return -1;
    }
    if(fcntl(_socketFd,F_SETFL,flags|O_NONBLOCK)==-1){
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
    while(serverFd_!=-1){
        SleepUs(200);
        currentCheckTime=GetCurrentTimeMs();
        int readByteSize=-1;
        if((readByteSize=read(serverFd_,&receivePackage_,sizeof(receivePackage_)))>0){
            ParseAndFeedback();
            lastCheckTime=GetCurrentTimeMs();
        }
        else if((currentCheckTime-lastCheckTime)>heartBeatIntervalMs_){
            cout << "[INFO] SocketThread: "<<thisName_<<" time out and ended!" << endl;
            close(serverFd_);
            serverFd_=-1;
            break;
        }
    }
    cout << "[INFO] SocketThread: "<<thisName_<<" ended!" << endl;
    return 0;
}


void SocketThread::ParseAndFeedback(void)
{
    switch(receivePackage_.command_){
	case Communication_Camera_Get_Status:{
		sendPackage_.status_=Communication_Camera_Get_Status_Ok;
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
            ///记录socket通信状态
		}
		break;
	}
	case Communication_Camera_Open_Box:{
		CameraOpenBoxPackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
		VerifyOpenBox(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
			///记录socket通信状态
		}
		break;
	}
	case Communication_Camera_Close_Box:{
		CameraCloseBoxPackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
		VerifyCloseBox(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
			///记录socket通信状态
		}
		break;
	}
	case Communication_Camera_Open_Camera:{
		CameraOpenCameraPackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyOpenCamera(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
			///记录socket通信状态
		}
		break;
	}
	case Communication_Camera_Close_Camera:{
		CameraCloseCameraPackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyCloseCamera(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
			///记录socket通信状态
		}
		break;
	}
	case Communication_Camera_Trigger_Continous:{
		CameraTriggerContinousPackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyTriggerContinous(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
			///记录socket通信状态
		}
		break;
	}
    case Communication_Camera_Trigger_Single:{
		CameraTriggerSinglePackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyTriggerSingle(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
			///记录socket通信状态
		}
		break;
	}
	case Communication_Camera_Reset_Id:{
		CameraResetIdPackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
        VerifyResetId(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)))>0){
			///记录socket通信状态
		}
		break;
	}
	case Communication_Camera_Get_Image:{
		CameraGetImagePackage dataTmp;
		memcpy(&dataTmp,receivePackage_.data,sizeof(dataTmp));
//		dataTmp.PrintInfo("[INFO] SocketThread: "+thisName_);
		VerifyGetImage(dataTmp);
		int writtenByteSize=-1;
		if((writtenByteSize=write(serverFd_,&sendPackage_,sizeof(sendPackage_.status_)+sizeof(sendPackage_.dataSize_)))>0){
            ///成功时返回图像数据
            if(sendPackage_.status_==Communication_Camera_Get_Image_Ok){
                int sum=0;
                long startTime=GetCurrentTimeMs();
                while(sum<sendPackage_.dataSize_){
                    if((writtenByteSize=write(serverFd_,sendPackage_.data_+sum,sendPackage_.dataSize_-sum))>0){
                        sum+=writtenByteSize;
                    }
                }
                cout<<"[INFO] SocketThread: "<< thisName_<< "send "<<sum<< " bytes,consume: "<<GetCurrentTimeMs()-startTime<<" ms"<<endl;
            }
		}
		break;
	}
	}
}


bool SocketThread::VerifyOpenBox(CameraOpenBoxPackage &_data)
{

    if(_data.boxAmount_>=boxAmountMin&&_data.boxAmount_<=boxAmountMax&&_data.boxIndex_<_data.boxAmount_){

        ///修改相机控制的参数列表
    	cameraServerUnit_->Lock();
    	cameraServerUnit_->ResizeBoxVec(_data.boxAmount_);
    	cameraServerUnit_->boxVec_[_data.boxIndex_].mac_=_data.macAddress_;
        cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Open_Box;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraOpenBoxMaxMs)){
			case CameraControl_Action_Invalid:{
				cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Box invalid!" <<endl;
				sendPackage_.status_=Communication_Camera_Open_Box_Invalid;
				return false;
			}
			case CameraControl_Action_Valid:{
				cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Box valid!" <<endl;
				sendPackage_.status_=Communication_Camera_Open_Box_Ok;
				return true;
			}
			case CameraControl_Action_Overtime:{
				cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Box overtime!" <<endl;
				cameraControlMessageDeque_->Erase(&cameraControlMessage_);
				sendPackage_.status_=Communication_Camera_Action_Overtime;
				return false;
			}
        }
}
    sendPackage_.status_=Communication_Camera_Action_Invalid ;
    return false;
}


bool SocketThread::VerifyCloseBox(CameraCloseBoxPackage &_data)
{
    if(_data.boxAmount_>=boxAmountMin&&_data.boxAmount_<=boxAmountMax&&_data.boxIndex_<_data.boxAmount_){
        ///修改相机参数列表
    	cameraServerUnit_->Lock();
        cameraServerUnit_->ResizeBoxVec(_data.boxAmount_);
        cameraServerUnit_->boxVec_[_data.boxIndex_].mac_=_data.macAddress_;
        cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Close_Box;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraCloseBoxMaxMs)){
			case CameraControl_Action_Invalid:{
				cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Box invalid!" <<endl;
				sendPackage_.status_=Communication_Camera_Close_Box_Invalid;
				return false;
			}
			case CameraControl_Action_Valid:{
				cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Box valid!" <<endl;
				sendPackage_.status_=Communication_Camera_Close_Box_Ok;
				return true;
			}
			case CameraControl_Action_Overtime:{
				cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Box overtime!" <<endl;
				cameraControlMessageDeque_->Erase(&cameraControlMessage_);
				sendPackage_.status_=Communication_Camera_Action_Overtime;
				return false;
			}
        }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyOpenCamera(CameraOpenCameraPackage &_data)
{
    if(_data.boxAmount_>0&&_data.boxAmount_==cameraServerUnit_->boxVec_.size()&&_data.boxIndex_<_data.boxAmount_&&      \
       _data.cameraAmount_>=cameraAmountMin&&_data.cameraAmount_<=cameraAmountMax&&_data.cameraIndex_<_data.cameraAmount_){
        ///修改相机参数列表
    	cameraServerUnit_->Lock();
        cameraServerUnit_->boxVec_[_data.boxIndex_].ResizeCameraVec(_data.cameraAmount_);
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].id_=_data.cameraId_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].height_=_data.height_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].width_=_data.width_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].exposure_=_data.exposure_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].gain_=_data.gain_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].brightness_=_data.brightness_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].contrast_=_data.contrast_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].bitLut_=_data.bitLut_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].saveFormat_=_data.saveFormat_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].skipNumber_=_data.skipNumber_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].triggerMode_=_data.triggerMode_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].savePath_=_data.savePath_;
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].saveName_=_data.saveName_;
        //cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].othersInfo_=_data.othersInfo_;
        cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Open_Camera;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
        cameraControlMessage_.openCameraOperationIndex_=_data.operationIndex_;
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraOpenCameraMaxMs)){
            case CameraControl_Action_Invalid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Camera invalid!" <<endl;
                sendPackage_.status_=Communication_Camera_Open_Camera_Invalid;
                return false;
            }
            case CameraControl_Action_Valid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Camera valid!" <<endl;
                sendPackage_.status_=Communication_Camera_Open_Camera_Ok;
                return true;
            }
            case CameraControl_Action_Overtime:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Open_Camera overtime!" <<endl;
                cameraControlMessageDeque_->Erase(&cameraControlMessage_);
                sendPackage_.status_=Communication_Camera_Action_Overtime;
                return false;
            }
        }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyCloseCamera(CameraCloseCameraPackage &_data)
{
    if(_data.boxAmount_>0&&_data.boxAmount_==cameraServerUnit_->boxVec_.size()&&_data.boxIndex_<_data.boxAmount_&&      \
       _data.cameraAmount_>=cameraAmountMin&&_data.cameraAmount_<=cameraAmountMax&&_data.cameraIndex_<_data.cameraAmount_){
        ///修改相机参数列表
    	cameraServerUnit_->Lock();
        cameraServerUnit_->boxVec_[_data.boxIndex_].ResizeCameraVec(_data.cameraAmount_);
        cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Close_Camera;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraCloseCameraMaxMs)){
            case CameraControl_Action_Invalid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Camera invalid!" <<endl;
                sendPackage_.status_=Communication_Camera_Close_Camera_Invalid;
                return false;
            }
            case CameraControl_Action_Valid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Camera valid!" <<endl;
                sendPackage_.status_=Communication_Camera_Close_Camera_Ok;
                return true;
            }
            case CameraControl_Action_Overtime:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Close_Camera overtime!" <<endl;
                cameraControlMessageDeque_->Erase(&cameraControlMessage_);
                sendPackage_.status_=Communication_Camera_Action_Overtime;
                return false;
            }
        }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyTriggerContinous(CameraTriggerContinousPackage &_data)
{
    if(_data.boxAmount_>0&&_data.boxAmount_==cameraServerUnit_->boxVec_.size()&&_data.boxIndex_<_data.boxAmount_&&      \
       _data.cameraAmount_>=cameraAmountMin&&_data.cameraAmount_<=cameraAmountMax&&_data.cameraIndex_<_data.cameraAmount_){
        ///修改相机参数列表
    	cameraServerUnit_->Lock();
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].saveName_=_data.saveName_;
        cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Trigger_Continous;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
        cameraControlMessage_.triggerNumber_=_data.triggerNumber_;
        cameraControlMessage_.saveName_=_data.saveName_;
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraTriggerContinousMaxMs)){
            case CameraControl_Action_Invalid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Trigger_Continous invalid!" <<endl;
                sendPackage_.status_=Communication_Camera_Trigger_Continous_Invalid;
                return false;
            }
            case CameraControl_Action_Valid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Trigger_Continous valid!" <<endl;
                sendPackage_.status_=Communication_Camera_Trigger_Continous_Ok;
                return true;
            }
            case CameraControl_Action_Overtime:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Trigger_Continous overtime!" <<endl;
                cameraControlMessageDeque_->Erase(&cameraControlMessage_);
                sendPackage_.status_=Communication_Camera_Action_Overtime;
                return false;
            }
        }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyTriggerSingle(CameraTriggerSinglePackage &_data)
{
    if(_data.boxAmount_>0&&_data.boxAmount_==cameraServerUnit_->boxVec_.size()&&_data.boxIndex_<_data.boxAmount_&&      \
       _data.cameraAmount_>=cameraAmountMin&&_data.cameraAmount_<=cameraAmountMax&&_data.cameraIndex_<_data.cameraAmount_){
        ///修改相机参数列表
        cameraServerUnit_->Lock();
        cameraServerUnit_->boxVec_[_data.boxIndex_].cameraVec_[_data.cameraIndex_].saveName_=_data.saveName_;
        cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Trigger_Single;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
        cameraControlMessage_.triggerNumber_=_data.triggerNumber_;
        cameraControlMessage_.saveName_=_data.saveName_;
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraTriggerSingleMaxMs)){
            case CameraControl_Action_Invalid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Trigger_Single invalid!" <<endl;
                sendPackage_.status_=Communication_Camera_Trigger_Single_Invalid;
                return false;
            }
            case CameraControl_Action_Valid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Trigger_Single valid!" <<endl;
                sendPackage_.status_=Communication_Camera_Trigger_Single_Ok;
                return true;
            }
            case CameraControl_Action_Overtime:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Trigger_Single overtime!" <<endl;
                cameraControlMessageDeque_->Erase(&cameraControlMessage_);
                sendPackage_.status_=Communication_Camera_Action_Overtime;
                return false;
            }
        }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyGetImage(CameraGetImagePackage &_data)
{
    if(_data.boxAmount_>0&&_data.boxAmount_==cameraServerUnit_->boxVec_.size()&&_data.boxIndex_<_data.boxAmount_&&      \
       _data.cameraAmount_>=cameraAmountMin&&_data.cameraAmount_<=cameraAmountMax&&_data.cameraIndex_<_data.cameraAmount_&& \
       _data.resizeFactor_>=imageResizedFactorMin&&_data.resizeFactor_<=imageResizedFactorMax){
        ///修改相机参数列表
//        cameraServerUnit_->Lock();
//        cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Get_Image;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
        cameraControlMessage_.imageType_=_data.imageType_;
        cameraControlMessage_.resizeFactor_=_data.resizeFactor_;
        cameraControlMessage_.resizedWidth_=_data.resizedWidth_;
        cameraControlMessage_.resizedHeight_=_data.resizedHeight_;
        ///为发送缓冲区申请地址空间
        sendPackage_.dataSize_=sizeof(CameraGetImagePackage)+cameraControlMessage_.imageType_*cameraControlMessage_.resizedWidth_*cameraControlMessage_.resizedHeight_;
        memcpy(sendPackage_.data_,&_data,sizeof(CameraGetImagePackage));
        cameraControlMessage_.imageData_=sendPackage_.data_+sizeof(CameraGetImagePackage);
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);
        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraGetImageMaxMs)){
            case CameraControl_Action_Invalid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Get_Image invalid!" <<endl;
                sendPackage_.status_=Communication_Camera_Get_Image_Invalid;
                return false;
            }
            case CameraControl_Action_Valid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Get_Image valid!" <<endl;
                sendPackage_.status_=Communication_Camera_Get_Image_Ok;
                return true;
            }
            case CameraControl_Action_Overtime:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Get_Image overtime!" <<endl;
                cameraControlMessageDeque_->Erase(&cameraControlMessage_);
                sendPackage_.status_=Communication_Camera_Action_Overtime;
                return false;
            }
        }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}


bool SocketThread::VerifyResetId(CameraResetIdPackage &_data)
{
    if(_data.boxAmount_>0&&_data.boxAmount_==cameraServerUnit_->boxVec_.size()&&_data.boxIndex_<_data.boxAmount_&&      \
       _data.cameraAmount_>=cameraAmountMin&&_data.cameraAmount_<=cameraAmountMax&&_data.cameraIndex_<_data.cameraAmount_){
        ///修改相机参数列表
//    	cameraServerUnit_->Lock();
//      cameraServerUnit_->Unlock();

        ///添加至消息队列并超时等待反馈
        cameraControlMessage_.action_=CameraControl_Reset_Id;
        cameraControlMessage_.boxIndex_=_data.boxIndex_;
        cameraControlMessage_.cameraIndex_=_data.cameraIndex_;
        cameraControlMessageDeque_->PushBack(&cameraControlMessage_);

        switch(cameraControlMessage_.VerifyAction(CameraControl_Action_Valid,verifyCameraResetIdMaxMs)){
            case CameraControl_Action_Invalid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Reset_Id invalid!" <<endl;
                sendPackage_.status_=Communication_Camera_Reset_Id_Invalid;
                return false;
            }
            case CameraControl_Action_Valid:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Reset_Id valid!" <<endl;
                sendPackage_.status_=Communication_Camera_Reset_Id_Ok;
                return true;
            }
            case CameraControl_Action_Overtime:{
                cout << "[INFO] SocketThread: "<<thisName_<<" CameraControl_Reset_Id overtime!" <<endl;
                cameraControlMessageDeque_->Erase(&cameraControlMessage_);
                sendPackage_.status_=Communication_Camera_Action_Overtime;
                return false;
            }
        }
    }
    sendPackage_.status_=Communication_Camera_Action_Invalid;
    return false;
}

















