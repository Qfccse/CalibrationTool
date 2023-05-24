#include "Calibrate.h"
void calibrate(QStringList fileNames) {
    // 1. ׼���궨����ͼ��
    int boardWidth = 9;  // ���̸�����ڽǵ�����
    int boardHeight = 6; // ���̸������ڽǵ�����
    float squareSize = 0.12f; // ���̸���ӵĴ�С����λΪ��,������ã���Ӱ������ڲμ���
    Size boardSize(boardWidth, boardHeight);

    vector<vector<Point3f>> objectPoints;
    vector<vector<Point2f>> imagePoints;
    vector<Point2f> corners;

    // 2. ��������ͼ��
    Mat image, gray;
    // namedWindow("image", WINDOW_NORMAL);

    for (size_t i = 0; i < fileNames.size(); i++)
    {
        image = imread(fileNames[i].toStdString(), IMREAD_COLOR);
        print(image);
        cvtColor(image, gray, COLOR_BGR2GRAY);

        // 3. ����ͼ�����ݣ�����ȡ�ǵ�
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

    // 4. �궨���
    Mat cameraMatrix, distCoeffs;
    vector<Mat> rvecs, tvecs;
    calibrateCamera(objectPoints, imagePoints, gray.size(), cameraMatrix, distCoeffs, rvecs, tvecs);
    // fisheye::calibrate(objectPoints, imagePoints, image.size(), cameraMatrix, distCoeffs, rvecs, tvecs);
    // ��������ڲξ���ͻ���ϵ�����ı��ļ�
    cv::FileStorage fs("E:\\cv\\camera_parameters.txt", cv::FileStorage::WRITE);
    fs << "CameraMatrix" << cameraMatrix;
    fs << "DistortionCoefficients" << distCoeffs;
    fs.release();
    // cout << "Camera matrix:" << endl << cameraMatrix << endl;
    // cout << "Distortion coefficients:" << endl << distCoeffs << endl;
    objectPoints.clear();
    imagePoints.clear();
}