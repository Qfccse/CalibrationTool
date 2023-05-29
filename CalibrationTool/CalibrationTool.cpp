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


#include <QtWidgets>
#include <Qt3DRender>
#include <Qt3DExtras>

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

	// ������ͼ����άͼ
	createBarChart();
	createPatternCentric();
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
	cam = cv::VideoCapture();//������ͷ��������ͷ�л�ȡ��Ƶ
	cam.open(1);
	if (!cam.isOpened()) {
		cam.open(0);
	}
	ui.openCam->setIcon(QIcon(":/picture/picture/openning.png"));
	// ����ÿһ֡�Ĵ�СΪָ��ֵ
	cam.set(cv::CAP_PROP_FRAME_WIDTH, IMAGE_WIN_WIDTH);
	cam.set(cv::CAP_PROP_FRAME_HEIGHT, IMAGE_WIN_HEIGHT);
	timer->start(SAMPLE_RATE + 3);              // ��ʼ��ʱ����ʱ�򷢳�timeout()�ź�
}

/*********************************
********* ��ȡ����ͷ��Ϣ ***********
**********************************/
void CalibrationTool::readFarme()
{
	cam >> frame;// ������ͷ��ץȡ������ÿһ֡
	cv::Mat flipedFrame;
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
	cv::Mat flipedFrame;
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
	this->imageCorners.push_back(findOneCorners(flipedFrame, cv::BOARD_SIZE));
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
	this->calibResults = calibarteWithCorners(this->imageCorners, image.size(), cv::BOARD_SIZE, NORMAL_CAM);
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
		this->imageCorners.push_back(findOneCorners(fileNames[i], cv::BOARD_SIZE));
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
	cv::drawChessboardCorners(flipedFrame, cv::Size(9, 6), corners, !corners.empty());

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


/***************************************
*Qt��ʹ��QCharts������ͼ��������:
* ****************************************/
void CalibrationTool::createBarChart() {
	// ����ͼ����
	QBarSet* projectionError = new QBarSet("Projection Error");
	vector<double> projectionError_ = { 3.0032143514447018e-01, 2.5005490108190759e-01, 2.2378858466658030e-01,
				   1.6412628748340338e-01, 1.9050650570901181e-01, 1.8053703768600146e-01,
				   2.1210603721570268e-01, 2.4443632141393190e-01, 2.6233146806962876e-01 };
	// ����ֵ��
	// �������ݵľ�ֵ
	double mean = 0;
	for (int i = 0; i < projectionError_.size(); ++i) {
		*projectionError << projectionError_[i];
		mean += projectionError_[i];
	}

	mean /= projectionError_.size();
	// ����QBarSeries
	QBarSeries* series = new QBarSeries();
	series->append(projectionError);

	// ����Q
	//QChart* chart = new QChart();
	QChart* chart = new QChart();
	chart->addSeries(series);
	chart->setTitle("Projection Error");
	chart->setAnimationOptions(QChart::SeriesAnimations);

	//chart->setBackgroundBrush(Qt::blue);


	// ������ʾͼ��Ĵ��ڲ�����
	QChartView* chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);

	// XY���ǩ
	QBarCategoryAxis* axisX = new QBarCategoryAxis();
	axisX->setTitleText("Images");
	//axisX->append(QStringList() << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8"); // ���X���ǩ
	chart->addAxis(axisX, Qt::AlignBottom);
	series->attachAxis(axisX);

	QValueAxis* axisY = new QValueAxis();
	//axisY->setRange(0, 15);
	axisY->setTitleText("Mean Erros in Pixels");
	chart->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisY);

	chart->legend()->setVisible(false);
	//chart->legend()->setAlignment(Qt::AlignBottom); /* ��ʾ�ڵײ� */


	// ����һ��QLineSeries���������ƾ�ֵ��
	QtCharts::QLineSeries* meanLine = new QtCharts::QLineSeries;
	meanLine->setName("Mean");
	meanLine->append(0, mean); // �����ʼ��
	meanLine->append(projectionError_.size() - 1, mean); // ��ӽ�����

	// ��QLineSeries������ӵ�QChart������
	chart->addSeries(meanLine);

	// ��QLineSeries����󶨵�X���Y��
	meanLine->attachAxis(axisX);
	meanLine->attachAxis(axisY);


	// չʾͼ��
	QGraphicsView* histogramView = ui.histogram; // histogram ��֮ǰ�� UI �ļ��ж���� QGraphicsView ���
	QGraphicsScene* scene = new QGraphicsScene(histogramView); // ����һ���������󣬹����� histogramView ���
	// ��ȡ histogram �����λ�úͳߴ�
	QRect histogramGeometry = ui.histogram->geometry();

	// �� chartView ��λ�úͳߴ�����Ϊ�� histogram ��ͬ
	chartView->setGeometry(histogramGeometry);
	chartView->resize(235, 235);


	histogramView->setScene(scene);
	scene->addWidget(chartView); // �� chartView ��ӵ�������
}

/***************************************
*Qt��ʹ��QCharts��createPatternCentricͼ��������:
* ****************************************/
void CalibrationTool::createPatternCentric() {

	//// ÿ��ͼƬ�����
	//vector<cv::Mat> R;
	//vector<cv::Mat> t;
	//// ����ϵ��
	//double D_[] = { -3.4351280917484162e-01, 1.5909766895881644e-01, -1.2375087184036587e-06, 7.4996411884060586e-04, -4.1226886540150574e-02 };
	//cv::Mat D = cv::Mat(1, 5, CV_64F, D_); // ����һ���յ� double ���;���

	//// ������ת���� R ����ά���� v
	//cv::Mat R_ = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1); // ʾ����ת����
	//cv::Mat v_ = (cv::Mat_<double>(3, 1) << 1, 2, 3); // ʾ����ά����

	//// ������ά����
	//cv::Mat result = R_ * v_;

	//// ������
	//std::cout << "Result:\n" << result << std::endl;
	// ��ӡ��������
	//cout << D.at<double>(1, 2) << endl;

	// չʾͼ��
	QGraphicsView* transformView = ui.transformGram; // histogram ��֮ǰ�� UI �ļ��ж���� QGraphicsView ���
	QGraphicsScene* scene = new QGraphicsScene(transformView); // ����һ���������󣬹����� histogramView ���

	// ���� 3D ����
	Qt3DExtras::Qt3DWindow* window = new Qt3DExtras::Qt3DWindow;
	window->defaultFrameGraph()->setClearColor(Qt::white);

	// ������ʵ��
	Qt3DCore::QEntity* rootEntity = new Qt3DCore::QEntity;

	//// �������
	//Qt3DRender::QCamera* camera = window.camera();
	//camera->setPosition(QVector3D(0, 0, 100));
	//camera->setViewCenter(QVector3D(0, 0, 0));

	// ��������ϵ
	Qt3DExtras::QCuboidMesh* xAxis = new Qt3DExtras::QCuboidMesh;
	xAxis->setXExtent(100);
	xAxis->setYExtent(0.5);
	xAxis->setZExtent(0.5);
	//xAxis->setColor(Qt::red);

	Qt3DExtras::QCuboidMesh* yAxis = new Qt3DExtras::QCuboidMesh;
	yAxis->setXExtent(0.5);
	yAxis->setYExtent(100);
	yAxis->setZExtent(0.5);
	//yAxis->setColor(Qt::green);

	Qt3DExtras::QCuboidMesh* zAxis = new Qt3DExtras::QCuboidMesh;
	zAxis->setXExtent(0.5);
	zAxis->setYExtent(0.5);
	zAxis->setZExtent(100);
	//zAxis->setColor(Qt::blue);

	//// ����������
	//Qt3DExtras::QCuboidMesh* cube1 = new Qt3DExtras::QCuboidMesh;
	//cube1->setXExtent(10);
	//cube1->setYExtent(10);
	//cube1->setZExtent(10);
	////cube1->setColor(Qt::yellow);

	//Qt3DExtras::QCuboidMesh* cube2 = new Qt3DExtras::QCuboidMesh;
	//cube2->setXExtent(20);
	//cube2->setYExtent(20);
	//cube2->setZExtent(20);
	////cube2->setColor(Qt::magenta);

	// ��������
	Qt3DExtras::QPhongMaterial* material = new Qt3DExtras::QPhongMaterial;
	material->setDiffuse(Qt::red); // ���ò�����ɫ

	// ����ʵ���������Ӳ���
	Qt3DCore::QEntity* coordinateSystem = new Qt3DCore::QEntity(rootEntity);
	coordinateSystem->addComponent(xAxis);
	coordinateSystem->addComponent(yAxis);
	coordinateSystem->addComponent(zAxis);
	coordinateSystem->addComponent(material);

	coordinateSystem->addComponent(xAxis);
	coordinateSystem->addComponent(yAxis);
	coordinateSystem->addComponent(zAxis);

	// ���ó�����ʵ��
	window->setRootEntity(rootEntity);

	// �� 3D ��ͼǶ�뵽 QGraphicsView
	QWidget* container = QWidget::createWindowContainer(window);
	transformView->setViewport(container);
	transformView->setRenderHint(QPainter::Antialiasing);
	transformView->show();
	


}
