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


CameraControlThread::CameraControlThread(std::shared_ptr<cam::GenCamera> _gencamera, CameraControlMessageDeque *_cameraControlMessageDeque)
{
    if(_gencamera && _cameraControlMessageDeque!=NULL)
    {
        cameraControlMessageDeque_=_cameraControlMessageDeque;
        gencamera_ = _gencamera;
        if(!StartInternalThread())
        {
            cout << "[INFO] CameraControlThread failed to start!" << endl;
            syslog(LOG_WARNING, "[Action] CameraControlThread failed to start!\n");
        }
    }
    else
    {
        cout << "[INFO] CameraControlThread failed to start!" << endl;
        syslog(LOG_WARNING, "[Action] CameraControlThread failed to start!\n");
    }
}


CameraControlThread::~CameraControlThread()
{

}



int CameraControlThread::Run(void)
{
    cout << "[INFO] CameraControlThread started!" << endl;


    ///维持自身线程运行的消息，不会发送到消息队列
    CameraControlMessage cameraControlMessageThis("",CameraControl_Thread_Wait);

    CameraControlMessage *cameraControlMessageTmpPtr=NULL;

    //omp_set_num_threads(4);

    //std::vector<cv::Mat> matTest(8,cv::Mat(2160,2560,CV_8UC1,cv::Scalar(128)));

    while(cameraControlMessageThis.action_!=CameraControl_Thread_Exit)
    {
//        cout<<"This is CameraControlThread"<<endl<<endl;
        if(!cameraControlMessageDeque_->Empty())
        {
            cameraControlMessageTmpPtr=cameraControlMessageDeque_->PopFront();
            cameraControlMessageThis=*cameraControlMessageTmpPtr;
        }
        if(cameraControlMessageThis.action_==CameraControl_Thread_Exit)
        {
            if(cameraControlMessageTmpPtr!=NULL)
            {
                cameraControlMessageTmpPtr->action_=CameraControl_Action_Valid;
            }
            break;
        }
        else if(cameraControlMessageThis.action_==CameraControl_Thread_Wait)
        {
            SleepUs(200);

        }
        switch(cameraControlMessageThis.action_)
        {

        case CameraControl_Open_Camera:
        {
            cout << Colormod::yellow << "[Action]"<< Colormod::def <<" CameraControlThread: CameraControl_Open_Camera:" << endl;
            syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Open_Camera\n");
            OpenCamera(cameraControlMessageTmpPtr);
            cameraControlMessageThis.action_=CameraControl_Thread_Wait;
            break;
        }
        case CameraControl_Close_Camera:
        {
            cout << Colormod::yellow << "[Action]"<< Colormod::def <<" CameraControlThread: CameraControl_Close_Camera:" << endl;
            syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Close_Camera\n");
            CloseCamera(cameraControlMessageTmpPtr);
            cameraControlMessageThis.action_=CameraControl_Thread_Wait;
            break;
        }
        case CameraControl_Get_Image:
        {
            cout << Colormod::yellow << "[Action]"<< Colormod::def <<" CameraControlThread: CameraControl_Get_Image:" << endl;
            syslog(LOG_INFO, "[Action] CameraControlThread: CameraControl_Get_Image\n");
            GetImage(cameraControlMessageTmpPtr);
            cameraControlMessageThis.action_=CameraControl_Thread_Wait;
            break;
        }
        }
    }
    cout << "[INFO] CameraControlThread ended!" << endl;
    syslog(LOG_INFO, "[INFO] CameraControlThread ended!\n");
    return 0;
}



bool CameraControlThread::OpenCamera(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL)
    {
        if(!opened)
        {
            gencamera_->init();
            gencamera_->startCapture();
            gencamera_->setFPS(-1, 20);
            gencamera_->setAutoExposure(-1, cam::Status::on);
            gencamera_->setAutoExposureLevel(-1, 25);
            gencamera_->setAutoExposureCompensation(-1, cam::Status::on, 0);
            gencamera_->setAutoWhiteBalance(-1);
            gencamera_->setCamBufferType(cam::GenCamBufferType::JPEG);
            gencamera_->setJPEGQuality(85, 0.15);
            gencamera_->setCaptureMode(cam::GenCamCaptureMode::Continous, 50);
            gencamera_->setCapturePurpose(cam::GenCamCapturePurpose::Streaming);
            cam::SysUtil::sleep(500);
            gencamera_->getCamInfos(camInfos);
            imgdata.resize(camInfos.size());
            gencamera_->startCaptureThreads();
            //cameraArray_->allocateBufferJPEG(10);
            opened = true;
            //cameraPtr->startCapture();
            
        }
        //cameraArray_->allocateBuffer(10);
        //cameraArray_->startCapture(10);
        requestorPtr_->action_=CameraControl_Action_Valid;
        return true;
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}

bool CameraControlThread::CloseCamera(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL)
    {
        gencamera_->stopCaptureThreads();
        gencamera_->release();
        requestorPtr_->action_=CameraControl_Action_Valid;
        return true;
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}

bool CameraControlThread::GetImage(CameraControlMessage *requestorPtr_)
{
    //printf("[SHADOWK] GetImage inside function\n");
    if(requestorPtr_!=NULL)
    {
        gencamera_->captureFrame(imgdata);
        cout <<Colormod::magenta<<"[SHDAOWK]"<<Colormod::def<<"[INFO] GenCamera captureFrame" << endl;
        requestorPtr_->imageamount = imgdata.size();
        int32_t pointer = 0;
        for(int i = 0;i < imgdata.size(); i++)
        {
            memcpy(requestorPtr_->imageData_ + pointer, (uint8_t *)(&(imgdata[i].length)), sizeof(int));
            pointer += sizeof(int);
            memcpy(requestorPtr_->imageData_ + pointer, (uint8_t *)(imgdata[i].data), imgdata[i].length);
            pointer += imgdata[i].length;
        }
        requestorPtr_->imagelen = pointer;
        requestorPtr_->action_=CameraControl_Action_Valid;
        return true;
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}


void CameraControlThread::Resize(uint16_t *imgSrc, uint16_t *imgDst, uint32_t scale, uint32_t widthDst, uint32_t heightDst)
{
    uint32_t widthSrc = imageWidthMax;
    uint32_t heightSrc = imageHeightMax;

    for (uint32_t j = 0; j < heightDst; ++j)
        for (uint32_t i = 0; i < widthDst; ++i)
        {
            float x = i*scale;
            float y = j*scale;
            float _X, _Y;
            int32_t _i, _j;
            float _u, _v;
            float weight0, weight1, weight2, weight3;
            if(x >= widthSrc-1 || y >= heightSrc-1)
            {
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
            else
            {
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






