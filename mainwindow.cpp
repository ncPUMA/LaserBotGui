#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>

class MainWindowPrivate
{
public:
    MainWindowPrivate() :
        viewer(nullptr),
        context(nullptr)
    { }

    ~MainWindowPrivate() {
        delete viewer;
        delete context;
    }

    void init(OpenGl_GraphicDriver &driver) {
        viewer = new V3d_Viewer(&driver);
        viewer->SetDefaultViewSize(1000.);
        viewer->SetDefaultViewProj(V3d_XposYnegZpos);
        viewer->SetComputedMode(Standard_True);
        viewer->SetDefaultComputedMode(Standard_True);
        viewer->SetDefaultLights();
        viewer->SetLightOn();

        context = new AIS_InteractiveContext(viewer);
    }

    V3d_Viewer             *viewer;
    AIS_InteractiveContext *context;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    d_ptr(new MainWindowPrivate())
{
    ui->setupUi(this);
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

