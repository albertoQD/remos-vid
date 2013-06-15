#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "capturethread.h"
#include "imagebuffer.h"
#include "processingthread.h"
#include <QtGui>
#include <opencv/highgui.h>


class Controller : public QObject
{
    Q_OBJECT

public:
    Controller();
    ~Controller();

    ImageBuffer      *inputBuffer;
    ImageBuffer      *outputBuffer;
    ProcessingThread *processingThread;
    CaptureThread    *captureThread;


    void stopThreads();
    void deleteThreads();
    void clearImageBuffers();
    void deleteImageBuffers();

    bool readVideo(QString);


public slots:
    void processFrame();

signals:
    void newInputFrame(QImage);

private:
    int imageBufferSize;

};

#endif // CONTROLLER_H
