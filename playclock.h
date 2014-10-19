#ifndef PLAYCLOCK_H
#define PLAYCLOCK_H

#include <QObject>
#include <QTimer>

class PlayClock : public QObject
{
    Q_OBJECT
public:
    explicit PlayClock(QObject *parent = 0);

    inline int step() { return mStep; }

    void start();

signals:

public slots:
    void timerTick();

private:
    int mTempo;
    QTimer* mTimer;
    int mStep;
};

#endif // PLAYCLOCK_H
