#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>
using namespace cv;
using namespace std;

int detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale, bool tryflip );

int main()
{	
    /*变量参数准备*/
	Mat frame; //用于保存摄像头采集得到的图片
	int number_of_face = 0;  //识别出的人脸数量
    CascadeClassifier cascade; //分类器

    //加载已训练好的分类模型，注意路径修改
    cascade.load("/home/pi/opencv-3.4.0/data/haarcascades/haarcascade_frontalface_alt.xml");
    
	//打开树莓派摄像头
	VideoCapture cap(0);    
    if (!cap.isOpened())
    {
        return -1;
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
			imwrite("test.jpg", frame); //保存当前摄像头捕捉到的图片至当前文件夹下
		
		//进行人脸检测
		number_of_face = detectAndDraw( frame, cascade, 2, 0 );
		
		if (number_of_face >= 1)
		{
			cout << "Attention!" << endl;
			/*触发报警部分功能*/
			
		}
		else
		{
			cout << "Don't worry" << endl;
		}
		
		//删除保存的图片
		int result_delete = remove("./test.jpg");
		if (!result_delete)
			cout << "delete succeeded" << endl;
		else
			cout << "delete failed" << endl;
		
		//27是键盘按下esc时，计算机收到的ascii码值，waitKey(30)表示等待30ms
		//在实际运行过程中，控制台是无法接受到waitkey的键值的，只有在显示的图像上才能收到键值
		/*if (waitKey(30)==27)
			break;
		*/
		
		//延时1000ms，用于控制监测实时性
		waitKey(1000);
	}
	
    return 0;
}

int detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale, bool tryflip )
{
    int i = 0;
    double t = 0;
    //建立用于存放人脸的向量容器
    vector<Rect> faces, faces2;
    //定义一些颜色，用来标示不同的人脸
    const static Scalar colors[] =  {
        CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;
    //建立缩小的图片，加快检测速度
    //nt cvRound (double value) 对一个double型的数进行四舍五入，并返回一个整型数！
    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
    //转成灰度图像，Harr特征基于灰度图
    cvtColor( img, gray, CV_BGR2GRAY );
    //imshow("灰度",gray);
    //改变图像大小，使用双线性差值
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    //imshow("缩小尺寸",smallImg);
    //变换后的图像进行直方图均值化处理
    equalizeHist( smallImg, smallImg );
    //imshow("直方图均值处理",smallImg);
    //程序开始和结束插入此函数获取时间，经过计算求得算法执行时间
    t = (double)cvGetTickCount();
    //检测人脸
    //detectMultiScale函数中smallImg表示的是要检测的输入图像为smallImg，faces表示检测到的人脸目标序列，1.1表示
    //每次图像尺寸减小的比例为1.1，2表示每一个目标至少要被检测到3次才算是真的目标(因为周围的像素和不同的窗口大
    //小都可以检测到人脸),CV_HAAR_SCALE_IMAGE表示不是缩放分类器来检测，而是缩放图像，Size(30, 30)为目标的
    //最小最大尺寸
    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        |CV_HAAR_SCALE_IMAGE
        ,Size(30, 30));
    //如果使能，翻转图像继续检测
    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        //imshow("反转图像",smallImg);
        cascade.detectMultiScale( smallImg, faces2,
            1.1, 2, 0
            //|CV_HAAR_FIND_BIGGEST_OBJECT
            //|CV_HAAR_DO_ROUGH_SEARCH
            |CV_HAAR_SCALE_IMAGE
            ,Size(30, 30) );
        for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++ )
        {
            faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    t = (double)cvGetTickCount() - t;
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    
	//用于在图片上标记出人脸位置
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r->width/r->height;
        //cout<<"ratio=" << aspect_ratio << endl;
		if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            //标示人脸时在缩小之前的图像上标示，所以这里根据缩放比例换算回去
            center.x = cvRound((r->x + r->width*0.5)*scale);
            center.y = cvRound((r->y + r->height*0.5)*scale);
            radius = cvRound((r->width + r->height)*0.25*scale);
            //cout <<"print cirle"<<endl;
	    circle( img, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( img, cvPoint(cvRound(r->x*scale), cvRound(r->y*scale)),
            cvPoint(cvRound((r->x + r->width-1)*scale), cvRound((r->y + r->height-1)*scale)),
            color, 3, 8, 0);
    }
    //imshow( "识别结果", img );
	//调试使用，输出检测到的人脸数量
    cout << "The number of detected faces is " << faces.size()<<endl;
	return faces.size();
}
