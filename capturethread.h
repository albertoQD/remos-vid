#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QtGui>
#include "opencv/highgui.h"
#include "imagebuffer.h"
#include "config.h"

class CaptureThread : public QThread
{
    Q_OBJECT

public:
    CaptureThread(ImageBuffer *buffer);


    int  getInputSourceWidth()  { return cap.get(CV_CAP_PROP_FRAME_WIDTH); }
    int  getInputSourceHeight() { return cap.get(CV_CAP_PROP_FRAME_HEIGHT); }
    void pause()                { QMutexLocker locker(&pauseMutex); paused = true; }
    void play();
    void stop()                 { QMutexLocker locker(&stoppedMutex); stopped = true; }
    bool isPaused()             { return paused; }
    bool isStopped()            { return stopped; }

    bool readVideo(QString fn);

private:
    ImageBuffer *inputBuffer;
    cv::VideoCapture cap;
    cv::Mat grabbedFrame;
    QMutex stoppedMutex;
    QMutex capMutex;
    QMutex pauseMutex;

    bool stopped;
    bool paused;
protected:
    void run();
};

#endif // CAPTURETHREAD_H
