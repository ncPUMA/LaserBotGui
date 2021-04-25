#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>

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
#include <BRepBuilderAPI_GTransform.hxx>
#include <AIS_ViewCube.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>

#include "ModelLoader/cmodelloaderfactorymethod.h"
#include "ModelLoader/cabstractmodelloader.h"

#include "Calibration/ccalibrationvertexdialog.h"

#include "BotSocket/cabstractbotsocket.h"
#include "cmodelmover.h"

#include "Primitives/cbotcross.h"
#include "Primitives/claservec.h"

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
    double getLaserX() const final { return 0; }
    double getLaserY() const final { return 0; }
    double getLaserZ() const final { return 0; }
    double getScale() const final { return 0; }
    GUI_TYPES::TMSAA getMsaa() const final { return 0; }

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
    void setLaserX(const double) final { }
    void setLaserY(const double) final { }
    void setLaserZ(const double) final { }
    void setScale(const double) final { }
    void setMsaa(const GUI_TYPES::TMSAA) final { }
};



class CEmptyBotSocket : public CAbstractBotSocket
{
public:
    CEmptyBotSocket() : CAbstractBotSocket() { }

protected:
    BotSocket::TSocketError startSocket() final { return BotSocket::ENSE_NO; }
    void stopSocket() final { }
    BotSocket::TSocketState socketState() const final { return BotSocket ::ENSS_FALL; }
};



static const Standard_Integer Z_LAYER = 100;

class CModelMover;

class MainWindowPrivate
{
    friend class MainWindow;
    friend class CModelMover;

private:
    MainWindowPrivate() :
        guiSettings(&emptySettings),
        zLayerId(Z_LAYER),
        botSocket(&emptySocket),
        stateLamp(new QLabel())
    { }

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

        const QString botTxt = MainWindow::tr("Смешение:\n   X: %1\n   Y: %2\n   Z: %3\n"
                                              "Наклон:\n   α: %4\n   β: %5\n   γ: %6")
                .arg(mdlMover.getTrX(), 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getTrY(), 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getTrZ(), 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getRX() , 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getRY() , 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getRZ() , 11, 'f', 6, QChar('0'));
        NCollection_Vector <Handle(AIS_InteractiveObject)> crossObj =
                cross.objects(botTxt.toLocal8Bit().constData());
        for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(crossObj);
            it.More(); it.Next()) {
            const Handle(AIS_InteractiveObject)& obj = it.Value();
            context->Display(obj, Standard_False);
            context->SetDisplayMode(obj, shading ? 1 : 0, Standard_False);
            context->SetZLayer(obj, zLayerId);
        }

        //translation
        gp_Trsf trsfTr = curModel.Location().Transformation();
        const gp_Vec translation(guiSettings->getTranslationX() + mdlMover.getTrX(),
                                 guiSettings->getTranslationY() + mdlMover.getTrY(),
                                 guiSettings->getTranslationZ() + mdlMover.getTrZ());
        trsfTr.SetTranslation(translation);

        //rotation
        const gp_Pnt anchor(guiSettings->getAnchorX(), guiSettings->getAnchorY(), guiSettings->getAnchorZ());
        gp_Trsf trsfRX = curModel.Location().Transformation();
        trsfRX.SetRotation(gp_Ax1(anchor, gp_Dir(1.,0.,0.)),
                           guiSettings->getRotationX() * DEGREE_K + mdlMover.getRX() * DEGREE_K);
        gp_Trsf trsfRY = curModel.Location().Transformation();
        trsfRY.SetRotation(gp_Ax1(anchor, gp_Dir(0.,1.,0.)),
                           guiSettings->getRotationY() * DEGREE_K + mdlMover.getRY() * DEGREE_K);
        gp_Trsf trsfRZ = curModel.Location().Transformation();
        trsfRZ.SetRotation(gp_Ax1(anchor, gp_Dir(0.,0.,1.)),
                           guiSettings->getRotationZ() * DEGREE_K + mdlMover.getRZ() * DEGREE_K);

        //scale
        gp_Trsf trsfSc = curModel.Location().Transformation();
        double scaleFactor = guiSettings->getScale();
        if (scaleFactor == 0.)
            scaleFactor = 1.;
        trsfSc.SetScale(anchor, scaleFactor);

        curModel.Location(trsfTr * trsfRX * trsfRY * trsfRZ * trsfSc);
        Handle(AIS_Shape) ais_shape = new AIS_Shape(curModel);
        context->SetDisplayMode(ais_shape, shading ? 1 : 0, Standard_False);
        context->Display(ais_shape, Standard_False);

        gp_Pnt aPnt1(0., 0., 0.);
        gp_Pnt aPnt2(guiSettings->getLaserX(), guiSettings->getLaserY(), guiSettings->getLaserZ());
        if (!aPnt1.IsEqual(aPnt2, gp::Resolution()))
        {
            gp_Vec aVec(aPnt1, aPnt2);

            IntCurvesFace_ShapeIntersector intersector;
            intersector.Load(curModel, Precision::Confusion());
            const gp_Lin lin = gp_Lin(aPnt1, gp_Dir(aVec));
            intersector.PerformNearest(lin, 0, RealLast());
            if (intersector.IsDone() && intersector.NbPnt() > 0)
            {
                gp_Pnt aPnt3 = intersector.Pnt(1);
                if (!aPnt1.IsEqual(aPnt3, gp::Resolution()))
                    aVec = gp_Vec(aPnt1, aPnt3);
            }

            Handle(CLaserVec) lVec = new CLaserVec(aPnt1, aVec, 0.5);
            context->SetDisplayMode(lVec, 1, Standard_False);
            context->Display(lVec, Standard_False);
        }

        Handle(AIS_ViewCube) aViewCube = new AIS_ViewCube();
        aViewCube->SetDrawEdges(Standard_False);
        aViewCube->SetDrawVertices(Standard_False);
        aViewCube->SetBoxTransparency(1.);
        TCollection_AsciiString emptyStr;
        aViewCube->SetBoxSideLabel(V3d_Xpos, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Ypos, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Zpos, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Xneg, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Yneg, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Zneg, emptyStr);
        context->SetDisplayMode(aViewCube, 1, Standard_False);
        context->Display(aViewCube, Standard_False);

        viewer->Redraw();
    }

    bool load(const QString &fName, CAbstractModelLoader &loader, const bool shading) {
        const TopoDS_Shape shape = loader.load(fName.toLocal8Bit().constData());
        curModel = affinityTransform(shape);
        reDrawScene(shading);
        return !curModel.IsNull();
    }

    void setMSAA(const GUI_TYPES::TMSAA value, CMainViewport &view) {
        for(auto pair : mapMsaa) {
            pair.second->blockSignals(true);
            pair.second->setChecked(pair.first == value);
            pair.second->blockSignals(false);
        }
        view.setMSAA(value);
    }

    TopoDS_Shape affinityTransform(const TopoDS_Shape &shape) const {
        if (guiSettings->getScaleX() != 0 &&
            guiSettings->getScaleY() != 0 &&
            guiSettings->getScaleZ() != 0) {
            const gp_Pnt anchor(guiSettings->getAnchorX(),
                                guiSettings->getAnchorY(),
                                guiSettings->getAnchorZ());
            gp_GTrsf gtrsfX(shape.Location().Transformation());
            gtrsfX.SetAffinity(gp_Ax1(anchor, gp_Dir(1., 0., 0.)), guiSettings->getScaleX());
            gp_GTrsf gtrsfY(shape.Location().Transformation());
            gtrsfY.SetAffinity(gp_Ax1(anchor, gp_Dir(0., 1., 0.)), guiSettings->getScaleY());
            gp_GTrsf gtrsfZ(shape.Location().Transformation());
            gtrsfZ.SetAffinity(gp_Ax1(anchor, gp_Dir(0., 0., 1.)), guiSettings->getScaleZ());
            return BRepBuilderAPI_GTransform(shape, gtrsfX * gtrsfY * gtrsfZ, true).Shape();
        }
        return shape;
    }

private:
    CEmptyGuiSettings emptySettings;
    CAbstractGuiSettings *guiSettings;

    Handle(V3d_Viewer)             viewer;
    Handle(AIS_InteractiveContext) context;
    Standard_Integer zLayerId;

    TopoDS_Shape curModel;

    CEmptyBotSocket emptySocket;
    CAbstractBotSocket *botSocket;

    CBotCross cross;

    CModelMover mdlMover;

    QLabel * const stateLamp;
    QAction *startAction, *stopAction;
    std::map <GUI_TYPES::TMSAA, QAction *> mapMsaa;
};



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    d_ptr(new MainWindowPrivate())
{
    ui->setupUi(this);

    d_ptr->mdlMover.setGui(this);

    //Menu "File"
    connect(ui->actionImport, SIGNAL(triggered(bool)), SLOT(slImport()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), SLOT(slExit()));

    //Menu "View"
    connect(ui->actionShading, SIGNAL(toggled(bool)), SLOT(slShading(bool)));
    ui->dockSettings->setVisible(false);
    connect(ui->actionCalib, SIGNAL(toggled(bool)), SLOT(slShowCalibWidget(bool)));
    //MSAA
    d_ptr->mapMsaa = std::map <GUI_TYPES::TMSAA, QAction *> {
        { GUI_TYPES::ENMSAA_OFF, ui->actionMSAA_Off },
        { GUI_TYPES::ENMSAA_2  , ui->actionMSAA_2X  },
        { GUI_TYPES::ENMSAA_4  , ui->actionMSAA_4X  },
        { GUI_TYPES::ENMSAA_8  , ui->actionMSAA_8X  }
    };
    for(auto pair : d_ptr->mapMsaa)
        connect(pair.second, SIGNAL(toggled(bool)), SLOT(slMsaa()));
    //FPS
    connect(ui->actionFPS, SIGNAL(toggled(bool)), SLOT(slFpsCounter(bool)));

    //ToolBar
    ui->toolBar->addAction(QIcon(":/icons/Data/Icons/open.png"),
                           tr("Импорт..."),
                           ui->actionImport,
                           SLOT(trigger()));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon(":/icons/Data/Icons/shading.png"),
                           tr("Shading"),
                           ui->actionShading,
                           SLOT(toggle()));
    ui->toolBar->addAction(QIcon(":/icons/Data/Icons/fps-counter.png"),
                           tr("Счетчик FPS"),
                           ui->actionFPS,
                           SLOT(toggle()));
//    ui->toolBar->addSeparator();
    QLabel * const strech = new QLabel(" ", ui->toolBar);
    strech->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolBar->addWidget(strech);
    d_ptr->startAction = ui->toolBar->addAction(QIcon(":/icons/Data/Icons/play.png"),
                                                tr("Старт"),
                                                this,
                                                SLOT(slStart()));
    d_ptr->startAction = ui->toolBar->addAction(QIcon(":/icons/Data/Icons/pause.png"),
                                                tr("Стоп"),
                                                this,
                                                SLOT(slStop()));
    QLabel * const txtState = new QLabel(tr("Соединение: "), ui->toolBar);
    QFont stateFnt = txtState->font();
    stateFnt.setPointSize(18);
    txtState->setFont(stateFnt);
    txtState->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->toolBar->addWidget(txtState);
    d_ptr->stateLamp->setParent(ui->toolBar);
    ui->toolBar->addWidget(d_ptr->stateLamp);
    const QPixmap red(":/Lamps/Data/Lamps/red.png");
    d_ptr->stateLamp->setPixmap(red.scaled(ui->toolBar->iconSize(),
                                           Qt::IgnoreAspectRatio,
                                           Qt::SmoothTransformation));

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
    ui->mainView->setStatsVisible(ui->actionFPS->isChecked());
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
    ui->dsbLaserX->setValue(settings.getLaserX());
    ui->dsbLaserY->setValue(settings.getLaserY());
    ui->dsbLaserZ->setValue(settings.getLaserZ());
    ui->dsbScale->setValue(settings.getScale());

    d_ptr->setMSAA(settings.getMsaa(), *ui->mainView);
}

void MainWindow::setBotSocket(CAbstractBotSocket &socket)
{
    d_ptr->botSocket = &socket;
    socket.setUi(d_ptr->mdlMover);
}

void MainWindow::updateMdlTransform()
{
//    QTime t;
//    t.start();
    const QString botTxt = QString("%1\t-->\tx: %2 y: %3 z: %4 "
                                   "α: %5 β: %6 γ: %7")
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
            .arg(d_ptr->mdlMover.getTrX(), 11, 'f', 6, QChar('0'))
            .arg(d_ptr->mdlMover.getTrY(), 11, 'f', 6, QChar('0'))
            .arg(d_ptr->mdlMover.getTrZ(), 11, 'f', 6, QChar('0'))
            .arg(d_ptr->mdlMover.getRX() , 11, 'f', 6, QChar('0'))
            .arg(d_ptr->mdlMover.getRY() , 11, 'f', 6, QChar('0'))
            .arg(d_ptr->mdlMover.getRZ() , 11, 'f', 6, QChar('0'));
    ui->teJrnl->append(botTxt);
    d_ptr->reDrawScene(ui->actionShading->isChecked());
//    qDebug() << t.elapsed();
}

void MainWindow::updateBotSocketState()
{
    static const QPixmap red =
            QPixmap(":/Lamps/Data/Lamps/red.png").scaled(ui->toolBar->iconSize(),
                                                         Qt::IgnoreAspectRatio,
                                                         Qt::SmoothTransformation);
    static const QPixmap green =
            QPixmap(":/Lamps/Data/Lamps/green.png").scaled(ui->toolBar->iconSize(),
                                                         Qt::IgnoreAspectRatio,
                                                         Qt::SmoothTransformation);

    if (d_ptr->mdlMover.socketState() != BotSocket::ENSS_OK)
    {
        d_ptr->stateLamp->setPixmap(red);
        d_ptr->stateLamp->setToolTip(tr("Авария"));
    }
    else
    {
        d_ptr->stateLamp->setPixmap(green);
        d_ptr->stateLamp->setToolTip(tr("OK"));
    }
}

void MainWindow::slImport()
{
    CModelLoaderFactoryMethod factory;
    QString selectedFilter;
    const bool socketStarted = d_ptr->botSocket->isStarted();
    if (socketStarted)
            d_ptr->botSocket->stop();
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
    if (socketStarted)
        d_ptr->botSocket->start();
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

void MainWindow::slMsaa()
{
    GUI_TYPES::TMSAA msaa = GUI_TYPES::ENMSAA_OFF;
    for(auto pair : d_ptr->mapMsaa)
    {
        if (pair.second == sender())
        {
            msaa = pair.first;
            break;
        }
    }
    d_ptr->setMSAA(msaa, *ui->mainView);
    d_ptr->guiSettings->setMsaa(msaa);
}

void MainWindow::slFpsCounter(bool enabled)
{
    ui->mainView->setStatsVisible(enabled);
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
    d_ptr->guiSettings->setLaserX(ui->dsbLaserX->value());
    d_ptr->guiSettings->setLaserY(ui->dsbLaserY->value());
    d_ptr->guiSettings->setLaserZ(ui->dsbLaserZ->value());
    d_ptr->guiSettings->setScale(ui->dsbScale->value());

//    d_ptr->curModel = d_ptr->affinityTransform(d_ptr->curModel);

    d_ptr->reDrawScene(ui->actionShading->isChecked());
}

void MainWindow::slStart()
{
    d_ptr->context->SetAutomaticHilight(Standard_False);
    d_ptr->botSocket->start();
}

void MainWindow::slStop()
{
    d_ptr->botSocket->stop();
    d_ptr->context->SetAutomaticHilight(Standard_True);
}

