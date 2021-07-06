#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Camera/OpenCVCamera.h"
#include "Rendering/Renderer.h"

using namespace cv;
using namespace std;


int main(int argc, char** argv) {
    Renderer renderer(argc, argv, 500, 500);
    float rot = 0.0f;
    namedWindow("OpenCV", WINDOW_NORMAL);
    resizeWindow("OpenCV", 500, 500);
    moveWindow("OpenCV", 500, 50);
    Mat frame(Size(500, 500), CV_8UC3);
    while (1) {
        renderer.setModel(0.0f, -50.0f, -150.0f, 0.0f, rot);
        renderer.renderModel("cube.STL", frame.data);
        cv::Mat flipped;
        cv::flip(frame, flipped, 0);
        imshow("OpenCV", flipped);
        rot += 0.01f;
        waitKey(10);
    }
    /*Camera* cam = new OpenCVCamera();
    namedWindow("Original", WINDOW_NORMAL);
    resizeWindow("Original", cam->getWidth() / 2, cam->getHeight() / 2);
    moveWindow("Original", 50, 50);
    namedWindow("Canny", WINDOW_NORMAL);
    resizeWindow("Canny", cam->getWidth()/2, cam->getHeight()/2);
    moveWindow("Canny", cam->getWidth() / 2+50, 50);
    while (1)
    {
        char* frame = cam->getNextFrame();
        Mat wrapped(Size(cam->getWidth(), cam->getHeight()), cam->getFormat(), frame);
        cout <<"\r"<<cam->getFPS();
        imshow("Original", wrapped);
        Mat dst, detected_edges;
        blur(wrapped, detected_edges, Size(5, 5));
        Canny(wrapped, detected_edges, 200, 30 * 3, 3);
        dst = Scalar::all(0);
        wrapped.copyTo(dst, detected_edges);
        imshow("Canny", dst);
        waitKey(1);
    }
    delete cam;*/
    return 0;
}