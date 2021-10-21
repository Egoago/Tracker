#include "PoseEstimator.hpp"
#include "../Misc/Constants.hpp"
#include "../Misc/ConfigParser.hpp"
#include "DirectEstimator.hpp"
#include "ParallelEstimator.hpp"
#include "CeresRegistrator.hpp"
#include <opencv2/core/utils/logger.hpp>

#include "../Misc/Log.hpp"  //TODO remove logging
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace tr;

Estimator* getEstimator(const Tensor<Template>& templates) {
    switch (strHash(ConfigParser::instance().getEntry<std::string>(CONFIG_SECTION_ESTIMATION, "estimator", "parallel").c_str())) {
    case strHash("direct"): return new DirectEstimator(templates);
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
    for (const auto& candidate : candidates) {
        cv::Mat canvas = frame.clone();
        for (const auto& rasterPoint : candidate->rasterPoints) {
            cv::circle(canvas,
                       cv::Point((int)(rasterPoint.uv.x() * frame.cols),
                                 (int)(rasterPoint.uv.y() * frame.rows)),
                       1, cv::Scalar(0, 0, 255), -1);
        }
        Logger::drawFrame(&canvas, "candidate");
        cv::waitKey(10000000);
    }
    //TODO parallel registration
    SixDOF sixDOF = registrator->registrate(distanceTensor, candidates[0]);

    //TODO remove logging
    std::stringstream str;
    str << candidates[0]->sixDOF;
    Logger::log("candidate:\t" + str.str());
    str.str("");
    str << sixDOF;
    Logger::log("registrated:\t" + str.str());
    for (auto rasterPoint : candidates[0]->rasterPoints) {
        cv::circle(frame,
                   cv::Point((int)(rasterPoint.uv.x() * frame.cols),
                       (int)(rasterPoint.uv.y() * frame.rows)),
                   1, cv::Scalar(0, 0, 255), -1);
        rasterPoint.render(p*sixDOF.getModelTransformMatrix());
        cv::circle(frame,
                   cv::Point((int)(rasterPoint.uv.x() * frame.cols),
                       (int)(rasterPoint.uv.y() * frame.rows)),
                   1, cv::Scalar(0, 255, 0), -1);
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return sixDOF;
}
