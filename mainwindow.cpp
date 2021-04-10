#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>

#include <AIS_Shape.hxx>

#include "ModelLoader/cmodelloaderfactorymethod.h"
#include "ModelLoader/cabstractmodelloader.h"

class MainWindowPrivate
{
public:
    MainWindowPrivate() { }

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

    Handle(V3d_Viewer)             viewer;
    Handle(AIS_InteractiveContext) context;

    NCollection_Vector <Handle(AIS_InteractiveObject)> curModels;
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
        d_ptr->context->RemoveAll(Standard_False);
        CAbstractModelLoader &loader = factory.loader(selectedFilter);
        d_ptr->curModels = loader.load(fName);
        if (d_ptr->curModels.IsEmpty())
        {
            QMessageBox::critical(this,
                                  tr("Ошибка загрузки файла"),
                                  tr("Ошибка загрузки файла"));
        }
        else
        {
            for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(d_ptr->curModels);
                it.More(); it.Next())
                d_ptr->context->Display(it.Value(), Standard_True);
        }
        d_ptr->viewer->Redraw();
        ui->mainView->fitInView();
    }
}

void MainWindow::slExit()
{
    close();
}

