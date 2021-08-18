#include "Misc/Links.hpp"
//#include "PoseEstimation/PoseEstimator.hpp"
//#include "Object/Model.hpp"
#include <opencv2/highgui.hpp>
//TODO remove logging
#include "Misc/Log.hpp"
#include "Detectors/MultiFlashDetector.hpp"

int main(int argc, char** argv) {
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    std::vector<cv::Mat> flashImages(9);
    for (tr::uint i = 0u; i < 9u; i++)
        flashImages[i] = cv::imread(tr::MFC_TEST_FRAME(i, "cube"));
    tr::MultiFlashDetector mfc(8);
    mfc.getDepthMap(flashImages);
    /*tr::Model model("cube");
    tr::PoseEstimator poseEstimator(model.getTemplates(), model.getP(), (float)frame.cols/frame.rows);
    poseEstimator.getPose(frame);*/
    tr::Logger::log("Waiting for key...");
    cv::waitKey(1000000);
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return 0;
}