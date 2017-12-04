/************************************************************************
//Describe: cameraControl.cpp: used for calling API
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/14/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
************************************************************************/
#include "cameraControl.h"


CameraParametersUnit::CameraParametersUnit()
{

}


CameraParametersUnit::~CameraParametersUnit()
{

}


void CameraParametersUnit::Lock()
{
    mutex_.Lock();
}

void CameraParametersUnit::Unlock()
{
    mutex_.Unlock();
}


CameraBoxUnit::CameraBoxUnit()
{

}


CameraBoxUnit::~CameraBoxUnit()
{

}


void CameraBoxUnit::ResizeCameraVec(uint32_t _cameraAmount)
{
    if(_cameraAmount>0){
        cameraAmount_=_cameraAmount;
        if(cameraVec_.size()<cameraAmount_){
            while(cameraVec_.size()<cameraAmount_){
                cameraVec_.push_back(CameraParametersUnit());
            }
        }
        else if(cameraVec_.size()>cameraAmount_){
            while(cameraVec_.size()>cameraAmount_){
                cameraVec_.pop_back();
            }
        }
        ///更新相机总数
        cameraAmount_=cameraVec_.size();
    }
}


void CameraBoxUnit::Lock()
{
    mutex_.Lock();
}


void CameraBoxUnit::Unlock()
{
    mutex_.Unlock();
}



CameraServerUnit::CameraServerUnit()
{

}



CameraServerUnit::~CameraServerUnit()
{

}



void CameraServerUnit::ResizeBoxVec(uint32_t _boxAmount)
{
    if(_boxAmount>0){
        boxAmount_=_boxAmount;
        if(boxVec_.size()<boxAmount_){
            while(boxVec_.size()<boxAmount_){
                boxVec_.push_back(CameraBoxUnit());
            }
        }
        else if(boxVec_.size()>boxAmount_){
            while(boxVec_.size()>boxAmount_){
                boxVec_.pop_back();
            }
        }
        ///更新盒子总数
        boxAmount_=boxVec_.size();
    }
}



void CameraServerUnit::Lock()
{
    mutex_.Lock();
}


void CameraServerUnit::Unlock()
{
    mutex_.Unlock();
}



CameraControlThread::CameraControlThread(CameraServerUnit *_cameraServerUnit,CameraControlMessageDeque *_cameraControlMessageDeque)
{
    if(_cameraServerUnit!=NULL&&_cameraControlMessageDeque!=NULL){
        cameraServerUnit_=_cameraServerUnit;
        cameraControlMessageDeque_=_cameraControlMessageDeque;
        if(!StartInternalThread()){
            cout << "[INFO] CameraControlThread failed to start!" << endl;
            syslog(LOG_WARNING, "[Action] CameraControlThread failed to start!\n");
       }
    }
    else{
         cout << "[INFO] CameraControlThread failed to start!" << endl;
         syslog(LOG_WARNING, "[Action] CameraControlThread failed to start!\n");
    }
}


CameraControlThread::~CameraControlThread()
{

}


/*int TestSave(std::vector<cv::Mat> &matVec)
{
    static int i=0;
    static long startTime=GetCurrentTimeMs(),interval=0;
    static long sumTime=0;
    startTime=GetCurrentTimeMs();
#pragma omp parallel for
    for(int j=0;j<matVec.size();++j){
        std::ostringstream str;
        str<<"/mnt/dayu/gp/Workspace/TestImages/"<<i<<"_"<<j;
        string saveName=str.str();
        cv::imwrite(saveName+".bmp",matVec[j]);
        cout<<"omp_get_thread_num: "<<omp_get_thread_num()<<endl;
    }
    interval=GetCurrentTimeMs()-startTime;
//    cout<<"consume: "<<interval<<endl;
    sumTime+=interval;
    i++;
    if(i%100==0){
//        i=0;
        cout<<"Get 100 iterations consume: "<<sumTime<<endl;
        sumTime=0;
    }
}*/


int CameraControlThread::Run(void)
{
    cout << "[INFO] CameraControlThread started!" << endl;


    ///维持自身线程运行的消息，不会发送到消息队列
    CameraControlMessage cameraControlMessageThis("",CameraControl_Thread_Wait);

    CameraControlMessage *cameraControlMessageTmpPtr=NULL;

    omp_set_num_threads(4);

    //std::vector<cv::Mat> matTest(8,cv::Mat(2160,2560,CV_8UC1,cv::Scalar(128)));

    while(cameraControlMessageThis.action_!=CameraControl_Thread_Exit){
//        cout<<"This is CameraControlThread"<<endl<<endl;
        if(!cameraControlMessageDeque_->Empty()){
            cameraControlMessageTmpPtr=cameraControlMessageDeque_->PopFront();
            cameraControlMessageThis=*cameraControlMessageTmpPtr;
        }
        if(cameraControlMessageThis.action_==CameraControl_Thread_Exit){
            if(cameraControlMessageTmpPtr!=NULL){
                cameraControlMessageTmpPtr->action_=CameraControl_Action_Valid;
            }
            break;
        }
        else if(cameraControlMessageThis.action_==CameraControl_Thread_Wait){
            SleepUs(200);
//            SleepMs(30);
//            long startTime=GetCurrentTimeMs();
//           TestSave(matTest);
 //           cout<<"consume: "<<GetCurrentTimeMs()-startTime<<endl;
        }
        switch(cameraControlMessageThis.action_){
            case CameraControl_Open_Box:{
                cout << "[Action] CameraControlThread: CameraControl_Open_Box:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Open_Box\n");
                OpenBox(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
            case CameraControl_Close_Box:{
                cout << "[Action] CameraControlThread: CameraControl_Close_Box:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Close_Box\n");
                CloseBox(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
            case CameraControl_Open_Camera:{
                cout << "[Action] CameraControlThread: CameraControl_Open_Camera:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Open_Camera\n");
                OpenCamera(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
            case CameraControl_Close_Camera:{
                cout << "[Action] CameraControlThread: CameraControl_Close_Camera:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Close_Camera\n");
                CloseCamera(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
            case CameraControl_Trigger_Continous:{
                cout << "[Action] CameraControlThread: CameraControl_Trigger_Continous:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Trigger_Continous\n");
                TriggerContinous(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
            case CameraControl_Trigger_Single:{
                cout << "[Action] CameraControlThread: CameraControl_Trigger_Single:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Trigger_Single\n");
                TriggerSingle(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
            case CameraControl_Reset_Id:{
                cout << "[Action] CameraControlThread: CameraControl_Reset_Id:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Reset_Id\n");
                ResetId(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
            case CameraCOntrol_Get_Temperature:{
                cout << "[Action] CameraControlThread: CameraControl_Get_Temperature:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Get_Temperature\n");
                GetTemperature(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }

            case CameraControl_Get_Image:{
                cout << "[Action] CameraControlThread: CameraControl_Get_Image:" << endl;
                syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Get_Image\n");
                GetImage(cameraControlMessageTmpPtr);
                cameraControlMessageThis.action_=CameraControl_Thread_Wait;
                break;
            }
        }
    }
    for(int32_t i=0;i<boxVec_.size();++i){
        ///释放相机控制
        if(boxVec_[i].ptr_!=NULL){
            boxVec_[i].ptr_->AcquireImages(false);
            int index=i;
            boxVec_[i].ptr_->ConnectToDevice(index,"",false);
            delete boxVec_[i].ptr_;
        }
    }
    cout << "[INFO] CameraControlThread ended!" << endl;
    syslog(LOG_INFO, "[INFO] CameraControlThread ended!\n");
    return 0;
}


///只保证容量是对的
void CameraControlThread::ResizeBoxVec(void)
{
    cameraServerUnit_->Lock();
    if(boxVec_.size()<cameraServerUnit_->boxVec_.size()){
        while(boxVec_.size()<cameraServerUnit_->boxVec_.size()){
            boxVec_.push_back(CameraControlThreadBoxUnit());
        }
    }
    else if(boxVec_.size()>cameraServerUnit_->boxVec_.size()){
        while(boxVec_.size()>cameraServerUnit_->boxVec_.size()){
            ///释放资源
            CMVCLTen *boxTmp=boxVec_.back().ptr_;
            if(boxTmp!=NULL){
                boxTmp->AcquireImages(false);
                int lastIndex=boxVec_.size()-1;
                boxTmp->ConnectToDevice(lastIndex, "", false);
                delete boxTmp;
            }
            boxVec_.pop_back();
        }
    }
    cameraServerUnit_->Unlock();
}


bool CameraControlThread::OpenBox(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        ResizeBoxVec();
        int i=requestorPtr_->boxIndex_;
        if(i<boxVec_.size()){
            if(boxVec_[i].ptr_==NULL){
                boxVec_[i].ptr_=new CMVCLTen();
            }
            if(boxVec_[i].ptr_->IsDevOpen()){
                requestorPtr_->action_=CameraControl_Action_Valid;
                return true;
            }
            else{
                string macTmp=cameraServerUnit_->boxVec_[i].mac_;
                if(boxVec_[i].ptr_->ConnectToDevice(i, macTmp, true)){
                    boxVec_[i].ptr_->SetDataCallBack(&CaptureCallbackFuncEntry,this);
                    boxVec_[i].ptr_->SetInfoCallBack(&MessageCallbackFuncEntry,this);
                    requestorPtr_->action_=CameraControl_Action_Valid;
                    return true;
                }
            }
        }
        cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<< endl;
        syslog(LOG_INFO, "[INFO OpenBox] CameraControlThread box: boxIndex= %d >= boxVec.size()\n", i);

    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::CloseBox(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        ResizeBoxVec();
        int i=requestorPtr_->boxIndex_;
        if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL){
            if(!boxVec_[i].ptr_->IsDevOpen()){
                requestorPtr_->action_=CameraControl_Action_Valid;
                return true;
            }
            else{
                string macTmp=cameraServerUnit_->boxVec_[i].mac_;
                boxVec_[i].ptr_->AcquireImages(false);
                if(boxVec_[i].ptr_->ConnectToDevice(i, macTmp, false)){
                    boxVec_[i].ptr_->SetDataCallBack(NULL,NULL);
                    boxVec_[i].ptr_->SetInfoCallBack(NULL,NULL);
                    ///注意要释放boxVec_[i]
                    delete boxVec_[i].ptr_;
                    boxVec_[i].ptr_=NULL;
                    ///关闭打开相机时申请的资源
                    for(int32_t k=0;k<boxVec_[i].imageVec_.size();++k){
                        //boxVec_[i].imageVec_[k].release();
                        if(boxVec_[i].imageVec_[k]!=NULL){
                                delete boxVec_[i].imageVec_[k];
                                boxVec_[i].imageVec_[k]=NULL;
                            }
                        boxVec_[i].imageFlushVec_[k]=false;
                        //if(boxVec_[i].imageShowEnable_){
                            //cv::destroyWindow(boxVec_[i].imageShowWindowVec_[k]);
                            ///多关闭几次以防止在ubuntu下无法关闭窗口
                            //for(int32_t i=0;i<3;++i){
                                //cv::waitKey(10);
                            //}
                        //}

                        boxVec_[i].capturedImageCountVec_[k]=0;
                        if(boxVec_[i].imageSaverVec_[k]!=NULL){
                            delete boxVec_[i].imageSaverVec_[k];
                            boxVec_[i].imageSaverVec_[k]=NULL;
                        }
                        for(int32_t r=0;r<boxVec_[i].resizedImagesVec_[k].size();++r){
                            if(boxVec_[i].temp_[k][r]!=NULL){
                                delete boxVec_[i].temp_[k][r];
                                boxVec_[i].temp_[k][r]=NULL;
                            }
                        }
                        //boxVec_[i].resizedImagesVec_[k][r].release();
                    }

                    requestorPtr_->action_=CameraControl_Action_Valid;
                    return true;
                }
            }
        }
        else{
            requestorPtr_->action_=CameraControl_Action_Valid;
            return true;
        }
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::OpenCamera(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        int32_t operationIndex=requestorPtr_->openCameraOperationIndex_;
        int32_t i=requestorPtr_->boxIndex_;
        int32_t j=requestorPtr_->cameraIndex_;
        int32_t width=cameraServerUnit_->boxVec_[i].cameraVec_[j].width_;
        int32_t height=cameraServerUnit_->boxVec_[i].cameraVec_[j].height_;
        int32_t exposure=cameraServerUnit_->boxVec_[i].cameraVec_[j].exposure_;
        int32_t gain=cameraServerUnit_->boxVec_[i].cameraVec_[j].gain_;
        int32_t brightness=cameraServerUnit_->boxVec_[i].cameraVec_[j].brightness_;
        int32_t contrast=cameraServerUnit_->boxVec_[i].cameraVec_[j].contrast_;
        //int32_t bitLut=cameraServerUnit_->boxVec_[i].cameraVec_[j].bitLut_;
        int32_t saveFormat=cameraServerUnit_->boxVec_[i].cameraVec_[j].saveFormat_;
        int32_t skipNumber=cameraServerUnit_->boxVec_[i].cameraVec_[j].skipNumber_;
        int32_t triggerMode=cameraServerUnit_->boxVec_[i].cameraVec_[j].triggerMode_;
        string savePath=cameraServerUnit_->boxVec_[i].cameraVec_[j].savePath_;
        string saveName=cameraServerUnit_->boxVec_[i].cameraVec_[j].saveName_;
        int32_t bitMode=abs(2-requestorPtr_->imageType_); ///0: 16bit, 1: 8bit

        saveFormat=0;           ///此处先关闭底层的保存

        if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL){

            ///先要判断是操作所有参数还是设置某一个参数
            if(operationIndex==-1){
                ///先关闭采集，然后初始化设备，区分盒子下的相机个数，这一部分应该在打开盒子中完成,耗时200ms
                boxVec_[i].ptr_->AcquireImages(false);

                int cameraAmount=cameraServerUnit_->boxVec_[i].cameraVec_.size();
                if(cameraAmount==1){
                    if(!boxVec_[i].ptr_->DeviceInit(width, height, j+1, packetDelay,captureBufferNumber,bitMode,ringBufferNumber)){        ///A+B enable,delay 500
                        requestorPtr_->action_=CameraControl_Action_Invalid;
                        return false;
                    }
                }
                else if(cameraAmount==2){
                    if(!boxVec_[i].ptr_->DeviceInit(width, height,0x3, packetDelay,captureBufferNumber,bitMode,ringBufferNumber)){        ///C+D enable,delay 500
                        requestorPtr_->action_=CameraControl_Action_Invalid;
                        return false;
                    }
                }
                else {
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///通道使能：1:A+B，2:C+D，3:A+B+C+D
                int channelIndex=j+1;
                int tmp=0;

                ///MV_PARAM_CAMTEMPERATURE：-5～65,超过温度直接退出
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_CAMTEMPERATURE,tmp);
                if(tmp>cameraTemperatureMin&&tmp<cameraTemperatureMax){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" temperature: "<<tmp<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" temperature warning!: "<<tmp<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///MV_PARAM_CAMCOOLENABLE：开启制冷，制冷开启失败直接退出
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_CAMCOOLENABLE,1);
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_CAMCOOLENABLE,tmp);
                if(tmp==1){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" coolingEnable: "<<1<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" coolingEnable: "<<tmp<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///MV_PARAM_CAMCOOLTARGET：设置制冷目标温度，设置失败直接退出
                tmp=cameraCoolingTarget;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_CAMCOOLTARGET,tmp);
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_CAMCOOLTARGET,tmp);
                if(tmp==cameraCoolingTarget){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" coolingTarget: "<<cameraCoolingTarget<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" coolingTarget: "<<cameraCoolingTarget<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                long startTime=GetCurrentTimeMs();
                ///先设置驱动层的软参数:saveFormat,skipNumber,不涉及盒子硬件，立即生效，无需校验
                ///MV_PARAM_SAVEENABLE：0:禁用,1:bmp,2:raw,3:bmp+raw, 设置为0，关闭驱动层的图片保存
                tmp=saveFormat;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_SAVEENABLE,tmp);
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_SAVEENABLE,tmp);
                if(tmp==saveFormat){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" saveFormat: "<<saveFormat<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" saveFormat: "<<saveFormat<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }
    //            cout<<"OpenCamera consume: "<<GetCurrentTimeMs()-startTime<<" ms"<<endl;

                ///MV_PARAM_SKIPNUM：0:不跳帧，else:1/(N+1)处理
                tmp=skipNumber;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_SKIPNUM,tmp);
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_SKIPNUM,tmp);
                if(tmp==skipNumber){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" skipNumber: "<<skipNumber<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" skipNumber: "<<skipNumber<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///MV_PARAM_BUFFENABLE：0:禁用，1:启用，恒定设置为关闭
                tmp=false;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_BUFFENABLE,tmp);
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_BUFFENABLE,tmp);
                if(tmp==false){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" bufferEnable: "<<false<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" bufferEnable: "<<true<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///创建和设置保存目录+名称
                if(CreatDirectory(savePath.c_str())){
                    boxVec_[i].ptr_->SetImagePath(j,savePath+saveName);  ///Path+Name
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" savePath+Name: \n       "<<savePath+saveName<<" valid"<<endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" savePath: \n       "<<savePath<< " invalid!"<< endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }


                ///为了保证相机参数生效，先设置为内部触发和曝光时间，设置其它参数后睡眠曝光时长
                ///以确保所有参数生效，然后读取参数进行检验，校验成功后再设置正确的触发模式
                ///MV_PARAM_TRIGGERMODE：0：两路相机独立触发，1：AB路触发全部，2：内部触发
                tmp=2;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_TRIGGERMODE,tmp);

                ///MV_PARAM_EXPOSURETIME：1~(x9us)
                tmp=exposure;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_EXPOSURETIME,tmp);

                ///MV_PARAM_GAIN：1～255(x0.125)
                tmp=gain;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_GAIN,tmp);

                ///MV_PARAM_BRIGHTNESS：1~96(x0.3125)
                tmp=brightness;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_BRIGHTNESS,tmp);

                ///MV_PARAM_CONTRAST：1~96(x0.3125)
                tmp=contrast;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_CONTRAST,tmp);

                ///MV_PARAM_BITLUT://0~8 (bit[7+x:x])
                //tmp=bitLut;
                //boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_BITLUT,tmp);


                ///睡眠曝光时间以确保所有参数生效
                SleepUs(exposure*9);


                ///校验曝光
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_EXPOSURETIME,tmp);
                if(tmp==exposure){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" exposure: "<<exposure<< " valid!"<< endl;
                    syslog(LOG_INFO, "[INFO OpenCamera] CameraControlThread box:%d-camera:%d exposure: %d valid!\n", i, j, exposure);
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" exposure: "<<exposure<< " invalid! get: "<<tmp<<endl;
                    syslog(LOG_WARNING, "[INFO OpenCamera] CameraControlThread box:%d-camera:%d exposure: %d invalid! get: %d\n", i, j, exposure, tmp);
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///校验增益
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_GAIN,tmp);
                if(tmp==gain){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" gain: "<<gain<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" gain: "<<gain<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///校验亮度
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_BRIGHTNESS,tmp);
                if(tmp==brightness){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" brightness: "<<brightness<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" brightness: "<<brightness<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///校验对比度
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_CONTRAST,tmp);
                if(tmp==contrast){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" contrast: "<<contrast<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" contrast: "<<contrast<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                ///校验位模式
//                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_BITLUT,tmp);
//                if(tmp==bitLut || tmp == -1){
//                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" bitLut: "<<bitLut<< " valid!"<< endl;
//                }
//                else{
//                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" bitLut: "<<bitLut<< " invalid! get: "<<tmp<<endl;
//                    requestorPtr_->action_=CameraControl_Action_Invalid;
//                    return false;
//                }


                ///设置真实的触发模式
                tmp=triggerMode;
                boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_TRIGGERMODE,tmp);
                boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_TRIGGERMODE,tmp);
                if(tmp==triggerMode){
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerMode: "<<triggerMode<< " valid!"<< endl;
                }
                else{
                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerMode: "<<triggerMode<< " invalid! get: "<<tmp<<endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }

                requestorPtr_->action_=CameraControl_Action_Valid;
                //[SHADOWK] OUTPUT info.
                //cout << "[SHADOWK] Othersinfo:"<<cameraServerUnit_->boxVec_[i].cameraVec_[j].othersInfo_<<endl;

                return true;
            }

            ///单独设置相机的某一个参数
            else{
                ///通道使能：1:A+B，2:C+D，3:A+B+C+D
                int channelIndex=j+1;
                int tmp=0;

                switch (operationIndex){
                    case  0:{           ///曝光
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            tmp=exposure;
                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_EXPOSURETIME,tmp);
                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_EXPOSURETIME,tmp);
                            if(tmp==exposure){
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" exposure: "<<exposure<< " valid!"<< endl;
                                syslog(LOG_INFO, "[INFO OpenCamera] CameraControlThread box: %d-camera: %d exposure: %d valid!\n", i, j, exposure);
                                requestorPtr_->action_=CameraControl_Action_Valid;
                                return true;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" exposure: "<<exposure<< " invalid! get: "<<tmp<<endl;
                                syslog(LOG_WARNING, "[INFO OpenCamera] CameraControlThread box: %d-camera: %d exposure: %d invalid! get: %d\n", i, j, exposure, tmp);
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            syslog(LOG_WARNING, "[INFO OpenCamera] CameraControlThread box: %d-camera: %d is not opened!\n", i, j);
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                            return false;
                        }
                        break;
                    }
                    case  1:{           ///增益
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            tmp=gain;
                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_GAIN,tmp);
                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_GAIN,tmp);
                            if(tmp==gain){
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" gain: "<<gain<< " valid!"<< endl;
                                requestorPtr_->action_=CameraControl_Action_Valid;
                                return true;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" gain: "<<gain<< " invalid! get: "<<tmp<<endl;
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                            return false;
                        }
                        break;
                    }
                    case  2:{           ///亮度
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            tmp=brightness;
                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_BRIGHTNESS,tmp);
                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_BRIGHTNESS,tmp);
                            if(tmp==brightness){
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" brightness: "<<brightness<< " valid!"<< endl;
                                requestorPtr_->action_=CameraControl_Action_Valid;
                                return true;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" brightness: "<<brightness<< " invalid! get: "<<tmp<<endl;
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                        }
                        break;
                    }
                    case  3:{           ///对比度
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            tmp=contrast;
                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_CONTRAST,tmp);
                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_CONTRAST,tmp);
                            if(tmp==contrast){
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" contrast: "<<contrast<< " valid!"<< endl;
                                requestorPtr_->action_=CameraControl_Action_Valid;
                                return true;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" contrast: "<<contrast<< " invalid! get: "<<tmp<<endl;
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                        }
                        break;
                    }
                    case  4:{           ///滑动窗
                        requestorPtr_->action_=CameraControl_Action_Valid;
                        return true;
//                        if(boxVec_[i].ptr_->IsDevOpen()){
//                            tmp=bitLut;
//                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_BITLUT,tmp);
//                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_BITLUT,tmp);
//                            if(tmp==exposure){
//                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" bitLut: "<<bitLut<< " valid!"<< endl;
//                                requestorPtr_->action_=CameraControl_Action_Valid;
//                                return true;
//                            }
//                            else{
//                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" bitLut: "<<bitLut<< " invalid! get: "<<tmp<<endl;
//                                requestorPtr_->action_=CameraControl_Action_Invalid;
//                                return false;
//                            }
//                        }
//                        else{
//                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
//                            requestorPtr_->action_=CameraControl_Action_Invalid;
//                        }
                        break;
                    }
                    case  5:{           ///保存格式
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            tmp=saveFormat;
                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_SAVEENABLE ,tmp);
                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_SAVEENABLE ,tmp);
                            if(tmp==saveFormat){
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" saveFormat: "<<saveFormat<< " valid!"<< endl;
                                requestorPtr_->action_=CameraControl_Action_Valid;
                                return true;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" saveFormat: "<<saveFormat<< " invalid! get: "<<tmp<<endl;
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                        }
                        break;
                    }
                    case  6:{           ///跳帧数
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            tmp=skipNumber;
                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_SKIPNUM ,tmp);
                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_SKIPNUM ,tmp);
                            if(tmp==skipNumber){
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" skipNumber: "<<skipNumber<< " valid!"<< endl;
                                requestorPtr_->action_=CameraControl_Action_Valid;
                                return true;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" skipNumber: "<<skipNumber<< " invalid! get: "<<tmp<<endl;
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                        }
                        break;
                    }
                    case  7:{           ///触发模式
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            tmp=triggerMode;
                            boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_TRIGGERMODE ,tmp);
                            //boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_TRIGGERMODE ,tmp);
                            if(tmp==triggerMode){
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerMode: "<<triggerMode<< " valid!"<< endl;
                                requestorPtr_->action_=CameraControl_Action_Valid;
                                return true;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerMode: "<<triggerMode<< " invalid! get: "<<tmp<<endl;
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                        }
                        break;
                    }
                    case  8:{           ///保存路径+名称
                        if(boxVec_[i].ptr_->IsDevOpen()){
                            ///创建和设置保存目录+名称
                            if(CreatDirectory(savePath.c_str())){
                                boxVec_[i].ptr_->SetImagePath(j,savePath+saveName);  ///Path+Name
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" savePath+Name: \n       "<<savePath+saveName<<" valid"<<endl;
                            }
                            else{
                                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" savePath+Name: \n       "<<savePath+saveName<< " invalid!"<< endl;
                                requestorPtr_->action_=CameraControl_Action_Invalid;
                                return false;
                            }
                        }
                        else{
                            cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" is not opened!"<<endl;
                            requestorPtr_->action_=CameraControl_Action_Invalid;
                        }
                        break;
                    }
                    default:{
                        cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" operationIndex: "<<operationIndex<< " invalid!"<<endl;
                        requestorPtr_->action_=CameraControl_Action_Invalid;
                        return false;
                        break;
                    }
                }

            }
        }
        cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<<"or boxVec["<<i<<"].ptr_ is NULL!"<< endl;
        syslog(LOG_WARNING, "[INFO OpenCamera] CameraControlThread box: boxIndex=%d >= boxVec.size() or boxVec[%d].ptr_ is NULL!\n", i, i);
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::CloseCamera(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        uint32_t i=requestorPtr_->boxIndex_;
        uint32_t j=requestorPtr_->cameraIndex_;
        if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL){
            boxVec_[i].ptr_->AcquireImages(false);

            ///关闭打开相机时申请的窗口资源
            for(int32_t k=0;k<boxVec_[i].imageVec_.size();++k){
                //boxVec_[i].imageVec_[k].release();
                if(boxVec_[i].imageVec_[k]!=NULL){
                    delete boxVec_[i].imageVec_[k];
                    boxVec_[i].imageVec_[k]=NULL;
                }
                boxVec_[i].imageFlushVec_[k]=false;
                //if(boxVec_[i].imageShowEnable_){
                    //cv::destroyWindow(boxVec_[i].imageShowWindowVec_[k]);
                    ///多关闭几次以防止在ubuntu下无法关闭窗口
                    //for(int32_t i=0;i<3;++i){
                    //cv::waitKey(10);
                    //}
                //}

                boxVec_[i].capturedImageCountVec_[k]=0;
                if(boxVec_[i].imageSaverVec_[k]!=NULL){
                    delete boxVec_[i].imageSaverVec_[k];
                    boxVec_[i].imageSaverVec_[k]=NULL;
                }
                for(int32_t r=0;r<boxVec_[i].resizedImagesVec_[k].size();++r){
                    if(boxVec_[i].temp_[k][r]!=NULL){
                        delete boxVec_[i].temp_[k][r];
                        boxVec_[i].temp_[k][r]=NULL;
                    }
                }
                //boxVec_[i].resizedImagesVec_[k][r].release();
            }

            requestorPtr_->action_=CameraControl_Action_Valid;
            return true;
        }
        else{
            requestorPtr_->action_=CameraControl_Action_Valid;
            return true;
        }
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::TriggerContinous(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        uint32_t i=requestorPtr_->boxIndex_;
        uint32_t j=requestorPtr_->cameraIndex_;
        int32_t triggerNumber=requestorPtr_->triggerNumber_;
        int32_t imageType=requestorPtr_->imageType_;
        if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL){
            if(boxVec_[i].ptr_->IsCapOpen()){                   ///连续拍摄模式下先关闭上一个进程，再重启
                requestorPtr_->action_=CameraControl_Action_Valid;
                return true;
            }

            ///先关闭之前申请的空间
            for(int32_t k=0;k<boxVec_[i].imageVec_.size();++k){
                //boxVec_[i].imageVec_[k].release();
                if(boxVec_[i].imageVec_[k]!=NULL){
                        delete boxVec_[i].imageVec_[k];
                        boxVec_[i].imageVec_[k]=NULL;
                }
                boxVec_[i].imageFlushVec_[k]=false;
                //if(boxVec_[i].imageShowEnable_){
                    //cv::destroyWindow(boxVec_[i].imageShowWindowVec_[k]);
                    ///多关闭几次以防止在ubuntu下无法关闭窗口
                    //for(int32_t i=0;i<3;++i){
                    //cv::waitKey(10);
                    //}
                //}
                boxVec_[i].capturedImageCountVec_[k]=0;
                if(boxVec_[i].imageSaverVec_[k]!=NULL){
                    delete boxVec_[i].imageSaverVec_[k];
                    boxVec_[i].imageSaverVec_[k]=NULL;
                }
                for(int32_t r=0;r<boxVec_[i].resizedImagesVec_[k].size();++r){
                    if(boxVec_[i].temp_[k][r]!=NULL){
                        delete boxVec_[i].temp_[k][r];
                        boxVec_[i].temp_[k][r]=NULL;
                    }
                }
                //boxVec_[i].resizedImagesVec_[k][r].release();
            }

            ///为图片存储申请空间
            uint32_t width=cameraServerUnit_->boxVec_[i].cameraVec_[j].width_;
            uint32_t height=cameraServerUnit_->boxVec_[i].cameraVec_[j].height_;
            uint32_t cameraAmount=cameraServerUnit_->boxVec_[i].cameraVec_.size();
            boxVec_[i].imageVec_.clear();
            boxVec_[i].last_sid.clear();
            boxVec_[i].imageFlushVec_.clear();
            //boxVec_[i].imageShowWindowVec_.clear();
            boxVec_[i].capturedImageCountVec_.clear();
            boxVec_[i].imageSaverVec_.clear();
            boxVec_[i].resizedImagesVec_.clear();
            boxVec_[i].imageVec_.resize(cameraAmount);
            boxVec_[i].last_sid.resize(cameraAmount);
            boxVec_[i].imageFlushVec_.resize(cameraAmount);
            //boxVec_[i].imageShowWindowVec_.resize(cameraAmount);
            boxVec_[i].capturedImageCountVec_.resize(cameraAmount);
            boxVec_[i].imageSaverVec_.resize(cameraAmount);
            boxVec_[i].resizedImagesVec_.resize(cameraAmount);
            //std::ostringstream strTmp;
            for(int32_t k=0;k<boxVec_[i].imageVec_.size();++k){
                uint32_t cameraId=cameraServerUnit_->boxVec_[i].cameraVec_[k].id_;

                //boxVec_[i].imageVec_[k]=cv::Mat::zeros(height,width,CV_8UC1);
                //boxVec_[i].imageVec_[k]=cv::Mat::zeros(height,width,CV_16UC1);
                boxVec_[i].imageVec_[k]=new uint16_t [height*width];
                for(int32_t l=0;l<height*width;++l)
                    boxVec_[i].imageVec_[k][l]=0;

                boxVec_[i].imageFlushVec_[k]=false;
                //strTmp.str("");
                //strTmp<<"Box_"<<i<<"-Cam_"<<k<<"-Num_"<<cameraId;
                //boxVec_[i].imageShowWindowVec_[k]=strTmp.str();
                //if(boxVec_[i].imageShowEnable_){
                    //cv::namedWindow(boxVec_[i].imageShowWindowVec_[k],CV_WINDOW_NORMAL|CV_WINDOW_KEEPRATIO);
                //}
                boxVec_[i].capturedImageCountVec_[k]=0;
                //cout<<"[SHADOWK] ready to new saver"<<endl;
                //boxVec_[i].imageSaverVec_[k]=new ImageSaverThread((i<<1)|k,height,width,CV_8UC1);
                //boxVec_[i].imageSaverVec_[k]=new ImageSaverThread((i<<1)|k,height,width,CV_16UC1);
                boxVec_[i].imageSaverVec_[k]=new ImageSaverThread((i<<1)|k,height,width,imageType);

                //boxVec_[i].resizedImagesVec_[k].push_back(cv::Mat::zeros(0,0,CV_8UC1));
                //boxVec_[i].resizedImagesVec_[k].push_back(cv::Mat::zeros(0,0,CV_16UC1)); ///???

                //for(int r=1;r<=boxVec_[i].resizedFactorMax_;++r){
                for(int32_t r=0;r<boxVec_[i].resizedFactorNumMax_;++r){

                    boxVec_[i].resizedPixelNum_[r]=height/(1<<r)*width/(1<<r);
                    boxVec_[i].temp_[k][r]=new uint16_t [boxVec_[i].resizedPixelNum_[r]];
                    for(int32_t l=0;l<boxVec_[i].resizedPixelNum_[r];++l)
                        boxVec_[i].temp_[k][r][l]=0;

                    //boxVec_[i].resizedImagesVec_[k].push_back(cv::Mat::zeros(height/r,width/r,CV_8UC1));
                    //boxVec_[i].resizedImagesVec_[k].push_back(cv::Mat::zeros(height/r,width/r,CV_16UC1));
                    boxVec_[i].resizedImagesVec_[k].push_back(boxVec_[i].temp_[k][r]); ///

                }
            }

            ///在触发模式为FreeRun时将会一直拍摄，在触发模式为Trigger时将会等待内部或外部触发信号
            if(boxVec_[i].ptr_->AcquireImages(true,500000)){
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerContinous: "<<triggerNumber<< " valid!"<< endl;
                syslog(LOG_INFO, "[INFO TriggerContinous] CameraControlThread box: %d-camera: %d triggerContinous: %d valid!\n", i, j, triggerNumber);
                requestorPtr_->action_=CameraControl_Action_Valid;
                return true;
            }
            else{
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerContinous: "<<triggerNumber<< " invalid!"<< endl;
                syslog(LOG_WARNING, "[INFO TriggerContinous] CameraControlThread box: %d-camera: %d triggerContinous: %d invalid!\n", i, j, triggerNumber);
                requestorPtr_->action_=CameraControl_Action_Invalid;
                return false;
            }
        }
        cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<<"or boxVec["<<i<<"].ptr_ is NULL!"<< endl;
        syslog(LOG_WARNING, "[INFO TriggerContinous] CameraControlThread box: boxIndex= %d >= boxVec.size() or boxVec[%d].ptr_ is NULL!\n", i, i);
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::TriggerSingle(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        uint32_t i=requestorPtr_->boxIndex_;
        uint32_t j=requestorPtr_->cameraIndex_;
        int32_t triggerNumber=requestorPtr_->triggerNumber_;
        //cout<<"[SHADOWK] i="<<i<<"boxVec.size()="<<boxVec_.size()<<endl;
        if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL&&j<boxVec_[i].imageVec_.size()){
            ///在捕获线程已经开启的情况下抓取单张
            int channelIndex=j;
            if(!boxVec_[i].ptr_->IsCapOpen()){
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerSingle: "<<triggerNumber<< " invalid!"<< endl;
                syslog(LOG_WARNING, "[INFO TriggerSingle] CameraControlThread box: %d-camera: %d triggerSingle: %d invalid!\n", i, j, triggerNumber);
                requestorPtr_->action_=CameraControl_Action_Invalid;
                return false;
            }
//            if(boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_TRIGGERSOFT,triggerNumber)){
//                boxVec_[i].imageFlushVec_[j]=false;
//                long startTime=GetCurrentTimeMs(),endTime=GetCurrentTimeMs()+1;
//                while(!boxVec_[i].imageFlushVec_[j]&&(endTime-startTime)<verifyCameraTriggerSingleMaxMs){
//                    SleepMs(2);
//                    endTime=GetCurrentTimeMs();
//                }
//                if(!boxVec_[i].imageFlushVec_[j]||(endTime-startTime)>verifyCameraTriggerSingleMaxMs){
//                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerSingle: "<<triggerNumber<< " Invalid!"<< endl;
//                    requestorPtr_->action_=CameraControl_Action_Invalid;
//                    return false;
//                }
//                else{
////                    boxVec_[i].Lock();
//                    cv::Mat tmp= boxVec_[i].imageVec_[j].clone();
////                    boxVec_[i].Unlock();
//                    string savePath=cameraServerUnit_->boxVec_[i].cameraVec_[j].savePath_;
//                    string saveName=requestorPtr_->saveName_;
//                    if(saveName==""){
//                        saveName="default";
//                    }
//                    cv::imwrite(savePath+saveName+boxVec_[i].saveSingleSuffix_,tmp);
//                    cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerSingle: "<<saveName<< " valid!"<< endl;
//                    requestorPtr_->action_=CameraControl_Action_Valid;
//                    return true;
//                }
//            }
            ///由于当前触发单张不稳定采取从当前缓存流里抓取一张图片的方式
            boxVec_[i].imageFlushVec_[j]=false;
            long startTime=GetCurrentTimeMs(),endTime=GetCurrentTimeMs()+1;
            while(!boxVec_[i].imageFlushVec_[j]&&(endTime-startTime)<verifyCameraTriggerSingleMaxMs){
                SleepMs(5);
                endTime=GetCurrentTimeMs();
            }
            if(!boxVec_[i].imageFlushVec_[j]||(endTime-startTime)>verifyCameraTriggerSingleMaxMs){
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerSingle: "<<triggerNumber<< " Invalid!"<< endl;
                syslog(LOG_WARNING, "[INFO TriggerSingle] CameraControlThread box: %d-camera: %d triggerSingle: %d invalid!\n", i, j, triggerNumber);
                requestorPtr_->action_=CameraControl_Action_Invalid;
                return false;
            }
            else{
                //boxVec_[i].Lock();
                //cv::Mat tmp= boxVec_[i].imageVec_[j].clone();

                //boxVec_[i].Unlock();
                string savePath=cameraServerUnit_->boxVec_[i].cameraVec_[j].savePath_;
                string saveName=requestorPtr_->saveName_;
                if(saveName==""){
                    saveName="default";
                }
                //cv::imwrite(savePath+saveName+boxVec_[i].saveSingleSuffix_,tmp);
                boxVec_[i].imageSaverVec_[j]->Write(boxVec_[i].imageVec_[j],
                                                    savePath+saveName+boxVec_[i].saveSuffix_,
                                                    cameraServerUnit_->boxVec_[i].cameraVec_[j].exposure_,
                                                    boxVec_[i].last_sid[j],
                                                    cameraServerUnit_->boxVec_[i].cameraVec_[j].bitLut_);
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" triggerSingle: "<<saveName<< " valid!"<< endl;
                syslog(LOG_INFO, "[INFO TriggerSingle] CameraControlThread box: %d-camera:%d triggerSingle: %s valid!\n", i, j, saveName.c_str());
                requestorPtr_->action_=CameraControl_Action_Valid;
                return true;
            }
        }
        cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<<"or boxVec["<<i<<"].ptr_ is NULL!"<< endl;
        syslog(LOG_WARNING, "[INFO TriggerSingle] CameraControlThread box: boxIndex= %d >= boxVec.size() or boxVec[%d].ptr_ is NULL!\n", i, i);
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::GetImage(CameraControlMessage *requestorPtr_)
{
     if(requestorPtr_!=NULL){
        uint32_t i=requestorPtr_->boxIndex_;
        uint32_t j=requestorPtr_->cameraIndex_;
        int32_t imageType=requestorPtr_->imageType_;
        if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL&&j<boxVec_[i].imageVec_.size()){
            ///在捕获线程已经开启的情况下抓取单张
            int channelIndex=j;
            if(!boxVec_[i].ptr_->IsCapOpen()){
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" capture is stopped,getImage invalid!"<< endl;
                syslog(LOG_WARNING, "[INFO GetImage] CameraControlThread box: %d-camera: %d capture is stopped, getImage invalid!\n", i, j);
                requestorPtr_->action_=CameraControl_Action_Invalid;
                return false;
            }

            //TODO:[zhu-ty]Resize的方式需要修改

            long startTime=GetCurrentTimeMs();

            //uint32_t resizedFactor=requestorPtr_->resizeFactor_;  ///1,2,3,4
            uint32_t resizedFactorIndex=requestorPtr_->resizeFactor_-1;
            uint32_t resizedFactor=1<<resizedFactorIndex;
            uint32_t resizedWidth=requestorPtr_->resizedWidth_;
            uint32_t resizedHeight=requestorPtr_->resizedHeight_;
            uint32_t resizedPixNumReq=resizedWidth*resizedHeight;
            //if(resizedFactor>0&&resizedFactor<=boxVec_[i].resizedFactorMax_&&resizedWidth==boxVec_[i].resizedImagesVec_[j][resizedFactor].cols&& \
                resizedHeight==boxVec_[i].resizedImagesVec_[j][resizedFactor].rows){
              if(resizedFactorIndex>=0 && resizedFactorIndex<boxVec_[i].resizedFactorNumMax_ && resizedPixNumReq==boxVec_[i].resizedPixelNum_[resizedFactorIndex]){  ///
                //cv::resize(boxVec_[i].imageVec_[j],boxVec_[i].resizedImagesVec_[j][resizedFactor],cv::Size(resizedWidth,resizedHeight));  ///
                Resize(boxVec_[i].imageVec_[j], boxVec_[i].resizedImagesVec_[j][resizedFactorIndex], resizedFactor, resizedWidth, resizedHeight);  ///
 //               int32_t rows=boxVec_[i].imageVec_[j].rows;
 //               int32_t cols=boxVec_[i].imageVec_[j].cols;

                //memcpy(requestorPtr_->imageData_,boxVec_[i].resizedImagesVec_[j][resizedFactor].ptr<uint8_t>(0),resizedWidth*resizedHeight);
                //memcpy(requestorPtr_->imageData_,boxVec_[i].resizedImagesVec_[j][resizedFactor].ptr<uint8_t>(0),resizedWidth*resizedHeight*(2-bitMode));
                memcpy(requestorPtr_->imageData_, (uint8_t *)boxVec_[i].resizedImagesVec_[j][resizedFactorIndex], resizedWidth*resizedHeight*imageType);

//               memcpy(requestorPtr_->imageData_,boxVec_[i].imageVec_[j].ptr<uint8_t>(0),rows*cols);
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" getImage valid!"<< endl;
                requestorPtr_->action_=CameraControl_Action_Valid;
                return true;

            }

//            if(boxVec_[i].imageVec_[j].cols==requestorPtr_->resizedWidth_&&boxVec_[i].imageVec_[j].rows==requestorPtr_->resizedHeight_&&requestorPtr_->imageData_!=NULL){
//                int32_t rows=boxVec_[i].imageVec_[j].rows;
//                int32_t cols=boxVec_[i].imageVec_[j].cols;
//                memcpy(requestorPtr_->imageData_,boxVec_[i].imageVec_[j].ptr<uint8_t>(0),rows*cols);
//                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" getImage valid!"<< endl;
//                requestorPtr_->action_=CameraControl_Action_Valid;
//                return true;
//            }
            else{
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" getImage invalid!"<<"resizeFactor/resizedWidth/resizedHeight is invalid or dataBuffer is NULL!"<< endl;
                syslog(LOG_WARNING, "[INFO GetImage] CameraControlThread box: %d-camera: %d getImage invalid! ResizeFactor/resizedWidth/resizedHeight is invalid or dataBuffer is NULL!\n", i, j);
                requestorPtr_->action_=CameraControl_Action_Invalid;
                return false;
            }
        }
        cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<<"or boxVec["<<i<<"].ptr_ is NULL!"<< endl;
        syslog(LOG_WARNING, "[INFO GetImage] CameraControlThread box: boxIndex= %d >= boxVec.size() or boxVec[%d].ptr_ is NULL!\n", i, i);
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::ResetId(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        uint32_t i=requestorPtr_->boxIndex_;
        uint32_t j=requestorPtr_->cameraIndex_;
        if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL){
            int channelIndex=j;
            if(boxVec_[i].ptr_->SetParam(channelIndex,MV_PARAM_CAPIDRESET,1)){
                cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" resetId valid!"<< endl;
                requestorPtr_->action_=CameraControl_Action_Valid;
                return true;
            }
        }
        cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<<"or boxVec["<<i<<"].ptr_ is NULL!"<< endl;
        syslog(LOG_WARNING, "[INFO ResetId] CameraControlThread box: boxIndex= %d >= boxVec.size() or boxVec[%d].ptr_ is NULL!\n", i, i);
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


bool CameraControlThread::GetTemperature(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL){
        for(int i=0;i<boxVec_.size();++i){
            if(boxVec_[i].ptr_!=NULL){
                for(int j=0;j<cameraServerUnit_->boxVec_[i].cameraVec_.size();++j){
                    int channelIndex=j+1;
                    int tmp=-INT_MAX;
                    boxVec_[i].ptr_->GetParam(channelIndex,MV_PARAM_CAMTEMPERATURE,tmp);
                    if(tmp>cameraTemperatureMin&&tmp<cameraTemperatureMax){
                        cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" temperature: "<<tmp<< endl;
                    }
                    else{
                        cout << "[INFO] CameraControlThread box:" << i << "-camera:" << j <<" temperature warning!: "<<tmp<<endl;
                        syslog(LOG_WARNING, "[INFO GetTemperature] CameraControlThread box: %d -camera: %d temperature warning: %d!\n", i, j, tmp);
                    }
                }
            }
            else{
                cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<<"or boxVec["<<i<<"].ptr_ is NULL!"<< endl;
                syslog(LOG_WARNING, "[INFO GetTemperature] CameraControlThread box: boxIndex= %d >= boxVec.size() or boxVec[%d].ptr_ is NULL!\n", i, i);
            }
        }
        requestorPtr_->action_=CameraControl_Action_Valid;
        return true;
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


// User DataCallback
void CameraControlThread::CaptureOneFinished(int lDevIndex, CMVImage* lCallBackImg)
{
//    cout<< "DataCallback image @ Dev["
//       	<< lDevIndex
//        << "]["
//        << lCallBackImg->imageCID
//        << "] VALID (" << lCallBackImg->bValidFlag
//        << "); NO: " << setw(4) << lCallBackImg->imageNO
//        << "; SID: " << setw(4) << lCallBackImg->imageSID
//        << "; FID: " << setw(4) << lCallBackImg->imageFID
//        << "; TID: " << setw(4) << lCallBackImg->imageTID
//        << "; W: " << lCallBackImg->width
//        << "; H: " << lCallBackImg->height
//        << "; D:" << lCallBackImg->channels*8
//       	<< endl;

    //cout<<"[SHADOWK] callback"<<endl;

    uint32_t i=lDevIndex;
    uint32_t j=lCallBackImg->imageCID;

    long startTime=GetCurrentTimeMs();
//    boxVec_[i].Lock();
    if(i<boxVec_.size()&&boxVec_[i].ptr_!=NULL&&j<boxVec_[i].imageVec_.size()){
        //uint32_t bitDepth=lCallBackImg->channels*8;     //bmp:8bit,raw:16bit
        int32_t imageType=lCallBackImg->channels;
        //if(bitDepth==8&&!boxVec_[i].imageVec_[j].empty())
        //if(bitDepth==16&&!boxVec_[i].imageVec_[j].empty())
        if((imageType==1||imageType==2) && boxVec_[i].imageVec_[j] != NULL)
        {

            //TODO:[zhu-ty]Resize的方式需要修改

            uint32_t srcSize=lCallBackImg->width*lCallBackImg->height;
            //uint32_t dstSize=boxVec_[i].imageVec_[j].rows*boxVec_[i].imageVec_[j].cols;  ///???
            uint32_t dstSize=cameraServerUnit_->boxVec_[i].cameraVec_[j].width_*cameraServerUnit_->boxVec_[i].cameraVec_[j].height_;  ///???
            //if(srcSize==dstSize&&boxVec_[i].imageVec_[j].isContinuous()){  ///???
            if(srcSize==dstSize){  ///???
                //memcpy(boxVec_[i].imageVec_[j].ptr<uint8_t>(0),lCallBackImg->imageData,srcSize);
                //memcpy(boxVec_[i].imageVec_[j].ptr<uint8_t>(0),lCallBackImg->imageData,srcSize*(2-bitMode));///???
                memcpy((uint8_t *)boxVec_[i].imageVec_[j],lCallBackImg->imageData,srcSize*imageType);
            }
            else{
                //int32_t cols=boxVec_[i].imageVec_[j].cols;
                int32_t cols=cameraServerUnit_->boxVec_[i].cameraVec_[j].width_;
                //for(int32_t r=0;r<boxVec_[i].imageVec_[j].rows;++r){
                for(int32_t r=0;r<cameraServerUnit_->boxVec_[i].cameraVec_[j].height_;++r){
                    //memcpy(boxVec_[i].imageVec_[j].ptr<uint8_t>(r),lCallBackImg->imageData+r*cols,cols);
                    //memcpy(boxVec_[i].imageVec_[j].ptr<uint8_t>(r),lCallBackImg->imageData+r*cols*(2-bitMode),cols*(2-bitMode));
                    memcpy((uint8_t *)boxVec_[i].imageVec_[j]+r*cols,lCallBackImg->imageData+r*cols*imageType,cols*imageType);  ///???
                }
            }
            boxVec_[i].imageFlushVec_[j]=true;
            //if(boxVec_[i].imageShowEnable_){
                //imshow(boxVec_[i].imageShowWindowVec_[j],boxVec_[i].imageVec_[j]);
                //cv::waitKey(1);
            //}

            ///save image
            //cout<<"[SHADOWK] about to save"<<endl;
            if(cameraServerUnit_->boxVec_[i].cameraVec_[j].saveFormat_!=0){
                //cout<<"[SHADOWK] about to save 2"<<endl;
                boxVec_[i].capturedImageCountVec_[j]++;
                std::ostringstream saveNameTmp;
                uint64_t sid_ = 0;
                if(cameraServerUnit_->boxVec_[i].cameraVec_.size()==1){
                    saveNameTmp<<cameraServerUnit_->boxVec_[i].cameraVec_[j].savePath_<<lCallBackImg->imageSID-1<<boxVec_[i].saveSuffix_;
                    sid_ = lCallBackImg->imageSID-1;
                }
                else if(cameraServerUnit_->boxVec_[i].cameraVec_.size()==2){
                    saveNameTmp<<cameraServerUnit_->boxVec_[i].cameraVec_[j].savePath_<<(lCallBackImg->imageSID+1)/2-1<<boxVec_[i].saveSuffix_;
                    sid_ = (lCallBackImg->imageSID+1)/2-1;
                }
                boxVec_[i].last_sid[j] = sid_;
                boxVec_[i].imageSaverVec_[j]->Write(boxVec_[i].imageVec_[j],saveNameTmp.str(), cameraServerUnit_->boxVec_[i].cameraVec_[j].exposure_, sid_, cameraServerUnit_->boxVec_[i].cameraVec_[j].bitLut_);
            }
        }
    }else{
        cout << "[INFO] CameraControlThread box: boxIndex=" <<i<<" >= boxVec.size() "<<"or boxVec["<<i<<"].ptr_ is NULL "<<"or cameraIndex="<<j<<">=boxVec["<<i<<"].imageVec_.size()"<< endl;
        syslog(LOG_WARNING, "[INFO CaptureOneFinished] CameraControlThread box: boxIndex= %d >= boxVec.size() or boxVec[%d].ptr_ or cameraIndex=%d >= boxVec[%d].imageVec_.size()is NULL!\n",
               i, i, j, i);
    }
//    boxVec_[i].Unlock();
//    cout<<"Capture one finished consume: "<<GetCurrentTimeMs()-startTime<<endl;

}


void CameraControlThread::CaptureMessage(int lDevIndex, MV_MESSAGE lMsg)
{
    switch(lMsg)
    {
        case MV_MESSAGE_IMGCAPOVER:
             //dosth...
            cout<< "InfoCallback message @ Dev["
               << lDevIndex
               << "] Get MV_MESSAGE_IMGCAPOVER!"
               << endl;
            break;
        default:
            break;
    }
}

void CameraControlThread::Resize(uint16_t *imgSrc, uint16_t *imgDst, uint32_t scale, uint32_t widthDst, uint32_t heightDst)
{
    uint32_t widthSrc = imageWidthMax;
    uint32_t heightSrc = imageHeightMax;

    for (uint32_t j = 0; j < heightDst; ++j)
        for (uint32_t i = 0; i < widthDst; ++i){
            float x = i*scale;
            float y = j*scale;
            float _X, _Y;
            int32_t _i, _j;
            float _u, _v;
            float weight0, weight1, weight2, weight3;
            if(x >= widthSrc-1 || y >= heightSrc-1){
                _X = widthSrc-1.00005;
                _Y = heightSrc-1.00005;
                _i = _X;
                _j = _Y;
                _u = _X - _i;
                _v = _Y - _j;
                weight0 = (1-_u) * (1-_v);
                weight1 = (1-_u) * _v;
                weight2 = _u * (1-_v);
                weight3 = _u * _v;
            }
            else{
                _X = x;
                _Y = y;
                _i = x;
                _j = y;
                _u = x - _i;
                _v = y - _j;
                weight0 = (1-_u) * (1-_v);
                weight1 = (1-_u) * _v;
                weight2 = _u * (1-_v);
                weight3 = _u * _v;
            }
            uint16_t * DataJ = imgSrc + _j * widthSrc;
            uint16_t * DataJP = imgSrc + (_j+1) * widthSrc;
            imgDst[i+j*widthDst]=weight0*DataJ[_i] + weight2*DataJ[_i+1] + weight1*DataJP[_i] + weight3*DataJP[_i+1];
        }
}






