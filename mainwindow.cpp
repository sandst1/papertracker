#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <highgui.h>
#include "synth/synth.h"
#include "synth/operator.h"

#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    mWebcamViewer = new WebcamViewer(this);
    mAudioControl = new AudioControl(this);
    mTimer = new QTimer(this);
    QObject::connect(mTimer, SIGNAL(timeout()), this, SLOT(timerEvent()));

    this->setCentralWidget(mWebcamViewer);

    if (!mCapture.isOpened())
    {
        mCapture.open(1);
    }

    mCapture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    mCapture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

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
    synth->setVolume(85);
    synth->octaveUp();

QMetaObject::invokeMethod(mAudioControl, "pressKey", Q_ARG(int, Synth::KEY_C2), Q_ARG(unsigned int, 0));

    setFixedSize(1280, 720);
}

MainWindow::~MainWindow()
{
}

void MainWindow::timerEvent()
{
    cv::Mat image;
    mCapture >> image;

    mWebcamViewer->showImage(image);


}
