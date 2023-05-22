#include "CalibrationTool.h"
#include "Const.h"

CalibrationTool::CalibrationTool(QWidget *parent)
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
}

CalibrationTool::~CalibrationTool()
{}


void CalibrationTool::initImageList() {
    createAction();
    createMenu();
    createContentMenu();
    //定义QListWidget对象
    //设置QListWidget的显示模式
    ui.imageList->setViewMode(QListView::IconMode);
    //设置QListWidget中单元项的图片大小
    ui.imageList->setIconSize(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH));
    //设置QListWidget中单元项的间距
    ui.imageList->setSpacing(10);
    //设置自动适应布局调整(Adjust适应，Fixed不适应)，默认不适应
    ui.imageList->setResizeMode(QListWidget::Adjust);
    //设置不能移动
    ui.imageList->setMovement(QListWidget::Static);
}
void CalibrationTool::openCamara()
{
    cam = VideoCapture();//打开摄像头，从摄像头中获取视频
    cam.open(1);
    if (!cam.isOpened()) {
        cam.open(0);
    }
    // 设置每一帧的大小为指定值
    cam.set(CAP_PROP_FRAME_WIDTH, IMAGE_WIN_WIDTH);
    cam.set(CAP_PROP_FRAME_HEIGHT, IMAGE_WIN_HEIGHT);
    timer->start(SAMPLE_RATE + 3);              // 开始计时，超时则发出timeout()信号
}

/*********************************
********* 读取摄像头信息 ***********
**********************************/
void CalibrationTool::readFarme()
{
    cam >> frame;// 从摄像头中抓取并返回每一帧
    Mat flipedFrame;
    flip(frame, flipedFrame, 1);
    // 将颜色格式从BGR转换为RGB
    cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
    // 将抓取到的帧，转换为QImage格式。QImage::Format_RGB888不同的摄像头用不同的格式。
    QImage image(flipedFrame.data, flipedFrame.cols, flipedFrame.rows, flipedFrame.step, QImage::Format_RGB888);

    ui.imageWindow->setPixmap(QPixmap::fromImage(image));  // 将图片显示到label上
}


void CalibrationTool::takingPictures()
{
    if (!cam.isOpened()) {
        QMessageBox::warning(this, tr("warning"), tr("please open camera"), QMessageBox::Ok);
        return;
    }
    cam >> frame;// 从摄像头中抓取并返回每一帧
    Mat flipedFrame;
    flip(frame, flipedFrame, 1);
    cvtColor(flipedFrame, flipedFrame, cv::COLOR_BGR2RGB);
    QImage image((const uchar*)flipedFrame.data, flipedFrame.size().width, flipedFrame.size().height, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image);
    double ratio = static_cast<double>(image.height()) / image.width();
    QListWidgetItem* item = new QListWidgetItem();
    item->setIcon(QIcon(pixmap)); // 或者 item->setIcon(QIcon::fromImage(image));
    item->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
    ui.imageList->addItem(item);
}

/*******************************
***关闭摄像头，释放资源，必须释放***
********************************/
void CalibrationTool::closeCamara()
{
    timer->stop();         // 停止读取数据。
    cam.release();
    ui.imageWindow->clear();
}

void CalibrationTool::createAction()
{
    //创建打开文件动作
    fileOpenAction = new QAction(tr("open"), this);
    //摄者打开文件的快捷方式
    fileOpenAction->setShortcut(tr("Ctr1+0"));
    //设置打开文件动作提示信息
    fileOpenAction->setStatusTip("open a folder");
    //关联打开文件动作的信号和槽
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
        fileNames = fileDialog->selectedFiles();
    showImageList();
    for (auto tmp : fileNames)
        qDebug() << tmp << endl;
}

/***************************************
*Qt中使用文件选择对话框步骤如下:
*1.定义一个QListWidget对象
* 2.设置ViewMode等属性
* 3.定义单元项并添加到QL istWidget中
* 4.调用QListWidget对象的show()方法
* ****************************************/
void CalibrationTool::showImageList() {

    for (auto tmp : fileNames) {
        // 定义QListWidgetItem对象
        QListWidgetItem* imageItem = new QListWidgetItem;
        QImage image(tmp);
        double ratio = static_cast<double>(image.height()) / image.width();
        // qDebug() << height << endl;
        imageItem->setIcon(QIcon(tmp));
        //重新设置单元项图片的宽度和高度
        imageItem->setSizeHint(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH * ratio));
        ui.imageList->addItem(imageItem);
    }
    //显示QListWidget
    ui.imageList->show();
}