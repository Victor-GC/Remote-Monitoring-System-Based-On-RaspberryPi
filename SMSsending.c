#include <SMSsending.h>

int fd;//串口符号句柄

bool SMSsending_init(char* port)//初始化与短信模块的通信
{
	fd = open(port, O_RDWR|O_NOCTTY|O_NDELAY);    
	if (FALSE == fd)    
	{    
		perror("Can't Open Serial Port");    
		return false;    
	}    
	//恢复串口为阻塞状态                                   
	if(fcntl(fd, F_SETFL, 0) < 0)    
	{    
		printf("fcntl failed!\n");    
		return false;    
	}         
	else    
	{    
		printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));    
	}    
	//测试是否为终端设备        
	if(0 == isatty(STDIN_FILENO))    
	{    
		printf("standard input is not a terminal device\n");    
		return false;    
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
		return false;     
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
	options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */      
	options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */    

	//如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读    
	tcflush(fd,TCIFLUSH);    

	//激活配置 (将修改后的termios数据设置到串口中）    
	if (tcsetattr(fd,TCSANOW,&options) != 0)      
	{    
		perror("com set error!\n");      
		return false;     
	}
	return true;
}

int send_core(char* data, int data_num, char* OKword)
{	
	write(fd, data, data_num);
	char* rdata = (char*)malloc(strlen(OKword));
	read(fd, rdata, strlen(OKword));
	if(!strcmp(Okword, rdata))
	{
		return -1;
	}
	free rdata;
	return 1;
}

int send_SMS(wchar_t* phone_num, wchar_t* send_data)
{
	char* data = "AT+CMGF=1";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Set text model failure!\n");
		return -1;
	}

	data = "AT+CSMP=17,167,2,25";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Set text model parameter failure!\n");
		return -1;
	}

	data = "AT+CSCS=\"UCS2\"";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Set UCS2 failure!\n");
		return -1;
	}

	unsigned int *p = (wchar_t *)phone_num;
	data = (char*)malloc(4 * wcslen(phone_num));
	int i;
	int j;
	for (i = 0, j = 0; i < wcslen(phone_num); i++)
	{
		j += sprintf(data + j, "%.4x", p[i]);
	}
	if(-1 == send_core(data, strlen(data), ">"))
	{
		printf("Set phone number failure!\n");
		return -1;
	}
	free(data);

	p = (wchar_t *)send_data;
	data = (char*)malloc(4 * wcslen(send_data);
	for (i = 0, j = 0; i < wcslen(send_data); i++)
	{
		j += sprintf(data + j, "%.4x", p[i]);
	}
	
	if(-1 == send_core(data, strlen(data), "+CMGS: "))
	{
		printf("Write send data failure!\n");
		return -1;
	}
	free(data);

	char end_index = 0X1A;
	if(-1 == send_core(end_index, 1, "OK"))
	{
		printf("Send SMS failure!\n");
		return -1;
	}
	return 1;
}

int MMS_init()
{
	char* data = "AT+CMMSINIT";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Init MMS model failure!\n");
		return -1;
	}

	data = "AT+CMMSCURL=\"mmsc.monternet.com\"";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("URL set failure!\n");
		return -1;
	}

	data = "AT+CMMSCID=1";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("ID failure!\n");
		return -1;
	}

	data = "AT+CMMSPROTO=\"10.0.0.172\",80";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("IP and port set failure!\n");
		return -1;
	}

	data = "AT+CMMSSENDCFG=6,3,0,0,2,4";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("PDU parameters set failure!\n");
		return -1;
	}

	data = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Contype and GPRS set failure!\n");
		return -1;
	}

	data = "AT+SAPBR=3,1,\"APN\",\"CMWAP\"";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("APN and CMWAP set failure!\n");
		return -1;
	}

	data = "AT+SAPBR=1,1";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Activate failure!\n");
		return -1;
	}

	data = "AT+SAPBR=2,1";
	if(-1 == send_core(data, strlen(data), "+SAPBR:1,1,\"10.3.126.164\""))
	{
		printf("State incorrect!\n");
		return -1;
	}

	return 1;
}

int send_MMS(char* phone_num)
{
	char* data = "AT+CMMSEDIT=1";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Enter edit model failure!\n");
		return -1;
	}

	int i;
	for(i = 0; i < 4; ++i)
	{
		data = "AT+CMMSDOWN=\"PIC\",6186,40000\n";
		if(1 == send_core(data, strlen(data), "CONNECT"))
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
	strcpy(phone_num_data,"AT+CMMSRECO=\"");
	strcat(phone_num_data,phone_num);
	strcpy(phone_num_data,"\"");
	if(-1 == send_core(phone_num_data, strlen(phone_num_data), "OK"))
	{
		printf("Set phone number failure!\n");
		return -1;
	}

	data = "AT+CMMSSEND";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Send MMS failure!\n");
		return -1;
	}

	data = "AT+CMMSEDIT=0";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Exit edit model failure!\n");
		return -1;
	}
	return 1;
}

int Close_MMS()
{
	char* data = "AT+SAPBR=0,1";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Exit SAPBR failure!\n");
		return -1;
	}

	data = "AT+CMMSTERM";
	if(-1 == send_core(data, strlen(data), "OK"))
	{
		printf("Exit MMS model failure!\n");
		return -1;
	}
	return 1;
}
