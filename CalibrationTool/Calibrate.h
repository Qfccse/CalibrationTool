#pragma once
#include "Const.h"

FullCalibrateResults calibrate(const QStringList& fileNames, const int cameraType);

double calculateReprojectionError(const vector<Point3f>& objectPoints,
    const vector<Point2f>& imagePoints,
    const Mat& cameraMatrix,
    const Mat& distCoeffs,
    const Mat& rvec,
    const Mat& tvec,
    const int cameraType);

vector<vector<cv::Point2f>> findCorners(const QStringList& fileNames, const Size boardSize);

CalibrateResults calibarteWithCorners(const vector<vector<cv::Point2f>>& imageCorners, 
    const Size imageSize, const Size boardSize,const int cameraType);