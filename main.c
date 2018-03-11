#include <stdio.h>
#include <stdlib.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "SMSsending.h"


int main(int argc, char *argv[])
{
    if(!SMSsending_init("/dev/ttyUSB0"))//报警模块初始化
    {
        printf("SerialPort open failed!\n");
        return 0;
    }

    wchar_t* phone_num = L"15765545478";
    wchar_t* send_data = L"测试一下，看行不行！";

    if(!send_SMS(phone_num, send_data))
    {
        printf("发送短信失败！\n");
    }


    MMS_init();
    Mat picture;
    IplImage *input = cvCloneImage(picture);
    char *image = input->imageData;
    if(!send_MMS(phone_num, image, image_size))
    {
        printf("发送彩信失败！\n");
    }

    Close_MMS();

    return 0;
}
