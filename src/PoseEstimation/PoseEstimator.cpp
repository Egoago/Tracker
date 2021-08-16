#include "PoseEstimator.hpp"
#include "../Misc/Links.hpp"
#include "DirectEstimator.hpp"
#include "CeresRegistrator.hpp"
#include <opencv2/core/utils/logger.hpp>

#include "../Misc/Log.hpp"  //TODO remove logging

using namespace tr;

ConfigParser PoseEstimator::config(POSE_CONFIG_FILE);

Estimator* getEstimator(ConfigParser& config, Tensor<Template>& templates) {
    const unsigned int candidateCount = config.getEntry("candidate count", 5);
    Estimator* estimator = nullptr;
    switch (strHash(config.getEntry<std::string>("estimator", "direct").c_str())) {
        //TODO add more
    case strHash("direct"): return new DirectEstimator(candidateCount, templates);
    default: return new DirectEstimator(candidateCount, templates);
    }
}

Registrator* getRegistrator(ConfigParser& config, const emat4& P) {
    Estimator* estimator = nullptr;
    switch (strHash(config.getEntry<std::string>("registrator", "ceres").c_str())) {
    case strHash("ceres"): return new CeresRegistrator(P);
    default: return new CeresRegistrator(P);
    }
}

glm::mat4 p; //TODO remove logging

PoseEstimator::PoseEstimator(Tensor<Template>& templates,
                             const glm::mat4& P,
                             const float aspect) :
    distanceTensor(aspect),
    estimator(getEstimator(config, templates)),
    registrator(getRegistrator(config, GLM2E<real>(P))) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
    p = P; //TODO remove logging
}

SixDOF PoseEstimator::getPose(const cv::Mat& frame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging

    distanceTensor.setFrame(frame);
    std::vector<Template*> candidates = estimator->estimate(distanceTensor);
    //TODO parallel registration
    SixDOF sixDOF = registrator->registrate(distanceTensor, candidates[0]);

    //TODO remove logging
    std::stringstream str;
    str << candidates[0]->sixDOF;
    Logger::log("candidate:\t" + str.str());
    str.str("");
    str << sixDOF;
    Logger::log("registrated:\t" + str.str());
    cv::Mat image = frame.clone();
    for (auto rasterPoint : candidates[0]->rasterPoints) {
        image.at<cv::Vec3b>(
            cv::Point((int)(rasterPoint.uv.x * frame.cols),
                      (int)(rasterPoint.uv.y * frame.rows))) = cv::Vec3b(0,0,255);
        rasterPoint.render(p*sixDOF.getModelTransformMatrix());
        image.at<cv::Vec3b>(
            cv::Point((int)(rasterPoint.uv.x * frame.cols),
                      (int)(rasterPoint.uv.y * frame.rows))) = cv::Vec3b(0, 255, 0);
    }
    Logger::drawFrame(&image, "registration", false);
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return sixDOF;
}
