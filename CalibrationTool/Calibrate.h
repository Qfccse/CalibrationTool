#pragma once
#include "Const.h"

CalibrateResults calibrate(QStringList fileNames, int cameraType);

double calculateReprojectionError(const vector<Point3f>& objectPoints,
    const vector<Point2f>& imagePoints,
    const Mat& cameraMatrix,
    const Mat& distCoeffs,
    const Mat& rvec,
    const Mat& tvec,
    const int cameraType);