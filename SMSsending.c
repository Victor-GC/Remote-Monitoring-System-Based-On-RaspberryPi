#include <stdio.h>      /*标准输入输出定义*/    
#include <stdlib.h>     /*标准函数库定义*/
#include <fcntl.h>      /*文件控制定义*/    
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <string.h>
#include <unistd.h>
#include <sys/types.h>     
#include <sys/stat.h> 
#include <wchar.h>

int fd;//串口符号句柄
int send_core(char* data, int data_num, char* OKword);

int SMSsending_init(char* port)//初始化与短信模块的通信
{
    fd = open(port, O_RDWR|O_NOCTTY|O_NDELAY);    
    if (-1 == fd)    
    {    
        perror("Can't Open Serial Port");    
        return 0;    
    }    
    //恢复串口为阻塞状态                                   
    if(fcntl(fd, F_SETFL, 0) < 0)    
    {    
        printf("fcntl failed!\n");    
        return 0;    
    }         
    else    
    {    
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));    
    }    
    //测试是否为终端设备        
    if(0 == isatty(STDIN_FILENO))    
    {    
        printf("standard input is not a terminal device\n");    
        return 0;    
    }    
    else    
    {    
        printf("isatty success!\n");    
    }                  
    printf("fd->open=%d\n",fd);


    struct termios options;    

    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.*/    
    if( tcgetattr( fd,&options)  !=  0)    
    {    
        perror("SetupSerial 1");        
        return 0;     
    }    

    //设置串口输入波特率和输出波特率    
    cfsetispeed(&options, 115200);     
    cfsetospeed(&options, 115200);      

    //修改控制模式，保证程序不会占用串口    
    options.c_cflag |= CLOCAL;    
    //修改控制模式，使得能够从串口中读取输入数据    
    options.c_cflag |= CREAD;    

    //设置数据流控制    
    //不使用流控制    
    options.c_cflag &= ~CRTSCTS;    

    //设置数据位    
    //屏蔽其他标志位    
    options.c_cflag &= ~CSIZE;    
    options.c_cflag |= CS8;    

    //设置校验位    
    //无奇偶校验位。    
    options.c_cflag &= ~PARENB;     
    options.c_iflag &= ~INPCK;        

    // 设置停止位     
    options.c_cflag &= ~CSTOPB;      

    //修改输出模式，原始数据输出    
    options.c_oflag &= ~OPOST;    

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    
    //options.c_lflag &= ~(ISIG | ICANON);    

    //设置等待时间和最小接收字符    
    options.c_cc[VTIME] = 0; /* 没有数据直接阻塞 */      
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */    

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读    
    tcflush(fd,TCIFLUSH);    

    //激活配置 (将修改后的termios数据设置到串口中）    
    if (tcsetattr(fd,TCSANOW,&options) != 0)      
    {    
        perror("com set error!\n");      
        return 0;     
    }

    char* data = "ATE0\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Close echo disp failure!\n");
        return 0;
    }

    return 1;
}

int send_core(char* data, int data_num, char* OKword)
{	
    tcflush(fd, TCIFLUSH);//清除接收缓存区
    int writed = write(fd, data, data_num);
    char* rdata = (char*)malloc(strlen(OKword));
    read(fd, rdata, strlen(OKword));
    if(0 != strcmp(OKword, rdata))
    {
        free(rdata);
        return -1;
    }
    free(rdata);
    return writed;
}

int send_SMS(wchar_t* phone_num, wchar_t* send_data)
{
    char* data = "AT+CMGF=1\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Set text model failure!\n");
        return 0;
    }

    data = " AT+CSMP=17,167,1,8\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Set text model parameter failure!\n");
        return 0;
    }

    data = "AT+CSCS=\"UCS2\"\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Set UCS2 failure!\n");
        return 0;
    }

    unsigned int *p = (wchar_t *)phone_num;
    data = (char*)malloc(4 * wcslen(phone_num) + 12);
    strcpy(data, "AT+CMGS=\"");//初始化设置号码的AT指令的前半段
    int i;
    int j;
    for (i = 0, j = 9; i < wcslen(phone_num); i++)
    {
        j += sprintf(data + j, "%.4x", p[i]);
    }
    strcpy(data + j, "\"\r\n");//初始化设置号码的AT指令的结尾

    if(-1 == send_core(data, strlen(data), "\r\n>"))
    {
        printf("Set phone number failure!\n");
        free(data);
        return 0;
    }
    free(data);

    p = (wchar_t *)send_data;
    data = (char*)malloc(4 * wcslen(send_data) + 1);
    for (i = 0, j = 0; i < wcslen(send_data); i++)
    {
        j += sprintf(data + j, "%.4x", p[i]);
    }
    *(data + j) = 0x1A;//写入结束符0x1A

    if(-1 == send_core(data, strlen(data), "\r\n+CMGS: "))
    {
        printf("Write send data failure!\n");
        free(data);
        return 0;
    }
free(data);

    return 1;
}

int MMS_init()
{
    char* data = "AT+CMMSINIT\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Init MMS model failure!\n");
        return 0;
    }

    data = "AT+CMMSCURL=\"mmsc.monternet.com\"\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("URL set failure!\n");
        return 0;
    }

    data = "AT+CMMSCID=1\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("ID failure!\n");
        return 0;
    }

    data = "AT+CMMSPROTO=\"10.0.0.172\",80\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("IP and port set failure!\n");
        return 0;
    }

    data = "AT+CMMSSENDCFG=6,3,0,0,2,4\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("PDU parameters set failure!\n");
        return 0;
    }

    data = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Contype and GPRS set failure!\n");
        return 0;
    }

    data = "AT+SAPBR=3,1,\"APN\",\"CMWAP\"\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("APN and CMWAP set failure!\n");
        return 0;
    }

    data = "AT+SAPBR=1,1\r\n";
if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
{
    printf("Activate failure!\n");
    return 0;
}

    data = "AT+SAPBR=2,1\r\n";
    if(-1 == send_core(data, strlen(data), "\r\n+SAPBR:1,1,\"10.3.126.164\"\r\n"))
    {
        printf("State incorrect!\n");
        return 0;
    }

    return 1;
}

int send_MMS(char* phone_num)
{
    char* data = "AT+CMMSEDIT=1\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Enter edit model failure!\n");
        return 0;
    }

    int i;
    for(i = 0; i < 4; ++i)
    {
        data = "AT+CMMSDOWN=\"PIC\",6186,40000\r\n";
        if(1 == send_core(data, strlen(data), "\r\nCONNECT\r\n"))
        {
            break;
        }
        printf("Connect failure,try again\n");
    }
    if(i == 4)
    {
        printf("Connect failure and try too many times, please check the software and hardware!\n");
    }

    /*TODO:发送图片*/


    char phone_num_data[80];
    strcpy(phone_num_data,"AT+CMMSRECP=\"");
    strcat(phone_num_data,phone_num);
    strcpy(phone_num_data,"\"\r\n");
    if(-1 == send_core(phone_num_data, strlen(phone_num_data), "\r\nOK\r\n"))
    {
        printf("Set phone number failure!\n");
        return 0;
    }

    data = "AT+CMMSSEND\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Send MMS failure!\n");
        return 0;
    }

    data = "AT+CMMSEDIT=0\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Exit edit model failure!\n");
        return 0;
    }
    return 1;
}

int Close_MMS()
{
    char* data = "AT+SAPBR=0,1\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Exit SAPBR failure!\n");
        return 0;
    }

    data = "AT+CMMSTERM\r\n";
    if(-1 == send_core(data, strlen(data), "\r\nOK\r\n"))
    {
        printf("Exit MMS model failure!\n");
        return 0;
    }
    return 1;
}
