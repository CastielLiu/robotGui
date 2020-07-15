#-------------------------------------------------
#
# Project created by QtCreator 2020-02-23T17:56:21
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia
QT       += multimediawidgets
QT       += network
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

message($$[QT_VERSION])

TARGET = robotGui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += serialPort

SOURCES += \
    cameraconfigdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    navigation.cpp \
    udpreceiver.cpp \
    audiohandler.cpp \
    utils.cpp \
    vediohandler.cpp \
    udpsender.cpp \
    myqlabel.cpp \
    cameraframegrabber.cpp \
    cvimagegraber.cpp \
    remotecontrol.cpp \
    mywidget.cpp \
    globalvariable.cpp \
    biologicalradar.cpp \
    uiaction.cpp \
    uipushbutton.cpp \
    audioconfigdialog.cpp


HEADERS += \
    cameraconfigdialog.h \
    fifo.hpp \
        mainwindow.h \
    navigation.h \
    udpreceiver.h \
    audiohandler.h \
    utils.h \
    vediohandler.h \
    udpsender.h \
    circlebuffer.h \
    myqlabel.h \
    cameraframegrabber.h \
    cvimagegraber.h \
    remotecontrol.h \
    mywidget.h \
    globalvariable.h \
    structs.h \
    enums.h \
    biologicalradar.h \
    audioconfigdialog.h

FORMS += \
        cameraconfigdialog.ui \
        mainwindow.ui \
    audioconfigdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

#如果使用TX2,需添加此宏，否则注释!
#DEFINES += TX2

win32{
    message("current os is win32")
    INCLUDEPATH += C:\opencv\build\include
    LIBS += C:\opencv\opencv-build\bin\libopencv_*
}
unix{
    message("current os is unix")
    QMAKE_CFLAGS_ISYSTEM = -I
    if(contains(DEFINES,TX2)){
        INCLUDEPATH += /usr/include /usr/include/opencv /usr/include/opencv2
        LIBS += /usr/lib/libopencv_*
    }else{
        INCLUDEPATH += /usr/local/include  /usr/local/include/opencv /usr/local/include/opencv2
        LIBS += /usr/local/lib/libopencv_*
    }
}
