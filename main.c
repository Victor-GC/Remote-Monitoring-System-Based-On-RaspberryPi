#include <stdio.h>
#include <stdlib.h>
#include "SMSsending.h"

/*全局变量*/
int alarm_index = 1;//报警标志位,值为1则报警

int main(int argc, char *argv[])
{
    if(!SMSsending_init("/dev/ttyUSB0"))//报警模块初始化
    {
        printf("SerialPort open failed!\n");
        return 0;
    }


    while(1)
    {
        /*TODO 添加摄像头检测*/

        /*短信报警部分*/
        if(alarm_index)//需要报警
        {
            wchar_t* phone_num = L"15765545478";
            wchar_t* send_data = L"测试一下，看行不行！";

            if(!send_SMS(phone_num, send_data))
            {
                printf("发送短信失败！\n");
            }
            break;
        }
    }
    return 0;
}
