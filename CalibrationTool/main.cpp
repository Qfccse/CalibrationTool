#include "CalibrationTool.h"
#include <QtWidgets/QApplication>


#include <QGuiApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QCommandLinkButton>
#include <QtGui/QScreen>

#include <Qt3DCore>
#include <Qt3DRender>
#include <Qt3DExtras>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CalibrationTool w;
    w.show();

    // Root entity
    Qt3DCore::QEntity* rootEntity = new Qt3DCore::QEntity();

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
    camController->setCamera(cameraEntity);

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 0, 20.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    Qt3DCore::QEntity* lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    Qt3DCore::QTransform* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(cameraEntity->position());
    lightEntity->addComponent(lightTransform);


    // Cuboid shape data
    Qt3DExtras::QCuboidMesh* cuboid = new Qt3DExtras::QCuboidMesh();
    // CuboidMesh Transform
    Qt3DCore::QTransform* cuboidTransform = new Qt3DCore::QTransform();
    cuboidTransform->setScale(4.0f);
    cuboidTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
    // CuboidMesh Material
    Qt3DExtras::QPhongMaterial* cuboidMaterial = new Qt3DExtras::QPhongMaterial();
    cuboidMaterial->setDiffuse(QColor(QRgb(0x665423)));
    //Cuboid
    Qt3DCore::QEntity* cuboidEntity = new Qt3DCore::QEntity(rootEntity);
    cuboidEntity->addComponent(cuboid);
    cuboidEntity->addComponent(cuboidMaterial);
    cuboidEntity->addComponent(cuboidTransform);

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
    xAxisTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
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
    Qt3DExtras::QPhongMaterial* yAxisMaterial = new Qt3DExtras::QPhongMaterial();
    yAxisMaterial->setAmbient(Qt::blue); //color
    // Y-axis transform
    Qt3DCore::QTransform* yAxisTransform = new Qt3DCore::QTransform();
    yAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), -90.0f));
    yAxisTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

    Qt3DCore::QEntity* yAxisEntity = new Qt3DCore::QEntity(axisEntity);
    yAxisEntity->addComponent(yAxisMesh);
    yAxisEntity->addComponent(yAxisMaterial);
    yAxisEntity->addComponent(yAxisTransform);


    // Z-axis
    // Z-axis mesh
    Qt3DExtras::QCylinderMesh* zAxisMesh = new Qt3DExtras::QCylinderMesh();
    zAxisMesh->setRadius(0.05f);
    zAxisMesh->setLength(20.0f);
    // Z-axis material
    Qt3DExtras::QPhongMaterial* zXxisMaterial = new Qt3DExtras::QPhongMaterial();
    zXxisMaterial->setAmbient(Qt::yellow); //color
    // Z-axis transform
    Qt3DCore::QTransform* zAxisTransform = new Qt3DCore::QTransform();
    //zAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f));
    zAxisTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));

    Qt3DCore::QEntity* zAxisEntity = new Qt3DCore::QEntity(axisEntity);
    zAxisEntity->addComponent(zAxisMesh);
    zAxisEntity->addComponent(zXxisMaterial);
    zAxisEntity->addComponent(zAxisTransform);

    // Show window
    widget->show();
    widget->resize(500, 500);

    return a.exec();
}
