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

glm::mat4 p;

PoseEstimator::PoseEstimator(const int width,
                             const int height,
                             Tensor<Template>& templates,
                             const glm::mat4& P) :
    distanceTensor(width, height),
    estimator(getEstimator(config, templates)),
    registrator(getRegistrator(config, GLM2E<real>(P))) {
    p = P;
}

SixDOF PoseEstimator::getPose(const cv::Mat& frame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    distanceTensor.setFrame(frame);
    std::vector<Template*> candidates = estimator->estimate(distanceTensor);
    //TODO parallel registration
    SixDOF sixDOF = registrator->registrate(distanceTensor, candidates[0]);
    //TODO remove logging
    cv::Mat image = frame.clone();
    for (const auto& rasterPoint : candidates[0]->rasterPoints) {
        image.at<cv::Vec3b>(cv::Point((int)(rasterPoint.uv.x * frame.cols),
            (int)(rasterPoint.uv.y * frame.rows))) = cv::Vec3b(0,0,255);
    }
    std::stringstream str;
    str << candidates[0]->sixDOF;
    Logger::log("candidate: " + str.str());
    for (auto rasterPoint : candidates[0]->rasterPoints) {
        rasterPoint.render(p*sixDOF.getModelTransformMatrix());
        image.at<cv::Vec3b>(cv::Point((int)(rasterPoint.uv.x * frame.cols),
            (int)(rasterPoint.uv.y * frame.rows))) = cv::Vec3b(0, 255, 0);
    }
    str.flush();
    str << sixDOF;
    Logger::log("registrated: " + str.str());
    cv::imshow("Output", image);
    cv::waitKey(1);
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return sixDOF;
}
