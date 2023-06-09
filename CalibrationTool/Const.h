#pragma once
#include <iostream>
#include <string>
#include <time.h> 
#include <vector>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <QString>
#include <QStringList>
#include <QtWidgets/QMainWindow>

using namespace std;
//using namespace cv;

#define IMAGE_LIST_WIDTH 150
#define IMAGE_WIN_HEIGHT 480
#define IMAGE_WIN_WIDTH 640
#define GRAPHIC_VIEW_WIDTH 256
#define GRAPHIC_VIEW_HEIGHT 240
#define SAMPLE_RATE 30
#define IMAGE_PADDING 9
#define CHART_EXPEND 55
#define CHART_FONT_SIZE 8

#define PI 3.14
#define MAX_RADIAN 180
#define TRANSLATION_BASE_SCALE 5
#define TRANSLATION_BASE_OFFSET 5
#define ROTATION_BASE_ANGLE 90
#define AXIS_ROTATION 50
#define Z_ROTATION 0.5


#define CHOSEN_ENTITY_COLOR QColor(192, 192, 192, 0.1)
#define DEFAULT_ENTITY_COLOR QColor(102, 255, 255, 0.1)
#define BASE_CUBOID_SCALE 480
#define CUBOID_SCALE_UNIT 4.0

#define NORMAL_CAM 1
#define FISH_EYE_CAM 2
#define BOARD_SIZE Size(9,6)
#define DEFAULT_INTRINSIC_SAVE_PATH_WIN "C:\\cv\\intrinsic.txt"
#define DEFAULT_INTRINSIC_SAVE_PATH_LINUX "/home/intrinsic.txt"


struct  FullCalibrateResults
{
    // �ǵ�ͼ
    vector<vector<cv::Point2f>> imageCorners;
    // ��ͶӰ���
    vector<double> reprojectionError;
    // �ڲ�
    cv::Mat cameraMatrix;
    // ����
    cv::Mat distCoeffs;
    // ���R
    vector<cv::Mat> rvecs;
    // ���t
    vector<cv::Mat> tvecs;
};

struct  CalibrateResults
{
    // ��ͶӰ���
    vector<double> reprojectionError;
    // �ڲ�
    cv::Mat cameraMatrix;
    // ����
    cv::Mat distCoeffs;
    // ���R
    vector<cv::Mat> rvecs;
    // ���t
    vector<cv::Mat> tvecs;
};