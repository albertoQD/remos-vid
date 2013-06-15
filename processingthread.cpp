#include "imagebuffer.h"
#include "processingthread.h"
#include "mattoqimage.h"
#include "config.h"
#include <QDebug>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/nonfree/nonfree.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/stitching/stitcher.hpp>

#include <math.h>

ProcessingThread::ProcessingThread(ImageBuffer *imageBuffer)
    : QThread()
    , outputBuffer(imageBuffer)
    , stopped(false)
    , paused(false)
{
    mosaic     = cv::Mat();
    prevIm     = cv::Mat();
}

ProcessingThread::~ProcessingThread()
{
}

void ProcessingThread::run()
{
    while(1)
    {
        // Check if paused
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
        /////////////////////////////////
        stoppedMutex.lock();
        if (stopped)
        {
            stopped = false;
            stoppedMutex.unlock();
            break;
        }
        stoppedMutex.unlock();
        /////////////////////////////////
        cv::Mat currIm = outputBuffer->getFrame();
        if (currIm.empty())
        {
            stoppedMutex.lock();
            stopped = true;
            stoppedMutex.unlock();
            break;
        }

        if (mosaic.empty())
        {
            mosaic = currIm;
            prevIm = currIm;
            continue;
        }

        // compute homography
        cv::Size size_(mosaic.cols+currIm.cols, mosaic.rows);
        cv::Mat H = computeHomography(prevIm, currIm);

        // warp image
        cv::Mat warpIm;
        warpPerspective(currIm, warpIm, H, size_);
        prevIm = fitImage(warpIm);

        // buscar el rango
        int max_col = mosaic.cols-1;
        int max_row = mosaic.rows-1;
        int min_col = findMinCol(warpIm);
        int min_row = findMinRow(warpIm);

        // add the images
        cv::Mat out = Mat::zeros(size_, mosaic.type());
        cv::Mat half(out, cv::Rect(0, 0, mosaic.cols, mosaic.rows));
        mosaic.copyTo(half);

        cv::addWeighted(out, 1, warpIm, 0.4, 0, mosaic);

        // update mosaic
        mosaic = fitImage(mosaic);
        cv::Mat part(mosaic, cv::Rect(min_col, min_row, max_col-min_col, max_row-min_row));
        part = part * 0.8;

        // Inform GUI thread of new frame (QImage)
        emit newProcessedFrame(MatToQImage(mosaic));
    }
}


int ProcessingThread::findMinRow(cv::Mat im)
{
    cv::Mat grayIm;
    if (im.channels() > 1)
    {
        cv::cvtColor(im, grayIm, CV_RGB2GRAY);
    }
    else
    {
        grayIm = im;
    }

    grayIm.convertTo(grayIm, CV_8UC1);

    cv::Mat col_sum(grayIm.rows, 1, CV_32F);
    cv::reduce(grayIm, col_sum, 1, CV_REDUCE_SUM, CV_32F);

    MatConstIterator_<float> it = col_sum.begin<float>()-1;
    for (; it != col_sum.end<float>(); ++it)
    {
        if (*it > 0) break;
    }

    return it - col_sum.begin<float>();
}


int ProcessingThread::findMinCol(cv::Mat im)
{
    cv::Mat grayIm;
    if (im.channels() > 1)
    {
        cv::cvtColor(im, grayIm, CV_RGB2GRAY);
    }
    else
    {
        grayIm = im;
    }

    grayIm.convertTo(grayIm, CV_8UC1);

    cv::Mat col_sum(1, grayIm.cols, CV_32F);
    cv::reduce(grayIm, col_sum, 0, CV_REDUCE_SUM, CV_32F);

    MatConstIterator_<float> it = col_sum.begin<float>()-1;
    for (; it != col_sum.end<float>(); ++it)
    {
        if (*it > 0) break;
    }

    return it - col_sum.begin<float>();
}

cv::Mat ProcessingThread::fitImage(cv::Mat im)
{
    cv::Mat grayIm;
    if (im.channels() > 1)
    {
        cv::cvtColor(im, grayIm, CV_RGB2GRAY);
    }
    else
    {
        grayIm = im;
    }

    grayIm.convertTo(grayIm, CV_8UC1);

    cv::Mat col_sum(1, grayIm.cols, CV_32F);
    cv::reduce(grayIm, col_sum, 0, CV_REDUCE_SUM, CV_32F);

    MatConstIterator_<float> it = col_sum.end<float>()-1;
    for (; it != col_sum.begin<float>(); --it)
    {
        if (*it > 0) break;
    }

    return im(cv::Rect(0, 0, it - col_sum.begin<float>() , im.rows));
}


cv::Mat ProcessingThread::computeHomography(cv::Mat im2, cv::Mat im1)
{
    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;
    cv::SurfFeatureDetector detector( minHessian );
    vector< cv::KeyPoint > keypoints_im1, keypoints_im2;

    detector.detect( im1, keypoints_im1 );
    detector.detect( im2, keypoints_im2 );

    //-- Step 2: Calculate descriptors (feature vectors)
    cv::SurfDescriptorExtractor extractor;
    cv::Mat descriptors_im1, descriptors_im2;

    extractor.compute( im1, keypoints_im1, descriptors_im1 );
    extractor.compute( im2, keypoints_im2, descriptors_im2 );


    //-- Step 3: Matching descriptor vectors using FLANN matcher
    cv::FlannBasedMatcher matcher;
    vector< DMatch > matches;
    matcher.match( descriptors_im1, descriptors_im2, matches );

    vector< cv::Point2f > points_im1;
    vector< cv::Point2f > points_im2;

    for( unsigned int i = 0; i < matches.size(); i++ )
    {
        points_im1.push_back( keypoints_im1[ matches[i].queryIdx ].pt );
        points_im2.push_back( keypoints_im2[ matches[i].trainIdx ].pt );
    }

    // Find the Homography Matrix
    return cv::findHomography( points_im1, points_im2, CV_RANSAC );
}
