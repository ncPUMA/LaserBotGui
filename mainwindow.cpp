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

constexpr double DEGREE_K = M_PI / 180.;

class CEmptyBotSocket : public CAbstractBotSocket
{
public:
    CEmptyBotSocket() : CAbstractBotSocket() { }

protected:
    BotSocket::TSocketError startSocket() final { return BotSocket::ENSE_NO; }
    void stopSocket() final { }
};

enum EN_UserActions
{
    ENUA_NO,

    ENUA_ADD_CALIB_POINT
};
typedef int TUserAction;

static const Standard_Integer Z_LAYER = 100;

class MainWindowPrivate
{
    friend class MainWindow;

private:
    MainWindowPrivate() :
        zLayerId(Z_LAYER),
        usrAction(ENUA_NO),
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

        for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(curModels);
            it.More(); it.Next()) {
            const Handle(AIS_InteractiveObject)& obj = it.Value();
            context->Display(obj, Standard_True);
            context->SetDisplayMode(obj, shading ? 1 : 0, false);
        }

        for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(vertices);
            it.More(); it.Next()) {
            const Handle(AIS_InteractiveObject)& obj = it.Value();
            context->Display(obj, Standard_False);
            context->SetZLayer(obj, zLayerId);
        }
        for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(labels);
            it.More(); it.Next()) {
            const Handle(AIS_InteractiveObject)& obj = it.Value();
            context->Display(obj, Standard_False);
            context->SetZLayer(obj, zLayerId);
        }

        context->Deactivate();
        context->Activate(AIS_Shape::SelectionMode(TopAbs_COMPOUND));
        viewer->Redraw();
    }

    bool load(const QString &fName, CAbstractModelLoader &loader, const bool shading) {
        curModels = loader.load(fName);
        vertices.Clear();
        labels.Clear();
        reDrawScene(shading);
        return !curModels.IsEmpty();
    }

    void startAppendCalibPoint() {
        if (!curModels.IsEmpty()) {
            context->Deactivate();
            context->Activate(curModels.First(), AIS_Shape::SelectionMode(TopAbs_VERTEX));
            context->SetSelected(curModels.First(), Standard_True);
            viewer->Redraw();
            usrAction = ENUA_ADD_CALIB_POINT;
        }
    }

    void mouseReleased(MainWindow * const qptr) {
        if (usrAction == ENUA_ADD_CALIB_POINT) {
            TopoDS_Vertex vertex;
            for (context->InitSelected(); context->MoreSelected(); context->NextSelected()) {
                Handle(SelectMgr_EntityOwner) owner = context->DetectedOwner();
                if (Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast (owner)) {
                    const TopoDS_Shape shape = brepOwner->Shape();
                    if (shape.ShapeType() == TopAbs_VERTEX) {
                        vertex = TopoDS::Vertex(shape);
                        break;
                    }
                }
            }

            if (!vertex.IsNull()) {
                const gp_Pnt pnt = BRep_Tool::Pnt(vertex);
                const QString caption = qptr->tr("Добавление реперной точки V%1")
                        .arg(vertices.Size() + 1);
                CCalibrationVertexDialog dialog(qptr);
                dialog.setWindowTitle(caption);
                dialog.setMdlVertex(pnt.X(), pnt.Y(), pnt.Z());
                if (dialog.exec() == QDialog::Accepted) {
                    const QString lblText = QString("V%1: %2 %3 %4")
                            .arg(vertices.Size() + 1)
                            .arg(dialog.getBotVertexX())
                            .arg(dialog.getBotVertexY())
                            .arg(dialog.getBotVertexZ());
                    Handle(AIS_Shape) vertexObj = new AIS_Shape(vertex);
                    Handle(AIS_TextLabel) label = new AIS_TextLabel();
                    label->SetText(lblText.toLocal8Bit().constData());
                    label->SetPosition(pnt);
                    context->Display(vertexObj, Standard_False);
                    context->Display(label, Standard_True);

                    context->SetZLayer(vertexObj, zLayerId);
                    context->SetZLayer(label, zLayerId);

                    vertices.Append(vertexObj);
                    labels.Append(label);
                }
            }
            usrAction = ENUA_NO;
            context->Deactivate();
            context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));
        }
    }

private:
    Handle(V3d_Viewer)             viewer;
    Handle(AIS_InteractiveContext) context;
    Standard_Integer zLayerId;

    NCollection_Vector <Handle(AIS_InteractiveObject)> curModels;

    NCollection_Vector <Handle(AIS_InteractiveObject)> vertices;
    NCollection_Vector <Handle(AIS_InteractiveObject)> labels;

    TUserAction usrAction;

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
    ui->actionShading->setCheckable(true);
    ui->actionShading->setChecked(true);
    connect(ui->actionShading, SIGNAL(toggled(bool)), SLOT(slShading(bool)));

    //Menu "Calibration"
    connect(ui->actionCalibApend, SIGNAL(triggered(bool)), SLOT(slCalibAppend()));
    connect(ui->actionCalibReset, SIGNAL(triggered(bool)), SLOT(slCalibReset()));
    ui->menuCalibration->deleteLater();

    //ToolBar
    ui->toolBar->addAction(QIcon::fromTheme("document-open"),
                           tr("Import"),
                           ui->actionImport,
                           SLOT(trigger()));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon::fromTheme("format-justify-fill"),
                           tr("Shading"),
                           ui->actionShading,
                           SLOT(toggle()));
    ui->toolBar->addSeparator();

    connect(ui->dockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
                            SLOT(slDockLocationChanged(Qt::DockWidgetArea)));

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
    connect(ui->mainView, SIGNAL(sigMouseReleased()),
            this, SLOT(slViewportMouseReleased()), Qt::QueuedConnection);
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

void MainWindow::slCalibAppend()
{
    d_ptr->startAppendCalibPoint();
}

void MainWindow::slCalibReset()
{
    d_ptr->vertices.Clear();
    d_ptr->labels.Clear();
    d_ptr->reDrawScene(ui->actionShading->isChecked());
}

void MainWindow::slViewportMouseReleased()
{
    d_ptr->mouseReleased(this);
}

void MainWindow::slDockLocationChanged(Qt::DockWidgetArea area)
{
    delete ui->dockWidgetContents->layout();
    QLayout *lay = nullptr;
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
        lay = new QVBoxLayout(ui->dockWidgetContents);
    else
        lay = new QHBoxLayout(ui->dockWidgetContents);
    lay->addWidget(ui->groupBox);
    lay->addWidget(ui->textEdit);
    ui->dockWidgetContents->setLayout(lay);
}

void MainWindow::slCallibApply()
{
    for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(d_ptr->curModels);
        it.More(); it.Next())
    {
        const Handle(AIS_Shape) ais_shape = Handle(AIS_Shape)::DownCast(it.Value());
        TopoDS_Shape shape = ais_shape->Shape();
        gp_Trsf trsf = shape.Location().Transformation();
        trsf.SetTranslation(gp_Pnt(0., 0., 0.),
                            gp_Pnt(ui->dsbTrX->value(), ui->dsbTrY->value(), ui->dsbTrZ->value()));
        gp_Quaternion quat =
                gp_Quaternion(gp_Vec(1., 0., 0.), ui->dsbRtX->value() * DEGREE_K) *
                gp_Quaternion(gp_Vec(0., 1., 0.), ui->dsbRtY->value() * DEGREE_K) *
                gp_Quaternion(gp_Vec(0., 0., 1.), ui->dsbRtZ->value() * DEGREE_K);
        trsf.SetRotation(quat);
        shape.Location(trsf);
        ais_shape->Set(shape);
        d_ptr->context->Redisplay(ais_shape, Standard_True);
    }

}

