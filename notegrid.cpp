#include "notegrid.h"

#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <math.h>
#include <stdio.h>

#include <set>

#include "synth/synth.h"

#include <QDebug>

using namespace cv;
using namespace std;

const int HOR_THOLD = 15; // px
const int VER_THOLD = 15; // px
const double ANGLE_THOLD = 10; // deg
const int MERGE_LINES_DIST = 5;

int findCount = 0;
int perspectiveSetup = false;
int perspectiveCount = 0;

NoteGrid::NoteGrid(QObject *parent) :
    QObject(parent)
{
    mStatus = FIXING_PERSPECTIVE;

    mSimpleBlobParams.minDistBetweenBlobs = 5.0f;
    mSimpleBlobParams.filterByInertia = false;
    mSimpleBlobParams.filterByConvexity = false;

    mSimpleBlobParams.filterByColor = false;

    mSimpleBlobParams.filterByCircularity = false;
    mSimpleBlobParams.filterByArea = true;
    mSimpleBlobParams.minArea = 40.0f;
    mSimpleBlobParams.maxArea = 400.0f;

    mSimpleBlob = new SimpleBlobDetector(mSimpleBlobParams);

    mSimpleBlob->create("SimpleBlob");

    mClock = new PlayClock(this);
}

void NoteGrid::setAudioControl(AudioControl* ac)
{
    mAudioControl = ac;
}

static int yIndexToNote(int index)
{
    if (index > 30)
    {
        return Synth::KEY_G3;
    }
    return (30 - index);
}

int median(vector<int> vec)
{
        typedef vector<int>::size_type vec_sz;

        vec_sz size = vec.size();
        if (size == 0)
             return 0;

        sort(vec.begin(), vec.end());

        vec_sz mid = size/2;

        double res = size % 2 == 0 ? (vec[mid] + vec[mid-1]) / 2 : vec[mid];
        return (int)res;
}

bool released = true;
Mat NoteGrid::gridFound(Mat *image)
{
    cvtColor(*image, mLatestFrame, CV_BGR2GRAY);

    int curStep = mClock->step();
    vector<Point> curCol = mNoteGrid.at(curStep);

    Mat mat;
    cvtColor(*image, mat, CV_BGR2GRAY);
    medianBlur(mat, mat, 3);
    medianBlur(mat, mat, 5);

    Mat noteDst;

    adaptiveThreshold(mat, noteDst, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 23, 0);

    /*
     *Debug rectangles!
     *
    cvtColor(mat, mat, CV_GRAY2BGR);
    for (int aa = 0; aa < mNoteGrid.size(); aa++)
    {
        vector<Point> aCol = mNoteGrid.at(aa);
        for (int i = 0; i < aCol.size(); i++)
        {
            Point curP = aCol.at(i);

            rectangle(mat, Rect(curP.x-2, curP.y-2, 6, 6),Scalar(0, 255, 0), 1);
            //circle(mat, Point(curP.x, curP.y), 5, Scalar(0, 255, 0), 1);
        }
    }

    return mat;
    */



    //medianBlur(mat, mat, 5);
    //GaussianBlur(mat, mat, Size(15,15), 0.4);
    //threshold(mat, mat, 135, 255, CV_THRESH_BINARY_INV);
    //qDebug() << mat.cols << ", " << mat.rows;
    //return mat;


    int activeNote = -1;

    // Find currently active note
    for (int i = 1; i < curCol.size(); i++)
    {
        Point curP = curCol.at(i);

        uchar* curPtr = &noteDst.data[(curP.y-2)*SCREEN_WIDTH + curP.x-2];

        //circle(*image, Point(curP.x+3, curP.y+3), 3, Scalar(0, 255, 0), 1);

        //qDebug() << curP.y;
        vector<int> pixvals;
        // get the mean of the note
        for (int k = 0; k < 6; k++)
        {
            for (int j = 0; j < 6; j++)
            {
                pixvals.push_back((int)(*curPtr));
                curPtr++;
            }
            curPtr += (SCREEN_WIDTH - 6);
        }


        //qDebug() << "step " << curStep << ", (x,y):" << curP.x << "," << curP.y << ", median: " << median(pixvals) << ", mean: " << sum;


        if (median(pixvals) > 128)
        {
            activeNote = i;
        }
    }

    if (activeNote != -1)
    {
        qDebug() << "step: " << curStep << ", note: " << activeNote;
        //mAudioControl->releaseKey(0);
        mAudioControl->pressKey(yIndexToNote(activeNote), 0);



    }
    else
    {
        mAudioControl->releaseKey(0);
        released = true;
    }

    Point p = curCol.at(0);
    line(*image, Point(p.x, 0), Point(p.x, SCREEN_HEIGHT), Scalar(0,0,255),2);






    return *image;
}

Mat NoteGrid::findGrid(Mat *image)
{
    if (mStatus == FIXING_PERSPECTIVE)
    {
        return *image;
    }

    if (mStatus == GRID_FOUND)
    {
        return gridFound(image);
    }

    Mat mat;
    cvtColor(*image, mat, CV_BGR2GRAY);

    adaptiveThreshold(mat, mat, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 21, -1);

    medianBlur(mat, mat, 3);
    //GaussianBlur(mat, mat, Size(5, 5), 1.6);
    //boxFilter(mat, mat, -1, Size(3,3));

    cvtColor(mat, mat, CV_GRAY2BGR);

    vector<KeyPoint> kpoints;
    mSimpleBlob->detect(mat, kpoints);

    multimap<int, int> coordsByX;
    multimap<int, int> coordsByY;

    for (int i = 0; i < kpoints.size(); i++)
    {
        float x = kpoints[i].pt.x;
        float y = kpoints[i].pt.y;

        int xint = (int)x;
        int yint = (int)y;

        coordsByX.insert(make_pair(xint, yint));
        coordsByY.insert(make_pair(yint, xint));


        circle(mat, kpoints[i].pt, 3, Scalar(0, 0, 255), 2);
    }

    findCount++;
    if (findCount == 15)
    {
        mStatus = GRID_FOUND;

        // Interpolate the note grid

        vector<int> xcoords;

        int prevx = -1;
        int xsum = 0;
        int xcnt = 0;
        for (multimap<int,int>::iterator it = coordsByX.begin(); it != coordsByX.end(); it++)
        {
            int x = it->first;
            if (prevx != -1)
            {
                int d = x - prevx;
                if (d < 10)
                {
                    xsum += x;
                    xcnt++;
                }
                else
                {
                    int xmean = xsum / xcnt;
                    xcoords.push_back(xmean);
                    xsum = 0;
                    xcnt = 0;
                    prevx = -1;
                }

            }
            prevx = x;

            //qDebug() << it->first << "," << it->second;
        }

        for (int i = 0; i < xcoords.size(); i++)
        {
            qDebug() << "x: " << xcoords[i];
        }


        vector<int> ycoords;

        int prevy = -1;
        int ysum = 0;
        int ycnt = 0;
        for (multimap<int,int>::iterator it = coordsByY.begin(); it != coordsByY.end(); it++)
        {
            int y = it->first;

            if (prevy != -1)
            {
                int d = y - prevy;
                if (d < 4)
                {
                    ysum += y;
                    ycnt++;
                }
                else
                {
                    int ymean = ysum;
                    if (ycnt != 0)
                    {
                        ymean /= ycnt;
                    }
                    ycoords.push_back(ymean);

                    ysum = 0;
                    ycnt = 0;
                    prevy = -1;
                }

            }
            prevy = y;
        }

        for (int i = 0; i < ycoords.size(); i++)
        {
            qDebug() << "y: " << ycoords[i];
        }


        for (int i = 0; i < xcoords.size(); i++)
        {
            vector<Point> column;
            for (int j = 0; j < ycoords.size(); j++)
            {
                column.push_back(Point(xcoords.at(i), ycoords.at(j)));
            }
            mNoteGrid.push_back(column);
        }

        mClock->start();
    }

    return mat;

}

string NoteGrid::getLineId(Vec4i line, bool horizontal)
{
    char dst[100];
    if (horizontal)
    {
        int key = (int)(((double)line[1]+line[3])/2);
        sprintf(dst, "%03d", key);
    }
    else
    {
        int key = (int)(((double)line[0]+line[2])/2);
        sprintf(dst, "%03d", key);
    }
    return string(dst);
}



Mat NoteGrid::correctPerspective(Mat* image)
{
    if (perspectiveSetup)
    {
        Mat src = image->clone();
        Mat dst = Mat::zeros(SCREEN_HEIGHT, SCREEN_WIDTH, CV_8UC3);
        warpPerspective(src, dst, mTransMtx, dst.size());
        return dst;
    }


    if (mStatus == CLICKS_GOT)
    {
        Mat dst = Mat::zeros(SCREEN_HEIGHT, SCREEN_WIDTH, CV_8UC3);

        vector<Point2f> dest_pts;
        dest_pts.push_back(Point2f(0, 0));
        dest_pts.push_back(Point2f(dst.cols, 0));
        dest_pts.push_back(Point2f(dst.cols, dst.rows));
        dest_pts.push_back(Point2f(0, dst.rows));
        mDestCoords = dest_pts;

        Mat src = image->clone();

        Mat transmtx = getPerspectiveTransform(mPaperCoords, dest_pts);
        mTransMtx = transmtx;

        warpPerspective(src, dst, transmtx, dst.size());

        perspectiveSetup = true;

        mStatus = FINDING_GRID;
        return dst;
    }
    return *image;
}

void NoteGrid::mousePressed(int x, int y)
{
    if (mStatus == FIXING_PERSPECTIVE)
    {
        mPaperCoords.push_back(Point2f(x, y));
        if (mPaperCoords.size() == 4)
        {
            mStatus = CLICKS_GOT;
        }
    }
    else if (mStatus == GRID_FOUND)
    {
        uchar* data = mLatestFrame.data;
        qDebug() << "pixel value at " << x << ", " << y << ": " << data[y*SCREEN_WIDTH + x];
    }
}

