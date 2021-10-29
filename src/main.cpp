#include "Misc/Constants.hpp"
#include "Pose/Pose.hpp"
#include "Object/Model.hpp"
#include <opencv2/highgui.hpp>
#include "Misc/Log.hpp"
#include "Math/EigenTransform.hpp"
#include <opencv2/imgproc.hpp>
#include "Object/AssimpLoader.hpp"
#include "Misc/ConfigParser.hpp"
#include "Camera/OpenCVCamera.hpp"

int main(int argc, char** argv) {
    std::unique_ptr<tr::Camera> camera(new tr::OpenCVCamera());
    if (!camera->isCalibrated()) camera->calibrate();
    while (1) {
        cv::imshow("calibrated", camera->undistort(camera->getNextFrame()));
        cv::waitKey(1);
    }
    
    tr::Logger::logProcess(__FUNCTION__);
    tr::Model model("cylinder", camera->getParameters());
    cv::Mat frame = cv::imread(tr::TEST_FRAME_CYLINDER);
    tr::PoseEstimator poseEstimator(model.getTemplates(), model.getP(), (float)frame.cols/frame.rows);
    const tr::SixDOF pose = poseEstimator.getPose(frame);
    /*tr::Geometry geo;
    tr::AssimpLoader::load("cube", geo);
    const tr::mat4f P = model.getP();
    for (uint index : geo.highEdgeIndices) {
        tr::vec2f aUV, bUV;
        tr::project(pose.posData, pose.orData, geo.edges[index * 2], aUV.data(), P);
        tr::project(pose.posData, pose.orData, geo.edges[index * 2 + 1], bUV.data(), P);
        cv::line(frame,
            cv::Point((int)(aUV[0] * frame.cols), (int)(aUV[1] * frame.rows)),
            cv::Point((int)(bUV[0] * frame.cols), (int)(bUV[1] * frame.rows)),
            cv::Scalar(100, 200, 50));
    }*/
    tr::Logger::drawFrame(&frame, "registered");
    tr::Logger::logProcess(__FUNCTION__);
    tr::Logger::log("Waiting for key...");
    cv::waitKey(1000000);
    return 0;
}