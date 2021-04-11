#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class OpenGl_GraphicDriver;
class MainWindowPrivate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init(OpenGl_GraphicDriver &driver);

private slots:
    //File menu slots
    void slImport();
    void slExit();

    //Menu Calibration
    void slCalibAppend();
    void slCalibReset();

    //viewport
    void slViewportMouseReleased();

private:
    Ui::MainWindow *ui;
    MainWindowPrivate * const d_ptr;
};
#endif // MAINWINDOW_H
