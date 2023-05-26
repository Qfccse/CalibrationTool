#pragma once
#include "Const.h"
class CalibrationTool;  // 前向声明


// 已作废
FullCalibrateResults calibrate(const QStringList& fileNames, const int cameraType);

double calculateReprojectionError(const vector<Point3f>& objectPoints,
    const vector<Point2f>& imagePoints,
    const Mat& cameraMatrix,
    const Mat& distCoeffs,
    const Mat& rvec,
    const Mat& tvec,
    const int cameraType);

vector<vector<cv::Point2f>> findCorners(const QStringList& fileNames, 
    const Size boardSize,
    CalibrationTool* ui);

vector<cv::Point2f> findOneCorners(const QString& fileName, const Size boardSize);

vector<cv::Point2f> findOneCorners(const Mat& imageFromCam, const Size boardSize);

CalibrateResults calibarteWithCorners(const vector<vector<cv::Point2f>>& imageCorners, 
    const Size imageSize, const Size boardSize,const int cameraType);