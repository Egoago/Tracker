#include "Misc/Links.h"
#include "PoseEstimation/PoseEstimator.h"
#include "Object/Model.h"
#include <opencv2/highgui.hpp>
#include <iostream>

int main(int argc, char** argv) {
    cv::Mat frame = cv::imread(tr::TEST_FRAME);
    tr::Model model("cube");
    tr::PoseEstimator poseEstimator(frame.cols, frame.rows, model.getTemplates());
    cv::imshow("jep", frame);
    std::cout << poseEstimator.getPose(frame);
    cv::waitKey(1000000);
    return 0;
}