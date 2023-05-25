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
private:
    void initImageList();
    void createAction();
    //��������
    void createMenu();
    //�����˵�
    void createContentMenu();//���� �����Ĳ˵�
private:
    void selectFile();
    //����ѡ���ļ��Ի���
    void showImageList();
    //������ͼ��ʾͼƬ
private:
    QAction* fileOpenAction; //����һ��QActionָ�룬 ���ļ�����
    QMenu* menu;  //����һ��QMenuָ��
private:
    QStringList fileNames;
    CalibrateResults calibResults;
};
