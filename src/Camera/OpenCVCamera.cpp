#include "OpenCVCamera.hpp"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "../Misc/Log.hpp"

using namespace cv;
using namespace tr;

OpenCVCamera::OpenCVCamera(int id) {
    cap = new VideoCapture(id);
    VideoCapture* camera = (VideoCapture*) cap;
    if (!cap || !camera->open(0)) 
        Logger::error("Couldn't open OpenCV camera");
    camera->set(CAP_PROP_FRAME_WIDTH, 2000);
    camera->set(CAP_PROP_FRAME_HEIGHT, 2000);
    width = (int)camera->get(CAP_PROP_FRAME_WIDTH);
    height = (int)camera->get(CAP_PROP_FRAME_HEIGHT);
    nBitsPerPixel = 24;
    load();
}

OpenCVCamera::~OpenCVCamera() {
    VideoCapture* camera = (VideoCapture*)cap;
    camera->release();
    free(cap);
}

char* OpenCVCamera::getNextFrameData() {
    VideoCapture* camera = (VideoCapture*)cap;
    if (!cap || !camera->isOpened())
        Logger::error("Couldn't open OpenCV camera");
    Mat bgrFrame;
    (*camera) >> bgrFrame;
    cvtColor(bgrFrame, frame, COLOR_BGR2GRAY);
    if (frame.empty())
        Logger::error("Couldn't open OpenCV camera");
    return (char*)frame.data;
}

double OpenCVCamera::getFPS() const {
    return 0.0;
}

int OpenCVCamera::getFormat() const {
    VideoCapture* camera = (VideoCapture*)cap;
    //return (int) camera->get(CAP_PROP_FORMAT);
    //TODO
    return CV_8U;
}
