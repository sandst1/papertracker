#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <highgui.h>
#include "synth/synth.h"
#include "synth/operator.h"

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    mWebcamViewer = new WebcamViewer(this);

    QObject::connect(mWebcamViewer, SIGNAL(mousePress(int,int)), this, SLOT(mousePressed(int,int)));

    mAudioControl = new AudioControl(this);
    mNoteGrid = new NoteGrid(this);

    mTimer = new QTimer(this);
    QObject::connect(mTimer, SIGNAL(timeout()), this, SLOT(timerEvent()));

    this->setCentralWidget(mWebcamViewer);

    if (!mCapture.isOpened())
    {
        mCapture.open(1);
    }

    mCapture.set(CV_CAP_PROP_FRAME_WIDTH, SCREEN_WIDTH);
    mCapture.set(CV_CAP_PROP_FRAME_HEIGHT, SCREEN_HEIGHT);

    mNoteGrid->setAudioControl(mAudioControl);
    mAudioControl->start();



    QMutex& waitAudio = mAudioControl->getStartLock();
    qDebug("main::waiting for Audio thread to initialize");
    waitAudio.lock();
    qDebug("main::Audio thread initialized");
    waitAudio.unlock();


    mTimer->setInterval(50);
    mTimer->start();

    Synth* synth = mAudioControl->getSynth();
    synth->setWaveType(Operator::SINE, 0);
    //synth->setVolume(85);
    synth->octaveDown();


//QMetaObject::invokeMethod(mAudioControl, "pressKey", Q_ARG(int, Synth::KEY_C2), Q_ARG(unsigned int, 0));
    //mAudioControl->pressKey(Synth::KEY_D, 0);
    //mAudioControl->pressKey(Synth::KEY_C2, 1);

    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);


}

MainWindow::~MainWindow()
{
}

void MainWindow::mousePressed(int x, int y)
{
    qDebug() << "mouse pressed" << x << ", " << y;
    mNoteGrid->mousePressed(x, y);
}

void MainWindow::timerEvent()
{
    Mat image;
    mCapture >> image;

    image = mNoteGrid->correctPerspective(&image);

    image = mNoteGrid->findGrid(&image);

    mWebcamViewer->showImage(image);

}
