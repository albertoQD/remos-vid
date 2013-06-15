#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H
#include <QThread>
#include <QtGui>
#include <vector>
#include <opencv/highgui.h>
#include "imagebuffer.h"

using namespace std;

class ProcessingThread : public QThread
{
    Q_OBJECT

public:
    ProcessingThread(ImageBuffer *imageBuffer);
    ~ProcessingThread();

    void    stop()                   { QMutexLocker locker(&stoppedMutex); stopped = true; }
    void    pause()                  { QMutexLocker locker(&pauseMutex);   paused  = true; }
    void    play()                   { QMutexLocker locker(&pauseMutex);   paused  = false; }
    bool    isPaused()               { return paused; }
    int     getCurrentSizeOfBuffer() { return outputBuffer->getSizeOfImageBuffer(); }
    cv::Mat computeHomography(cv::Mat, cv::Mat);
    cv::Mat fitImage(cv::Mat);
    int     findMinCol(cv::Mat);
    int     findMinRow(cv::Mat);

private:
    ImageBuffer   *outputBuffer;
    volatile bool stopped;
    bool          paused;
    int           currentSizeOfBuffer;

    QMutex        stoppedMutex;
    QMutex        pauseMutex;

    vector<cv::Mat> images;

    cv::Mat mosaic;
    cv::Mat prevIm;


protected:
    void run();

signals:
    void newProcessedFrame(const QImage &frame);

};

#endif // PROCESSINGTHREAD_H
