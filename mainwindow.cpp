#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QLabel>

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
#include <Prs3d_DatumAspect.hxx>

#include "ModelLoader/cmodelloaderfactorymethod.h"
#include "ModelLoader/csteploader.h"

#include "BotSocket/cabstractbotsocket.h"
#include "cmodelmover.h"

#include "Primitives/cbotcross.h"
#include "Primitives/claservec.h"

#include "cabstractguisettings.h"

static constexpr double DEGREE_K = M_PI / 180.;

static const Quantity_Color BG_CLR  = Quantity_Color(.7765,  .9, 1.  , Quantity_TOC_RGB);
static const Quantity_Color TXT_CLR = Quantity_Color(  .15, .15, 0.15, Quantity_TOC_RGB);

static constexpr int MAX_JRNL_ROW_COUNT = 15000;

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
        stateLamp(new QLabel()),
        attachLamp(new QLabel())
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
        Handle(Prs3d_DatumAspect) datum = context->DefaultDrawer()->DatumAspect();
        datum->TextAspect(Prs3d_DatumParts_XAxis)->SetColor(TXT_CLR);
        datum->TextAspect(Prs3d_DatumParts_YAxis)->SetColor(TXT_CLR);
        datum->TextAspect(Prs3d_DatumParts_ZAxis)->SetColor(TXT_CLR);

        viewer->AddZLayer(zLayerId);
        Graphic3d_ZLayerSettings settings = viewer->ZLayerSettings(zLayerId);
        settings.SetEnableDepthTest(Standard_False);
        viewer->SetZLayerSettings(zLayerId, settings);

        //load gripModel
        QFile gripFile(":/Models/Data/Models/gripper_v1.step");
        if (gripFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            const QByteArray stepData = gripFile.readAll();
            CStepLoader loader;
            gripModel = loader.loadFromBinaryData(stepData.constData(), static_cast <size_t> (stepData.size()));
        }
    }

    void updateModelsDefaultPosition(const bool shading) {
        context->RemoveAll(Standard_False);

        //Draw AIS_ViewCube
        Handle(AIS_ViewCube) aViewCube = new AIS_ViewCube();
        aViewCube->SetDrawEdges(Standard_False);
        aViewCube->SetDrawVertices(Standard_False);
        aViewCube->SetBoxTransparency(1.);
        aViewCube->AIS_InteractiveObject::SetColor(TXT_CLR);
        TCollection_AsciiString emptyStr;
        aViewCube->SetBoxSideLabel(V3d_Xpos, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Ypos, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Zpos, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Xneg, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Yneg, emptyStr);
        aViewCube->SetBoxSideLabel(V3d_Zneg, emptyStr);
        context->SetDisplayMode(aViewCube, 1, Standard_False);
        context->Display(aViewCube, Standard_False);

        //The Part
        {
            gp_Trsf trsfTr = curModel.Location().Transformation();
            const gp_Vec translation(guiSettings->getPartTrX(),
                                     guiSettings->getPartTrY(),
                                     guiSettings->getPartTrZ());
            trsfTr.SetTranslation(translation);

            const gp_Pnt localAnchor(guiSettings->getPartCenterX(),
                                     guiSettings->getPartCenterY(),
                                     guiSettings->getPartCenterZ());
            gp_Trsf trsfLRX = curModel.Location().Transformation();
            trsfLRX.SetRotation(gp_Ax1(localAnchor, gp_Dir(1.,0.,0.)),
                                guiSettings->getPartRotationX() * DEGREE_K);
            gp_Trsf trsfLRY = curModel.Location().Transformation();
            trsfLRY.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,1.,0.)),
                                guiSettings->getPartRotationY() * DEGREE_K);
            gp_Trsf trsfLRZ = curModel.Location().Transformation();
            trsfLRZ.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,0.,1.)),
                                guiSettings->getPartRotationZ() * DEGREE_K);

            gp_Trsf trsfSc = curModel.Location().Transformation();
            double scaleFactor = guiSettings->getPartScale();
            if (scaleFactor == 0.)
                scaleFactor = 1.;
            trsfSc.SetScale(localAnchor, scaleFactor);

            ais_mdl = new AIS_Shape(curModel);
            context->SetDisplayMode(ais_mdl, shading ? 1 : 0, Standard_False);
            context->Display(ais_mdl, Standard_False);
            context->SetLocation(ais_mdl, trsfTr * trsfLRX * trsfLRY * trsfLRZ * trsfSc);
        }

        //The Grip
        {
            gp_Trsf trsfTr = gripModel.Location().Transformation();
            const gp_Vec translation(guiSettings->getGripTrX(),
                                     guiSettings->getGripTrY(),
                                     guiSettings->getGripTrZ());
            trsfTr.SetTranslation(translation);

            const gp_Pnt localAnchor(guiSettings->getGripCenterX(),
                                     guiSettings->getGripCenterY(),
                                     guiSettings->getGripCenterZ());
            gp_Trsf trsfLRX = gripModel.Location().Transformation();
            trsfLRX.SetRotation(gp_Ax1(localAnchor, gp_Dir(1.,0.,0.)),
                                guiSettings->getGripRotationX() * DEGREE_K);
            gp_Trsf trsfLRY = gripModel.Location().Transformation();
            trsfLRY.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,1.,0.)),
                                guiSettings->getGripRotationY() * DEGREE_K);
            gp_Trsf trsfLRZ = gripModel.Location().Transformation();
            trsfLRZ.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,0.,1.)),
                                guiSettings->getGripRotationZ() * DEGREE_K);

            gp_Trsf trsfSc = gripModel.Location().Transformation();
            double scaleFactor = guiSettings->getGripScale();
            if (scaleFactor == 0.)
                scaleFactor = 1.;
            trsfSc.SetScale(localAnchor, scaleFactor);

            if (guiSettings->isGripVisible()) {
                ais_grip = new AIS_Shape(gripModel);
                context->SetDisplayMode(ais_grip, shading ? 1 : 0, Standard_False);
                context->Display(ais_grip, Standard_False);
                context->SetLocation(ais_grip, trsfTr * trsfLRX * trsfLRY * trsfLRZ * trsfSc);
            }
        }

        //Draw The Laser
        const QString botTxt = MainWindow::tr("Смещение:\n   X: 000.000000\n   Y: 000.000000\n   Z: 000.000000\n"
                                              "Наклон:\n   α: 000.000000\n   β: 000.000000\n   γ: 000.000000");
        NCollection_Vector <Handle(AIS_InteractiveObject)> crossObj =
                cross.objects(botTxt.toStdString().c_str());
        for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(crossObj);
            it.More(); it.Next()) {
            const Handle(AIS_InteractiveObject)& obj = it.Value();
            context->Display(obj, Standard_False);
            context->SetZLayer(obj, zLayerId);
        }

        gp_Pnt aPnt1(0., 0., 0.);
        gp_Pnt aPnt2(guiSettings->getBotLaserX(),
                     guiSettings->getBotLaserY(),
                     guiSettings->getBotLaserZ());
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

            lVec = new CLaserVec(aPnt1, aVec, 0.5);
            context->SetDisplayMode(lVec, 1, Standard_False);
            context->Display(lVec, Standard_False);
        }
    }

    void reDrawScene() {
        const gp_Pnt globalAnchor(guiSettings->getBotAnchorX(),
                                  guiSettings->getBotAnchorY(),
                                  guiSettings->getBotAnchorZ());
        //The Part
        if(!ais_mdl.IsNull())
        {
            gp_Trsf trsfTr = curModel.Location().Transformation();
            const gp_Vec translation(guiSettings->getPartTrX() + mdlMover.getTrX(),
                                     guiSettings->getPartTrY() + mdlMover.getTrY(),
                                     guiSettings->getPartTrZ() + mdlMover.getTrZ());
            trsfTr.SetTranslation(translation);

            const gp_Pnt localAnchor(guiSettings->getPartCenterX(),
                                     guiSettings->getPartCenterY(),
                                     guiSettings->getPartCenterZ());
            gp_Trsf trsfLRX = curModel.Location().Transformation();
            trsfLRX.SetRotation(gp_Ax1(localAnchor, gp_Dir(1.,0.,0.)),
                                guiSettings->getPartRotationX() * DEGREE_K);
            gp_Trsf trsfLRY = curModel.Location().Transformation();
            trsfLRY.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,1.,0.)),
                                guiSettings->getPartRotationY() * DEGREE_K);
            gp_Trsf trsfLRZ = curModel.Location().Transformation();
            trsfLRZ.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,0.,1.)),
                                guiSettings->getPartRotationZ() * DEGREE_K);

            gp_Trsf trsfGRX = curModel.Location().Transformation();;
            trsfGRX.SetRotation(gp_Ax1(globalAnchor, gp_Dir(1.,0.,0.)),
                                mdlMover.getRX() * DEGREE_K);
            gp_Trsf trsfGRY = curModel.Location().Transformation();
            trsfGRY.SetRotation(gp_Ax1(globalAnchor, gp_Dir(0.,1.,0.)),
                                mdlMover.getRY() * DEGREE_K);
            gp_Trsf trsfGRZ = curModel.Location().Transformation();
            trsfGRZ.SetRotation(gp_Ax1(globalAnchor, gp_Dir(0.,0.,1.)),
                                mdlMover.getRZ() * DEGREE_K);

            gp_Trsf trsfSc = curModel.Location().Transformation();
            double scaleFactor = guiSettings->getPartScale();
            if (scaleFactor == 0.)
                scaleFactor = 1.;
            trsfSc.SetScale(localAnchor, scaleFactor);

            context->SetLocation(ais_mdl,
                                 trsfTr *
                                 trsfGRX * trsfGRY * trsfGRZ *
                                 trsfLRX * trsfLRY * trsfLRZ *
                                 trsfSc);
        }

        //The Grip
        if (guiSettings->isGripVisible() && !ais_grip.IsNull())
        {
            gp_Trsf trsfTr = gripModel.Location().Transformation();
            const gp_Vec translation(guiSettings->getGripTrX() + mdlMover.getTrX(),
                                     guiSettings->getGripTrY() + mdlMover.getTrY(),
                                     guiSettings->getGripTrZ() + mdlMover.getTrZ());
            trsfTr.SetTranslation(translation);

            const gp_Pnt localAnchor(guiSettings->getGripCenterX(),
                                     guiSettings->getGripCenterY(),
                                     guiSettings->getGripCenterZ());
            gp_Trsf trsfLRX = gripModel.Location().Transformation();
            trsfLRX.SetRotation(gp_Ax1(localAnchor, gp_Dir(1.,0.,0.)),
                                guiSettings->getGripRotationX() * DEGREE_K);
            gp_Trsf trsfLRY = gripModel.Location().Transformation();
            trsfLRY.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,1.,0.)),
                                guiSettings->getGripRotationY() * DEGREE_K);
            gp_Trsf trsfLRZ = gripModel.Location().Transformation();
            trsfLRZ.SetRotation(gp_Ax1(localAnchor, gp_Dir(0.,0.,1.)),
                                guiSettings->getGripRotationZ() * DEGREE_K);

            gp_Trsf trsfGRX = gripModel.Location().Transformation();
            trsfGRX.SetRotation(gp_Ax1(globalAnchor, gp_Dir(1.,0.,0.)),
                                mdlMover.getRX() * DEGREE_K);
            gp_Trsf trsfGRY = gripModel.Location().Transformation();
            trsfGRY.SetRotation(gp_Ax1(globalAnchor, gp_Dir(0.,1.,0.)),
                                mdlMover.getRY() * DEGREE_K);
            gp_Trsf trsfGRZ = gripModel.Location().Transformation();
            trsfGRZ.SetRotation(gp_Ax1(globalAnchor, gp_Dir(0.,0.,1.)),
                                mdlMover.getRZ() * DEGREE_K);

            gp_Trsf trsfSc = gripModel.Location().Transformation();
            double scaleFactor = guiSettings->getGripScale();
            if (scaleFactor == 0.)
                scaleFactor = 1.;
            trsfSc.SetScale(localAnchor, scaleFactor);

            context->SetLocation(ais_grip,
                                 trsfTr *
                                 trsfGRX * trsfGRY * trsfGRZ *
                                 trsfLRX * trsfLRY * trsfLRZ *
                                 trsfSc);
        }

        //Draw The Laser
        const QString botTxt = MainWindow::tr("Смещение:\n   X: %1\n   Y: %2\n   Z: %3\n"
                                              "Наклон:\n   α: %4\n   β: %5\n   γ: %6")
                .arg(mdlMover.getTrX(), 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getTrY(), 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getTrZ(), 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getRX() , 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getRY() , 11, 'f', 6, QChar('0'))
                .arg(mdlMover.getRZ() , 11, 'f', 6, QChar('0'));
        NCollection_Vector <Handle(AIS_InteractiveObject)> crossObj =
                cross.objects(botTxt.toStdString().c_str());
        for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(crossObj);
            it.More(); it.Next()) {
            const Handle(AIS_InteractiveObject)& obj = it.Value();
            context->Redisplay(obj, Standard_False);
        }

        gp_Pnt aPnt1(0., 0., 0.);
        gp_Pnt aPnt2(guiSettings->getBotLaserX(),
                     guiSettings->getBotLaserY(),
                     guiSettings->getBotLaserZ());
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

            context->Remove(lVec, Standard_False);
            lVec = new CLaserVec(aPnt1, aVec, 0.5);
            context->SetDisplayMode(lVec, 1, Standard_False);
            context->Display(lVec, Standard_False);
        }

        viewer->Redraw();
    }

    bool load(const QString &fName, CAbstractModelLoader &loader, const bool shading) {
        const TopoDS_Shape shape = loader.load(fName.toStdString().c_str());
        curModel = shape;
        updateModelsDefaultPosition(shading);
        reDrawScene();
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

private:
    CEmptyGuiSettings emptySettings;
    CAbstractGuiSettings *guiSettings;

    Handle(V3d_Viewer)             viewer;
    Handle(AIS_InteractiveContext) context;
    Standard_Integer zLayerId;

    TopoDS_Shape curModel;
    Handle(AIS_Shape) ais_mdl;
    TopoDS_Shape gripModel;
    Handle(AIS_Shape) ais_grip;
    CBotCross cross;
    Handle(CLaserVec) lVec;

    CEmptyBotSocket emptySocket;
    CAbstractBotSocket *botSocket;

    CModelMover mdlMover;

    QLabel * const stateLamp, * const attachLamp;
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

    configMenu();
    configToolBar();

    //Callib
    connect(ui->wSettings, SIGNAL(sigApplyRequest()), SLOT(slCallibApply()));

    ui->teJrnl->document()->setMaximumBlockCount(MAX_JRNL_ROW_COUNT);
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
    ui->mainView->setBackgroundColor(BG_CLR);
    ui->mainView->setStatsVisible(ui->actionFPS->isChecked());
}

void MainWindow::setSettings(CAbstractGuiSettings &settings)
{
    d_ptr->guiSettings = &settings;

    d_ptr->setMSAA(settings.getMsaa(), *ui->mainView);

    ui->wSettings->initFromGuiSettings(settings);

    V3d_TypeOfOrientation orientation = V3d_XposYnegZpos;
    if (d_ptr->guiSettings->getBotCoordType() == GUI_TYPES::ENCS_LEFT)
        orientation = V3d_XposYnegZneg;
    if (d_ptr->viewer->DefaultViewProj() != orientation) {
        d_ptr->viewer->SetDefaultViewProj(orientation);
        ui->mainView->setCoord(d_ptr->guiSettings->getBotCoordType());
    }

    d_ptr->updateModelsDefaultPosition(ui->actionShading->isChecked());
    d_ptr->reDrawScene();
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

    if (d_ptr->botSocket->isStarted() && d_ptr->botSocket->state() == BotSocket::ENSS_ATTACHED)
        d_ptr->reDrawScene();
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

    switch(d_ptr->mdlMover.socketState())
    {
        using namespace BotSocket;

        case ENSS_FALL:
            d_ptr->stateLamp->setPixmap(red);
            d_ptr->stateLamp->setToolTip(tr("Авария"));
            d_ptr->attachLamp->setPixmap(red);
            d_ptr->attachLamp->setToolTip(tr("Нет данных"));
            break;
        case ENSS_NOT_ATTACHED:
            d_ptr->stateLamp->setPixmap(green);
            d_ptr->stateLamp->setToolTip(tr("ОК"));
            d_ptr->attachLamp->setPixmap(red);
            d_ptr->attachLamp->setToolTip(tr("Нет захвата"));
            break;
        case ENSS_ATTACHED:
            d_ptr->stateLamp->setPixmap(green);
            d_ptr->stateLamp->setToolTip(tr("ОК"));
            d_ptr->attachLamp->setPixmap(green);
            d_ptr->attachLamp->setToolTip(tr("Захват"));
            break;
        default:
            qDebug() << "ERROR: MainWindow::updateBotSocketState: "
                        "unkown socket state == "
                     << d_ptr->mdlMover.socketState();
            break;
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
    d_ptr->updateModelsDefaultPosition(enabled);
    d_ptr->reDrawScene();
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

void MainWindow::slClearJrnl()
{
    ui->teJrnl->clear();
}

void MainWindow::slCallibApply()
{
    ui->wSettings->applyToGuiSettings(*d_ptr->guiSettings);

    V3d_TypeOfOrientation orientation = V3d_XposYnegZpos;
    if (d_ptr->guiSettings->getBotCoordType() == GUI_TYPES::ENCS_LEFT)
        orientation = V3d_XposYnegZneg;
    if (d_ptr->viewer->DefaultViewProj() != orientation) {
        d_ptr->viewer->SetDefaultViewProj(orientation);
        ui->mainView->setCoord(d_ptr->guiSettings->getBotCoordType());
    }

    d_ptr->updateModelsDefaultPosition(ui->actionShading->isChecked());
    d_ptr->reDrawScene();
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

void MainWindow::configMenu()
{
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

    //teJrnl
    connect(ui->actionClearJrnl, SIGNAL(triggered(bool)), SLOT(slClearJrnl()));
}

void MainWindow::configToolBar()
{
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
    d_ptr->startAction = ui->toolBar->addAction(QIcon(":/icons/Data/Icons/stop.png"),
                                                tr("Стоп"),
                                                this,
                                                SLOT(slStop()));

    //State and attach
    QFont fnt = font();
    fnt.setPointSize(18);
    const QPixmap red =
            QPixmap(":/Lamps/Data/Lamps/red.png").scaled(ui->toolBar->iconSize(),
                                                         Qt::IgnoreAspectRatio,
                                                         Qt::SmoothTransformation);

    QLabel * const txtState = new QLabel(tr("Соединение: "), ui->toolBar);
    txtState->setFont(fnt);
    txtState->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->toolBar->addWidget(txtState);
    d_ptr->stateLamp->setParent(ui->toolBar);
    ui->toolBar->addWidget(d_ptr->stateLamp);
    d_ptr->stateLamp->setPixmap(red);

    QLabel * const txtAttach = new QLabel(tr(" Захват: "), ui->toolBar);
    txtAttach->setFont(fnt);
    txtAttach->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->toolBar->addWidget(txtAttach);
    d_ptr->attachLamp->setParent(ui->toolBar);
    ui->toolBar->addWidget(d_ptr->attachLamp);
    d_ptr->attachLamp->setPixmap(red);
}

