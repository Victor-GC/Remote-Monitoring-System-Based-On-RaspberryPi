#include "detect.h"

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
    //detectMultiScale函数中smallImg表示的是要检测的输入图像为smallImg，faces表示检测到的人脸目标序列，1.1表示每次图像尺寸减小的比例为1.1
    //4表示每一个目标至少要被检测到4次才算是真的目标(因为周围的像素和不同的窗口大小都可以检测到人脸)
    //#define CV_HAAR_DO_CANNY_PRUNING 1    //这个值告诉分类器跳过平滑（无边缘）区域
    //#define CV_HAAR_SCALE_IMAGE 2   //这个值告诉分类器不要缩放分类器，而是缩放图像
    //#define CV_HAAR_FIND_BIGGEST_OBJECT 4  //告诉分类器只返回最大的目标
    //#define CV_HAAR_DO_ROUGH_SEARCH 8  //它只能和上面一个参数一起使用，告诉分类器在任何窗口，只要第一个候选者被发现则结束搜寻
    //Size(35, 35)为目标的最小最大尺寸
    cascade.detectMultiScale( smallImg, faces,
                             1.1, 4, 0
                             //|CV_HAAR_FIND_BIGGEST_OBJECT
                             //|CV_HAAR_DO_ROUGH_SEARCH
                             |CV_HAAR_SCALE_IMAGE
                             |CV_HAAR_DO_CANNY_PRUNING
                             ,Size(35, 35));
    //如果使能，翻转图像继续检测
    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        //imshow("反转图像",smallImg);
        cascade.detectMultiScale( smallImg, faces2,
                                 1.1, 4, 0
                                 //|CV_HAAR_FIND_BIGGEST_OBJECT
                                 //|CV_HAAR_DO_ROUGH_SEARCH
                                 |CV_HAAR_SCALE_IMAGE
                                 |CV_HAAR_DO_CANNY_PRUNING
                                 ,Size(35, 35) );
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
