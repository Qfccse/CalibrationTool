#pragma once
#include "Const.h"
class CalibrationTool;  // 前向声明


// 已作废
FullCalibrateResults calibrate(const QStringList& fileNames, const int cameraType);

double calculateReprojectionError(const vector<cv::Point3f>& objectPoints,
    const vector<cv::Point2f>& imagePoints,
    const cv::Mat& cameraMatrix,
    const cv::Mat& distCoeffs,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    const int cameraType);

vector<vector<cv::Point2f>> findCorners(const QStringList& fileNames,
    const cv::Size boardSize,
    CalibrationTool* ui);

vector<cv::Point2f> findOneCorners(const QString& fileName, const cv::Size boardSize);

vector<cv::Point2f> findOneCorners(const cv::Mat& imageFromCam, const cv::Size boardSize);

CalibrateResults calibarteWithCorners(const vector<vector<cv::Point2f>>& imageCorners,
    const cv::Size imageSize, const cv::Size boardSize, const int cameraType);