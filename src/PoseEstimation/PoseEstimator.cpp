#include "PoseEstimator.h"
#include "../Misc/Links.h"
#include "DirectEstimator.h"

//TODO remove debug
#include <opencv2/highgui.hpp>

using namespace tr;

ConfigParser PoseEstimator::config(POSE_CONFIG_FILE);

Estimator* getEstimator(ConfigParser& config) {
    const unsigned int candidateCount = std::stoi(config.getEntry("candidate count", "10"));
    switch (strHash(config.getEntry("estimator", "direct").c_str())) {
        //TODO add more
    case strHash("direct"): return new DirectEstimator(candidateCount);
    default: return new DirectEstimator(candidateCount);
    }
}

PoseEstimator::PoseEstimator(const int width, const int height, Tensor<Template>& templates) :
    distanceTensor(DistanceTensor(width, height)) {
    estimator = getEstimator(config);
    estimator->setTemplates(&templates);
}

tr::PoseEstimator::~PoseEstimator() {
    if (estimator != nullptr)
        delete estimator;
    if (registrator != nullptr)
        delete registrator;
}

SixDOF PoseEstimator::getPose(const cv::Mat& frame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    distanceTensor.setFrame(frame);
    std::vector<Template*> candidates = estimator->estimate(distanceTensor);
    int c=0;
    for (auto& candidate : candidates) {
        cv::Mat image(frame.rows, frame.cols, CV_8U, cv::Scalar(0));
        for (auto& i : candidate->uv)
            image.at<uchar>(cv::Point((int)(i.x * frame.cols), (int)(i.y * frame.rows))) = 255;
        cv::imshow("candidate", image);
        cv::waitKey(1);
        Logger::log(std::to_string(++c) + ". candidate. " + std::to_string(candidate->uv.size()) + " points.");
        cv::waitKey(1000000000);
    }
    //TODO parallel registration
    //return registrator.registrate(templates.begin(),)
    
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    //TODO registration
    return candidates.front()->sixDOF;
}
