#include <stdio.h>
#include <stdlib.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <vector>
#include "SMSsending.h"

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
    if(!SMSsending_init("/dev/ttyUSB0"))//报警模块初始化
    {
        printf("SerialPort open failed!\n");
        return 0;
    }
    wchar_t* phone_num = L"15765545478";
    wchar_t* send_data = L"测试一下，看行不行！";

    /*if(!send_SMS(phone_num, send_data))
    {
    printf("发送短信失败！\n");
}*/


    if(!MMS_init())
    {
    printf("彩信模块初始化失败！\n");
    return 0;
}

    Mat picture;
    picture = imread("111.jpg",IMREAD_UNCHANGED);
    vector<int> jpg_params;
    jpg_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    jpg_params.push_back(50);
    vector<uchar> image_vec;
    imencode(".jpg", picture, image_vec);

    int image_size = image_vec.size();
    char* image = (char*)malloc(image_size); 

    for(int i =0; i < image_size; ++i)
    {
        image[i] = image_vec[i];
    }

    FILE *fp = fopen("111.jpg","r");
    fseek(fp, 0, SEEK_END);//将文件位置指针置于文件结尾
    int image1_size = ftell(fp);
    char* image1 = (char*)malloc(image1_size); 
    fseek(fp, 0, SEEK_SET);

    int got_size = 0;
    while(!feof(fp)) 
    {
        image1[got_size] = fgetc(fp);
        ++got_size;
    }

    if(!send_MMS("15765545478", image, image_size))
    {
        printf("发送彩信失败！\n");
    }
free(image);

    Close_MMS();
    return 0;
}
