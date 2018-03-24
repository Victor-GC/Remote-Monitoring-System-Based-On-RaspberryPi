#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "SMSsending.h"//短信彩信报警头文件
#include "http_work.h" //网页访问头文件
#include "detect.h" //人脸识别检测头文件

using namespace cv;
using namespace std;
char LOGBUF[1024];//用于记录程序错误信息并保存到本地日志
#define PHONE_NUM L"15765545478" //监控者的电话号码，报警使用

#define neighbor 4 //在一张图像中需要在连续区域检测到的次数
#define alarm_times 5 //触发报警所需要连续检测到目标的次数

int main()
{	
	/*变量参数准备*/
	Mat frame; //用于保存摄像头采集得到的图片
	int number_of_face = 0;  //识别出的人脸数量
	int alarm_flag = 0;  //设置报警标志位
	int number_of_alarm = 0; //连续检测出人脸的次数
	CascadeClassifier cascade; //分类器

	/*http服务器设置所需参数*/
	int port = 8090;
	int server_sock=-1;
	int ret;
	pthread_t newthread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //初始化线程为可分离的
	server_sock=socket_create(port);//建立socket，监听端口
	if(server_sock==-1)
	{
		printf ("Create http_socket error!\n");
		exit (1);
	}
	else
	{
		int *tmp = (int *) malloc(sizeof(int));
		*tmp = server_sock;
		ret=pthread_create(&newthread,&attr,socket_accept,tmp);//创建http侦听和服务线程
		if(ret!=0){
			printf ("Create http_pthread error!\n");
			exit (1);
		}
	}

	//加载已训练好的分类模型，注意路径修改
	//使用haarcascade_frontalface_alt或haarcascade_frontalface_alt2分类器 较为严格，只有在正脸的情况下才会检测到
	//cascade.load("/home/pi/opencv-3.4.0/data/haarcascades/h
	//aarcascade_frontalface_alt.xml");
	//使用haarcascade_frontalface_default分类器条件放松
	cascade.load("/home/pi/opencv/data/haarcascades/haarcascade_frontalface_default.xml");
	//打开树莓派摄像头
	VideoCapture cap(0);    
	if (!cap.isOpened())
	{
		cout << "camera fail!" << endl;
		return -1;
	}

	/*彩信短信报警模块初始化*/
	if(!SMSsending_init("/dev/ttyUSB0"))
	{
		printf("短信模块串口通信打开失败！\n");
		return 0;        
	}
	else
	{
		printf("短信模块串口通信打开成功！\n");
	}

	if(!MMS_init())
	{
		printf("彩信模块初始化失败！\n");
		return 0;
	}
	else
	{
		printf("彩信模块初始化失败！\n");
	}

	while (1)
	{
		//从本地读取摄像头图片 调试使用
		//frame = imread("/home/pi/opencv-3.4.0/demo/face_only/lena.jpg");
		//读取摄像头的当前帧
		cap >> frame;

		if (frame.empty())
			cout << "Fail to capture frame!" << endl;
		else
			imwrite("./images/real_image.jpg", frame); //保存当前摄像头捕捉到的图片至当前文件夹下

		//进行人脸检测
		number_of_face = detectAndDraw( frame, cascade, 2, 0 );

		if (number_of_face >= 1)
			number_of_alarm++;
		else
			number_of_alarm = 0;
		if (number_of_alarm == alarm_times)
		{
			alarm_flag = 1;
			number_of_alarm = 0;			
		}
		if (alarm_flag)
		{
			cout << "Attention!" << endl;

			/*触发报警部分功能*/

			/*短信报警部分*/
			const wchar_t* phone_num = PHONE_NUM;
			const wchar_t* send_data = L"检测到有人闯入，请注意监控区域安全！可远程查看摄像头进一步确认！";
			if(!send_SMS(phone_num, send_data))
			{
				printf("发送短信失败！\n");
			}
			
			/*彩信报警部分*/
			vector<int> jpg_params;//jpg图片质量参数
			jpg_params.push_back(CV_IMWRITE_JPEG_QUALITY);
			jpg_params.push_back(50);
			vector<uchar> image_vec;
			imencode(".jpg", frame, image_vec);//将mat转成内存中的jpg

			int image_size = image_vec.size();
			char* image = (char*)malloc(image_size); 

			for(int i =0; i < image_size; ++i)
			{
				image[i] = image_vec[i];
			}

			char phone_num_mms[15];
			memset(phone_num_mms, 0, 15);
			wcstombs(phone_num_mms, phone_num, 11);
			if(!send_MMS(phone_num_mms, image, image_size))
			{
				printf("发送彩信失败！\n");
			}
			free(image);

			/*报警结束后延迟1秒*/
			sleep(1);
			alarm_flag = 0;
		}
		else
		{
			cout << "Don't worry" << endl;
		}

		//删除保存的图片 :不用删除，图片来的时候自动写入同一个.jpg文件即可
		/*
		   int result_delete = remove("./images/real_image.jpg");
		   if (!result_delete)
		   cout << "delete succeeded" << endl;
		   else
		   cout << "delete failed" << endl;
		 */

		//27是键盘按下esc时，计算机收到的ascii码值，waitKey(30)表示等待30ms
		//在实际运行过程中，控制台是无法接受到waitkey的键值的，只有在显示的图像上才能收到键值
		/*if (waitKey(30)==27)
		  break;
		 */
		//延时1000ms，用于控制监测实时性
		// waitKey(1000);
	}
	Close_MMS();
	pthread_attr_destroy(&attr);//释放资源
	return 0;
}
