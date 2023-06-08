#include "Calibrate.h"
#include "CalibrationTool.h"

FullCalibrateResults calibrate(const QStringList& fileNames, const int cameraType) {
    // 1. ׼���궨����ͼ��
    int boardWidth = 9;  // ���̸�����ڽǵ�����
    int boardHeight = 6; // ���̸������ڽǵ�����
    float squareSize = 0.12f; // ���̸���ӵĴ�С����λΪ��,������ã���Ӱ������ڲμ���
    cv::Size boardSize(boardWidth, boardHeight);

    vector<vector<cv::Point3f>> objectPoints;
    vector<vector<cv::Point2f>> imagePoints;
    vector<cv::Point2f> corners;

    // 2. ��������ͼ��
    cv::Mat image, gray;
    // namedWindow("image", WINDOW_NORMAL);
    FullCalibrateResults res;

    // 3. ����ͼ�����ݣ�����ȡ�ǵ�
    for (size_t i = 0; i < fileNames.size(); i++)
    {
        image = cv::imread(fileNames[i].toStdString(), cv::IMREAD_COLOR);
        print(image);
        cvtColor(image, gray, cv::COLOR_BGR2GRAY);

        bool found = findChessboardCorners(image, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
        if (found)
        {
            cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
            drawChessboardCorners(image, boardSize, corners, found);
            res.imageCorners.push_back(corners);
            // imshow("image", image);
            // waitKey();

            vector<cv::Point3f> objectCorners;
            for (int j = 0; j < boardHeight; j++)
            {
                for (int k = 0; k < boardWidth; k++)
                {
                    objectCorners.push_back(cv::Point3f(k * squareSize, j * squareSize, 0));
                }
            }
            objectPoints.push_back(objectCorners);
            imagePoints.push_back(corners);
        }
        else
        {
            std::vector<cv::Point2f> emptyVector;
            // �� emptyVector ��ӵ� imageCorners
            res.imageCorners.push_back(emptyVector);
        }

        // cout << i << endl;
    }

    // 4. �궨���
    cv::Mat cameraMatrix, distCoeffs;
    vector<cv::Mat> rvecs, tvecs;
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
    // ��������ڲξ���ͻ���ϵ�����ı��ļ�
    /*cv::FileStorage fs(INTRINSIC_SAVE_PATH, cv::FileStorage::WRITE);
    fs << "CameraMatrix" << cameraMatrix;
    fs << "DistortionCoefficients" << distCoeffs;
    fs << "R" << rvecs;
    fs << "t" << tvecs;*/
    // cout << "Camera matrix:" << endl << cameraMatrix << endl;
    // cout << "Distortion coefficients:" << endl << distCoeffs << endl;
    // ����ÿ��ͼ�����ͶӰ���
    vector<double> errors;
    for (size_t i = 0; i < objectPoints.size(); i++) {
        double reprojectionError = calculateReprojectionError(objectPoints[i], imagePoints[i], cameraMatrix, distCoeffs, rvecs[i], tvecs[i], cameraType);
        errors.push_back(reprojectionError);
        // cout << "Reprojection error for image " << i << ": " << reprojectionError << endl;
    }
    res.reprojectionError = errors;
   // fs << "errors" << errors;
    //fs.release();
    objectPoints.clear();
    imagePoints.clear();

    return res;
}


vector<vector<cv::Point2f>> findCorners(const QStringList& fileNames,
    const cv::Size boardSize,
    CalibrationTool* ui) {
    // 1. ׼���궨����ͼ��
    float squareSize = 0.12f; // ���̸���ӵĴ�С����λΪ��,������ã���Ӱ������ڲμ���
    vector<cv::Point2f> corners;

    // 2. ��������ͼ��
    cv::Mat image, gray;
    vector<vector<cv::Point2f>> res;

    // 3. ����ͼ�����ݣ�����ȡ�ǵ�
    for (size_t i = 0; i < fileNames.size(); i++)
    {
        image = cv::imread(fileNames[i].toStdString(), cv::IMREAD_COLOR);
        print(image);
        cvtColor(image, gray, cv::COLOR_BGR2GRAY);

        bool found = findChessboardCorners(image, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
        if (found)
        {
            cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
            drawChessboardCorners(image, boardSize, corners, found);
            res.push_back(corners);
        }
        else
        {
            std::vector<cv::Point2f> emptyVector;
            // �� emptyVector ��ӵ� imageCorners
            res.push_back(emptyVector);
        }
        // ���½���
        int progress = (i + 1) * 100 / fileNames.size();
        emit ui->progressUpdate(progress); // ���ͽ��ȸ����ź�
    }

    return res;
}

vector<cv::Point2f> findOneCorners(const QString& fileName, const cv::Size boardSize) {
    float squareSize = 0.12f; // ���̸���ӵĴ�С����λΪ��,������ã���Ӱ������ڲμ���
    vector<cv::Point2f> corners;
    cv::Mat image, gray;
    image = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
    print(image);
    cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    bool found = findChessboardCorners(image, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
    if (found)
    {
        cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
        drawChessboardCorners(image, boardSize, corners, found);
        return corners;
    }
    else
    {
        std::vector<cv::Point2f> emptyVector;
        return emptyVector;
    }
}

vector<cv::Point2f> findOneCorners(const cv::Mat& imageFromCam, const cv::Size boardSize) {
    float squareSize = 0.12f; // ���̸���ӵĴ�С����λΪ��,������ã���Ӱ������ڲμ���
    vector<cv::Point2f> corners;
    cv::Mat image, gray;
    image = imageFromCam;
    print(image);
    cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    bool found = findChessboardCorners(image, boardSize, corners, cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
    if (found)
    {
        cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
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
    const cv::Size imageSize, const cv::Size boardSize, const int cameraType) {
    CalibrateResults res;
    vector<vector<cv::Point3f>> objectPoints;
    vector<vector<cv::Point2f>> imagePoints;

    for (size_t i = 0; i < imageCorners.size(); i++) {
        if (!imageCorners[i].empty()) {
            vector<cv::Point3f> objectCorners;
            for (int j = 0; j < boardSize.height; j++)
            {
                for (int k = 0; k < boardSize.width; k++)
                {
                    objectCorners.push_back(cv::Point3f(k * 0.12f, j * 0.12f, 0));
                }
            }
            objectPoints.push_back(objectCorners);
            imagePoints.push_back(imageCorners[i]);
        }
    }

    // 4. �궨���
    cv::Mat cameraMatrix, distCoeffs;
    vector<cv::Mat> rvecs, tvecs;
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

    // ��������ڲξ���ͻ���ϵ�����ı��ļ�
    //cv::FileStorage fs(INTRINSIC_SAVE_PATH, cv::FileStorage::WRITE);
    //fs << "CameraMatrix" << cameraMatrix;
    //fs << "DistortionCoefficients" << distCoeffs;
    //fs << "R" << rvecs;
    //fs << "t" << tvecs;
    // cout << "Camera matrix:" << endl << cameraMatrix << endl;
    // cout << "Distortion coefficients:" << endl << distCoeffs << endl;
    // ����ÿ��ͼ�����ͶӰ���
    vector<double> errors;
    for (size_t i = 0; i < objectPoints.size(); i++) {
        double reprojectionError = calculateReprojectionError(objectPoints[i], imagePoints[i], cameraMatrix, distCoeffs, rvecs[i], tvecs[i], cameraType);
        errors.push_back(reprojectionError);
        // cout << "Reprojection error for image " << i << ": " << reprojectionError << endl;
    }
    res.reprojectionError = errors;
    //fs << "errors" << errors;
    //fs.release();
    objectPoints.clear();
    imagePoints.clear();

    return res;
}

double calculateReprojectionError(const vector<cv::Point3f>& objectPoints,
    const vector<cv::Point2f>& imagePoints,
    const cv::Mat& cameraMatrix,
    const cv::Mat& distCoeffs,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    const int cameraType) {
    vector<cv::Point2f> projectedPoints;
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


