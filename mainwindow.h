#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class OpenGl_GraphicDriver;
class MainWindowPrivate;
class CAbstractBotSocket;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init(OpenGl_GraphicDriver &driver);
    void setBotSocket(CAbstractBotSocket &botSocket);

private slots:
    //Menu "File"
    void slImport();
    void slExit();

    //Menu "View"
    void slShading(bool enabled);

    //Menu "Calibration"
    void slCalibAppend();
    void slCalibReset();

    //viewport
    void slViewportMouseReleased();

    //dockWidget
    void slDockLocationChanged(Qt::DockWidgetArea area);

    //callib
    void slCallibApply();

private:
    Ui::MainWindow *ui;
    MainWindowPrivate * const d_ptr;
};
#endif // MAINWINDOW_H
