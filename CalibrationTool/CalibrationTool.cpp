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
	/*初始化*/
	ui.setupUi(this);
	initImageList();
	timer = new QTimer(this);

	/*信号和槽*/
	connect(timer, SIGNAL(timeout()), this, SLOT(readFarme()));  // 时间到，读取当前摄像头信息
	connect(ui.openCam, SIGNAL(clicked()), this, SLOT(openCamara()));
	connect(ui.takePic, SIGNAL(clicked()), this, SLOT(takingPictures()));
	connect(ui.closeCam, SIGNAL(clicked()), this, SLOT(closeCamara()));
	connect(ui.calib, SIGNAL(clicked()), this, SLOT(startCalibrate()));
	connect(ui.open, SIGNAL(clicked()), this, SLOT(fileOpenActionSlot()));

	// connect(ui.imageList);
	connect(ui.imageList, &QListWidget::itemClicked, this, &CalibrationTool::handleListItemClick);

	// 画条形图和三维图
	createBarChart();
	createPatternCentric();
}

CalibrationTool::~CalibrationTool()
{}


void CalibrationTool::initImageList() {
	//createAction();
	//createMenu();
	//createContentMenu();
	//定义QListWidget对象
	//设置QListWidget的显示模式
	ui.imageList->setViewMode(QListView::ListMode);
	//设置QListWidget中单元项的图片大小
	ui.imageList->setIconSize(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH));
	//设置QListWidget中单元项的间距
	ui.imageList->setSpacing(IMAGE_PADDING);
	//设置自动适应布局调整(Adjust适应，Fixed不适应)，默认不适应
	ui.imageList->setResizeMode(QListWidget::Adjust);
	//设置不能移动
	ui.imageList->setMovement(QListWidget::Static);
	//setAttribute(Qt::WA_Hover, true)
	ui.imageList->setAttribute(Qt::WA_Hover, true);
}
void CalibrationTool::openCamara()
{
	cam = cv::VideoCapture();//打开摄像头，从摄像头中获取视频
	cam.open(1);
	if (!cam.isOpened()) {
		cam.open(0);
	}
	ui.openCam->setIcon(QIcon(":/picture/picture/openning.png"));
	// 设置每一帧的大小为指定值
	cam.set(cv::CAP_PROP_FRAME_WIDTH, IMAGE_WIN_WIDTH);
	cam.set(cv::CAP_PROP_FRAME_HEIGHT, IMAGE_WIN_HEIGHT);
	timer->start(SAMPLE_RATE + 3);              // 开始计时，超时则发出timeout()信号
}

/*********************************
********* 读取摄像头信息 ***********
**********************************/
void CalibrationTool::readFarme()
{
	cam >> frame;// 从摄像头中抓取并返回每一帧
	cv::Mat flipedFrame;
	flip(frame, flipedFrame, 1);
	// 将颜色格式从BGR转换为RGB
	cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
	// 将抓取到的帧，转换为QImage格式。QImage::Format_RGB888不同的摄像头用不同的格式。
	QImage image(flipedFrame.data, flipedFrame.cols, flipedFrame.rows, flipedFrame.step, QImage::Format_RGB888);
	//ui.imageWindow->setPixmap(QPixmap::fromImage(image));  // 将图片显示到label上
	//创建显示容器
	QGraphicsScene* scene = new QGraphicsScene;
	//向容器中添加文件路径为fileName（QString类型）的文件
	scene->addPixmap(QPixmap::fromImage(image));
	//借助graphicsView（QGraphicsView类）控件显示容器的内容
	ui.imageWindow->setScene(scene);
	//开始显示
	ui.imageWindow->show();
}


void CalibrationTool::takingPictures()
{
	if (!cam.isOpened()) {
		QMessageBox::warning(this, tr("warning"), tr("please open camera"), QMessageBox::Ok);
		return;
	}
	cam >> frame;// 从摄像头中抓取并返回每一帧
	cv::Mat flipedFrame;
	flip(frame, flipedFrame, 1);
	cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);

	// 这里设置Mat在map里的key为imageNameMap的size
	this->camImageMap[this->imageNameMap.size()] = flipedFrame;
	this->imageNameMap[this->imageNameMap.size()] = "";

	QImage image((const uchar*)flipedFrame.data, flipedFrame.size().width, flipedFrame.size().height, QImage::Format_RGB888);
	QPixmap pixmap = QPixmap::fromImage(image);
	double ratio = static_cast<double>(image.height()) / image.width();
	QListWidgetItem* item = new QListWidgetItem();
	item->setIcon(QIcon(pixmap)); // 或者 item->setIcon(QIcon::fromImage(image));
	item->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
	item->setText(QString::number(this->imageNameMap.size()));

	QDateTime currentDateTime = QDateTime::currentDateTime(); // 获取当前时间
	qint64 timestamp = currentDateTime.toSecsSinceEpoch(); // 转换为时间戳（秒级别）
	QString timestampText = QString::number(timestamp); // 将时间戳转换为字符串

	item->setTextAlignment(Qt::AlignVCenter);
	item->setToolTip(timestampText + ".tmp.png");

	ui.imageList->addItem(item);

	// 拍照后检测角点
	this->imageCorners.push_back(findOneCorners(flipedFrame, cv::BOARD_SIZE));
}


/*******************************
***关闭摄像头，释放资源，必须释放***
********************************/
void CalibrationTool::closeCamara()
{
	ui.openCam->setIcon(QIcon(":/picture/picture/camera.png"));
	timer->stop(); // 停止读取数据。
	cam.release();
	QGraphicsScene* scene = new QGraphicsScene;
	ui.imageWindow->setScene(scene);
	//开始显示
	ui.imageWindow->show();
	//ui.imageWindow->clear();
}


void CalibrationTool::startCalibrate() {
	// 弹窗提示未添加图片
	if (this->fileNames.length() == 0) {
		QMessageBox::warning(this, tr("warning"),
			tr("You haven't upload any image"));

		return;
	}
	// 当图片小于10张的时候，提示是否继续标定
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
// 在窗口类的实现文件中实现槽函数来更新进度条
void CalibrationTool::updateProgress(int value)
{
	// 更新进度条的值
	this->progressBar->setValue(value);
}


void CalibrationTool::fileOpenActionSlot()
{
	selectFile();
}

// 创建进度条
void CalibrationTool::createProgressBar(bool isBatch) {
	// 创建进度条对话框
	// this->progressBar = new QProgressDialog ("Checking corner points...", "Cancel", 0, 100);
	// this->progressBar->setWindowTitle("Progress");
	if (!this->progressBar)
	{
		this->progressBar = new QProgressDialog("Checking corner points...", nullptr, 0, 100);
		this->progressBar->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
		this->progressBar->setWindowModality(Qt::WindowModal);
		this->progressBar->setMinimumDuration(0);
		this->progressBar->setCancelButton(nullptr); // 移除取消按钮
		Qt::WindowFlags flags = this->progressBar->windowFlags();
		flags &= ~Qt::WindowCloseButtonHint;  // 移除关闭按钮
		this->progressBar->setWindowFlags(flags);

		// 设置固定的宽度和高度
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
* Qt中使用文件选择对话框步骤如下:
* 1.定义一个QFileDialog对象
* 2.设置路径、过滤器等属性
******************************************/
void CalibrationTool::selectFile() {
	//定义文件对话框类
	QFileDialog* fileDialog = new QFileDialog(this);
	//定义文件对话框标题
	fileDialog->setWindowTitle(tr("open fold"));
	//设置默认文件路径
	fileDialog->setDirectory(".");
	//设置文件过滤器
	fileDialog->setNameFilter(tr("Images(*.png *.jpg *.jpeg * .bmp)"));
	//设置可以选择多个文件，默认为只能选择一个文件QF ileDialog: :ExistingFiles
	fileDialog->setFileMode(QFileDialog::ExistingFiles);
	//设置视图模式
	fileDialog->setViewMode(QFileDialog::Detail);
	//打印所有选择的文件的路径
	if (fileDialog->exec())
	{
		// 选择图片上传的时候检测
		// this->createProgressBar(false);

		// 这里每次都会重新赋值，所以下面用map存
		fileNames = fileDialog->selectedFiles();
		int len = this->imageNameMap.size();
		for (int i = 0; i < fileNames.length(); i++) {
			fileNames[i] = QDir::toNativeSeparators(fileNames[i]);
			this->imageNameMap[i + len] = fileNames[i];
			// 在上传图片的时候检测角点
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
*Qt中使用文件选择对话框步骤如下:
* 1.定义一个QListWidget对象
* 2.设置ViewMode等属性
* 3.定义单元项并添加到QL istWidget中
* 4.调用QListWidget对象的show()方法
* ****************************************/
void CalibrationTool::showImageList() {

	int stIndex = this->imageCorners.size();
	this->createProgressBar(false);
	for (int i = 0; i < fileNames.length(); i++) {
		// 定义QListWidgetItem对象
		QListWidgetItem* imageItem = new QListWidgetItem;
		QImage image(fileNames[i]);
		double ratio = static_cast<double>(image.height()) / image.width();
		// qDebug() << height << endl;
		imageItem->setIcon(QIcon(fileNames[i]));
		//重新设置单元项图片的宽度和高度
		imageItem->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
		imageItem->setText(QString::number(stIndex + i + 1));

		// 悬浮显示文件名
		QFileInfo fileInfo(fileNames[i]);
		QString fileName = fileInfo.fileName();
		imageItem->setTextAlignment(Qt::AlignVCenter);
		imageItem->setToolTip(fileName);
		ui.imageList->addItem(imageItem);

		//在上传图片的时候检测角点
		this->imageCorners.push_back(findOneCorners(fileNames[i], cv::BOARD_SIZE));
		this->progressBar->setValue((i + 1) * 100 / fileNames.length());

	}
	//this->progressBar->setValue(100);

	//显示QListWidget
	ui.imageList->show();
	// 批量检测角点
	// this->createProgressBar(true);
	// this->imageCorners = findCorners(fileNames,BOARD_SIZE,this);

}

void CalibrationTool::handleListItemClick(QListWidgetItem* item)
{
	// 处理 QListWidgetItem 的点击事件
	// 可以获取 item 的数据、索引等进行处理
	// 示例：获取 item 的文本
	//int index = item->text().toInt();
	int index = ui.imageList->row(item);

	qDebug() << "Clicked item text: " << index << "\n";
	qDebug() << this->imageCorners.size() << "\n";
	vector<cv::Point2f> corners = this->imageCorners[index];
	QString fileName = this->imageNameMap[index];
	cv::Mat flipedFrame;
	if (!fileName.isEmpty()) {
		flipedFrame = cv::imread(fileName.toStdString());
		// 将颜色格式从BGR转换为RGB
		cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
	}
	else
	{
		flipedFrame = this->camImageMap[index];
	}
	cv::drawChessboardCorners(flipedFrame, cv::Size(9, 6), corners, !corners.empty());

	// 将抓取到的帧，转换为QImage格式。QImage::Format_RGB888不同的摄像头用不同的格式。
	QImage image(flipedFrame.data, flipedFrame.cols, flipedFrame.rows, flipedFrame.step, QImage::Format_RGB888);
	//创建显示容器
	QGraphicsScene* scene = new QGraphicsScene;
	//向容器中添加文件路径为fileName（QString类型）的文件
	scene->addPixmap(QPixmap::fromImage(image));
	//借助graphicsView（QGraphicsView类）控件显示容器的内容
	ui.imageWindow->setScene(scene);
	//开始显示
	ui.imageWindow->show();
}


/***************************************
*Qt中使用QCharts画条形图步骤如下:
* ****************************************/
void CalibrationTool::createBarChart() {
	// 条形图数据
	QBarSet* projectionError = new QBarSet("Projection Error");
	vector<double> projectionError_ = { 3.0032143514447018e-01, 2.5005490108190759e-01, 2.2378858466658030e-01,
				   1.6412628748340338e-01, 1.9050650570901181e-01, 1.8053703768600146e-01,
				   2.1210603721570268e-01, 2.4443632141393190e-01, 2.6233146806962876e-01 };
	// 画均值线
	// 计算数据的均值
	double mean = 0;
	for (int i = 0; i < projectionError_.size(); ++i) {
		*projectionError << projectionError_[i];
		mean += projectionError_[i];
	}

	mean /= projectionError_.size();
	// 创建QBarSeries
	QBarSeries* series = new QBarSeries();
	series->append(projectionError);

	// Set the width of the bars
	qreal barWidth = 1.0; // Adjust the value to your desired width
	series->setBarWidth(barWidth);


	QChart* chart = new QChart();
	chart->addSeries(series);
	chart->setTitle("Projection Error");
	chart->setAnimationOptions(QChart::SeriesAnimations);
	//chart->setBackgroundBrush(Qt::blue);


	// 用于显示图表的窗口部件类
	QChartView* chartView = new QChartView(chart);
	chartView->setRenderHint(QPainter::Antialiasing);

	// XY轴标签
	QBarCategoryAxis* axisX = new QBarCategoryAxis();
	axisX->setTitleText("Images");
	//axisX->append(QStringList() << "0" << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8"); // 添加X轴标签
	chart->addAxis(axisX, Qt::AlignBottom);

	QValueAxis* axisY = new QValueAxis();
	//axisY->setRange(0, 15);
	axisY->setTitleText("Mean Erros in Pixels");
	chart->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisY);

	chart->legend()->setVisible(false);
	//chart->legend()->setAlignment(Qt::AlignBottom); /* 显示在底部 */


	// 创建一个QLineSeries对象来绘制均值线
	QtCharts::QLineSeries* meanLine = new QtCharts::QLineSeries;
	meanLine->setName("Mean");
	meanLine->append(0, mean); // 添加起始点
	meanLine->append(projectionError_.size() - 1, mean); // 添加结束点

	// 将QLineSeries对象添加到QChart对象中
	chart->addSeries(meanLine);

	// 将QLineSeries对象绑定到X轴和Y轴
	meanLine->attachAxis(axisX);
	meanLine->attachAxis(axisY);


	// 展示图表
	QGraphicsView* histogramView = ui.histogram; // histogram 是之前在 UI 文件中定义的 QGraphicsView 组件
	QGraphicsScene* scene = new QGraphicsScene(histogramView); // 创建一个场景对象，关联到 histogramView 组件
	// 获取 histogram 组件的位置和尺寸
	QRect histogramGeometry = ui.histogram->geometry();

	// 将 chartView 的位置和尺寸设置为与 histogram 相同
	chartView->setGeometry(histogramGeometry);
	chartView->resize(235, 235);


	histogramView->setScene(scene);
	scene->addWidget(chartView); // 将 chartView 添加到场景中
}

/***************************************
*Qt中使用QCharts画createPatternCentric图步骤如下:
* ****************************************/
void CalibrationTool::createPatternCentric() {

	//// 每张图片的外参
	//vector<cv::Mat> R;
	//vector<cv::Mat> t;
	//// 畸变系数
	//double D_[] = { -3.4351280917484162e-01, 1.5909766895881644e-01, -1.2375087184036587e-06, 7.4996411884060586e-04, -4.1226886540150574e-02 };
	//cv::Mat D = cv::Mat(1, 5, CV_64F, D_); // 创建一个空的 double 类型矩阵

	//// 定义旋转矩阵 R 和三维向量 v
	//cv::Mat R_ = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, 1, 0, 0, 0, 1); // 示例旋转矩阵
	//cv::Mat v_ = (cv::Mat_<double>(3, 1) << 1, 2, 3); // 示例三维向量

	//// 计算三维坐标
	//cv::Mat result = R_ * v_;

	//// 输出结果
	//std::cout << "Result:\n" << result << std::endl;
	// 打印矩阵内容
	//cout << D.at<double>(1, 2) << endl;

	// 展示图表
	QGraphicsView* transformView = ui.transformGram; // histogram 是之前在 UI 文件中定义的 QGraphicsView 组件
	QGraphicsScene* scene = new QGraphicsScene(transformView); // 创建一个场景对象，关联到 histogramView 组件

	// 创建 3D 窗口
	Qt3DExtras::Qt3DWindow* window = new Qt3DExtras::Qt3DWindow;
	window->defaultFrameGraph()->setClearColor(Qt::white);

	// 创建根实体
	Qt3DCore::QEntity* rootEntity = new Qt3DCore::QEntity;

	//// 创建相机
	//Qt3DRender::QCamera* camera = window.camera();
	//camera->setPosition(QVector3D(0, 0, 100));
	//camera->setViewCenter(QVector3D(0, 0, 0));

	// 创建坐标系
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

	//// 创建长方体
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

	// 创建材质
	Qt3DExtras::QPhongMaterial* material = new Qt3DExtras::QPhongMaterial;
	material->setDiffuse(Qt::red); // 设置材质颜色

	// 创建实体组件并添加材质
	Qt3DCore::QEntity* coordinateSystem = new Qt3DCore::QEntity(rootEntity);
	coordinateSystem->addComponent(xAxis);
	coordinateSystem->addComponent(yAxis);
	coordinateSystem->addComponent(zAxis);
	coordinateSystem->addComponent(material);

	// 设置场景根实体
	window->setRootEntity(rootEntity);

	// 将 3D 视图嵌入到 QGraphicsView
	QWidget* container = QWidget::createWindowContainer(window);
	transformView->setViewport(container);
	transformView->setRenderHint(QPainter::Antialiasing);
	transformView->show();
}
