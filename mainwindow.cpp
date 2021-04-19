#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>

#include <AIS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <AIS_TextLabel.hxx>
#include <Graphic3d_ZLayerSettings.hxx>
#include <gp_Quaternion.hxx>

#include "ModelLoader/cmodelloaderfactorymethod.h"
#include "ModelLoader/cabstractmodelloader.h"

#include "Calibration/ccalibrationvertexdialog.h"

#include "BotSocket/cabstractbotsocket.h"

#include "Primitives/cbotcross.h"

#include "cabstractguisettings.h"

constexpr double DEGREE_K = M_PI / 180.;

class CEmptyGuiSettings : public CAbstractGuiSettings
{
public:
    CEmptyGuiSettings() { }

    double getTranslationX() const final { return 0; }
    double getTranslationY() const final { return 0; }
    double getTranslationZ() const final { return 0; }
    double getRotationX() const final { return 0; }
    double getRotationY() const final { return 0; }
    double getRotationZ() const final { return 0; }
    double getScaleX() const final { return 0; }
    double getScaleY() const final { return 0; }
    double getScaleZ() const final { return 0; }
    double getAnchorX() const final { return 0; }
    double getAnchorY() const final { return 0; }
    double getAnchorZ() const final { return 0; }
    double getLaserLenght() const final { return 0; }

    void setTranslationX(const double) final { }
    void setTranslationY(const double) final { }
    void setTranslationZ(const double) final { }
    void setRotationX(const double) final { }
    void setRotationY(const double) final { }
    void setRotationZ(const double) final { }
    void setScaleX(const double) final { }
    void setScaleY(const double) final { }
    void setScaleZ(const double) final { }
    void setAnchorX(const double) final { }
    void setAnchorY(const double) final { }
    void setAnchorZ(const double) final { }
    void setLaserLenght(const double) final { }
};

class CEmptyBotSocket : public CAbstractBotSocket
{
public:
    CEmptyBotSocket() : CAbstractBotSocket() { }

protected:
    BotSocket::TSocketError startSocket() final { return BotSocket::ENSE_NO; }
    void stopSocket() final { }
};

static const Standard_Integer Z_LAYER = 100;

class MainWindowPrivate
{
    friend class MainWindow;

private:
    MainWindowPrivate() :
        guiSettings(&emptySettings),
        zLayerId(Z_LAYER),
        botSocket(&emptySocket) { }

    void init(OpenGl_GraphicDriver &driver) {
        viewer = new V3d_Viewer(&driver);
        viewer->SetDefaultViewSize(1000.);
        viewer->SetDefaultViewProj(V3d_XposYnegZpos);
        viewer->SetComputedMode(Standard_True);
        viewer->SetDefaultComputedMode(Standard_True);
        viewer->SetDefaultLights();
        viewer->SetLightOn();

        context = new AIS_InteractiveContext(viewer);

        viewer->AddZLayer(zLayerId);
        Graphic3d_ZLayerSettings settings = viewer->ZLayerSettings(zLayerId);
        settings.SetEnableDepthTest(Standard_False);
        viewer->SetZLayerSettings(zLayerId, settings);
    }

    void reDrawScene(const bool shading) {
        context->RemoveAll(Standard_False);

        NCollection_Vector <Handle(AIS_InteractiveObject)> crossObj = cross.objects();
        for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(crossObj);
            it.More(); it.Next()) {
            const Handle(AIS_InteractiveObject)& obj = it.Value();
            context->Display(obj, Standard_False);
            context->SetDisplayMode(obj, shading ? 1 : 0, false);
            context->SetZLayer(obj, zLayerId);
        }

        gp_Trsf trsf = curModel.Location().Transformation();
        const gp_Vec translation(guiSettings->getTranslationX(),
                                 guiSettings->getTranslationY(),
                                 guiSettings->getTranslationZ());
        const gp_Quaternion quat =
                gp_Quaternion(gp_Vec(1., 0., 0.), guiSettings->getRotationX() * DEGREE_K) *
                gp_Quaternion(gp_Vec(0., 1., 0.), guiSettings->getRotationY() * DEGREE_K) *
                gp_Quaternion(gp_Vec(0., 0., 1.), guiSettings->getRotationZ() * DEGREE_K);
        trsf.SetTransformation(quat, translation);
        curModel.Location(trsf);
        Handle(AIS_Shape) ais_shape = new AIS_Shape(curModel);
        context->Display(ais_shape, Standard_True);
        context->SetDisplayMode(ais_shape, shading ? 1 : 0, false);

        viewer->Redraw();
    }

    bool load(const QString &fName, CAbstractModelLoader &loader, const bool shading) {
        curModel = loader.load(fName.toLocal8Bit().constData());
        reDrawScene(shading);
        return !curModel.IsNull();
    }

private:
    CEmptyGuiSettings emptySettings;
    CAbstractGuiSettings *guiSettings;

    Handle(V3d_Viewer)             viewer;
    Handle(AIS_InteractiveContext) context;
    Standard_Integer zLayerId;

//    NCollection_Vector <Handle(AIS_InteractiveObject)> curModels;
    TopoDS_Shape curModel;

    CEmptyBotSocket emptySocket;
    CAbstractBotSocket *botSocket;

    CBotCross cross;
};



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    d_ptr(new MainWindowPrivate())
{
    ui->setupUi(this);

    //Menu "File"
    connect(ui->actionImport, SIGNAL(triggered(bool)), SLOT(slImport()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(slExit()));

    //Menu "View"
    connect(ui->actionShading, SIGNAL(toggled(bool)), SLOT(slShading(bool)));
    ui->dockSettings->setVisible(false);
    connect(ui->actionCalib, SIGNAL(toggled(bool)), SLOT(slShowCalibWidget(bool)));

    //ToolBar
    ui->toolBar->addAction(QIcon::fromTheme("document-open"),
                           tr("Импорт..."),
                           ui->actionImport,
                           SLOT(trigger()));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon::fromTheme("format-justify-fill"),
                           tr("Shading"),
                           ui->actionShading,
                           SLOT(toggle()));
    ui->toolBar->addSeparator();

    //Callib
    connect(ui->pbApplyCalib, SIGNAL(clicked(bool)), SLOT(slCallibApply()));
}

MainWindow::~MainWindow()
{
    delete d_ptr;
    delete ui;
}

void MainWindow::init(OpenGl_GraphicDriver &driver)
{
    d_ptr->init(driver);
    ui->mainView->init(*d_ptr->context);
}

void MainWindow::setSettings(CAbstractGuiSettings &settings)
{
    d_ptr->guiSettings = &settings;
    ui->dsbTrX->setValue(settings.getTranslationX());
    ui->dsbTrY->setValue(settings.getTranslationY());
    ui->dsbTrZ->setValue(settings.getTranslationZ());
    ui->dsbRtX->setValue(settings.getRotationX());
    ui->dsbRtY->setValue(settings.getRotationY());
    ui->dsbRtZ->setValue(settings.getRotationZ());
    ui->dsbMapX->setValue(settings.getScaleX());
    ui->dsbMapY->setValue(settings.getScaleY());
    ui->dsbMapZ->setValue(settings.getScaleZ());
    ui->dsbAnchX->setValue(settings.getAnchorX());
    ui->dsbAnchY->setValue(settings.getAnchorY());
    ui->dsbAnchZ->setValue(settings.getAnchorZ());
    ui->dsbLL->setValue(settings.getLaserLenght());
}

void MainWindow::setBotSocket(CAbstractBotSocket &botSocket)
{
    d_ptr->botSocket = &botSocket;
    d_ptr->botSocket->gui = this;
}

void MainWindow::slImport()
{
    CModelLoaderFactoryMethod factory;
    QString selectedFilter;
    const QString fName =
            QFileDialog::getOpenFileName(this,
                                         tr("Выбор файла"),
                                         QString(),
                                         factory.supportedFilters(),
                                         &selectedFilter);
    if (!fName.isNull())
    {
        CAbstractModelLoader &loader = factory.loader(selectedFilter);
        if (d_ptr->load(fName, loader, ui->actionShading->isChecked()))
            ui->mainView->fitInView();
        else
            QMessageBox::critical(this,
                                  tr("Ошибка загрузки файла"),
                                  tr("Ошибка загрузки файла"));
    }
}

void MainWindow::slExit()
{
    close();
}

void MainWindow::slShading(bool enabled)
{
    d_ptr->reDrawScene(enabled);
}

void MainWindow::slShowCalibWidget(bool enabled)
{
    ui->dockSettings->setVisible(enabled);
}

void MainWindow::slCallibApply()
{
    d_ptr->guiSettings->setTranslationX(ui->dsbTrX->value());
    d_ptr->guiSettings->setTranslationY(ui->dsbTrY->value());
    d_ptr->guiSettings->setTranslationZ(ui->dsbTrZ->value());
    d_ptr->guiSettings->setRotationX(ui->dsbRtX->value());
    d_ptr->guiSettings->setRotationY(ui->dsbRtY->value());
    d_ptr->guiSettings->setRotationZ(ui->dsbRtZ->value());
    d_ptr->guiSettings->setScaleX(ui->dsbMapX->value());
    d_ptr->guiSettings->setScaleY(ui->dsbMapY->value());
    d_ptr->guiSettings->setScaleZ(ui->dsbMapZ->value());
    d_ptr->guiSettings->setAnchorX(ui->dsbAnchX->value());
    d_ptr->guiSettings->setAnchorY(ui->dsbAnchY->value());
    d_ptr->guiSettings->setAnchorZ(ui->dsbAnchZ->value());
    d_ptr->guiSettings->setLaserLenght(ui->dsbLL->value());

    d_ptr->reDrawScene(ui->actionShading->isChecked());
}

