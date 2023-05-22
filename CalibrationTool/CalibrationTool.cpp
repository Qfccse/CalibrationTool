#include "CalibrationTool.h"
#include "Const.h"

CalibrationTool::CalibrationTool(QWidget *parent)
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
}

CalibrationTool::~CalibrationTool()
{}


void CalibrationTool::initImageList() {
    createAction();
    createMenu();
    createContentMenu();
    //����QListWidget����
    //����QListWidget����ʾģʽ
    ui.imageList->setViewMode(QListView::IconMode);
    //����QListWidget�е�Ԫ���ͼƬ��С
    ui.imageList->setIconSize(QSize(IMAGE_LIST_WIDTH, IMAGE_LIST_WIDTH));
    //����QListWidget�е�Ԫ��ļ��
    ui.imageList->setSpacing(10);
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

    ui.imageWindow->setPixmap(QPixmap::fromImage(image));  // ��ͼƬ��ʾ��label��
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
    timer->stop();         // ֹͣ��ȡ���ݡ�
    cam.release();
    ui.imageWindow->clear();
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
        fileNames = fileDialog->selectedFiles();
    showImageList();
    for (auto tmp : fileNames)
        qDebug() << tmp << endl;
}

/***************************************
*Qt��ʹ���ļ�ѡ��Ի���������:
*1.����һ��QListWidget����
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
        ui.imageList->addItem(imageItem);
    }
    //��ʾQListWidget
    ui.imageList->show();
}