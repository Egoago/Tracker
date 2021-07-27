#include "Misc/Links.h"
#include "PoseEstimation/PoseEstimator.h"
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    Mat frame = imread(TEST_FRAME);
    Model model("cylinder.stl");
    /*PoseEstimator poseEstimator(frame.cols, frame.rows, model);
    namedWindow("jep");
    imshow("jep", frame);
    waitKey(1);
    poseEstimator.getPose(frame).print(std::cout);*/
    return 0;
}