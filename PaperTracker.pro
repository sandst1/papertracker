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
    webcamviewer.cpp \
    synth/audiocontrol.cpp \
    synth/effect.cpp \
    synth/effects.cpp \
    synth/flanger.cpp \
    synth/lfo.cpp \
    synth/operator.cpp \
    synth/phaser.cpp \
    synth/reverb.cpp \
    synth/synth.cpp \
    synth/wah.cpp

HEADERS  += mainwindow.h \
    webcamviewer.h \
    synth/audiocontrol.h \
    synth/constants_types.h \
    synth/effect.h \
    synth/effects.h \
    synth/flanger.h \
    synth/lfo.h \
    synth/operator.h \
    synth/phaser.h \
    synth/reverb.h \
    synth/synth.h \
    synth/wah.h

FORMS    += mainwindow.ui
