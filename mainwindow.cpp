#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <highgui.h>
#include "synth/synth.h"
#include "synth/operator.h"

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

    mCapture.set(CV_CAP_PROP_FRAME_WIDTH, SCREEN_WIDTH);
    mCapture.set(CV_CAP_PROP_FRAME_HEIGHT, SCREEN_HEIGHT);


//    mAudioControl->start();

//    QMutex& waitAudio = mAudioControl->getStartLock();
//    qDebug("main::waiting for Audio thread to initialize");
//    waitAudio.lock();
//    qDebug("main::Audio thread initialized");
//    waitAudio.unlock();


    mTimer->setInterval(50);
    mTimer->start();

//    Synth* synth = mAudioControl->getSynth();
//    synth->setWaveType(Operator::SQUARE, 0);
//    synth->setVolume(85);
//    synth->octaveDown();

//    LFO* lfo = synth->getLfo();

//    lfo->setAmp(100);
//    lfo->setFreq(15);


//QMetaObject::invokeMethod(mAudioControl, "pressKey", Q_ARG(int, Synth::KEY_C2), Q_ARG(unsigned int, 0));
    //mAudioControl->pressKey(Synth::KEY_D, 0);
    //mAudioControl->pressKey(Synth::KEY_C2, 1);

    setFixedSize(SCREEN_WIDTH, SCREEN_HEIGHT);
}

MainWindow::~MainWindow()
{
}


static cv::Mat processImg(cv::Mat* image)
{

    cv::Mat mat;

    cv::cvtColor(*image, mat, CV_BGR2GRAY);

    //cv::threshold(mat, mat, 150, 255, CV_THRESH_BINARY_INV);
    //cv::medianBlur(mat, mat, 3);
    //cv::adaptiveThreshold(mat, mat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 15, 0);

    cv::adaptiveThreshold(mat, mat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 15, 0);


    cv::Mat cmat;
    cv::cvtColor(mat, cmat, CV_GRAY2BGR);
//    vector<cv::Vec2f> lines;
//    cv::HoughLines(mat, lines, 1, CV_PI/180, 150, 0, 0);

//    // draw lines
//       for( size_t i = 0; i < lines.size(); i++ )
//       {
//           float rho = lines[i][0], theta = lines[i][1];
//           cv::Point pt1, pt2;
//           double a = cos(theta), b = sin(theta);
//           double x0 = a*rho, y0 = b*rho;
//           pt1.x = cvRound(x0 + 1000*(-b));
//           pt1.y = cvRound(y0 + 1000*(a));
//           pt2.x = cvRound(x0 - 1000*(-b));
//           pt2.y = cvRound(y0 - 1000*(a));
//           line( cmat, pt1, pt2, cv::Scalar(0,0,255), 3, CV_AA);
//       }

    vector<Vec4i> lines;

    int rho = 1;
    float theta = CV_PI/180;
    int threshold = 75;
    int minLength = 75;
    int maxGap = 2;

    HoughLinesP(mat, lines, rho, theta, threshold, minLength, maxGap);
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( cmat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
    }



    return cmat;

}

void MainWindow::timerEvent()
{
    cv::Mat image;
    mCapture >> image;

    image = processImg(&image);



    mWebcamViewer->showImage(image);

}

