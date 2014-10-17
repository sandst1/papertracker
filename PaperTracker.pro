#-------------------------------------------------
#
# Project created by QtCreator 2014-10-17T20:08:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl


TARGET = PaperTracker
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += opencv
LIBS += -lopencv_core -lopencv_highgui


LIBS += -lportaudio

INCLUDEPATH += /usr/local/opencv2

SOURCES += main.cpp\
        mainwindow.cpp \
    webcamviewer.cpp

HEADERS  += mainwindow.h \
    webcamviewer.h

FORMS    += mainwindow.ui
