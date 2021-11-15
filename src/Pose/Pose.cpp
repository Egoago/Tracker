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

PoseEstimator::PoseEstimator(const Tensor<Template>& templates,
                             const mat4f& P,
                             const float aspect) :
    distanceTensor(aspect),
    estimator(getEstimator(templates)),
    registrator(getRegistrator(P)),
    P(P){
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);
}

void gradOrientation(const cv::Mat& frame, Eigen::ArrayXd& gradX, Eigen::ArrayXd& gradY) {
    cv::Mat img = frame.clone();
    //cv::blur(img, img, cv::Size(3, 3));
    if (img.type() != CV_8U) {
        cvtColor(frame, img, cv::COLOR_BGR2GRAY);
    }
    cv::Mat temp;
    //cv::blur(img, img, cv::Size(3, 3));
    cv::Sobel(img, temp, CV_64F, 1, 0, 3);
    //Logger::drawFrame(&temp, "x grad", 1.0f);
    gradX = Eigen::Map<Eigen::ArrayXd>((double*)temp.data, temp.rows, temp.cols).eval();
    cv::Sobel(img, temp, CV_64F, 0, 1, 3);
    //Logger::drawFrame(&temp, "Y grad", 1.0f);
    gradY = Eigen::Map<Eigen::ArrayXd>((double*)temp.data, temp.rows, temp.cols).eval();
}

std::vector<float> getScores(const std::vector<const Template*>& candidates,
                             const std::vector<SixDOF>& poses,
                             const cv::Mat& frame,
                             const mat4f& P) {
    if (candidates.size() != poses.size())
        Logger::error("Same amount of registrations and candidates expected");
    Eigen::ArrayXd gradX, gradY;
    gradOrientation(frame, gradX, gradY);
    std::vector<float> scores(candidates.size());
    for (uint i = 0; i < candidates.size(); i++) {
        float score = 0.f;
        for (auto rasterPoint : candidates[i]->rasterPoints) {
            const mat4f mvp = P * poses[i].getModelTransformMatrix();
            const vecm2f renderGrad = rasterPoint.renderOffset(mvp);
            rasterPoint.render(mvp);
            const uvec2 pixel((int)(rasterPoint.uv.x() * frame.cols),
                              (int)(rasterPoint.uv.y() * frame.rows));
            vecm2f imgGrad(gradX(pixel.y(), pixel.x()), gradY(pixel.y(), pixel.x()));
            imgGrad.normalize();
            score += fabs(imgGrad.dot(renderGrad))/0.637f; //rescale for cosine avg power
        }
        scores[i] = std::max(1.f - score / candidates[i]->rasterPoints.size(), 0.0f);
    }
    return scores;
}

SixDOF PoseEstimator::getPose(const cv::Mat& frame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const cv::Mat originalFrame = frame.clone();
    distanceTensor.setFrame(frame);
    const std::vector<const Template*> candidates = estimator->estimate(distanceTensor);
    std::vector<float> losses;
    std::vector<SixDOF> poses;
    losses.reserve(candidates.size());
    poses.reserve(candidates.size());
    //TODO parallel registration
    Logger::logProcess("Registration");   //TODO remove logging
    for (const auto& candidate : candidates) {
        const Registrator::Registration registration = registrator->registrate(distanceTensor, candidate);
        losses.emplace_back(registration.finalLoss);
        poses.emplace_back(registration.pose);
    }
    Logger::logProcess("Registration");   //TODO remove logging

    std::vector<float> scores = getScores(candidates, poses, originalFrame, P);
    int minLossIndex = int(std::min_element(losses.begin(), losses.end()) - losses.begin());
    int maxScoreIndex = int(std::max_element(scores.begin(), scores.end()) - scores.begin());

    //TODO remove logging
    for (uint i = 0; i < candidates.size(); i++)
        for (auto rasterPoint : candidates[i]->rasterPoints) {
            cv::circle(frame,
                cv::Point((int)(rasterPoint.uv.x() * frame.cols),
                          (int)(rasterPoint.uv.y() * frame.rows)), 1,
                          (i == minLossIndex || i == maxScoreIndex) ? cv::Scalar(255,0,0): cv::Scalar(0, 0, 255),
                          -1);
            if (i == minLossIndex || i == maxScoreIndex) {
                if (rasterPoint.render(P * poses[i].getModelTransformMatrix())) {
                    cv::Scalar color(0, 255, 0);
                    if (i == maxScoreIndex)
                        color[2] = 255;
                    cv::circle(frame,
                        cv::Point((int)(rasterPoint.uv.x() * frame.cols),
                            (int)(rasterPoint.uv.y() * frame.rows)),
                        1, color, -1);
                }
            }
        }

    Logger::log("\tIndex\t\tScore\t\tLoss");
    for (uint i = 0; i < candidates.size(); i++)
        Logger::log("\t" + tr::string(i+1, 3)
                  + "\t\t" + tr::string(scores[i], 3)
                  + "\t\t" + tr::string(losses[i], 3));
    Logger::log("min loss:" + tr::string(minLossIndex + 1));
    Logger::log("max score:" + tr::string(maxScoreIndex + 1));
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return poses[minLossIndex];
}
