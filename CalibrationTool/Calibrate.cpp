#include "Calibrate.h"
void calibrate(QStringList fileNames) {
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

    for (size_t i = 0; i < fileNames.size(); i++)
    {
        image = imread(fileNames[i].toStdString(), IMREAD_COLOR);
        print(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);

        // 3. 读入图像数据，并提取角点
        bool found = findChessboardCorners(image, boardSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);
        if (found)
        {
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.1));
            drawChessboardCorners(image, boardSize, corners, found);
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

        cout << i << endl;
    }

    // 4. 标定相机
    Mat cameraMatrix, distCoeffs;
    vector<Mat> rvecs, tvecs;
    calibrateCamera(objectPoints, imagePoints, gray.size(), cameraMatrix, distCoeffs, rvecs, tvecs);
    // fisheye::calibrate(objectPoints, imagePoints, image.size(), cameraMatrix, distCoeffs, rvecs, tvecs);
    // 保存相机内参矩阵和畸变系数到文本文件
    cv::FileStorage fs("E:\\cv\\camera_parameters.txt", cv::FileStorage::WRITE);
    fs << "CameraMatrix" << cameraMatrix;
    fs << "DistortionCoefficients" << distCoeffs;
    fs.release();
    // cout << "Camera matrix:" << endl << cameraMatrix << endl;
    // cout << "Distortion coefficients:" << endl << distCoeffs << endl;
    objectPoints.clear();
    imagePoints.clear();
}