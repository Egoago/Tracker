#include "Object/Model.h"

int main(int argc, char** argv) {
    //Mat frame = imread(TEST_FRAME);
    tr::Model model("cylinder.stl");
    /*PoseEstimator poseEstimator(frame.cols, frame.rows, model);
    namedWindow("jep");
    imshow("jep", frame);
    waitKey(1);
    poseEstimator.getPose(frame).print(std::cout);*/
    return 0;
}