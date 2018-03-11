#include <stdio.h>
#include <stdlib.h>
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
    
    return 0;
}
