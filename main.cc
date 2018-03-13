#include <stdio.h>
#include <stdlib.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include "SMSsending.h"

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
    if(!SMSsending_init("/dev/ttyUSB1"))//报警模块初始化
    {
        printf("SerialPort open failed!\n");
        return 0;
    }
    wchar_t* phone_num = L"15765545478";
    wchar_t* send_data = L"测试一下，看行不行！";

   /* if(!send_SMS(phone_num, send_data))
    {
        printf("发送短信失败！\n");
    }*/


    if(!MMS_init())
    {
        printf("彩信模块初始化失败！\n");
        return 0;
    }
    Mat picture;
    picture = imread("/home/mylove-chloe/桌面/Remote-Monitoring-System-Based-On-RaspberryPi/111.jpg");
    IplImage tempinput1 = IplImage(picture);
    IplImage *tempinput = &tempinput1;
    IplImage *input = cvCloneImage(tempinput);
    char *image = input->imageData;
    int image_size = 111;
    if(!send_MMS("15765545478", image, image_size))
    {
        printf("发送彩信失败！\n");
        return 0;
    }

    Close_MMS();
    return 0;
}
