#-------------------------------------------------
#
# Project created by QtCreator 2013-05-19T14:46:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Mos
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mattoqimage.cpp \
    processingthread.cpp \
    imagebuffer.cpp \
    controller.cpp \
    capturethread.cpp

HEADERS  += mainwindow.h \
    mattoqimage.h \
    processingthread.h \
    imagebuffer.h \
    controller.h \
    capturethread.h \
    config.h

LIBS += -lopencv_core \
        -lopencv_imgproc \
        -lopencv_highgui \
        -lopencv_ml \
        -lopencv_video \
        -lopencv_features2d \
        -lopencv_calib3d \
        -lopencv_objdetect \
        -lopencv_contrib \
        -lopencv_legacy \
        -lopencv_flann \
        -lopencv_nonfree \
        -lopencv_stitching

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc
