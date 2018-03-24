#ifndef __DETECT_H__
#define __DETECT_H__

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

#ifdef __cplusplus
extern "C"{
#endif

    int detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale, bool tryflip );//识别检测

#ifdef __cplusplus
}
#endif

#endif

