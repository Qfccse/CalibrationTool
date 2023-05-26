#pragma once
#include <iostream>
#include <string>
#include <time.h>
#include <ctime> 
#include <vector>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <QString>
#include <QStringList>
#include <QtWidgets/QMainWindow>

using namespace std;
using namespace cv;

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
    // �ǵ�ͼ
    vector<vector<cv::Point2f>> imageCorners;
    // ��ͶӰ���
    vector<double> reprojectionError;
    // �ڲ�
    Mat cameraMatrix;
    // ����
    Mat distCoeffs;
    // ���R
    vector<Mat> rvecs;
    // ���t
    vector<Mat> tvecs;
};

struct  CalibrateResults
{
    // ��ͶӰ���
    vector<double> reprojectionError;
    // �ڲ�
    Mat cameraMatrix;
    // ����
    Mat distCoeffs;
    // ���R
    vector<Mat> rvecs;
    // ���t
    vector<Mat> tvecs;
};