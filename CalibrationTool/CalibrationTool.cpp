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

    // ui.imageList
    connect(ui.imageList, &QListWidget::itemClicked, this, &CalibrationTool::handleListItemClick);
    // 右键菜单绑定点击事件
    connect(this->action_Delete_In_ListWidget_, SIGNAL(triggered()), this, SLOT(onActionDelete()));
    connect(this->action_Clear_In_ListWidget_, SIGNAL(triggered()), this, SLOT(onActionClear()));
    connect(this->action_Delete_And_ReCalibrate_In_ListWidget_, SIGNAL(triggered()), this, SLOT(onActionRemoveAndReCalibrate()));
    // 绑定右键显示菜单：在单击右键之后会执行槽函数， 槽函数中负责弹出右键菜单
    ui.imageList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui.imageList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenuRequested(const QPoint&)));

    createBarChart();
}

CalibrationTool::~CalibrationTool()
{}


void CalibrationTool::onCustomContextMenuRequested(const QPoint& pos)
{
    /*弹出右键菜单*/
    popMenu_In_ListWidget_->exec(QCursor::pos());
}

void CalibrationTool::onActionDelete()
{
    /*获取当前选中的Item*/
    QList<QListWidgetItem*> items = ui.imageList->selectedItems();
    if (items.count() > 0)
    {
        int index = ui.imageList->row(items[0]);
        /*弹出询问对话框*/
        if (QMessageBox::Yes == QMessageBox::question(this, QStringLiteral("Remove Item"),
            QStringLiteral("Remove %1 item").arg(QString::number(items.count())), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
        {
            qDebug() << "remove " << index << endl;
            foreach(QListWidgetItem * var, items) {
                ui.imageList->removeItemWidget(var);
                items.removeOne(var);
                delete var;
            }
            QGraphicsScene* scene = new QGraphicsScene;
            ui.imageWindow->setScene(scene);
            ui.imageWindow->show();
            this->imageCorners.erase(this->imageCorners.begin() + index);
            this->imageNameList.erase(this->imageNameList.begin() + index);
            this->imageMatList.erase(this->imageMatList.begin() + index);
        }
    }
}

void CalibrationTool::onActionClear() {
    if (QMessageBox::Yes == QMessageBox::question(this, QStringLiteral("Clear All"),
        QStringLiteral("Remove All %1 items？").arg(QString::number(imageCorners.size())), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
    {
        ui.imageList->clear();
        this->imageCorners.clear();
        this->imageNameList.clear();
        this->imageMatList.clear();
        this->maxNameIndex = 0;
        this->calibResults = CalibrateResults();
        QGraphicsScene* scene = new QGraphicsScene;
        ui.imageWindow->setScene(scene);
        ui.imageWindow->show();
        // 先取消绑定，然后再绑定
        popMenu_In_ListWidget_->removeAction(action_Delete_And_ReCalibrate_In_ListWidget_);
        popMenu_In_ListWidget_->removeAction(action_Delete_In_ListWidget_);

        popMenu_In_ListWidget_->addAction(action_Delete_In_ListWidget_);
        popMenu_In_ListWidget_->addAction(action_Clear_In_ListWidget_);

        createBarChart();
    }
}

void CalibrationTool::onActionRemoveAndReCalibrate() {
    QList<QListWidgetItem*> items = ui.imageList->selectedItems();
    if (items.count() > 0)
    {
        int index = ui.imageList->row(items[0]);
        qDebug() << "remove " << index << "  and calib" << endl;
        if (QMessageBox::Yes == QMessageBox::question(this, QStringLiteral("Remove And Calibrate"),
            QStringLiteral("Remove %1 and calibrate").arg(QString::number(index)), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes))
        {
            foreach(QListWidgetItem * var, items) {
                ui.imageList->removeItemWidget(var);
                items.removeOne(var);
                delete var;
            }

            QGraphicsScene* scene = new QGraphicsScene;
            ui.imageWindow->setScene(scene);
            ui.imageWindow->show();

            this->imageCorners.erase(this->imageCorners.begin() + index);
            this->imageNameList.erase(this->imageNameList.begin() + index);
            this->imageMatList.erase(this->imageMatList.begin() + index);

            this->calcSizeAndCalib();
        }
    }
}

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

    // 右键菜单的初始化
    popMenu_In_ListWidget_ = new QMenu(this);
    action_Delete_In_ListWidget_ = new QAction(tr("Delete"), this);
    action_Clear_In_ListWidget_ = new QAction(tr("ClearAll"), this);
    action_Delete_And_ReCalibrate_In_ListWidget_ = new QAction(tr("Delete And Recalibrate"), this);
    popMenu_In_ListWidget_->addAction(action_Delete_In_ListWidget_);
    popMenu_In_ListWidget_->addAction(action_Clear_In_ListWidget_);
}
void CalibrationTool::calcSizeAndCalib() {
    bool find = false;
    cv::Size imgSize;
    for (int i = 0; i < this->fileNames.size(); i++) {
        if (!this->fileNames[i].isEmpty()) {
            imgSize = cv::imread(this->fileNames[0].toStdString()).size();
            find = true;
            break;
        }
    }
    if (!find) {
        imgSize.height = IMAGE_WIN_HEIGHT;
        imgSize.width = IMAGE_WIN_WIDTH;
    }

    qDebug() << "select mode " << endl;
    qDebug() << ui.fisheyeMode->isChecked() << endl;
    qDebug() << ui.standardMode->isChecked() << endl;
    if (ui.fisheyeMode->isChecked()) {
        this->calibResults = calibarteWithCorners(this->imageCorners, imgSize, cv::BOARD_SIZE, FISH_EYE_CAM);
    }
    else {
        this->calibResults = calibarteWithCorners(this->imageCorners, imgSize, cv::BOARD_SIZE, NORMAL_CAM);
    }
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

    // 这里设置拍照时将图片加入两个list
    this->imageMatList.push_back(flipedFrame);
    this->imageNameList.push_back("");
    QImage image((const uchar*)flipedFrame.data, flipedFrame.size().width, flipedFrame.size().height, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);
    double ratio = static_cast<double>(image.height()) / image.width();
    QListWidgetItem* item = new QListWidgetItem();
    item->setIcon(QIcon(pixmap)); // 或者 item->setIcon(QIcon::fromImage(image));
    item->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
    item->setText(QString::number(++this->maxNameIndex));

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
    if (this->imageCorners.size() == 0) {
        QMessageBox::warning(this, tr("warning"),
            tr("You haven't upload any avaliable image"));

        return;
    }
    // 当图片小于10张的时候，提示是否继续标定
    if (this->imageCorners.size() <= 10) {
        int reply = QMessageBox::warning(this, tr("warning"),
            tr("You should upload more than 10 avaliable images, would you want to continue?"),
            QMessageBox::Yes, QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }
    this->calcSizeAndCalib();

    // 先取消原本的绑定，然后再绑定新的
    if (!popMenu_In_ListWidget_->actions().contains(action_Delete_And_ReCalibrate_In_ListWidget_))
    {
        popMenu_In_ListWidget_->removeAction(action_Delete_In_ListWidget_);
        popMenu_In_ListWidget_->removeAction(action_Clear_In_ListWidget_);

        popMenu_In_ListWidget_->addAction(action_Delete_And_ReCalibrate_In_ListWidget_);
        popMenu_In_ListWidget_->addAction(action_Clear_In_ListWidget_);
    }

    // 画条形图和三维图
    createBarChart();
    createPatternCentric();
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

        fileNames = fileDialog->selectedFiles();
        for (int i = 0; i < fileNames.length(); i++) {
            fileNames[i] = QDir::toNativeSeparators(fileNames[i]);
            this->imageNameList.push_back(fileNames[i]);
            cv::Mat emptyMat;
            this->imageMatList.push_back(emptyMat);
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
    // int stIndex = this->imageCorners.size();
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
        imageItem->setText(QString::number(maxNameIndex + 1));

        // 悬浮显示文件名
        QFileInfo fileInfo(fileNames[i]);
        QString fileName = fileInfo.fileName();
        imageItem->setTextAlignment(Qt::AlignVCenter);
        imageItem->setToolTip(fileName);
        ui.imageList->addItem(imageItem);

        //在上传图片的时候检测角点
        this->imageCorners.push_back(findOneCorners(fileNames[i], cv::BOARD_SIZE));
        this->progressBar->setValue((i + 1) * 100 / fileNames.length());
        this->maxNameIndex++;
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

    this->clickToShow(index);

}
void CalibrationTool::clickToShow(int index) {
    qDebug() <<"total image num is" << this->imageCorners.size() << "  Clicked item text: " << index << "\n";
    vector<cv::Point2f> corners = this->imageCorners[index];
    QString fileName = this->imageNameList[index];
    cv::Mat flipedFrame;
    if (!fileName.isEmpty()) {
        flipedFrame = cv::imread(fileName.toStdString());
        // 将颜色格式从BGR转换为RGB
        cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
    }
    else
    {
       // flipedFrame = this->camImageMap[index];
        flipedFrame = this->imageMatList[index];
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
*** Qt中使用QCharts画条形图BarChart ***
*****************************************/

void CalibrationTool::createBarChart() {
    // 条形图数据
    QBarSet* projectionError = new QBarSet("Projection Error");
    /*vector<double> projectionError_ = {
                   3.0032143514447018e-01, 2.5005490108190759e-01, 2.2378858466658030e-01,
                   1.6412628748340338e-01, 1.9050650570901181e-01, 1.8053703768600146e-01,
                   2.1210603721570268e-01, 2.4443632141393190e-01, 3.0032143514447018e-01,
                   2.5005490108190759e-01, 2.2378858466658030e-01, 3.0032143514447018e-01,
                   1.6412628748340338e-01, 1.9050650570901181e-01, 1.8053703768600146e-01,
                   2.1210603721570268e-01, 2.4443632141393190e-01, 2.6233146806962876e-01 };*/

    vector<double> projectionError_ = calibResults.reprojectionError;
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
    // 创建一个自定义的点击处理函数
    QObject::connect(series, &QBarSeries::clicked, this, [this](int index, QBarSet* barSet) {
        int realIndex = 0;
        qDebug() << "Clicked on bar:" << barSet->label() << "at index:" << index;
        for (int i = 0; i < this->imageCorners.size(); i++) {
            if (!imageCorners[i].empty()) {
                qDebug() << "not empty" << endl;
                qDebug() << "realIndex" << realIndex << endl;
                if (realIndex == index) {
                    this->clickToShow(i);
                    break;
                }
                realIndex++;
            }
            else {
                qDebug() << "empty" << endl;
            }
              
        }
        // 执行点击事件的处理逻辑
        });
    series->append(projectionError);

    // Set the width of the bars
    qreal barWidth = 1.0; // Adjust the value to your desired width
    series->setBarWidth(barWidth);


    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Projection Error");
    chart->setAnimationOptions(QChart::SeriesAnimations);


    // 创建一个字体对象，并设置字体大小
    QFont fontX, fontY;
    fontX.setPointSize(CHART_FONT_SIZE); // 设置字体大小为10
    fontY.setPointSize(CHART_FONT_SIZE);

    // XY轴标签
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    // axisX->setTitleText("Images");
    int skipNum = projectionError_.size() / 9 + 1;
    for (int i = 0; i < projectionError_.size(); i += skipNum) {
        axisX->append(QString::number(i + 1));
    }
    axisX->setLabelsFont(fontX);
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis* axisY = new QValueAxis();
    // 设置Y轴刻度标签的角度为斜着显示
    axisY->setLabelsAngle(-20);

    // 设置刻度标签的格式，例如使用QString::number函数保留一位小数
    axisY->setLabelFormat("%.2f");
    // 设置Y轴刻度标签的字体
    axisY->setLabelsFont(fontY);

    // axisY->setTitleText("Mean Erros in Pixels");
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
    //QGraphicsView* histogramView = ui.histogram; // histogram 是之前在 UI 文件中定义的 QGraphicsView 组件
    QGraphicsScene* scene = new QGraphicsScene(ui.histogram); // 创建一个场景对象，关联到 histogramView 组件
    // 获取 histogram 组件的位置和尺寸
    QRect histogramGeometry = ui.histogram->geometry();
    histogramGeometry.setWidth(histogramGeometry.width() + CHART_EXPEND);
    histogramGeometry.setHeight(histogramGeometry.height() + CHART_EXPEND);
    // 将 chartView 的位置和尺寸设置为与 histogram 相同
    chart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 用于显示图表的窗口部件类
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    chartView->setGeometry(histogramGeometry);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 设置图表的背景透明
    chartView->setAutoFillBackground(true);

    ui.histogram->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.histogram->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 调整场景的大小以匹配histogramView的几何形状
    scene->setSceneRect(histogramGeometry);

    scene->addWidget(chartView); // 将 chartView 添加到场景中

    ui.histogram->setScene(scene);

    // 创建一个自定义的悬停处理函数
    QObject::connect(series, &QBarSeries::hovered, this, [this, series, chart, chartView](bool status, int index, QBarSet* barset) {
        //鼠标指向图表柱时提示数值文本
        QChart* pchart = chart;
        if (this->m_tooltip == nullptr)
        {
            m_tooltip = new  QLabel(chartView);    //头文件中的定义 QLabel*   m_tooltip = nullptr;  //柱状体鼠标提示信息
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
*** Qt中使用Qt3D画三维场景图 ***
*****************************************/
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


    // 展示图表
    QGraphicsView* transformView = ui.transformGram; // histogram 是之前在 UI 文件中定义的 QGraphicsView 组件
    QGraphicsScene* scene = new QGraphicsScene(transformView); // 创建一个场景对象，关联到 histogramView 组件

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

    // (x→,y↑,z●)
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
