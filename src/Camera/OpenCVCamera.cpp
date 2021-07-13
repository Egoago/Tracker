#include "OpenCVCamera.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

OpenCVCamera::OpenCVCamera(int id)
{
    cap = new VideoCapture(id);
    VideoCapture* camera = (VideoCapture*) cap;
    if (!cap || !camera->open(0)) {
        std::cerr << "Couldn't open OpenCV camera";
        exit(1);
    }
    width = (int)camera->get(CAP_PROP_FRAME_WIDTH);
    height = (int)camera->get(CAP_PROP_FRAME_HEIGHT);
    nBitsPerPixel = 24;
}

OpenCVCamera::~OpenCVCamera()
{
    free(cap);
}

char* OpenCVCamera::getNextFrame()
{
    VideoCapture* camera = (VideoCapture*)cap;
    Mat bgrFrame;
    *camera >> bgrFrame;
    cvtColor(bgrFrame, frame, COLOR_BGR2GRAY);
    return (char*)frame.data;
}

double OpenCVCamera::getFPS() const
{
    return 0.0;
}

int OpenCVCamera::getFormat() const {
    VideoCapture* camera = (VideoCapture*)cap;
    //return (int) camera->get(CAP_PROP_FORMAT);
    //TODO
    return CV_8U;
}
