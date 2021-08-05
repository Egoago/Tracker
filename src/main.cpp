#include "Misc/Links.hpp"
#include "PoseEstimation/PoseEstimator.hpp"
#include "Object/Model.hpp"
//TODO remove logging
#include <opencv2/highgui.hpp>
#include <iostream>
#include "Misc/Log.hpp"

int main(int argc, char** argv) {
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    cv::Mat frame = cv::imread(tr::TEST_FRAME_CUBE);
    cv::Mat dst;
    cv::flip(frame, dst, 1);
    tr::Model model("cube");
    tr::PoseEstimator poseEstimator(frame.cols, frame.rows, model.getTemplates());
    cv::imshow("Original frame", dst);
    cv::waitKey(1);
    std::cout << "Estimated pose: " << poseEstimator.getPose(dst);
    tr::Logger::log("Waiting for key...");
    cv::waitKey(1000000);
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return 0;
}