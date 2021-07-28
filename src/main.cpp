#include "Misc/Links.h"
#include "PoseEstimation/PoseEstimator.h"
#include <opencv2/highgui.hpp>
#include "Misc/Log.h"
#include "Math/Tensor.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    Mat frame = imread(TEST_FRAME);
    //Model model("cylinder.stl");

    Logger::logProcess("testing tensor");
    tr::Tensor<int> tensor({1,2,3});
    Logger::log(tensor.toString());
    Logger::logProcess("testing tensor");
    /*PoseEstimator poseEstimator(frame.cols, frame.rows, model);
    namedWindow("jep");
    imshow("jep", frame);
    waitKey(1);
    poseEstimator.getPose(frame).print(std::cout);*/
    return 0;
}