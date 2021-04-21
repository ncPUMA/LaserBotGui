#include "mainwindow.h"

#include <QApplication>

#include <OpenGl_GraphicDriver.hxx>
#include <OSD_Environment.hxx>

#include "cmainsettings.h"

#include "BotSocket/cbotsocketimitator.h"

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

    CMainSettings settings;
#ifdef Q_OS_WIN
    settings.setSettingsFName("LBOT.INI");
#else
    settings.setSettingsFName("conf.cfg");
#endif
    MainWindow w;
    w.setSettings(settings.guiSettings());
    w.init(*aGraphicDriver);

    CBotSocketImitator imitator;
    imitator.setSettings(settings.socketSettings());
    imitator.setMessageInterval(20);
    w.setBotSocket(imitator);

    w.show();
    return a.exec();
}
