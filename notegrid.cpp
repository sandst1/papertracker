#include "notegrid.h"

#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <math.h>
#include <stdio.h>

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
}

Mat NoteGrid::gridFound(Mat *image)
{
    for (int i = 0; i < mNoteGrid.size(); i++)
    {
        vector<Point> col = mNoteGrid.at(i);
        for (int j = 0; j < col.size(); j++)
        {
            Point p = col.at(j);
            circle(*image, p, 5, Scalar(0,0,255),2);
        }
    }

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
}

