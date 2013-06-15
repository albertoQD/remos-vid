#include "controller.h"
#include "imagebuffer.h"
#include "mattoqimage.h"
#include <QtGui>
#include "config.h"

Controller::Controller()
{
    imageBufferSize = DEFAULT_IMAGE_BUFFER_SIZE;
    inputBuffer = new ImageBuffer(this, imageBufferSize);
    outputBuffer = new ImageBuffer(this, imageBufferSize);

    captureThread    = new CaptureThread(inputBuffer);
    processingThread = new ProcessingThread(outputBuffer);
    connect(inputBuffer, SIGNAL(newFrame()), this, SLOT(processFrame()));

}

Controller::~Controller()
{
}

bool Controller::readVideo(QString filename)
{
    bool res = false;
    if (captureThread->isRunning())
    {
        captureThread->pause();
        processingThread->pause();
    }

    if ((res = captureThread->readVideo(filename)))
    {

        if (captureThread->isPaused())
        {
            captureThread->play();
            processingThread->play();
        }
        else
        {
            captureThread->start(DEFAULT_CAP_THREAD_PRIO);
            processingThread->start(DEFAULT_PROC_THREAD_PRIO);
        }
    }

    return res;
}


void Controller::stopThreads()
{
    captureThread->stop();

    // Take one frame off a FULL queue to allow the capture thread to finish
    if(inputBuffer->getSizeOfImageBuffer() == imageBufferSize)
        inputBuffer->getFrame();

    if (!captureThread->wait(2000))
    {
        captureThread->terminate();
    }

    processingThread->stop();
    if (!processingThread->wait(2000))
    {
        processingThread->terminate();
    }
}


void Controller::deleteThreads()
{
    delete captureThread;
    delete processingThread;
}


void Controller::clearImageBuffers()
{
    inputBuffer->clearBuffer();
    outputBuffer->clearBuffer();
}

void Controller::deleteImageBuffers()
{
    delete inputBuffer;
    delete outputBuffer;
}

void Controller::processFrame()
{
    // get the frame from inputbuffer
    cv::Mat frame = inputBuffer->getFrame();
    // add the new frame to the outputbuffer, so the processingThread can take it
    outputBuffer->addFrame(frame);
    // send signal to update the inputlabel in the UI
    emit newInputFrame(MatToQImage(frame));
}
