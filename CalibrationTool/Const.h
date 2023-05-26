#pragma once
#include <iostream>
#include <string>
#include <time.h> 
#include <vector>
#include <opencv2/opencv.hpp>
#include <QString>
#include <QStringList>

using namespace std;
using namespace cv;

#define IMAGE_LIST_WIDTH 150
#define IMAGE_WIN_HEIGHT 480
#define IMAGE_WIN_WIDTH 640
#define SAMPLE_RATE 30
#define IMAGE_PADDING 9


#define NORMAL_CAM 1
#define FISH_EYE_CAM 2
#define INTRINSIC_SAVE_PATH "E:\\cv\\camera_parameters.txt"


struct  FullCalibrateResults
{
    // 角点图
    vector<vector<cv::Point2f>> imageCorners;
    // 重投影误差
    vector<double> reprojectionError;
    // 内参
    Mat cameraMatrix;
    // 畸变
    Mat distCoeffs;
    // 外参R
    vector<Mat> rvecs;
    // 外参t
    vector<Mat> tvecs;
};

struct  CalibrateResults
{
    // 重投影误差
    vector<double> reprojectionError;
    // 内参
    Mat cameraMatrix;
    // 畸变
    Mat distCoeffs;
    // 外参R
    vector<Mat> rvecs;
    // 外参t
    vector<Mat> tvecs;
};