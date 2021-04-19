QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BotSocket/cabstractbotsocket.cpp \
    Calibration/ccalibrationvertexdialog.cpp \
    ModelLoader/cabstractmodelloader.cpp \
    ModelLoader/cbreploader.cpp \
    ModelLoader/cigesloader.cpp \
    ModelLoader/cmodelloaderfactorymethod.cpp \
    ModelLoader/cobjloader.cpp \
    ModelLoader/csteploader.cpp \
    ModelLoader/cstlloader.cpp \
    Primitives/cbotcross.cpp \
    caspectwindow.cpp \
    cmainsettings.cpp \
    cmainviewport.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    BotSocket/bot_socket_types.h \
    BotSocket/cabstractbotsocket.h \
    Calibration/ccalibrationvertexdialog.h \
    ModelLoader/cabstractmodelloader.h \
    ModelLoader/cbreploader.h \
    ModelLoader/cigesloader.h \
    ModelLoader/cmodelloaderfactorymethod.h \
    ModelLoader/cobjloader.h \
    ModelLoader/csteploader.h \
    ModelLoader/cstlloader.h \
    Primitives/cbotcross.h \
    cabstractguisettings.h \
    caspectwindow.h \
    cmainsettings.h \
    cmainviewport.h \
    mainwindow.h

FORMS += \
    Calibration/ccalibrationvertexdialog.ui \
    mainwindow.ui

INCLUDEPATH += $$quote($$(CSF_OCCTIncludePath))

LIBS += -lTKernel -lTKMath -lTKService -lTKV3d -lTKOpenGl \
        -lTKBRep -lTKIGES -lTKSTL -lTKVRML -lTKSTEP -lTKSTEPAttr -lTKSTEP209 \
        -lTKSTEPBase -lTKGeomBase -lTKGeomAlgo -lTKG3d -lTKG2d \
        -lTKXSBase -lTKShHealing -lTKHLR -lTKTopAlgo -lTKMesh -lTKPrim \
        -lTKCDF -lTKBool -lTKBO -lTKFillet -lTKOffset -lTKLCAF -lTKCAF -lTKVCAF \
                -lTKBin -lTKXml -lTKRWMesh

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .gitignore \
    LICENSE \
    README.md
