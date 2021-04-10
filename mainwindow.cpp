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
    friend class MainWindow;

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

    bool load(const QString &fName, CAbstractModelLoader &loader) {
        context->RemoveAll(Standard_False);
        curModels = loader.load(fName);
        const bool result = !curModels.IsEmpty();
        if (result)
        {
            for(NCollection_Vector <Handle(AIS_InteractiveObject)>::Iterator it(curModels);
                it.More(); it.Next())
            {
                const Handle(AIS_InteractiveObject)& obj = it.Value();
                context->Display(obj, Standard_True);
                context->SetDisplayMode(obj, 1, false);
            }
            context->Deactivate();
            context->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));
        }
        viewer->Redraw();
        return result;
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
        CAbstractModelLoader &loader = factory.loader(selectedFilter);
        if (d_ptr->load(fName, loader))
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

