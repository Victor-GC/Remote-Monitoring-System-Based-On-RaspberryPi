#ifndef __SMSSENDING_H__
#define __SMSSENDING_H__

#ifdef __cplusplus
extern "C"{
#endif

    int SMSsending_init(const char* port);//初始化
    int send_SMS(const wchar_t* phone_num,const  wchar_t* send_data);//发送短信
    int MMS_init();//彩信初始化
    int send_MMS(const char* phone_num, const char* image, const int image_size);//发送彩信
    int Close_MMS();//关闭彩信

#ifdef __cplusplus
}
#endif

#endif

