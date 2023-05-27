#include "CalibrationTool.h"
#include "Const.h"
#include "Calibrate.h"

#include <QtCharts/QChartGlobal>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QLineSeries>


QT_CHARTS_USE_NAMESPACE

CalibrationTool::CalibrationTool(QWidget* parent)
    : QMainWindow(parent)
{
    /*��ʼ��*/
    ui.setupUi(this);
    initImageList();
    timer = new QTimer(this);

    /*�źźͲ�*/
    connect(timer, SIGNAL(timeout()), this, SLOT(readFarme()));  // ʱ�䵽����ȡ��ǰ����ͷ��Ϣ
    connect(ui.openCam, SIGNAL(clicked()), this, SLOT(openCamara()));
    connect(ui.takePic, SIGNAL(clicked()), this, SLOT(takingPictures()));
    connect(ui.closeCam, SIGNAL(clicked()), this, SLOT(closeCamara()));
    connect(ui.calib, SIGNAL(clicked()), this, SLOT(startCalibrate()));
    connect(ui.open, SIGNAL(clicked()), this, SLOT(fileOpenActionSlot()));
    // connect(ui.imageList);
    connect(ui.imageList, &QListWidget::itemClicked, this, &CalibrationTool::handleListItemClick);

    //������ͼ
    // ����
    QBarSet* projectionError = new QBarSet("Projection Error");
    int data[] = { 5, 10, 15, 20, 25, 30, 35, 40, 45, 50 };
    int dataSize = sizeof(data) / sizeof(int);
    for (int i = 0; i < dataSize; ++i) {
        *projectionError << data[i];
    }

    QBarSeries* series = new QBarSeries();
    series->append(projectionError);

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Projection Error");

    chart->setAnimationOptions(QChart::SeriesAnimations);

    // ������ʾͼ��Ĵ��ڲ�����
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // XY���ǩ
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->setTitleText("Images");
    axisX->append(QStringList() << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9"); // ���X���ǩ
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    //axisY->setRange(0, 15);
    axisY->setTitleText("Mean Erros in Pixels");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(false);
    //chart->legend()->setAlignment(Qt::AlignBottom); /* ��ʾ�ڵײ� */


    // ����ֵ��
    // �������ݵľ�ֵ
    double mean = 0;
    for (int i = 0; i < dataSize; ++i) {
        mean += data[i];
    }
    mean /= dataSize;

    // ����һ��QLineSeries���������ƾ�ֵ��
    QtCharts::QLineSeries* meanLine = new QtCharts::QLineSeries;
    meanLine->setName("Mean");
    meanLine->append(0, mean); // �����ʼ��
    meanLine->append(dataSize - 1, mean); // ��ӽ�����

    // ��QLineSeries������ӵ�QChart������
    chart->addSeries(meanLine);

    // ��QLineSeries����󶨵�X���Y��
    meanLine->attachAxis(axisX);
    meanLine->attachAxis(axisY);


    //ui.myChart->SetChart(chart);
    ui.haha->setChart(chart);

    // չʾͼ��
    //QGraphicsView* histogramView = ui.histogram; // histogram ��֮ǰ�� UI �ļ��ж���� QGraphicsView ���
    //QGraphicsScene* scene = new QGraphicsScene(histogramView); // ����һ���������󣬹����� histogramView ���

    //// ��ȡ histogram �����λ�úͳߴ�
    //QRect histogramGeometry = ui.histogram->geometry();

    //// �� chartView ��λ�úͳߴ�����Ϊ�� histogram ��ͬ
    //chartView->setGeometry(histogramGeometry);
    //chartView->resize(300, 300);

    //histogramView->setScene(scene);
    //scene->addWidget(chartView); // �� chartView ��ӵ�������

}

CalibrationTool::~CalibrationTool()
{}


void CalibrationTool::initImageList() {
    //createAction();
    //createMenu();
    //createContentMenu();
    //����QListWidget����
    //����QListWidget����ʾģʽ
    ui.imageList->setViewMode(QListView::IconMode);
    //����QListWidget�е�Ԫ���ͼƬ��С
    ui.imageList->setIconSize(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH));
    //����QListWidget�е�Ԫ��ļ��
    ui.imageList->setSpacing(IMAGE_PADDING);
    //�����Զ���Ӧ���ֵ���(Adjust��Ӧ��Fixed����Ӧ)��Ĭ�ϲ���Ӧ
    ui.imageList->setResizeMode(QListWidget::Adjust);
    //���ò����ƶ�
    ui.imageList->setMovement(QListWidget::Static);
}
void CalibrationTool::openCamara()
{
    cam = VideoCapture();//������ͷ��������ͷ�л�ȡ��Ƶ
    cam.open(1);
    if (!cam.isOpened()) {
        cam.open(0);
    }
    // ����ÿһ֡�Ĵ�СΪָ��ֵ
    cam.set(CAP_PROP_FRAME_WIDTH, IMAGE_WIN_WIDTH);
    cam.set(CAP_PROP_FRAME_HEIGHT, IMAGE_WIN_HEIGHT);
    timer->start(SAMPLE_RATE + 3);              // ��ʼ��ʱ����ʱ�򷢳�timeout()�ź�
}

/*********************************
********* ��ȡ����ͷ��Ϣ ***********
**********************************/
void CalibrationTool::readFarme()
{
    cam >> frame;// ������ͷ��ץȡ������ÿһ֡
    Mat flipedFrame;
    flip(frame, flipedFrame, 1);
    // ����ɫ��ʽ��BGRת��ΪRGB
    cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
    // ��ץȡ����֡��ת��ΪQImage��ʽ��QImage::Format_RGB888��ͬ������ͷ�ò�ͬ�ĸ�ʽ��
    QImage image(flipedFrame.data, flipedFrame.cols, flipedFrame.rows, flipedFrame.step, QImage::Format_RGB888);
    //ui.imageWindow->setPixmap(QPixmap::fromImage(image));  // ��ͼƬ��ʾ��label��
    //������ʾ����
    QGraphicsScene* scene = new QGraphicsScene;
    //������������ļ�·��ΪfileName��QString���ͣ����ļ�
    scene->addPixmap(QPixmap::fromImage(image));
    //����graphicsView��QGraphicsView�ࣩ�ؼ���ʾ����������
    ui.imageWindow->setScene(scene);
    //��ʼ��ʾ
    ui.imageWindow->show();
}


void CalibrationTool::takingPictures()
{
    if (!cam.isOpened()) {
        QMessageBox::warning(this, tr("warning"), tr("please open camera"), QMessageBox::Ok);
        return;
    }
    cam >> frame;// ������ͷ��ץȡ������ÿһ֡
    Mat flipedFrame;
    flip(frame, flipedFrame, 1);
    cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
    QImage image((const uchar*)flipedFrame.data, flipedFrame.size().width, flipedFrame.size().height, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);
    double ratio = static_cast<double>(image.height()) / image.width();
    QListWidgetItem* item = new QListWidgetItem();
    item->setIcon(QIcon(pixmap)); // ���� item->setIcon(QIcon::fromImage(image));
    item->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
    ui.imageList->addItem(item);
}

/*******************************
***�ر�����ͷ���ͷ���Դ�������ͷ�***
********************************/
void CalibrationTool::closeCamara()
{
    timer->stop(); // ֹͣ��ȡ���ݡ�
    cam.release();
    QGraphicsScene* scene = new QGraphicsScene;
    ui.imageWindow->setScene(scene);
    //��ʼ��ʾ
    ui.imageWindow->show();
    //ui.imageWindow->clear();
}


void CalibrationTool::startCalibrate() {
    // ������ʾδ���ͼƬ
    if (this->fileNames.length() == 0) {
        QMessageBox::warning(this, tr("warning"),
            tr("You haven't upload any image"));

        return;
    }
    // ��ͼƬС��10�ŵ�ʱ����ʾ�Ƿ�����궨
    if (this->fileNames.length() <= 10) {
        int reply = QMessageBox::warning(this, tr("warning"),
            tr("You should upload more than 10 images, would you want to continue?"),
            QMessageBox::Yes, QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }
    //ui.openCam->setEnabled(false);
    //ui.closeCam->setEnabled(false);
    //ui.takePic->setEnabled(false);
    //ui.calib->setEnabled(false);
    // 
    this->calibResults = calibrate(fileNames, NORMAL_CAM);
    //qDebug() 
       // << this->calibResults.rvecs
        //<< this->calibResults.tvecs
        //<< this->calibResults.rvecs
        //<< endl;
    //ui.openCam->setEnabled(true);
    //ui.closeCam->setEnabled(true);
    //ui.takePic->setEnabled(true);
    //ui.calib->setEnabled(true);
}

void CalibrationTool::createAction()
{
    //�������ļ�����
    fileOpenAction = new QAction(tr("open"), this);
    //���ߴ��ļ��Ŀ�ݷ�ʽ
    fileOpenAction->setShortcut(tr("Ctr1+0"));
    //���ô��ļ�������ʾ��Ϣ
    fileOpenAction->setStatusTip("open a folder");
    //�������ļ��������źźͲ�
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpenActionSlot()));
}

void CalibrationTool::createMenu()
{
    menu = this->menuBar()->addMenu(tr("Add Images"));
    menu->addAction(fileOpenAction);
}

void CalibrationTool::createContentMenu() {
    this->addAction(fileOpenAction);
    this->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void CalibrationTool::fileOpenActionSlot()
{
    selectFile();
    /*int reply = QMessageBox::warning(this, tr("warning"),tr("cam want to open file explore"),QMessageBox::Yes , QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        selectFile();
    }
    else
    {
        return;
    }*/
}

/***************************************
* Qt��ʹ���ļ�ѡ��Ի���������:
* 1.����һ��QFileDialog����
* 2.����·����������������
******************************************/
void CalibrationTool::selectFile() {
    //�����ļ��Ի�����
    QFileDialog* fileDialog = new QFileDialog(this);
    //�����ļ��Ի������
    fileDialog->setWindowTitle(tr("open fold"));
    //����Ĭ���ļ�·��
    fileDialog->setDirectory(".");
    //�����ļ�������
    fileDialog->setNameFilter(tr("Images(*.png *.jpg *.jpeg * .bmp)"));
    //���ÿ���ѡ�����ļ���Ĭ��Ϊֻ��ѡ��һ���ļ�QF ileDialog: :ExistingFiles
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    //������ͼģʽ
    fileDialog->setViewMode(QFileDialog::Detail);
    //��ӡ����ѡ����ļ���·��
    if (fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();
        for (int i = 0; i < fileNames.length(); i++) {
            fileNames[i] = QDir::toNativeSeparators(fileNames[i]);
        }
    }
    showImageList();
    for (auto tmp : fileNames)
    {
        qDebug() << tmp << endl;
    }
}

/***************************************
*Qt��ʹ���ļ�ѡ��Ի���������:
* 1.����һ��QListWidget����
* 2.����ViewMode������
* 3.���嵥Ԫ���ӵ�QL istWidget��
* 4.����QListWidget�����show()����
* ****************************************/
void CalibrationTool::showImageList() {

    for (auto tmp : fileNames) {
        // ����QListWidgetItem����
        QListWidgetItem* imageItem = new QListWidgetItem;
        QImage image(tmp);
        double ratio = static_cast<double>(image.height()) / image.width();
        // qDebug() << height << endl;
        imageItem->setIcon(QIcon(tmp));
        //�������õ�Ԫ��ͼƬ�Ŀ�Ⱥ͸߶�
        imageItem->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
        //imageItem->setText(QFileInfo::fileName(tmp));
        
        ui.imageList->addItem(imageItem);
    }
    //��ʾQListWidget
    ui.imageList->show();
}

void CalibrationTool::handleListItemClick(QListWidgetItem* item)
{
    // ���� QListWidgetItem �ĵ���¼�
    // ���Ի�ȡ item �����ݡ������Ƚ��д���
    // ʾ������ȡ item ���ı�
    QString text = item->text();
    qDebug() << "Clicked item text: " << text;
}
