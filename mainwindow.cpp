#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    mWebcamViewer = new WebcamViewer(this);
    mTimer = new QTimer(this);
    QObject::connect(mTimer, SIGNAL(timeout()), this, SLOT(timerEvent()));

    this->setCentralWidget(mWebcamViewer);

    if (!mCapture.isOpened())
    {
        mCapture.open(1);
    }


    mTimer->setInterval(50);
    mTimer->start();
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
