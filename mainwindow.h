#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/highgui/highgui.hpp>
#include <QTimer>
#include <QObject>
#include "webcamviewer.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void timerEvent();

private:
    cv::VideoCapture mCapture;
    QTimer* mTimer;
    WebcamViewer* mWebcamViewer;
};

#endif // MAINWINDOW_H
