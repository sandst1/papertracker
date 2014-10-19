#include "playclock.h"

const int MSEC_IN_MIN = 60*1000; // ms

PlayClock::PlayClock(QObject *parent) :
    QObject(parent)
{
    mTempo = 120;
    mTimer = new QTimer(this);
    mStep = 0;

    int quarterNotesPerMin = MSEC_IN_MIN / mTempo;
    //int note16perMin = quarterNotesPerMin * 4;
    int note8perMin = quarterNotesPerMin * 2;

    int noteTimeMs = MSEC_IN_MIN / note8perMin;
    mTimer->setInterval(noteTimeMs);

    QObject::connect(mTimer, SIGNAL(timeout()), this, SLOT(timerTick()));
}

void PlayClock::start()
{
    mTimer->start();
}

void PlayClock::timerTick()
{
    mStep++;
    if (mStep == 16)
    {
        mStep = 0;
    }
}

