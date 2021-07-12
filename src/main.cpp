#include "Camera/OpenCVCamera.h"
#include "Camera/UEyeCamera.h"
#include <iostream>
#include "Detectors/LSDDetector.h"

using namespace cv;
using namespace std;


int main(int argc, char** argv) {
    Camera& cam = OpenCVCamera();
    EdgeDetector& edgeDetector = LSDDetector(0.2f);
    namedWindow("Original", WINDOW_NORMAL);
    resizeWindow("Original", cam.getWidth() / 2, cam.getHeight() / 2);
    moveWindow("Original", 50, 50);
    namedWindow("LSD", WINDOW_NORMAL);
    resizeWindow("LSD", cam.getWidth()/2, cam.getHeight()/2);
    moveWindow("LSD", cam.getWidth()/2+50, 50);
    while (1)
    {
        char* frame = cam.getNextFrame();
        Mat wrapped(Size(cam.getWidth(), cam.getHeight()), cam.getFormat(), frame);
        imshow("Original", wrapped);
        vector<Edge<glm::vec2>> edges = edgeDetector.detectEdges(wrapped);
        imshow("LSD", wrapped);
        waitKey(1);
    }
    return 0;
}