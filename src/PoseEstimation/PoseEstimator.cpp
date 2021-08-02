#include "PoseEstimator.h"
#include "../Misc/Links.h"
#include "DirectEstimator.h"

//TODO remove debug
#include <opencv2/highgui.hpp>

using namespace tr;

ConfigParser PoseEstimator::config(POSE_CONFIG_FILE);

Estimator* getEstimator(ConfigParser& config) {
    const unsigned int candidateCount = std::stoi(config.getEntry("candidate count", "5"));
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
    distanceTensor.setFrame(frame);
    std::vector<Template*> candidates = estimator->estimate(distanceTensor);
    cv::Mat image(frame.rows, frame.cols,CV_8U, cv::Scalar(0));
    for (auto& i : candidates.front()->uv)
        image.at<uchar>(cv::Point((int)(i.x * frame.cols), (int)(i.y * frame.rows))) = 255;
    cv::imshow("top candidate", image);
    //TODO parallel registration
    //return registrator.registrate(templates.begin(),)
    
    //TODO registration
    return candidates.front()->sixDOF;
}
