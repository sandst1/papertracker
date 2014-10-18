#ifndef NOTEGRID_H
#define NOTEGRID_H

#include <QObject>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

class NoteGrid : public QObject
{
    Q_OBJECT
public:
    enum Status {
        NOT_INITIALIZED,
        FINDING_GRID,
        GRID_FOUND
    };

    explicit NoteGrid(QObject *parent = 0);

    inline Status status() { return mStatus; }

signals:

public slots:
    cv::Mat findGridLines(cv::Mat* image);
    cv::Mat findGrid(cv::Mat* image);
    cv::Mat gridFound(cv::Mat* image);
    cv::Mat gridLinesFound(cv::Mat* image);

    cv::Mat correctPerspective(cv::Mat* image);

private:
    std::string getLineId(cv::Vec4i line, bool horizontal);

    Status mStatus;

    std::map<std::string, cv::Vec4i> mHorMap;
    std::map<std::string, cv::Vec4i> mVerMap;

    std::vector<cv::Vec4i> mHorLines;
    std::vector<cv::Vec4i> mVerLines;

    cv::SimpleBlobDetector::Params mSimpleBlobParams;
    cv::FeatureDetector* mSimpleBlob;

    std::vector<cv::Point2f> mPaperCoords;
    std::vector<cv::Point2f> mDestCoords;
    cv::Mat mTransMtx;

    std::vector<std::vector<cv::Point> > mNoteGrid;


};

#endif // NOTEGRID_H
