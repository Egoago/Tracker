#include "PoseEstimator.hpp"
#include "../Misc/Links.hpp"
#include "DirectEstimator.hpp"
#include "CeresRegistrator.hpp"

//TODO remove debug
#include <opencv2/highgui.hpp>

using namespace tr;

ConfigParser PoseEstimator::config(POSE_CONFIG_FILE);

Estimator* getEstimator(ConfigParser& config, Tensor<Template>& templates) {
    const unsigned int candidateCount = std::stoi(config.getEntry("candidate count", "10"));
    Estimator* estimator = nullptr;
    switch (strHash(config.getEntry("estimator", "direct").c_str())) {
        //TODO add more
    case strHash("direct"): return new DirectEstimator(candidateCount, templates);
    default: return new DirectEstimator(candidateCount, templates);
    }
}

Registrator* getRegistrator(ConfigParser& config, const emat4& P) {
    Estimator* estimator = nullptr;
    switch (strHash(config.getEntry("registrator", "ceres").c_str())) {
    case strHash("ceres"): return new CeresRegistrator(P);
    default: return new CeresRegistrator(P);
    }
}

PoseEstimator::PoseEstimator(const int width,
                             const int height,
                             Tensor<Template>& templates,
                             const emat4& P) :
    distanceTensor(width, height),
    estimator(getEstimator(config, templates)),
    registrator(getRegistrator(config, P)) {
}

SixDOF PoseEstimator::getPose(const cv::Mat& frame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    distanceTensor.setFrame(frame);
    std::vector<Template*> candidates = estimator->estimate(distanceTensor);
    int c=0;
    //TODO remove logging
    for (auto& candidate : candidates) {
        cv::Mat image(frame.rows, frame.cols, CV_8U, cv::Scalar(0));
        for (const auto& rasterPoint : candidate->rasterPoints) {
            image.at<uchar>(cv::Point((int)(rasterPoint.uv.x * frame.cols),
                                      (int)(rasterPoint.uv.y * frame.rows))) = 255;
        }
        cv::imshow("candidate", image);
        cv::waitKey(1);
        Logger::log(std::to_string(++c) + ". candidate. " + std::to_string(candidate->rasterPoints.size()) + " points.");
        cv::waitKey(1000000000);
    }
    //TODO parallel registration
    
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return registrator->registrate(distanceTensor, candidates[0]);
}
