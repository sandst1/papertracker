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

}

int findCount = 0;

Mat NoteGrid::gridFound(Mat* image)
{


    qDebug("gridFound");
    for ( int i = 0; i < mHorLines.size(); i++)
    {
        Vec4i l = mHorLines.at(i);
        line( *image, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);

        qDebug()<< "y: " << l[1] << ", " << l[3];
    }

    for ( int i = 0; i < mVerLines.size(); i++)
    {
        Vec4i l = mVerLines.at(i);
        line( *image, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,255,0), 1, CV_AA);
    }

    return *image;

}

Mat NoteGrid::findGrid(Mat* image)
{
    if (mStatus == NOT_INITIALIZED)
    {
        mStatus = FINDING_GRID;
    }
    else if (mStatus == GRID_FOUND)
    {
        qDebug("gridFound!");
        return gridFound(image);
    }

    Mat mat;

    cvtColor(*image, mat, CV_BGR2GRAY);

    //threshold(mat, mat, 150, 255, CV_THRESH_BINARY_INV);
    //medianBlur(mat, mat, 3);
    //adaptiveThreshold(mat, mat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 15, 0);

    adaptiveThreshold(mat, mat, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 15, 0);

    Mat cmat;
    cvtColor(mat, cmat, CV_GRAY2BGR);
//    vector<Vec2f> lines;
//    HoughLines(mat, lines, 1, CV_PI/180, 150, 0, 0);

//    // draw lines
//       for( size_t i = 0; i < lines.size(); i++ )
//       {
//           float rho = lines[i][0], theta = lines[i][1];
//           Point pt1, pt2;
//           double a = cos(theta), b = sin(theta);
//           double x0 = a*rho, y0 = b*rho;
//           pt1.x = cvRound(x0 + 1000*(-b));
//           pt1.y = cvRound(y0 + 1000*(a));[
//           pt2.x = cvRound(x0 - 1000*(-b));
//           pt2.y = cvRound(y0 - 1000*(a));
//           line( cmat, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
//       }

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
