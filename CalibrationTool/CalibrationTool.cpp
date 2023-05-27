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
    ui.imageList->setViewMode(QListView::ListMode);
    //����QListWidget�е�Ԫ���ͼƬ��С
    ui.imageList->setIconSize(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH));
    //����QListWidget�е�Ԫ��ļ��
    ui.imageList->setSpacing(IMAGE_PADDING);
    //�����Զ���Ӧ���ֵ���(Adjust��Ӧ��Fixed����Ӧ)��Ĭ�ϲ���Ӧ
    ui.imageList->setResizeMode(QListWidget::Adjust);
    //���ò����ƶ�
    ui.imageList->setMovement(QListWidget::Static);
    //setAttribute(Qt::WA_Hover, true)
    ui.imageList->setAttribute(Qt::WA_Hover, true);
}
void CalibrationTool::openCamara()
{
    cam = VideoCapture();//������ͷ��������ͷ�л�ȡ��Ƶ
    cam.open(1);
    if (!cam.isOpened()) {
        cam.open(0);
    }
    ui.openCam->setIcon(QIcon(":/picture/picture/openning.png"));
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

    // ��������Mat��map���keyΪimageNameMap��size
    this->camImageMap[this->imageNameMap.size()] = flipedFrame;
    this->imageNameMap[this->imageNameMap.size()] = "";

    QImage image((const uchar*)flipedFrame.data, flipedFrame.size().width, flipedFrame.size().height, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);
    double ratio = static_cast<double>(image.height()) / image.width();
    QListWidgetItem* item = new QListWidgetItem();
    item->setIcon(QIcon(pixmap)); // ���� item->setIcon(QIcon::fromImage(image));
    item->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
    item->setText(QString::number(this->imageNameMap.size()));

    QDateTime currentDateTime = QDateTime::currentDateTime(); // ��ȡ��ǰʱ��
    qint64 timestamp = currentDateTime.toSecsSinceEpoch(); // ת��Ϊʱ������뼶��
    QString timestampText = QString::number(timestamp); // ��ʱ���ת��Ϊ�ַ���
    
    item->setTextAlignment(Qt::AlignVCenter);
    item->setToolTip(timestampText + ".tmp.png");

    ui.imageList->addItem(item);

    // ���պ���ǵ�
    this->imageCorners.push_back(findOneCorners(flipedFrame, BOARD_SIZE));
}


/*******************************
***�ر�����ͷ���ͷ���Դ�������ͷ�***
********************************/
void CalibrationTool::closeCamara()
{
    ui.openCam->setIcon(QIcon(":/picture/picture/camera.png"));
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
    cv::Mat image = cv::imread(this->fileNames[0].toStdString());
    // this->fullCalibResults = calibrate(fileNames, NORMAL_CAM);
    this->calibResults = calibarteWithCorners(this->imageCorners, image.size(), BOARD_SIZE, NORMAL_CAM);
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
// �ڴ������ʵ���ļ���ʵ�ֲۺ��������½�����
void CalibrationTool::updateProgress(int value)
{
    // ���½�������ֵ
    this->progressBar->setValue(value);
}


void CalibrationTool::fileOpenActionSlot()
{
    selectFile();
}

// ����������
void CalibrationTool::createProgressBar(bool isBatch) {
    // �����������Ի���
    // this->progressBar = new QProgressDialog ("Checking corner points...", "Cancel", 0, 100);
    // this->progressBar->setWindowTitle("Progress");
    if (!this->progressBar)
    {
        this->progressBar = new QProgressDialog("Checking corner points...", nullptr, 0, 100);
        this->progressBar->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
        this->progressBar->setWindowModality(Qt::WindowModal);
        this->progressBar->setMinimumDuration(0);
        this->progressBar->setCancelButton(nullptr); // �Ƴ�ȡ����ť
        Qt::WindowFlags flags = this->progressBar->windowFlags();
        flags &= ~Qt::WindowCloseButtonHint;  // �Ƴ��رհ�ť
        this->progressBar->setWindowFlags(flags);

        // ���ù̶��Ŀ�Ⱥ͸߶�
        this->progressBar->setFixedSize(250, 80);
        this->progressBar->setStyleSheet(
            "QProgressBar { background-color: white; text-align: center; border-radius:5px; font-weight:bolder }"
            "QProgressBar::chunk { background-color:#3366CC; border-radius:5px }"
        );
    }

    if (isBatch)
    {
        connect(this, SIGNAL(progressUpdate(int)), this, SLOT(updateProgress(int)));
    }

    this->progressBar->show();
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
        // ѡ��ͼƬ�ϴ���ʱ����
        // this->createProgressBar(false);

        // ����ÿ�ζ������¸�ֵ������������map��
        fileNames = fileDialog->selectedFiles();
        int len = this->imageNameMap.size();
        for (int i = 0; i < fileNames.length(); i++) {
            fileNames[i] = QDir::toNativeSeparators(fileNames[i]);
            this->imageNameMap[i + len] = fileNames[i];
            // ���ϴ�ͼƬ��ʱ����ǵ�
            // this->imageCorners.push_back(findOneCorners(fileNames[i], BOARD_SIZE));
            // this->progressBar->setValue((i+1)*100/ fileNames.length());
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

    int stIndex = this->imageCorners.size();
    this->createProgressBar(false);
    for (int i = 0; i < fileNames.length(); i++) {
        // ����QListWidgetItem����
        QListWidgetItem* imageItem = new QListWidgetItem;
        QImage image(fileNames[i]);
        double ratio = static_cast<double>(image.height()) / image.width();
        // qDebug() << height << endl;
        imageItem->setIcon(QIcon(fileNames[i]));
        //�������õ�Ԫ��ͼƬ�Ŀ�Ⱥ͸߶�
        imageItem->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
        imageItem->setText(QString::number(stIndex + i + 1));

        // ������ʾ�ļ���
        QFileInfo fileInfo(fileNames[i]);
        QString fileName = fileInfo.fileName();
        imageItem->setTextAlignment(Qt::AlignVCenter);
        imageItem->setToolTip(fileName);
        ui.imageList->addItem(imageItem);

        //���ϴ�ͼƬ��ʱ����ǵ�
        this->imageCorners.push_back(findOneCorners(fileNames[i], BOARD_SIZE));
        this->progressBar->setValue((i + 1) * 100 / fileNames.length());

    }
    //this->progressBar->setValue(100);

    //��ʾQListWidget
    ui.imageList->show();
    // �������ǵ�
    // this->createProgressBar(true);
    // this->imageCorners = findCorners(fileNames,BOARD_SIZE,this);

}

void CalibrationTool::handleListItemClick(QListWidgetItem* item)
{
    // ���� QListWidgetItem �ĵ���¼�
    // ���Ի�ȡ item �����ݡ������Ƚ��д���
    // ʾ������ȡ item ���ı�
    //int index = item->text().toInt();
    int index = ui.imageList->row(item);

    qDebug() << "Clicked item text: " << index << "\n";
    qDebug() << this->imageCorners.size() << "\n";
    vector<cv::Point2f> corners = this->imageCorners[index];
    QString fileName = this->imageNameMap[index];
    cv::Mat flipedFrame;
    if (!fileName.isEmpty()) {
        flipedFrame = cv::imread(fileName.toStdString());
        // ����ɫ��ʽ��BGRת��ΪRGB
        cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
    }
    else
    {
        flipedFrame = this->camImageMap[index];
    }
    cv::drawChessboardCorners(flipedFrame, Size(9, 6), corners, !corners.empty());
    
    // ��ץȡ����֡��ת��ΪQImage��ʽ��QImage::Format_RGB888��ͬ������ͷ�ò�ͬ�ĸ�ʽ��
    QImage image(flipedFrame.data, flipedFrame.cols, flipedFrame.rows, flipedFrame.step, QImage::Format_RGB888);
    //������ʾ����
    QGraphicsScene* scene = new QGraphicsScene;
    //������������ļ�·��ΪfileName��QString���ͣ����ļ�
    scene->addPixmap(QPixmap::fromImage(image));
    //����graphicsView��QGraphicsView�ࣩ�ؼ���ʾ����������
    ui.imageWindow->setScene(scene);
    //��ʼ��ʾ
    ui.imageWindow->show();
}