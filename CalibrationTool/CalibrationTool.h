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

class CalibrationTool : public QMainWindow
{
    Q_OBJECT

public:
    CalibrationTool(QWidget *parent = nullptr);
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
private:
    void initImageList();
    void createAction();
    //创建动作
    void createMenu();
    //创建菜单
    void createContentMenu();//创建 上下文菜单
private:
    void selectFile();
    //弹出选择文件对话框
    void showImageList();
    //用缩略图显示图片
private:
    QAction* fileOpenAction; //创建一个QAction指针， 打开文件动作
    QMenu* menu;  //创建一个QMenu指针
private:
    QStringList fileNames;
    CalibrateResults calibResults;
};
