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

NoteGrid::NoteGrid(QObject *parent) :
    QObject(parent)
{
    mStatus = NOT_INITIALIZED;

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

int findCount = 0;
int perspectiveSetup = false;

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

Mat NoteGrid::gridLinesFound(Mat* image)
{
    qDebug("gridFound");
    for ( int i = 0; i < mHorLines.size(); i++)
    {
        Vec4i l = mHorLines.at(i);
        line( *image, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);
    }

    for ( int i = 0; i < mVerLines.size(); i++)
    {
        Vec4i l = mVerLines.at(i);
        line( *image, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,255,0), 1, CV_AA);
    }

    return *image;

}

Mat NoteGrid::findGrid(Mat *image)
{
    if (mStatus == NOT_INITIALIZED)
    {
        mStatus = FINDING_GRID;
    }
    else if (mStatus == GRID_FOUND)
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
                    int ymean = ysum / ycnt;
                    ycoords.push_back(ymean);
                    qDebug() << "ymean " << ymean;

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

Mat NoteGrid::findGridLines(Mat* image)
{
    if (mStatus == NOT_INITIALIZED)
    {
        mStatus = FINDING_GRID;
    }
    else if (mStatus == GRID_FOUND)
    {
        return gridLinesFound(image);
    }

    Mat mat;

    cvtColor(*image, mat, CV_BGR2GRAY);

    adaptiveThreshold(mat, mat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 19, 0);


    medianBlur(mat, mat, 5);





    Mat cmat;
    cvtColor(mat, cmat, CV_GRAY2BGR);

    vector<Vec4i> lines;

    int rho = 1;
    float theta = CV_PI/180;
    int threshold = 75;
    int minLength = 60;
    int maxGap = 2;

    HoughLinesP(mat, lines, rho, theta, threshold, minLength, maxGap);
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];

        // Find horizontal lines

        double dx = abs((double)(l[0]-l[2]));
        double dy = abs((double)(l[1]-l[3]));

        // where |y1-y2| < something
        if (abs(l[1] - l[3]) < HOR_THOLD)
        {
            string lineId = getLineId(l, true);

            //double angleDeg = abs(atan2(dy, dx) * 180 / CV_PI);

            //if (angleDeg < ANGLE_THOLD)
            //{
                mHorMap[lineId] = l;

                line( cmat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);
            //}


        }

        // Find vertical lines
        // where |x1-x2| < something
        if (abs(l[0] - l[2]) < VER_THOLD)
        {
            string lineId = getLineId(l, false);
            //double angleDeg = abs(atan2(dx, dy) * 180 / CV_PI);

            //if (angleDeg < ANGLE_THOLD)
            //{
                mVerMap[lineId] = l;
                line( cmat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,255,0), 1, CV_AA);
            //}
        }
    }


    findCount++;
    if (findCount == 15)
    {
        mStatus = GRID_FOUND;

        for (map<string,Vec4i>::iterator iter = mHorMap.begin(); iter != mHorMap.end(); iter++)
        {
            mHorLines.push_back(iter->second);
        }

        for (map<string,Vec4i>::iterator iter = mVerMap.begin(); iter != mVerMap.end(); iter++)
        {
            mVerLines.push_back(iter->second);
        }
    }




    return cmat;
}

string NoteGrid::getLineId(Vec4i line, bool horizontal)
{
    char dst[100];

    //sprintf(dst,"%d,%d,%d,%d", line[0], line[1], line[2], line[3]);

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
    corners.push_back(Point2f(x3, mat.rows));
    corners.push_back(Point2f(x4, mat.rows));

    mPaperCoords = corners;

    Mat dst = Mat::zeros(SCREEN_HEIGHT, SCREEN_WIDTH, CV_8UC3);

    vector<Point2f> dest_pts;
    dest_pts.push_back(Point2f(0, 0));
    dest_pts.push_back(Point2f(dst.cols, 0));
    dest_pts.push_back(Point2f(dst.cols, dst.rows));
    dest_pts.push_back(Point2f(0, dst.rows));
    mDestCoords = dest_pts;

    Mat src = image->clone();

    Mat transmtx = getPerspectiveTransform(corners, dest_pts);
    mTransMtx = transmtx;

    warpPerspective(src, dst, transmtx, dst.size());

    perspectiveSetup = true;
    return dst;
}

