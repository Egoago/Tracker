#include "Camera/OpenCVCamera.h"
#include "DCDT3Generator.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    Camera& cam = OpenCVCamera();
    DCDT3Generator generator(cam.getWidth(), cam.getHeight());
    namedWindow("jep");

    while (1)
    {
        char* frame = cam.getNextFrame();
        Mat wrapped(Size(cam.getWidth(), cam.getHeight()), cam.getFormat(), frame);
        Mat tmp = generator.setFrame(wrapped)[0];
        normalize(tmp, tmp, 0.0, 1.0, NORM_MINMAX);
        imshow("jep", tmp);
        waitKey(1);
    }
    return 0;
}