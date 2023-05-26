#include "Calibrate.h"
#include "CalibrationTool.h"

FullCalibrateResults calibrate(const QStringList& fileNames, const int cameraType) {
    // 1. 准备标定棋盘图像
    int boardWidth = 9;  // 棋盘格横向内角点数量
    int boardHeight = 6; // 棋盘格纵向内角点数量
    float squareSize = 0.12f; // 棋盘格格子的大小，单位为米,随便设置，不影响相机内参计算
    Size boardSize(boardWidth, boardHeight);

    vector<vector<Point3f>> objectPoints;
    vector<vector<Point2f>> imagePoints;
    vector<Point2f> corners;

    // 2. 拍摄棋盘图像
    Mat image, gray;
    // namedWindow("image", WINDOW_NORMAL);
    FullCalibrateResults res;

    // 3. 读入图像数据，并提取角点
    for (size_t i = 0; i < fileNames.size(); i++)
    {
        image = imread(fileNames[i].toStdString(), IMREAD_COLOR);
        print(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);

        bool found = findChessboardCorners(image, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
        if (found)
        {
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
            drawChessboardCorners(image, boardSize, corners, found);
            res.imageCorners.push_back(corners);
            // imshow("image", image);
            // waitKey();

            vector<Point3f> objectCorners;
            for (int j = 0; j < boardHeight; j++)
            {
                for (int k = 0; k < boardWidth; k++)
                {
                    objectCorners.push_back(Point3f(k * squareSize, j * squareSize, 0));
                }
            }
            objectPoints.push_back(objectCorners);
            imagePoints.push_back(corners);
        }
        else
        {
            std::vector<cv::Point2f> emptyVector;
            // 将 emptyVector 添加到 imageCorners
            res.imageCorners.push_back(emptyVector);
        }

        // cout << i << endl;
    }

    // 4. 标定相机
    Mat cameraMatrix, distCoeffs;
    vector<Mat> rvecs, tvecs;
    double totalReprojectionError;
    if (cameraType == NORMAL_CAM) 
    {
        totalReprojectionError = cv::calibrateCamera(objectPoints, imagePoints, gray.size(), cameraMatrix, distCoeffs, rvecs, tvecs);
    }
    else //FISH_EYE_TYPE
    {
        totalReprojectionError = cv::fisheye::calibrate(objectPoints, imagePoints, image.size(), cameraMatrix, distCoeffs, rvecs, tvecs);
    }
    res.cameraMatrix = cameraMatrix;
    res.distCoeffs = distCoeffs;
    res.rvecs = rvecs;
    res.tvecs = tvecs;
    // 保存相机内参矩阵和畸变系数到文本文件
    cv::FileStorage fs(INTRINSIC_SAVE_PATH, cv::FileStorage::WRITE);
    fs << "CameraMatrix" << cameraMatrix;
    fs << "DistortionCoefficients" << distCoeffs;
    fs << "R" << rvecs;
    fs << "t" << tvecs;
    // cout << "Camera matrix:" << endl << cameraMatrix << endl;
    // cout << "Distortion coefficients:" << endl << distCoeffs << endl;
    // 计算每张图像的重投影误差
    vector<double> errors;
    for (size_t i = 0; i < objectPoints.size(); i++) {
        double reprojectionError = calculateReprojectionError(objectPoints[i], imagePoints[i], cameraMatrix, distCoeffs, rvecs[i], tvecs[i],cameraType);
        errors.push_back(reprojectionError);
        // cout << "Reprojection error for image " << i << ": " << reprojectionError << endl;
    }
    res.reprojectionError = errors;
    fs << "errors" << errors;
    fs.release();
    objectPoints.clear();
    imagePoints.clear();

    return res;
}


vector<vector<cv::Point2f>> findCorners(const QStringList& fileNames,
    const Size boardSize, 
    CalibrationTool* ui) {
    // 1. 准备标定棋盘图像
    float squareSize = 0.12f; // 棋盘格格子的大小，单位为米,随便设置，不影响相机内参计算
    vector<Point2f> corners;

    // 2. 拍摄棋盘图像
    Mat image, gray;
    vector<vector<cv::Point2f>> res;

    // 3. 读入图像数据，并提取角点
    for (size_t i = 0; i < fileNames.size(); i++)
    {
        image = imread(fileNames[i].toStdString(), IMREAD_COLOR);
        print(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);

        bool found = findChessboardCorners(image, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
        if (found)
        {
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
            drawChessboardCorners(image, boardSize, corners, found);
            res.push_back(corners);
        }
        else
        {
            std::vector<cv::Point2f> emptyVector;
            // 将 emptyVector 添加到 imageCorners
            res.push_back(emptyVector);
        }
        // 更新进度
        int progress = (i + 1) * 100 / fileNames.size();
        emit ui->progressUpdate(progress); // 发送进度更新信号
    }

    return res;
}

vector<cv::Point2f> findOneCorners(const QString& fileName, const Size boardSize) {
    float squareSize = 0.12f; // 棋盘格格子的大小，单位为米,随便设置，不影响相机内参计算
    vector<Point2f> corners;
    Mat image, gray;
    image = imread(fileName.toStdString(), IMREAD_COLOR);
    print(image);
    cvtColor(image, gray, COLOR_BGR2GRAY);

    bool found = findChessboardCorners(image, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
    if (found)
    {
        cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
        drawChessboardCorners(image, boardSize, corners, found);
        return corners;
    }
    else
    {
        std::vector<cv::Point2f> emptyVector;
        return emptyVector;
    }
}

vector<cv::Point2f> findOneCorners(const Mat& imageFromCam, const Size boardSize) {
    float squareSize = 0.12f; // 棋盘格格子的大小，单位为米,随便设置，不影响相机内参计算
    vector<Point2f> corners;
    Mat image, gray;
    image = imageFromCam;
    print(image);
    cvtColor(image, gray, COLOR_BGR2GRAY);

    bool found = findChessboardCorners(image, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
    if (found)
    {
        cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
        drawChessboardCorners(image, boardSize, corners, found);
        return corners;
    }
    else
    {
        std::vector<cv::Point2f> emptyVector;
        return emptyVector;
    }
}


CalibrateResults calibarteWithCorners(const vector<vector<cv::Point2f>>& imageCorners,
    const Size imageSize, const Size boardSize, const int cameraType) {
    CalibrateResults res;
    vector<vector<Point3f>> objectPoints;
    vector<vector<Point2f>> imagePoints;

    for (size_t i = 0; i < imageCorners.size(); i++) {
        if (!imageCorners[i].empty()) {
            vector<Point3f> objectCorners;
            for (int j = 0; j < boardSize.height; j++)
            {
                for (int k = 0; k < boardSize.width; k++)
                {
                    objectCorners.push_back(Point3f(k * 0.12f, j * 0.12f, 0));
                }
            }
            objectPoints.push_back(objectCorners);
            imagePoints.push_back(imageCorners[i]);
        }
    }

    // 4. 标定相机
    Mat cameraMatrix, distCoeffs;
    vector<Mat> rvecs, tvecs;
    double totalReprojectionError;
    if (cameraType == NORMAL_CAM)
    {
        totalReprojectionError = cv::calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs);
    }
    else //FISH_EYE_TYPE
    {
        totalReprojectionError = cv::fisheye::calibrate(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs);
    }
    res.cameraMatrix = cameraMatrix;
    res.distCoeffs = distCoeffs;
    res.rvecs = rvecs;
    res.tvecs = tvecs;

    // 保存相机内参矩阵和畸变系数到文本文件
    cv::FileStorage fs(INTRINSIC_SAVE_PATH, cv::FileStorage::WRITE);
    fs << "CameraMatrix" << cameraMatrix;
    fs << "DistortionCoefficients" << distCoeffs;
    fs << "R" << rvecs;
    fs << "t" << tvecs;
    // cout << "Camera matrix:" << endl << cameraMatrix << endl;
    // cout << "Distortion coefficients:" << endl << distCoeffs << endl;
    // 计算每张图像的重投影误差
    vector<double> errors;
    for (size_t i = 0; i < objectPoints.size(); i++) {
        double reprojectionError = calculateReprojectionError(objectPoints[i], imagePoints[i], cameraMatrix, distCoeffs, rvecs[i], tvecs[i], cameraType);
        errors.push_back(reprojectionError);
        // cout << "Reprojection error for image " << i << ": " << reprojectionError << endl;
    }
    res.reprojectionError = errors;
    fs << "errors" << errors;
    fs.release();
    objectPoints.clear();
    imagePoints.clear();

    return res;
}

double calculateReprojectionError(const vector<Point3f>& objectPoints,
    const vector<Point2f>& imagePoints,
    const Mat& cameraMatrix,
    const Mat& distCoeffs,
    const Mat& rvec,
    const Mat& tvec,
    const int cameraType) {
    vector<Point2f> projectedPoints;
    if (cameraType == NORMAL_CAM)
    {
        cv::projectPoints(objectPoints, rvec, tvec, cameraMatrix, distCoeffs, projectedPoints);
    }
    else //FISH_EYE_TYPE
    {
        cv::fisheye::projectPoints(objectPoints, rvec, tvec, cameraMatrix, distCoeffs, projectedPoints);
    }

    double reprojectionError = 0.0;
    int numPoints = objectPoints.size();

    for (int i = 0; i < numPoints; i++) {
        double dx = projectedPoints[i].x - imagePoints[i].x;
        double dy = projectedPoints[i].y - imagePoints[i].y;
        reprojectionError += sqrt(dx * dx + dy * dy);
    }

    return reprojectionError / numPoints;
}


