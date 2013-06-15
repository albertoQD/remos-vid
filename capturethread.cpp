#include "capturethread.h"
#include "imagebuffer.h"
#include "opencv/highgui.h"
#include <opencv/cv.h>
#include "config.h"
#include <QDebug>

CaptureThread::CaptureThread(ImageBuffer *imageBuffer)
    : QThread()
    , inputBuffer(imageBuffer)
    , stopped(false)
    , paused(false)
{
}

void CaptureThread::run()
{
    int cont = 0;
    while(1)
    {
        // Check if it is paused
        pauseMutex.lock();
        if (paused)
        {
            pauseMutex.unlock();
            sleep(3);
            continue;
        }
        pauseMutex.unlock();

        /////////////////////////////////
        // Stop thread if stopped=TRUE //
        /////////////////////////////////paussed
        stoppedMutex.lock();
        if (stopped)
        {
            stoppedMutex.unlock();
            break;
        }
        stoppedMutex.unlock();


        capMutex.lock();
            cap >> grabbedFrame;
        capMutex.unlock();

        capMutex.lock();
        if (cap.get(CV_CAP_PROP_FRAME_COUNT)-2 <= cap.get(CV_CAP_PROP_POS_FRAMES))
        {
            capMutex.unlock();
            // STOP IT
            stoppedMutex.lock();
                stopped = true;
            stoppedMutex.unlock();
            cap.set(CV_CAP_PROP_POS_FRAMES, 0);
        }
        capMutex.unlock();

        if (cont == 4)
        {
            cont = 0;
            continue;
        }
        cont += 1;

        if (grabbedFrame.empty()) continue;

        cv::resize(grabbedFrame, grabbedFrame, cv::Size(386, 200));
        // add the frame to the buffer
        inputBuffer->addFrame(grabbedFrame.clone());

        msleep(1000/cap.get(CV_CAP_PROP_FPS));
    }
}


bool CaptureThread::readVideo(QString fn)
{
    bool res = false;

    capMutex.lock();
    if (cap.isOpened())
        cap.release();

    res = cap.open(fn.toStdString());
    capMutex.unlock();

    stoppedMutex.lock();
    if (stopped) stopped = false;
    stoppedMutex.unlock();

    return res;
}

void CaptureThread::play()
{
    pauseMutex.lock();
    if (paused) paused = false;
    pauseMutex.unlock();

    stoppedMutex.lock();
    if (stopped)
    {
        stopped = false;
        stoppedMutex.unlock();
        start(DEFAULT_CAP_THREAD_PRIO);
    }
    stoppedMutex.unlock();

}
