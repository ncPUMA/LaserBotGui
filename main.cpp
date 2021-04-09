#include "mainwindow.h"

#include <QApplication>

#include <OpenGl_GraphicDriver.hxx>
#include <OSD_Environment.hxx>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    static Handle(OpenGl_GraphicDriver) aGraphicDriver;
    if (aGraphicDriver.IsNull())
    {
        Handle(Aspect_DisplayConnection) aDisplayConnection;
#if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
        aDisplayConnection = new Aspect_DisplayConnection(OSD_Environment("DISPLAY").Value());
#endif
        aGraphicDriver = new OpenGl_GraphicDriver(aDisplayConnection);
    }

    MainWindow w;
    w.init(*aGraphicDriver);
    w.show();
    return a.exec();
}
