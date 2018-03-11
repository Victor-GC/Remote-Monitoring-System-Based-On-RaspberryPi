#ifndef __SMSSENDING_H__
#define __SMSSENDING_H__

int SMSsending_init(char* port);//初始化
int send_SMS(wchar_t* phone_num, wchar_t* send_data);//发送短信
int MMS_init();//彩信初始化
int send_MMS(char* phone_num, char* image, int image_size);//发送彩信
int Close_MMS();//关闭彩信

#endif

