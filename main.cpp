

#include "common.h"
#include "cameraControl.h"
#include "communication.h"

#include "colormod.h"


#include "camera_driver/GenCameraDriver.h"

#include <time.h>
#include <algorithm> //transform

//#define MEASURE_KERNEL_TIME

int main(int argc, char* argv[])
{
    cout << endl;
    std::vector<std::shared_ptr<cam::GenCamera>> camPtrVec;
    if(argc <= 1)
    {
        cout << Colormod::red << "[ERROR]" << Colormod::def <<
        "param can't be none, example\n" <<
        "./NetVirtualCameraServer PointGrey_u3 XIMEA_xiC\n" <<
        "fomat:\n" <<
        "./NetVirtualCameraServer [CameraType1] [CameraType2] ..." << endl;
        return 0;
    }
    else
    {
        for(int i = 1; i < argc; i++)
        {
            std::string tmp_str(argv[i]);
            transform(tmp_str.begin(), tmp_str.end(), tmp_str.begin(), ::tolower);
            if(tmp_str.compare("pointgrey_u3") == 0 || tmp_str.compare("p") == 0)
            {
                std::shared_ptr<cam::GenCamera> cameraPtr = cam::createCamera(cam::CameraModel::PointGrey_u3);
                camPtrVec.push_back(cameraPtr);
                cout <<
                Colormod::green << 
                "Added PointGrey_u3 camera type to the list.\n" << 
                Colormod::def << endl;
            }
            else if(tmp_str.compare("ximea_xic") == 0 || tmp_str.compare("x") == 0)
            {
                std::shared_ptr<cam::GenCamera> cameraPtr = cam::createCamera(cam::CameraModel::XIMEA_xiC);
                camPtrVec.push_back(cameraPtr);
                cout <<
                Colormod::green << 
                "Added XIMEA_xiC camera type to the list.\n" << 
                Colormod::def << endl;
            }
            else
            {
                cout << Colormod::red << "[ERROR]" << Colormod::def <<
                "Met unsupported camera type, now only support:\n" <<
                Colormod::cyan << "PointGrey_u3 XIMEA_xiC\n" << Colormod::def <<
                "unsupported string is:\n" <<
                Colormod::red << tmp_str << Colormod::def << endl;
                return 0;
            }
        }
    }
    //std::shared_ptr<cam::GenCamera> cameraPtr = cam::createCamera(cam::CameraModel::PointGrey_u3);
    //std::shared_ptr<cam::GenCamera> cameraPtr = cam::createCamera(cam::CameraModel::XIMEA_xiC);
    
    //camPtrVec.push_back(cameraPtr);


    cout<< "[ACTION] Camera driver start!" << endl;
    syslog(LOG_INFO, "[ACTION] Camera driver start!\n");

    ///相机控制线程参数和消息队列
    //CameraServerUnit   *cameraServerUnit=new CameraServerUnit();
    CameraControlMessageDeque *cameraControlMessageDeque=new CameraControlMessageDeque();

    ///通信线程消息队列
    CommunicationMessageDeque *communicationMessageDeque=new CommunicationMessageDeque();

    ///键盘控制线程消息队列
    //KeyboardMessageDeque  *keyboardMessageDeque=new KeyboardMessageDeque();

    ///注册并启动各线程
    CameraControlThread cameraControlThread(camPtrVec, cameraControlMessageDeque);
    CommunicationThread communicationThread(communicationMessageDeque,cameraControlMessageDeque);
    //KeyboardControlThread keyboardControlThread(keyboardMessageDeque,cameraControlMessageDeque,communicationMessageDeque);

    ///等待各线程退出
    cameraControlThread.WaitForInternalThreadToExit();
    communicationThread.WaitForInternalThreadToExit();
    //keyboardControlThread.WaitForInternalThreadToExit();

    SleepMs(500);
    //delete cameraServerUnit;
    delete cameraControlMessageDeque;
    delete communicationMessageDeque;
    //delete keyboardMessageDeque;

    cout<< "[ACTION] Camera driver end!" << endl;
    syslog(LOG_INFO, "[ACTION] Camera driver end!\n");
    closelog();

    return 0;
}
