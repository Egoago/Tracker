#include "Misc/Links.hpp"
#include "PoseEstimation/PoseEstimator.hpp"
#include "Object/Model.hpp"
#include <opencv2/highgui.hpp>
//TODO remove logging
#include "Misc/Log.hpp"

int main(int argc, char** argv) {
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    cv::Mat frame = cv::imread(tr::TEST_FRAME_CUBE);
    tr::Model model("cube");
    tr::PoseEstimator poseEstimator(model.getTemplates(), model.getP(), (float)frame.cols/frame.rows);
    poseEstimator.getPose(frame);
    tr::Logger::log("Waiting for key...");
    cv::waitKey(1000000);
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return 0;
}