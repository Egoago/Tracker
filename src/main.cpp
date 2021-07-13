#include "Camera/OpenCVCamera.h"
#include "DCDT3Generator.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    Camera& cam = OpenCVCamera();
    DCDT3Generator generator;

    while (1)
    {
        char* frame = cam.getNextFrame();
        Mat wrapped(Size(cam.getWidth(), cam.getHeight()), cam.getFormat(), frame);
        generator.setFrame(wrapped);
    }
    return 0;
}