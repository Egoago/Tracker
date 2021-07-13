#include "Camera/OpenCVCamera.h"
#include "Camera/UEyeCamera.h"
#include <iostream>
#include "Detectors/LSDDetector.h"
#include "Detectors/CannyDetector.h"
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;


int main(int argc, char** argv) {
    Camera& cam = OpenCVCamera();
    EdgeDetector& edgeDetector = LSDDetector();
    namedWindow("Original", WINDOW_NORMAL);
    resizeWindow("Original", cam.getWidth() / 2, cam.getHeight() / 2);
    moveWindow("Original", 50, 50);
    namedWindow("Canny", WINDOW_NORMAL);
    resizeWindow("Canny", cam.getWidth()/2, cam.getHeight()/2);
    moveWindow("Canny", cam.getWidth()/2+50, 50);
    namedWindow("Distance transform", WINDOW_NORMAL);
    resizeWindow("Distance transform", cam.getWidth() / 2, cam.getHeight() / 2);
    moveWindow("Distance transform", cam.getWidth() + 50, 50);
    while (1)
    {
        char* frame = cam.getNextFrame();
        Mat wrapped(Size(cam.getWidth(), cam.getHeight()), cam.getFormat(), frame);
        imshow("Original", wrapped);
        vector<Edge<glm::vec2>> edges = edgeDetector.detectEdges(wrapped);
        imshow("Canny", wrapped);
        Mat out;
        distanceTransform(wrapped, out, DIST_L2, DIST_MASK_PRECISE);
        normalize(out, out, 0.0, 1.0, NORM_MINMAX);
        imshow("Distance transform", out);
        waitKey(1);
    }
    return 0;
}