#include "CameraUtil.h"
#include "CameraArray.h"

#include "common.h"
#include "cameraControl.h"
#include "communication.h"

#include "NPPJpegCoder.h"
#include "NPPJpegCoderKernel.h"

#include <time.h>

#define MEASURE_KERNEL_TIME

int main(int argc, char* argv[])
{
    CameraArray array;
    // array.init();
    // array.setWhiteBalance(1.10f, 1.65f);
    // //array.allocateBuffer(20);
    // array.allocateBufferJPEG(400);
    // //array.startRecord(12);
    // array.startRecordJPEG(40);
    // //array.saveCapture("E:\\Project\\CameraUtil\\data");
    // array.saveCaptureJPEGCompressed("./data/");
    // array.release();

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
    CameraControlThread cameraControlThread(&array, cameraControlMessageDeque);
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

/*
npp::NPPJpegCoder decoder;
decoder.init(4000, 3000, 75);
cv::cuda::GpuMat img(3000, 4000, CV_8UC3);
cv::Mat img_h;
decoder.decode(reinterpret_cast<unsigned char*>(jpegdatas[3]), dataLengths[3], img);
img.download(img_h);
cv::imwrite("decoding.jpg", img_h);
*/
