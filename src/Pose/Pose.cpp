#include "Pose.hpp"
#include "../Misc/Constants.hpp"
#include "../Misc/ConfigParser.hpp"
#include "Estimation/DirectEstimator.hpp"
#include "Estimation/ParallelEstimator.hpp"
#include "Estimation/SmartEstimator.hpp"
#include "Registration/CeresRegistrator.hpp"
#include <opencv2/core/utils/logger.hpp>

#include "../Misc/Log.hpp"  //TODO remove logging
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace tr;

Estimator* getEstimator(const Tensor<Template>& templates) {
    switch (strHash(ConfigParser::instance().getEntry<std::string>(CONFIG_SECTION_ESTIMATION, "estimator", "smart").c_str())) {
    case strHash("direct"): return new DirectEstimator(templates);
    case strHash("smart"): return new SmartEstimator(templates);
    case strHash("parallel"): return new ParallelEstimator(templates);
    default: return new DirectEstimator(templates);
    }
}

Registrator* getRegistrator(const mat4f& P) {
    Estimator* estimator = nullptr;
    switch (strHash(ConfigParser::instance().getEntry<std::string>(CONFIG_SECTION_REGISTRATION, "registrator", "ceres").c_str())) {
    case strHash("ceres"): return new CeresRegistrator(P.cast<double>());
    default: return new CeresRegistrator(P.cast<double>());
    }
}

mat4f p; //TODO remove logging

PoseEstimator::PoseEstimator(const Tensor<Template>& templates,
                             const mat4f& P,
                             const float aspect) :
    distanceTensor(aspect),
    estimator(getEstimator(templates)),
    registrator(getRegistrator(P)) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
    p = P; //TODO remove logging
}

SixDOF PoseEstimator::getPose(const cv::Mat& frame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging

    distanceTensor.setFrame(frame);
    const std::vector<const Template*> candidates = estimator->estimate(distanceTensor);

    //TODO parallel registration
    Registrator::Registration bestRegistration;
    bestRegistration.pose = candidates[0]->sixDOF;
    bestRegistration.finalLoss = std::numeric_limits<double>::max();
    uint finalIndex = 0;
    for (uint i = 0; i < candidates.size(); i++) {
        const Registrator::Registration registration = registrator->registrate(distanceTensor, candidates[i]);
        if (bestRegistration.finalLoss > registration.finalLoss) {
            bestRegistration = registration;
            finalIndex = i;
        }
    }

    //TODO remove logging
    for (uint i = 0; i < candidates.size(); i++)
        for (auto rasterPoint : candidates[i]->rasterPoints) {
            cv::circle(frame,
                cv::Point((int)(rasterPoint.uv.x() * frame.cols),
                          (int)(rasterPoint.uv.y() * frame.rows)), 1,
                          (i == finalIndex) ? cv::Scalar(255,0,0): cv::Scalar(0, 0, 255),
                          -1);
            if (i == finalIndex) {
                rasterPoint.render(p * bestRegistration.pose.getModelTransformMatrix());
                cv::circle(frame,
                    cv::Point((int)(rasterPoint.uv.x() * frame.cols),
                              (int)(rasterPoint.uv.y() * frame.rows)),
                              1, cv::Scalar(0, 255, 0), -1);
            }
        }
    Logger::log("Final loss:" + std::to_string(bestRegistration.finalLoss));
    Logger::log("Final index:" + std::to_string(finalIndex));
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return bestRegistration.pose;
}
