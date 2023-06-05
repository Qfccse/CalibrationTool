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
#include <QtCharts>
#include <Qt3DCore>
#include <Qt3DRender>
#include <Qt3DExtras>
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
    cv::VideoCapture cam;// ��Ƶ��ȡ�ṹ�� ������Ϊ��Ƶ��ȡ������һ������
    cv::Mat frame;//����IplImage����ָ�룬���������ڴ�ռ������ÿһ֡ͼ��

public slots:
    //�ۺ���
    void openCamara();      // ������ͷ
    void readFarme();       // ��ȡ��ǰ֡��Ϣ
    void closeCamara();     // �ر�����ͷ��
    void takingPictures();  // ����
    void fileOpenActionSlot();//���� ��������Ӧ�Ĳۺ���
    void startCalibrate();
    void handleListItemClick(QListWidgetItem* item); // ���ϴ���ͼƬ��ӵ���¼�
    void updateProgress(int value); //����궨�Ľ��������º���
    void clickToUndistort();
    void clickToShow(int index);

    /*��������Ҽ��¼��������˵�*/
    void onCustomContextMenuRequested(const QPoint & pos);
    /*�������˵��ϵ�Delete�¼�*/
    void onActionDelete();
    void onActionClear();
    void onActionRemoveAndReCalibrate();
    
    void changeShowUndistorted();//�л�ͼƬ��ʽ

signals:
    void progressUpdate(int value); // ����궨�Ľ������Ľ������
private:
    void initImageList();
    void createProgressBar(bool isBatch);
    void createLoading();
    void createBarChart(); // ������ͼ
    void createPatternCentric();
    void createAxis();
    void createPlane();
    void createScale(Qt3DRender::QCamera* camera);
    Qt3DCore::QEntity* createCuboid(QVector3D transformMatrix, QVector3D rotationMatrix);
    void addCuboidToCentric();
private:
    //����ѡ���ļ��Ի���
    void selectFile();
    //������ͼ��ʾͼƬ
    void showImageList();
    //�궨����
    void calcSizeAndCalib();
private:
    QAction* fileOpenAction = nullptr; //����һ��QActionָ�룬 ���ļ�����
    QMenu* menu = nullptr;  //����һ��QMenuָ��
    QProgressDialog* progressBar = nullptr; // ������

    QLabel* m_tooltip = nullptr; // ����ͼָ��

    QMenu* popMenu_In_ListWidget_ = nullptr;/*�����˵�*/
    QAction* action_Delete_In_ListWidget_ = nullptr;/*�˵��ϵ�Action*/
    QAction* action_Clear_In_ListWidget_ = nullptr;/*�˵��ϵ�Action*/
    QAction* action_Delete_And_ReCalibrate_In_ListWidget_ = nullptr;
private:
    bool showUndistored = false;
    int maxNameIndex = 0;
    int clickedIndex = 0;
    QStringList fileNames;
    // FullCalibrateResults fullCalibResults;
    CalibrateResults calibResults;
    vector<vector<cv::Point2f>> imageCorners;
    vector<QString> imageNameList;
    vector<cv::Mat> imageMatList;
    vector<cv::Mat> undistortedImageList;
    Qt3DExtras::Qt3DWindow* view3D;
    // 3D ʵ��
    Qt3DCore::QEntity* rootEntity;
    // ���� ���λ��
    vector<Qt3DCore::QEntity*> cuboids;
};
