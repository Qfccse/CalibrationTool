#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_CalibrationTool.h"
#include <opencv.hpp>
#include <opencv2/highgui.hpp>
#include <qimage.h>
#include <qtimer.h>
#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#include <QString>
#include <QListWidget>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QAction>
#include <QDateTime>
#include "Const.h"

class CalibrationTool : public QMainWindow
{
    Q_OBJECT

public:
    CalibrationTool(QWidget* parent = nullptr);
    ~CalibrationTool();

private:
    Ui::CalibrationToolClass ui;
    QTimer* timer;
    QImage imag;
    cv::VideoCapture cam;// 视频获取结构， 用来作为视频获取函数的一个参数
    cv::Mat frame;//申请IplImage类型指针，就是申请内存空间来存放每一帧图像

public slots:
    //槽函数
    void openCamara();      // 打开摄像头
    void readFarme();       // 读取当前帧信息
    void closeCamara();     // 关闭摄像头。
    void takingPictures();  // 拍照
    void fileOpenActionSlot();//打开文 件动作对应的槽函数
    void startCalibrate();
    void handleListItemClick(QListWidgetItem* item); // 给上传的图片添加点击事件
    void updateProgress(int value); //相机标定的进度条更新函数
    void clickToUndistort();
    void clickToShow(int index);

    /*处理鼠标右键事件，弹出菜单*/
    void onCustomContextMenuRequested(const QPoint & pos);
    /*处理弹出菜单上的Delete事件*/
    void onActionDelete();
    void onActionClear();
    void onActionRemoveAndReCalibrate();
    
    void changeShowUndistorted();//切换图片样式

signals:
    void progressUpdate(int value); // 相机标定的进度条的进度序号
private:
    void initImageList();
    void createProgressBar(bool isBatch);
    void createLoading();
    void createBarChart(); // 画条形图
    void createPatternCentric();
    void createPatternCentric2();


    void calculateAxisAngle();
private:
    //弹出选择文件对话框
    void selectFile();
    //用缩略图显示图片
    void showImageList();
    //标定函数
    void calcSizeAndCalib();
private:
    QAction* fileOpenAction = nullptr; //创建一个QAction指针， 打开文件动作
    QMenu* menu = nullptr;  //创建一个QMenu指针
    QProgressDialog* progressBar = nullptr; // 进度条

    QLabel* m_tooltip = nullptr; // 条形图指针

    QMenu* popMenu_In_ListWidget_ = nullptr;/*弹出菜单*/
    QAction* action_Delete_In_ListWidget_ = nullptr;/*菜单上的Action*/
    QAction* action_Clear_In_ListWidget_ = nullptr;/*菜单上的Action*/
    QAction* action_Delete_And_ReCalibrate_In_ListWidget_ = nullptr;
public:
    bool showUndistored;
private:
    int maxNameIndex = 0;
    QStringList fileNames;
    // FullCalibrateResults fullCalibResults;
    CalibrateResults calibResults;
    vector<vector<cv::Point2f>> imageCorners;
    vector<QString> imageNameList;
    vector<cv::Mat> imageMatList;
    vector<cv::Mat> undistortedImageList;
};
