/************************************************************************
//Describe: cameraControl.cpp: used for calling API
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/14/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
************************************************************************/
#include "cameraControl.h"
#include <numeric>      // std::accumulate


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


CameraControlThread::CameraControlThread(std::vector<std::shared_ptr<cam::GenCamera>> _gencamera_s, CameraControlMessageDeque *_cameraControlMessageDeque)
{
    if(_gencamera_s[0] && _cameraControlMessageDeque!=NULL)
    {
        cameraControlMessageDeque_=_cameraControlMessageDeque;
        gencamera_s_ = _gencamera_s;
        if(!StartInternalThread())
        {
            cout << "[INFO] CameraControlThread failed to start!" << endl;
            syslog(LOG_WARNING, "[Action] CameraControlThread failed to start!\n");
        }
        if(gencamera_s_.size() == 0)
        {
            cout << "[INFO] CameraControlThread::CameraControlThread failed to start! Too few camera type" << endl;
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
        string gfun = requestorPtr_->genfunc_;
        if(gfun.compare("init") == 0)
        {
            camInfos.clear();
            camera_count_.clear();
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->init();
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
                //TODO: now only JPEG!
                gencamera_s_[i]->setCamBufferType(cam::GenCamBufferType::JPEG);

                //Get count for each type of the camera
                std::vector<cam::GenCamInfo> tmp_camInfos;
                gencamera_s_[i]->getCamInfos(tmp_camInfos);
                camInfos.insert(camInfos.end(), tmp_camInfos.begin(), tmp_camInfos.end());
                camera_count_.push_back(tmp_camInfos.size());
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("setImageRatios") == 0)
        {
            int tmp_ret_all = 0;
            int data_idx = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                std::vector<cam::GenCamImgRatio> ratio_vector(camera_count_[i]);
                for(int j = 0; j < camera_count_[i];j++)
                {
                    ratio_vector[j] = (cam::GenCamImgRatio)requestorPtr_->gendata_.param_func.param_int[data_idx];
                    data_idx++;
                }
                int tmp_ret = gencamera_s_[i]->setImageRatios(ratio_vector);
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("startCapture") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->startCapture();
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("getCamInfos") == 0)
        {
            camInfos.clear();
            int tmp_ret_all = 0;
            imgdata_s.resize(gencamera_s_.size());
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                std::vector<cam::GenCamInfo> tmp_camInfos;
                int tmp_ret = gencamera_s_[i]->getCamInfos(
                tmp_camInfos
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
                camInfos.insert(camInfos.end(), tmp_camInfos.begin(), tmp_camInfos.end());
                imgdata_s[i].resize(tmp_camInfos.size());
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            //imgdata.resize(camInfos.size());
            if(camInfos.size() > MAX_CAMERA_NUM)
            {
                cout << Colormod::red << "[ERROR]" << Colormod::def << 
                " CameraControlThread::OpenCamera getCamInfos too many cameras!" << 
                " Maximum is : " << MAX_CAMERA_NUM <<
                " Now is : " << camInfos.size() << endl;
                requestorPtr_->action_=CameraControl_Action_Invalid;
                return false;
            }
            for(int i = 0;i < camInfos.size(); i++)
            {
                if(camInfos[i].sn.size() >= MAX_SN_LEN)
                {
                    cout << Colormod::red << "[ERROR]" << Colormod::def << 
                    " CameraControlThread::OpenCamera getCamInfos sn too long!" << 
                    " Maximum is : " << MAX_SN_LEN <<
                    " Now is : " << camInfos[i].sn.size() << endl;
                    requestorPtr_->action_=CameraControl_Action_Invalid;
                    return false;
                }
                memcpy(requestorPtr_->gendata_.caminfo_func.camInfos[i].sn, camInfos[i].sn.c_str(),camInfos[i].sn.size());
                
                requestorPtr_->gendata_.caminfo_func.camInfos[i].width = camInfos[i].width;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].height = camInfos[i].height;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].fps = camInfos[i].fps;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].autoExposure = (int)camInfos[i].autoExposure;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].bayerPattern = (int)camInfos[i].bayerPattern;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].redGain = camInfos[i].redGain;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].greenGain = camInfos[i].greenGain;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].blueGain = camInfos[i].blueGain;
                requestorPtr_->gendata_.caminfo_func.camInfos[i].isWBRaw = camInfos[i].isWBRaw;

                requestorPtr_->gendata_.caminfo_func.camInfos[i].sn[camInfos[i].sn.size()] = 0;//make sure ends with '\0'
                cout << Colormod::magenta << "[SHADOWK]"<< Colormod::def 
                << "camInfos[i].sn.size() : " << camInfos[i].sn.size()
                << " Caminfo_sn "<< i << " : " 
                << requestorPtr_->gendata_.caminfo_func.camInfos[i].sn << endl;
            }

        }
        else if(gfun.compare("setFPS") == 0)
        {
            // cout << Colormod::magenta << "[SHADOWK]"<< Colormod::def 
            // << "Set fps :"<< requestorPtr_->gendata_.param_func.param_int[0] << " " <<  
            //     requestorPtr_->gendata_.param_func.param_float[0] << " " <<
            //     requestorPtr_->gendata_.param_func.param_float[1] << endl;

            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                int tmp_ret_all = 0;
                for(int i = 0;i < gencamera_s_.size(); i ++)
                {
                    int tmp_ret = gencamera_s_[i]->setFPS(
                    requestorPtr_->gendata_.param_func.param_int[0],
                    requestorPtr_->gendata_.param_func.param_float[0],
                    requestorPtr_->gendata_.param_func.param_float[1]
                    );
                    if(tmp_ret != 0)
                        tmp_ret_all = tmp_ret;
                }
                requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->setFPS(
                        sub_index,
                        requestorPtr_->gendata_.param_func.param_float[0],
                        requestorPtr_->gendata_.param_func.param_float[1]
                        );
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("setAutoWhiteBalance") == 0)
        {
            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                int tmp_ret_all = 0;
                for(int i = 0;i < gencamera_s_.size(); i ++)
                {
                    int tmp_ret = gencamera_s_[i]->setAutoWhiteBalance(
                    requestorPtr_->gendata_.param_func.param_int[0]
                    );
                    if(tmp_ret != 0)
                        tmp_ret_all = tmp_ret;
                }
                requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->setAutoWhiteBalance(
                        sub_index
                        );
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("setWhiteBalance") == 0)  
        {            
            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                int tmp_ret_all = 0;
                for(int i = 0;i < gencamera_s_.size(); i ++)
                {
                    int tmp_ret = gencamera_s_[i]->setWhiteBalance(
                    requestorPtr_->gendata_.param_func.param_int[0],
                    requestorPtr_->gendata_.param_func.param_float[0],
                    requestorPtr_->gendata_.param_func.param_float[1],
                    requestorPtr_->gendata_.param_func.param_float[2]
                    );
                    if(tmp_ret != 0)
                        tmp_ret_all = tmp_ret;
                }
                requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->setWhiteBalance(
                        sub_index,
                        requestorPtr_->gendata_.param_func.param_float[0],
                        requestorPtr_->gendata_.param_func.param_float[1],
                        requestorPtr_->gendata_.param_func.param_float[2]
                    );
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("setAutoExposure") == 0)
        {
            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                int tmp_ret_all = 0;
                for(int i = 0;i < gencamera_s_.size(); i ++)
                {
                    int tmp_ret = gencamera_s_[i]->setAutoExposure(
                    requestorPtr_->gendata_.param_func.param_int[0],
                    (cam::Status)requestorPtr_->gendata_.param_func.param_enum[0]
                    );
                    if(tmp_ret != 0)
                        tmp_ret_all = tmp_ret;
                }
                requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->setAutoExposure(
                        sub_index,
                        (cam::Status)requestorPtr_->gendata_.param_func.param_enum[0]
                        );
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("setAutoExposureLevel") == 0)
        {
            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                int tmp_ret_all = 0;
                for(int i = 0;i < gencamera_s_.size(); i ++)
                {
                    int tmp_ret = gencamera_s_[i]->setAutoExposureLevel(
                    requestorPtr_->gendata_.param_func.param_int[0],
                    requestorPtr_->gendata_.param_func.param_float[0]
                    );
                    if(tmp_ret != 0)
                        tmp_ret_all = tmp_ret;
                }
                requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->setAutoExposureLevel(
                        sub_index,
                        requestorPtr_->gendata_.param_func.param_float[0]
                        );
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("setAutoExposureCompensation") == 0)
        {
            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                int tmp_ret_all = 0;
                for(int i = 0;i < gencamera_s_.size(); i ++)
                {
                    int tmp_ret = gencamera_s_[i]->setAutoExposureCompensation(
                    requestorPtr_->gendata_.param_func.param_int[0],
                    (cam::Status)requestorPtr_->gendata_.param_func.param_enum[0],
                    requestorPtr_->gendata_.param_func.param_float[0]
                    );
                    if(tmp_ret != 0)
                        tmp_ret_all = tmp_ret;
                }
                requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->setAutoExposureCompensation(
                        sub_index,
                        (cam::Status)requestorPtr_->gendata_.param_func.param_enum[0],
                        requestorPtr_->gendata_.param_func.param_float[0]
                        );
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("setExposure") == 0)
        {
            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                int tmp_ret_all = 0;
                for(int i = 0;i < gencamera_s_.size(); i ++)
                {
                    int tmp_ret = gencamera_s_[i]->setExposure(
                    requestorPtr_->gendata_.param_func.param_int[0],
                    requestorPtr_->gendata_.param_func.param_int[1]
                    );
                    if(tmp_ret != 0)
                        tmp_ret_all = tmp_ret;
                }
                requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->setExposure(
                        sub_index,
                        requestorPtr_->gendata_.param_func.param_int[1]
                        );
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("getBayerPattern") == 0)
        {
            if(requestorPtr_->gendata_.param_func.param_int[0] == -1)
            {
                cout << Colormod::red << "[ERROR]" << Colormod::def <<
                "CameraControlThread::getBayerPattern CameraIndex can't be -1 !" << endl;
                requestorPtr_->gendata_.void_func.return_val = -1;
            }
            else
            {
                int driver_index, sub_index;
                if(get_camera_index(requestorPtr_->gendata_.param_func.param_int[0],
                driver_index, sub_index))
                {
                    cam::GenCamBayerPattern tmp;
                    requestorPtr_->gendata_.void_func.return_val = 
                    gencamera_s_[driver_index]->getBayerPattern(
                        sub_index,
                        tmp
                    );
                    requestorPtr_->gendata_.param_func.param_enum[0] = (int)tmp;
                }
                else
                {
                    requestorPtr_->gendata_.void_func.return_val = -1;
                }
            }
        }
        else if(gfun.compare("makeSetEffective") == 0)
        {

            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->makeSetEffective(
                requestorPtr_->gendata_.param_func.param_int[0]
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("setCaptureMode") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->setCaptureMode(
                (cam::GenCamCaptureMode)requestorPtr_->gendata_.param_func.param_enum[0],
                requestorPtr_->gendata_.param_func.param_int[0]
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("startCaptureThreads") == 0 && opened == false)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->startCaptureThreads();
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
            opened = true;
        }
        else if(gfun.compare("setVerbose") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->setVerbose(
                requestorPtr_->gendata_.param_func.param_bool[0]
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("setCamBufferType") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->setCamBufferType(
                    (cam::GenCamBufferType)requestorPtr_->gendata_.param_func.param_enum[0]
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("setJPEGQuality") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->setJPEGQuality(
                    requestorPtr_->gendata_.param_func.param_int[0],
                    requestorPtr_->gendata_.param_func.param_float[0]
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("setCapturePurpose") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->setCapturePurpose(
                    (cam::GenCamCapturePurpose)requestorPtr_->gendata_.param_func.param_enum[0]
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("saveImages") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->saveImages(
                    requestorPtr_->gendata_.str_func.str
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else if(gfun.compare("saveVideos") == 0)
        {
            int tmp_ret_all = 0;
            for(int i = 0;i < gencamera_s_.size(); i ++)
            {
                int tmp_ret = gencamera_s_[i]->saveVideos(
                    requestorPtr_->gendata_.str_func.str
                );
                if(tmp_ret != 0)
                    tmp_ret_all = tmp_ret;
            }
            requestorPtr_->gendata_.void_func.return_val = tmp_ret_all;
        }
        else
        {
            cout << Colormod::red << "[ERROR]" << Colormod::def << 
            " CameraControlThread::OpenCamera called a unknown function!" << endl;
            requestorPtr_->gendata_.void_func.return_val = -10;
        }

        if(requestorPtr_->gendata_.void_func.return_val == 0)
        {
            requestorPtr_->action_=CameraControl_Action_Valid;
            return true;
        }
        else
        {
            cout << Colormod::red << "[ERROR]" << Colormod::def << 
            " CameraControlThread::OpenCamera return not zero!" << 
            " Return val: " << requestorPtr_->gendata_.void_func.return_val << endl;
            requestorPtr_->action_=CameraControl_Action_Invalid;
            return false;
        }
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}

bool CameraControlThread::CloseCamera(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL && opened == true)
    {
        for(int i = 0;i < gencamera_s_.size(); i ++)
        {
            gencamera_s_[i]->stopCaptureThreads();
            gencamera_s_[i]->release();
        }
        opened = false;
        requestorPtr_->action_=CameraControl_Action_Valid;
        return true;
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}

bool CameraControlThread::GetImage(CameraControlMessage *requestorPtr_)
{
    if(requestorPtr_!=NULL)
    {
        int32_t pointer = 0;
        requestorPtr_->imageamount = 0;
        for(int j = 0;j < gencamera_s_.size(); j++)
        {
            std::vector<cam::GenCamImgRatio> ratios;
            for(int factor_idx = 0;factor_idx < camera_count_[j]; factor_idx++)
            {
                ratios[i] = (requestorPtr->resizeFactor_ >> (factor_idx * 4)) & (int)0x0f;
            }
            gencamera_s_[j]->setImageRatios(ratios);
            //jpeg len / ratio size / img data
            gencamera_s_[j]->captureFrame(imgdata_s[j]);
            requestorPtr_->imageamount += imgdata_s[j].size();
            
            for(int i = 0;i < imgdata_s[j].size(); i++)
            {
                cout << Colormod::magenta<<"[SHADOWK] imgdata_size"<<Colormod::def<<imgdata_s[j][i].length <<endl;
                memcpy(requestorPtr_->imageData_ + pointer, (uint8_t *)(&(imgdata_s[j][i].length)), sizeof(int));
                pointer += sizeof(int);
                memcpy(requestorPtr_->imageData_ + pointer, (uint8_t *)(&(imgdata_s[j][i].ratio)), sizeof(int));
                pointer += sizeof(int);
                memcpy(requestorPtr_->imageData_ + pointer, (uint8_t *)(imgdata_s[j][i].data), imgdata_s[j][i].length);
                pointer += imgdata_s[j][i].length;
            }
        }
        requestorPtr_->imagelen = pointer;
        requestorPtr_->action_=CameraControl_Action_Valid;
        return true;
    }
    requestorPtr_->action_=CameraControl_Action_Invalid;
    return false;
}

bool CameraControlThread::get_camera_index(int index_all, int &index_camera, int &index_sub)
{
    if(camera_count_.size() <= 0)
    {
        cout << Colormod::red << "[ERROR] " << Colormod::def <<
        "CameraControlThread::get_camera_index camera_count_.size() <= 0 !" << endl;
        return false;
    }
    if(index_all <= -1 || 
    index_all >= std::accumulate(camera_count_.begin(), camera_count_.end(), 0))
    {
        cout << Colormod::red << "[ERROR] " << Colormod::def <<
        "CameraControlThread::get_camera_index CameraIndex too large !" << endl;
        return false;
    }
    int acc = 0;
    for(int i = 0;i < camera_count_.size(); i ++)
    {
        if(camera_count_[i] == 0)
        {
            cout << Colormod::red << "[ERROR] " << Colormod::def <<
            "CameraControlThread::get_camera_index camera_count_[" << i << "] == 0 !" << endl;
            return false;
        }
        if(index_all >= camera_count_[i] + acc)
        {
            acc += camera_count_[i];
            continue;
        }
        index_camera = i;
        index_sub = index_all - acc;
        break;
    }
    return true;
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






