#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/highgui/highgui.hpp>
#include <QTimer>
#include <QObject>
#include "webcamviewer.h"
#include "synth/audiocontrol.h"
#include "notegrid.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void mousePressed(int x, int y);

private slots:
    void timerEvent();

private:

    cv::VideoCapture mCapture;
    QTimer* mTimer;
    WebcamViewer* mWebcamViewer;
    AudioControl* mAudioControl;
    NoteGrid* mNoteGrid;

};

#endif // MAINWINDOW_H
