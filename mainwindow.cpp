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


void MainWindow::timerEvent()
{
    Mat image;
    mCapture >> image;

    image = correctPerspective(&image);

    //image = mNoteGrid->findGrid(&image);

    mWebcamViewer->showImage(image);

}

Mat MainWindow::correctPerspective(Mat* image)
{
    Mat mat;
    cvtColor(*image, mat, CV_BGR2GRAY);

    threshold(mat, mat, 120, 255, CV_THRESH_BINARY);

    bool x1found = false;
    bool x2found = false;

    bool x3found = false;
    bool x4found = false;


    int x1 = 0;
    int x2 = 0;
    int x3 = 0;
    int x4 = 0;


    uchar* left = &mat.data[0];
    uchar* right = &mat.data[mat.cols-1];
    uchar* left2 = &mat.data[mat.cols*(mat.rows-1)];
    uchar* right2 = &mat.data[mat.cols*(mat.rows-1)+mat.cols-1];

    for (int i = 0; i < mat.cols; i++)
    {
        if (!x1found)
        {
            if (*left == 255)
            {
                x1found = true;
                x1 = i;
            }
            else
            {
                left++;
            }
        }

        if (!x2found)
        {
            if (*right == 255)
            {
                x2found = true;
                x2 = SCREEN_WIDTH - i;
            }
            else
            {
                right--;
            }
        }

        if (!x3found)
        {
            if (*right2 == 255)
            {
                x3found = true;
                x3 = SCREEN_WIDTH - i;
            }
            else
            {
                right2--;
            }
        }

        if (!x4found)
        {
            if (*left2 == 255)
            {
                x4found = true;
                x4 = i;
            }
            else
            {
                left2++;
            }
        }


        if (x1found && x2found && x3found && x4found)
        {
            break;
        }
    }

    vector<Point2f> corners;
    corners.push_back(Point2f(x1, 10));
    corners.push_back(Point2f(x2, 10));
    corners.push_back(Point2f(x3, mat.rows-10));
    corners.push_back(Point2f(x4, mat.rows-10));

    Mat dst = Mat::zeros(SCREEN_HEIGHT, SCREEN_WIDTH, CV_8UC3);

    vector<Point2f> dest_pts;
    dest_pts.push_back(Point2f(0, 0));
    dest_pts.push_back(Point2f(dst.cols, 0));
    dest_pts.push_back(Point2f(dst.cols, dst.rows));
    dest_pts.push_back(Point2f(0, dst.rows));

    Mat src = image->clone();

    Mat transmtx = getPerspectiveTransform(corners, dest_pts);

    warpPerspective(src, dst, transmtx, dst.size());

    return dst;
}
