#include "CalibrationTool.h"
#include "Const.h"
#include "Calibrate.h"

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
    connect(ui.changePicMode, SIGNAL(clicked()), this, SLOT(changeShowUndistorted()));

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
    //createPatternCentric();
    //createPatternCentric2();
    createLoading();
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
        this->undistortedImageList.clear();
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
        const Qt3DCore::QComponentVector components = this->rootEntity->components();

        // 逐个移除所有的组件
        for (Qt3DCore::QComponent* component : components) {
            qDebug() << "iter all component " << endl;
            rootEntity->removeComponent(component);
        }
        this->showUndistored = false;
        ui.changePicMode->setIcon(QIcon(":/picture/picture/distortedChess.png"));
        createBarChart();
        for (int i = 0; i < this->cuboids.size(); i++) {
            this->cuboids[i]->setEnabled(false);
        }
        this->cuboids.clear();
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
            this->undistortedImageList.clear();
            this->showUndistored = false;
            ui.changePicMode->setIcon(QIcon(":/picture/picture/distortedChess.png"));
            this->calcSizeAndCalib();
            this->createBarChart();
            for (int i = 0; i < this->cuboids.size(); i++) {
                this->cuboids[i]->setEnabled(false);
            }
            this->cuboids.clear();
            //this->createPatternCentric2();
            this->addCuboidToCentric();
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
    // ui.loadingLabel->setVisible(true);
    this->calcSizeAndCalib();
    this->clickToUndistort();
    //ui.loadingLabel->setVisible(false);

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
    if (rootEntity != nullptr) {

        delete rootEntity;
    }
    if (view3D != nullptr) {

        delete view3D;
    }
    createPatternCentric();
    addCuboidToCentric();
}
// 在窗口类的实现文件中实现槽函数来更新进度条
void CalibrationTool::updateProgress(int value)
{
    // 更新进度条的值
    this->progressBar->setValue(value);
}


void CalibrationTool::createLoading() {
    QMovie* m_pLoadingMovie = new QMovie(":/picture/picture/loading.gif");
    m_pLoadingMovie->setScaledSize(QSize(120, 120));
    QFrame* m_pCenterFrame = new QFrame(this);
    m_pCenterFrame->setGeometry(200, 200, 230, 230);
    ui.loadingLabel->setScaledContents(true);
    ui.loadingLabel->setMovie(m_pLoadingMovie);
    m_pLoadingMovie->start();
    ui.loadingLabel->setVisible(false);
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
            cv::Mat flipedFrame = cv::imread(fileNames[i].toStdString());
            // 将颜色格式从BGR转换为RGB
            cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
            this->imageMatList.push_back(flipedFrame);
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

void CalibrationTool::changeShowUndistorted() {
    if (this->showUndistored)
    {
        qDebug() << "this->showUndistored = false";
        this->showUndistored = false;
        ui.changePicMode->setIcon(QIcon(":/picture/picture/distortedChess.png"));
        //切换图片为鱼眼样式
    }
    else
    {
        if (calibResults.distCoeffs.empty())
        {
            qDebug() << "this->showUndistored = empty";
            return;
        }
        qDebug() << "this->showUndistored = true";
        ui.changePicMode->setIcon(QIcon(":/picture/picture/undistortedChess.png"));
        this->showUndistored = true;
        //切换图片为正常样式
    }
    clickToShow(clickedIndex);
}

void CalibrationTool::clickToShow(int index) {
    clickedIndex = index;
    qDebug() << "total image num is" << this->imageCorners.size() << "  Clicked item text: " << index << "\n";
    vector<cv::Point2f> corners = this->imageCorners[index];
    QString fileName = this->imageNameList[index];
    cv::Mat flipedFrame;
    qDebug() << "cube size " << this->cuboids.size() << endl;
    if (!this->calibResults.rvecs.empty()) {
        if (!this->imageCorners[index].empty()) {
            int realIndex = 0;
            for (int i = 0; i < this->imageCorners.size(); i++) {
                if (!imageCorners[i].empty()) {
                    if (realIndex == index) {
                        break;
                    }
                    realIndex++;
                }
            }
            qDebug() << "real show cube " << realIndex-- << endl;
            for (int i = 0; i < this->cuboids.size(); i++) {
                if (i == realIndex) {
                    Qt3DCore::QEntity* cube = this->cuboids[i];
                    Qt3DExtras::QPhongMaterial* cuboidMaterial = new Qt3DExtras::QPhongMaterial();
                    cuboidMaterial->setAmbient(CHOSEN_ENTITY_COLOR);
                    cube->addComponent(cuboidMaterial);
                }
                else {
                    Qt3DCore::QEntity* cube = this->cuboids[i];
                    Qt3DExtras::QPhongMaterial* cuboidMaterial = new Qt3DExtras::QPhongMaterial();
                    cuboidMaterial->setAmbient(DEFAULT_ENTITY_COLOR);
                    cube->addComponent(cuboidMaterial);
                }
            }
        }
        else {
            for (int i = 0; i < this->cuboids.size(); i++) {

                Qt3DCore::QEntity* cube = this->cuboids[i];
                Qt3DExtras::QPhongMaterial* cuboidMaterial = new Qt3DExtras::QPhongMaterial();
                cuboidMaterial->setAmbient(QColor(102, 255, 255, 0.1));
                cube->addComponent(cuboidMaterial);

            }
        }

    }
    // 获取点击的图片
    if (!this->showUndistored) {
        flipedFrame = this->imageMatList[index];
        cv::drawChessboardCorners(flipedFrame, cv::Size(9, 6), corners, !corners.empty());
    }
    else
    {
        flipedFrame = this->undistortedImageList[index];
    }

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

void CalibrationTool::clickToUndistort() {
    for (int i = 0; i < imageMatList.size(); i++) {
        cv::Mat undistortedImg;
        cv::undistort(imageMatList[i], undistortedImg, this->calibResults.cameraMatrix, this->calibResults.distCoeffs);
        undistortedImageList.push_back(undistortedImg);
    }
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
void CalibrationTool::createAxis() {
    // 创建坐标轴 (x→,y↑,z●)
    // Axis entity
    Qt3DCore::QEntity* axisEntity = new Qt3DCore::QEntity(rootEntity);

    // X-axis
    // X-axis mesh
    Qt3DExtras::QCylinderMesh* xAxisMesh = new Qt3DExtras::QCylinderMesh();
    xAxisMesh->setRadius(0.1f);
    xAxisMesh->setLength(20.0f);
    // X-axis material
    Qt3DExtras::QPhongMaterial* xAxisMaterial = new Qt3DExtras::QPhongMaterial();
    xAxisMaterial->setAmbient(Qt::red); //color
    // X-axis transform
    Qt3DCore::QTransform* xAxisTransform = new Qt3DCore::QTransform();
    xAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f));
    xAxisTransform->setTranslation(QVector3D(0.0f, -10.0f, 10.0f));
    // X-axis entity
    Qt3DCore::QEntity* xAxisEntity = new Qt3DCore::QEntity(axisEntity);
    xAxisEntity->addComponent(xAxisMesh);
    xAxisEntity->addComponent(xAxisMaterial);
    xAxisEntity->addComponent(xAxisTransform);

    // Y-axis
    // Y-axis mesh
    Qt3DExtras::QCylinderMesh* yAxisMesh = new Qt3DExtras::QCylinderMesh();
    yAxisMesh->setRadius(0.1f);
    yAxisMesh->setLength(30.0f);
    // Y-axis material
    Qt3DExtras::QPhongMaterial* zXxisMaterial = new Qt3DExtras::QPhongMaterial();
    zXxisMaterial->setAmbient(Qt::green); //color
    // Y-axis transform
    Qt3DCore::QTransform* yAxisTransform = new Qt3DCore::QTransform();
    yAxisTransform->setTranslation(QVector3D(-10.0f, 5.0f, 10.0f));
    // Y-axis entity
    Qt3DCore::QEntity* yAxisEntity = new Qt3DCore::QEntity(axisEntity);
    yAxisEntity->addComponent(yAxisMesh);
    yAxisEntity->addComponent(zXxisMaterial);
    yAxisEntity->addComponent(yAxisTransform);

    // Z-axis
    // Z-axis mesh
    Qt3DExtras::QCylinderMesh* zAxisMesh = new Qt3DExtras::QCylinderMesh();
    zAxisMesh->setRadius(0.1f);
    zAxisMesh->setLength(20.0f);
    // Z-axis material
    Qt3DExtras::QPhongMaterial* zAxisMaterial = new Qt3DExtras::QPhongMaterial();
    zAxisMaterial->setAmbient(Qt::blue); //color
    // Z-axis transform
    Qt3DCore::QTransform* zAxisTransform = new Qt3DCore::QTransform();
    zAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), -90.0f));
    zAxisTransform->setTranslation(QVector3D(10.0f, -10.0f, 0.0f));
    // Z-axis entity
    Qt3DCore::QEntity* zAxisEntity = new Qt3DCore::QEntity(axisEntity);
    zAxisEntity->addComponent(zAxisMesh);
    zAxisEntity->addComponent(zAxisMaterial);
    zAxisEntity->addComponent(zAxisTransform);

    // 轴线
    // axis mesh
    Qt3DExtras::QCylinderMesh* AxisMesh = new Qt3DExtras::QCylinderMesh();
    AxisMesh->setRadius(0.05f);
    AxisMesh->setLength(20.0f);
    // axis material
    Qt3DExtras::QPhongMaterial* AxisMaterial = new Qt3DExtras::QPhongMaterial();
    AxisMaterial->setAmbient(Qt::gray); //color

    vector<Qt3DCore::QTransform*> AxisTransforms;
    vector<Qt3DCore::QEntity*> AxisEntitys;
    vector<QVector3D> AxisTransforms_ = {
        {0.0f, -10.0f, 5.0f}, {0.0f, -10.0f, 0.0f}, {0.0f, -10.0f, -5.0f}, {0.0f, -10.0f, -10.0f},
        {0.0f, -5.0f, -10.0f}, {0.0f, 0.0f, -10.0f}, {0.0f, 5.0f, -10.0f}, {0.0f, 10.0f, -10.0f}, {0.0f, 15.0f, -10.0f}, {0.0f, 20.0f, -10.0f},

        {-10.0f, 0.0f, 5.0f}, {-10.0f, 0.0f, 0.0f}, {-10.0f, 0.0f, -5.0f}, {-10.0f, 0.0f, -10.0f},
        {-5.0f, 0.0f, -10.0f}, {-0.0f, 0.0f, -10.0f}, {5.0f, 0.0f, -10.0f}, {10.0f, 0.0f, -10.0f},
        {-10.0f, 10.0f, 5.0f}, {-10.0f, 10.0f, 0.0f}, {-10.0f, 10.0f, -5.0f}, {-10.0f, 10.0f, -10.0f},
        {-5.0f, 10.0f, -10.0f}, {-0.0f, 10.0f, -10.0f}, {5.0f, 10.0f, -10.0f}, {10.0f, 10.0f, -10.0f},

        {5.0f, -10.0f, 0.0f}, {0.0f, -10.0f, 0.0f}, {-5.0f, -10.0f, 0.0f}, {-10.0f, -10.0f, 0.0f},
        {-10.0f, -5.0f, 0.0f}, {-10.0f, 0.0f, 0.0f}, {-10.0f, 5.0f, 0.0f}, {-10.0f, 10.0f, 0.0f},{-10.0f, 15.0f, 0.0f},{-10.0f, 20.0f, 0.0f},
    };
    for (int i = 0; i < AxisTransforms_.size(); i++) {
        AxisTransforms.push_back(new Qt3DCore::QTransform());

        if (i >= 0 && i <= 9) {
            // 平行x轴的轴线
            AxisTransforms[i]->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f));
        }
        else if (i >= 10 && i <= 25) {
            // 平行y轴的轴线
            //AxisTransforms[i]->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f));
        }
        else if (i >= 26) {
            // 平行z轴的轴线
            AxisTransforms[i]->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), -90.0f));
        }

        AxisTransforms[i]->setTranslation(AxisTransforms_[i]);

        AxisEntitys.push_back(new Qt3DCore::QEntity(axisEntity));
        AxisEntitys[i]->addComponent(AxisMesh);
        AxisEntitys[i]->addComponent(AxisMaterial);
        AxisEntitys[i]->addComponent(AxisTransforms[i]);
    }

}

void CalibrationTool::createPlane() {
    // Plane mesh
    Qt3DExtras::QPlaneMesh* planeMesh = new Qt3DExtras::QPlaneMesh();
    planeMesh->setWidth(1);
    planeMesh->setHeight(1);
    // Plane transform
    Qt3DCore::QTransform* planeTransform = new Qt3DCore::QTransform();
    planeTransform->setScale(8.0f);
    planeTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
    // Plane material
    Qt3DExtras::QPhongMaterial* planeMaterial = new Qt3DExtras::QPhongMaterial();
    planeMaterial->setAmbient(QColor(Qt::yellow));
    // Plane
    Qt3DCore::QEntity* planeEntity = new Qt3DCore::QEntity(rootEntity);
    planeEntity->addComponent(planeMesh);
    planeEntity->addComponent(planeMaterial);
    planeEntity->addComponent(planeTransform);
}

void CalibrationTool::createScale(Qt3DRender::QCamera* camera) {

    vector<Qt3DCore::QTransform*> transforms;
    vector<QVector3D> transforms_ = {
        // x-Axis Scale
        {-5.0f, -10.0f, 15.0f}, {0.0f, -10.0f, 15.0f}, {5.0f, -10.0f, 15.0f},
        // y-Axis Scale
        {-10.0f, -5.0f, 15.0f}, {-10.0f, 0.0f, 15.0f}, {-10.0f, 5.0f, 15.0f}, {-10.0f, 10.0f, 15.0f}, {-10.0f, 15.0f, 15.0f},
        // z-Axis Scale
        {15.0f, -10.0f, -5.0f}, {15.0f, -10.0f, 0.0f}, {15.0f, -10.0f, 5.0f},
    };
    vector<Qt3DExtras::QText2DEntity*> texts;
    vector<int> scales = {
        -5, 0, 5,
        -5, 0, 5, 10, 15,
        -5, 0, 5
    };

    for (int i = 0; i < transforms_.size(); i++) {
        transforms.push_back(new Qt3DCore::QTransform());
        transforms[i]->setScale(0.15f);
        transforms[i]->setTranslation(transforms_[i]);
        transforms[i]->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), 90.0f));

        texts.push_back(new Qt3DExtras::QText2DEntity(rootEntity));
        texts[i]->setFont(QFont("Calibri"));
        texts[i]->setHeight(20);
        texts[i]->setWidth(20);
        texts[i]->setText(QString::number(scales[i]));
        texts[i]->setColor(Qt::black);
        texts[i]->addComponent(transforms[i]);

    }

    // Reference: https://github.com/Nonmant/Qt3DExtras-QText2DEntity-Example/blob/master/main.cpp
}

Qt3DCore::QEntity* CalibrationTool::createCuboid(QVector3D transformMatrix, QVector3D rotationMatrix) {
    // Cuboid mesh
    Qt3DExtras::QCuboidMesh* cuboid = new Qt3DExtras::QCuboidMesh();
    cuboid->setXExtent(4.0);
    cuboid->setZExtent(0.1);
    cuboid->setYExtent(4.0);
    // CuboidMesh Transform
    Qt3DCore::QTransform* cuboidTransform = new Qt3DCore::QTransform();
    cuboidTransform->setScale(2.0f);
    cuboidTransform->setRotation(QQuaternion::fromEulerAngles(rotationMatrix));
    cuboidTransform->setTranslation(transformMatrix);
    // CuboidMesh Material
    Qt3DExtras::QPhongMaterial* cuboidMaterial = new Qt3DExtras::QPhongMaterial();
    cuboidMaterial->setAmbient(DEFAULT_ENTITY_COLOR);
    // Cuboid entity
    Qt3DCore::QEntity* cuboidEntity = new Qt3DCore::QEntity(this->rootEntity);
    cuboidEntity->addComponent(cuboid);
    cuboidEntity->addComponent(cuboidMaterial);
    cuboidEntity->addComponent(cuboidTransform);

    return cuboidEntity;
}

void CalibrationTool::createPatternCentric() {
    // 创建子窗口并设置大小
    QWidget* childWidget = new QWidget(this);
    childWidget->setFixedSize(250, 250);
    // 创建 Qt3D 窗口,这是设置大小的，不要改
    view3D = new Qt3DExtras::Qt3DWindow();
    view3D->defaultFrameGraph()->setClearColor(Qt::white);
    QWidget* container = QWidget::createWindowContainer(view3D, childWidget);
    container->setGeometry(0, 0, GRAPHIC_VIEW_WIDTH - 2, GRAPHIC_VIEW_HEIGHT - 6);
    // 创建 3D 实体
    rootEntity = new Qt3DCore::QEntity();
    // 创建 3D 相机
    Qt3DRender::QCamera* camera = view3D->camera();
    camera->lens()->setPerspectiveProjection(50.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(35.0f, 18.0f, 30.0f));
    camera->setUpVector(QVector3D(0, 1, 0));
    camera->setViewCenter(QVector3D(0, 0, 0));

    // 创建 3D 相机控制器
    Qt3DExtras::QOrbitCameraController* cameraController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    cameraController->setLinearSpeed(100.0f);  // 设置相机的线性速度
    cameraController->setLookSpeed(100.0f);  // 设置相机的旋转速度
    cameraController->setCamera(camera);

    // Light
    //Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
    //Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    //light->setColor("white");
    //light->setIntensity(1);

    //lightEntity->addComponent(light);
    //Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
    //lightTransform->setTranslation(camera->position());
    ////lightTransform->setTranslation(QVector3D(0, 50, 0));
    //lightEntity->addComponent(lightTransform);

    // 设置根实体
    view3D->setRootEntity(rootEntity);

    // 让子窗口固定在右下角，最好不要改动
    QGridLayout* gridLayout = new QGridLayout(ui.centralWidget);
    gridLayout->setContentsMargins(0, 0, 40, 50);
    gridLayout->addWidget(childWidget, 0, 0, Qt::AlignBottom | Qt::AlignRight);

    // ************************************************
    // 向场景中添加物体
    // ************************************************
    // 创建 坐标轴
    createAxis();
    // 创建 标定板平面
    createPlane();
    // 创建 刻度
    createScale(camera);

}

void CalibrationTool::addCuboidToCentric() {
    // 通过遍历所给的位置向量，创建多个
   // qDebug() << "rvec   " << endl;
   // qDebug() << this->calibResults.rvecs.size() << endl;
    for (int i = 0; i < this->calibResults.rvecs.size(); i++) {
        // 平移立方体
        QVector3D translation(
            static_cast<float>(this->calibResults.tvecs[i].at<double>(0, 0) * TRANSLATION_BASE_SCALE),
            static_cast<float>(this->calibResults.tvecs[i].at<double>(1, 0) * TRANSLATION_BASE_SCALE + TRANSLATION_BASE_OFFSET),
            static_cast<float>(this->calibResults.tvecs[i].at<double>(2, 0) * TRANSLATION_BASE_SCALE)
        );

        // 旋转立方体
        QVector3D rotationVector(
            static_cast<float>(this->calibResults.rvecs[i].at<double>(0, 0) * MAX_RADIAN / PI + ROTATION_BASE_X_TRANSLATION),
            static_cast<float>(this->calibResults.rvecs[i].at<double>(1, 0) * MAX_RADIAN / PI),
            static_cast<float>(this->calibResults.rvecs[i].at<double>(2, 0) * MAX_RADIAN / PI)
        );
        //QQuaternion rotation = QQuaternion::fromEulerAngles(rotationVector);
        this->cuboids.push_back(createCuboid(translation, rotationVector));
    }


    //std::vector<double> temp11 = { 3.7255595916094270e-01, -3.2286315095976620e-01, 2.5998065119306002e+00 };
    //std::vector<double> temp12 = { 6.1493345305296354e-01, 1.8370675629054839e-01, -3.0332846618352547e+00 };
    //std::vector<double> temp13 = { -3.1328447296075423e-01, -1.8878982619689827e-01, 3.1227561623556346e+00 };
    //std::vector<std::vector<double> > r = { temp11,temp12,temp13 };

    //std::vector<double> temp21 = { 3.6222302556773556e-01, 1.8124125958992307e-01, 9.1641694205212221e-01 };
    //std::vector<double> temp22 = { 5.1387812804136901e-02, 4.1264447962786516e-01, 1.2561808957122409e+00 };
    //std::vector<double> temp23 = { 1.6484524415527824e-01, 7.4722399627735903e-01, 1.4310930451633073e+00 };
    //std::vector<std::vector<double> > t = { temp21,temp22,temp23 };
    /*cuboids.push_back(createCuboid(rootEntity,
        QVector3D(3.6222302556773556e-01, 1.8124125958992307e-01 + 10, 9.1641694205212221e-01),
        QVector3D(3.7255595916094270e-01*10 +90 , -3.2286315095976620e-01*10, 2.5998065119306002e+00*10)));*/
}