#pragma once
#include <iostream>
#include <string>
#include <time.h> 
#include <vector>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <QString>
#include <QStringList>
#include <QtWidgets/QMainWindow>

using namespace std;
//using namespace cv;

#define IMAGE_LIST_WIDTH 150
#define IMAGE_WIN_HEIGHT 480
#define IMAGE_WIN_WIDTH 640
#define SAMPLE_RATE 30
#define IMAGE_PADDING 9


#define NORMAL_CAM 1
#define FISH_EYE_CAM 2
#define BOARD_SIZE Size(9,6)
#define INTRINSIC_SAVE_PATH "E:\\cv\\camera_parameters2.txt"


struct  FullCalibrateResults
{
    // 角点图
    vector<vector<cv::Point2f>> imageCorners;
    // 重投影误差
    vector<double> reprojectionError;
    // 内参
    cv::Mat cameraMatrix;
    // 畸变
    cv::Mat distCoeffs;
    // 外参R
    vector<cv::Mat> rvecs;
    // 外参t
    vector<cv::Mat> tvecs;
};

struct  CalibrateResults
{
    // 重投影误差
    vector<double> reprojectionError;
    // 内参
    cv::Mat cameraMatrix;
    // 畸变
    cv::Mat distCoeffs;
    // 外参R
    vector<cv::Mat> rvecs;
    // 外参t
    vector<cv::Mat> tvecs;
};