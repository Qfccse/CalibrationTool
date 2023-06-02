#include "CalibrationTool.h"
#include "Const.h"
#include "Calibrate.h"

#include <QtCharts>
#include <Qt3DCore>
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
	createBarChart();
	// createPatternCentric();
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

		// ������ͼ����άͼ
	createBarChart();
	createPatternCentric();
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
*** Qt��ʹ��QCharts������ͼBarChart ***
*****************************************/


void CalibrationTool::createBarChart() {
	// ����ͼ����
	QBarSet* projectionError = new QBarSet("Projection Error");
	// vector<double> projectionError_ = calibResults.reprojectionError;
	vector<double> projectionError_ = { 3.0032143514447018e-01, 2.5005490108190759e-01, 2.2378858466658030e-01,
				   1.6412628748340338e-01, 1.9050650570901181e-01, 1.8053703768600146e-01,
				   2.1210603721570268e-01, 2.4443632141393190e-01, 3.0032143514447018e-01, 2.5005490108190759e-01, 2.2378858466658030e-01,
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

	//series->setLabelsPosition(QAbstractBarSeries::LabelsOutsideEnd);
	//series->setLabelsVisible(true);
	//barOK->setLabelColor(Qt::black);
	//barNG->setLabelColor(Qt::black);
	//barNAN->setLabelColor(Qt::black);

	// ����һ���Զ���ĵ��������
	QObject::connect(series, &QBarSeries::clicked, this, [](int index, QBarSet* barSet) {
		qDebug() << "Clicked on bar:" << barSet->label() << "at index:" << index;
		// ִ�е���¼��Ĵ����߼�
		});
	series->append(projectionError);

	// Set the width of the bars
	qreal barWidth = 1.0; // Adjust the value to your desired width
	series->setBarWidth(barWidth);


	QChart* chart = new QChart();
	chart->addSeries(series);
	chart->setTitle("Projection Error");
	chart->setAnimationOptions(QChart::SeriesAnimations);

	
	// ����һ��������󣬲����������С
	QFont fontX,fontY;
	fontX.setPointSize(CHART_FONT_SIZE); // ���������СΪ10
	fontY.setPointSize(CHART_FONT_SIZE);

	// XY���ǩ
	QBarCategoryAxis* axisX = new QBarCategoryAxis();
	// axisX->setTitleText("Images");
	int skipNum = projectionError_.size() / 10 + 1;
	for (int i = 0; i < projectionError_.size(); i+=skipNum) {
		axisX->append(QString::number(i+1));
	}
	axisX->setLabelsFont(fontX);
	chart->addAxis(axisX, Qt::AlignBottom);

	QValueAxis* axisY = new QValueAxis();
	// ����Y��̶ȱ�ǩ�ĽǶ�Ϊб����ʾ
	axisY->setLabelsAngle(-20);

	// ���ÿ̶ȱ�ǩ�ĸ�ʽ������ʹ��QString::number��������һλС��
	axisY->setLabelFormat("%.2f");
	// ����Y��̶ȱ�ǩ������
	axisY->setLabelsFont(fontY);

	// axisY->setTitleText("Mean Erros in Pixels");
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
	//QGraphicsView* histogramView = ui.histogram; // histogram ��֮ǰ�� UI �ļ��ж���� QGraphicsView ���
	QGraphicsScene* scene = new QGraphicsScene(ui.histogram); // ����һ���������󣬹����� histogramView ���
	// ��ȡ histogram �����λ�úͳߴ�
	QRect histogramGeometry = ui.histogram->geometry();
	histogramGeometry.setWidth(histogramGeometry.width() + CHART_EXPEND);
	histogramGeometry.setHeight(histogramGeometry.height() + CHART_EXPEND);
	// �� chartView ��λ�úͳߴ�����Ϊ�� histogram ��ͬ
	chart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	// ������ʾͼ��Ĵ��ڲ�����
	QChartView* chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);

	chartView->setGeometry(histogramGeometry);
	chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// ����ͼ��ı���͸��
	chartView->setAutoFillBackground(true);

	ui.histogram->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.histogram->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	// ���������Ĵ�С��ƥ��histogramView�ļ�����״
	scene->setSceneRect(histogramGeometry);
	
	scene->addWidget(chartView); // �� chartView ��ӵ�������

	ui.histogram->setScene(scene);

	// ����һ���Զ������ͣ������
	QObject::connect(series, &QBarSeries::hovered, this, [this, series, chart, chartView](bool status, int index, QBarSet* barset) {
			//���ָ��ͼ����ʱ��ʾ��ֵ�ı�
			QChart* pchart = chart;
			if (this->m_tooltip == nullptr)
			{
				m_tooltip = new  QLabel(chartView);    //ͷ�ļ��еĶ��� QLabel*   m_tooltip = nullptr;  //��״�������ʾ��Ϣ
				m_tooltip->setStyleSheet("background: rgba(95,166,250,185);color: rgb(248, 248, 255);"
					"border:0px groove gray;border-radius:10px;padding:2px 4px;"
					"border:2px groove gray;border-radius:10px;padding:2px 4px");
				m_tooltip->setVisible(false);
			}
			if (status)
			{
				double val = barset->at(index);
				QPointF point(index, barset->at(index));
				QPointF pointLabel = pchart->mapToPosition(point);
				QString sText = QString("%1").arg(val);

				m_tooltip->setText(sText);
				m_tooltip->move(pointLabel.x() - 50, pointLabel.y() - m_tooltip->height() * 1.5);
				m_tooltip->show();
			}
			else
			{
				m_tooltip->hide();
			}
		});
}

/***************************************
*** Qt��ʹ��Qt3D����ά����ͼ ***
*****************************************/
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


	// չʾͼ��
	QGraphicsView* transformView = ui.transformGram; // histogram ��֮ǰ�� UI �ļ��ж���� QGraphicsView ���
	QGraphicsScene* scene = new QGraphicsScene(transformView); // ����һ���������󣬹����� histogramView ���

	// Root entity
	Qt3DCore::QEntity* rootEntity = new Qt3DCore::QEntity();
	// 3D Window
	Qt3DExtras::Qt3DWindow* view = new Qt3DExtras::Qt3DWindow();
	view->defaultFrameGraph()->setClearColor(QColor(QRgb(0xffffffff)));
	QWidget* container = QWidget::createWindowContainer(view);
	QSize screenSize = view->screen()->size();
	container->setMinimumSize(QSize(200, 100));
	container->setMaximumSize(screenSize);
	view->setRootEntity(rootEntity);

	QWidget* widget = new QWidget;
	// layout
	QHBoxLayout* hLayout = new QHBoxLayout(widget);
	QVBoxLayout* vLayout = new QVBoxLayout();
	vLayout->setAlignment(Qt::AlignTop);
	hLayout->addWidget(container, 1);
	hLayout->addLayout(vLayout);
	widget->setWindowTitle(QStringLiteral("Basic shapes"));

	// Camera
	Qt3DRender::QCamera* cameraEntity = view->camera();
	// For camera controls
	Qt3DExtras::QFirstPersonCameraController* camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
	cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	cameraEntity->setPosition(QVector3D(30.0f, 30.0f, 30.0f));
	cameraEntity->setUpVector(QVector3D(0, 1, 0));
	cameraEntity->setViewCenter(QVector3D(0, 0, 0));
	camController->setCamera(cameraEntity);

	// Light
	Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
	Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
	light->setColor("white");
	light->setIntensity(1);
	lightEntity->addComponent(light);
	Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
	lightTransform->setTranslation(cameraEntity->position());
	lightEntity->addComponent(lightTransform);


	// Cuboid mesh
	Qt3DExtras::QCuboidMesh* cuboid = new Qt3DExtras::QCuboidMesh();
	// CuboidMesh Transform
	Qt3DCore::QTransform* cuboidTransform = new Qt3DCore::QTransform();
	cuboidTransform->setScale(1.0f);
	cuboidTransform->setTranslation(QVector3D(10.0f, 10.0f, 10.0f));
	// CuboidMesh Material
	Qt3DExtras::QPhongMaterial* cuboidMaterial = new Qt3DExtras::QPhongMaterial();
	cuboidMaterial->setDiffuse(QColor(Qt::black));
	// Cuboid
	Qt3DCore::QEntity* cuboidEntity = new Qt3DCore::QEntity(rootEntity);
	cuboidEntity->addComponent(cuboid);
	cuboidEntity->addComponent(cuboidMaterial);
	cuboidEntity->addComponent(cuboidTransform);

	// Plane mesh
	Qt3DExtras::QPlaneMesh* planeMesh = new Qt3DExtras::QPlaneMesh();
	planeMesh->setWidth(2);
	planeMesh->setHeight(2);
	// Plane transform
	Qt3DCore::QTransform* planeTransform = new Qt3DCore::QTransform();
	planeTransform->setScale(5.0f);
	planeTransform->setTranslation(QVector3D(10.0f, 0.0f, 10.0f));
	// Plane material
	Qt3DExtras::QPhongMaterial* planeMaterial = new Qt3DExtras::QPhongMaterial();
	planeMaterial->setDiffuse(QColor(65, 205, 82));
	// Plane
	Qt3DCore::QEntity* planeEntity = new Qt3DCore::QEntity(rootEntity);
	planeEntity->addComponent(planeMesh);
	planeEntity->addComponent(planeMaterial);
	planeEntity->addComponent(planeTransform);

	// (x��,y��,z��)
	// Axis entity
	Qt3DCore::QEntity* axisEntity = new Qt3DCore::QEntity(rootEntity);

	// X-axis
	// X-axis mesh
	Qt3DExtras::QCylinderMesh* xAxisMesh = new Qt3DExtras::QCylinderMesh();
	xAxisMesh->setRadius(0.05f);
	xAxisMesh->setLength(20.0f);
	// X-axis material
	Qt3DExtras::QPhongMaterial* xAxisMaterial = new Qt3DExtras::QPhongMaterial();
	xAxisMaterial->setAmbient(Qt::red); //color
	// X-axis transform
	Qt3DCore::QTransform* xAxisTransform = new Qt3DCore::QTransform();
	xAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f));
	xAxisTransform->setTranslation(QVector3D(10.0f, 0.0f, 0.0f));
	// X-axis entity
	Qt3DCore::QEntity* xAxisEntity = new Qt3DCore::QEntity(axisEntity);
	xAxisEntity->addComponent(xAxisMesh);
	xAxisEntity->addComponent(xAxisMaterial);
	xAxisEntity->addComponent(xAxisTransform);

	// Y-axis
	// Y-axis mesh
	Qt3DExtras::QCylinderMesh* yAxisMesh = new Qt3DExtras::QCylinderMesh();
	yAxisMesh->setRadius(0.05f);
	yAxisMesh->setLength(20.0f);
	// Y-axis material
	Qt3DExtras::QPhongMaterial* zXxisMaterial = new Qt3DExtras::QPhongMaterial();
	zXxisMaterial->setAmbient(Qt::yellow); //color
	// Y-axis transform
	Qt3DCore::QTransform* yAxisTransform = new Qt3DCore::QTransform();
	yAxisTransform->setTranslation(QVector3D(0.0f, 10.0f, 0.0f));
	// Y-axis entity
	Qt3DCore::QEntity* yAxisEntity = new Qt3DCore::QEntity(axisEntity);
	yAxisEntity->addComponent(yAxisMesh);
	yAxisEntity->addComponent(zXxisMaterial);
	yAxisEntity->addComponent(yAxisTransform);

	// Z-axis
	// Z-axis mesh
	Qt3DExtras::QCylinderMesh* zAxisMesh = new Qt3DExtras::QCylinderMesh();
	zAxisMesh->setRadius(0.05f);
	zAxisMesh->setLength(20.0f);
	// Z-axis material
	Qt3DExtras::QPhongMaterial* zAxisMaterial = new Qt3DExtras::QPhongMaterial();
	zAxisMaterial->setAmbient(Qt::blue); //color
	// Z-axis transform
	Qt3DCore::QTransform* zAxisTransform = new Qt3DCore::QTransform();
	zAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), -90.0f));
	zAxisTransform->setTranslation(QVector3D(0.0f, 0.0f, 10.0f));
	// Z-axis entity
	Qt3DCore::QEntity* zAxisEntity = new Qt3DCore::QEntity(axisEntity);
	zAxisEntity->addComponent(zAxisMesh);
	zAxisEntity->addComponent(zAxisMaterial);
	zAxisEntity->addComponent(zAxisTransform);

	// Show window
	widget->show();
	widget->resize(500, 500);

	//scene->addWidget(widget);
}
