#ifndef __SMSSENDING_H__
#define __SMSSENDING_H__

extern bool SMSsending_init();//初始化
extern int send_SMS(char* data, int data_num);
extern int send_MMS();

#endif

