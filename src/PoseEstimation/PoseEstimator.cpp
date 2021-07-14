#include "PoseEstimator.h"

PoseEstimator::PoseEstimator(const int width, const int height, const Model& model) :
    generator(DCDT3Generator(width, height)),
    model(model) {
    
}

SixDOF PoseEstimator::getPose(cv::Mat& frame)
{
    std::vector<cv::Mat>& dcd3t = generator.setFrame(frame);
    auto& templates = model.getTemplates();
    float constexpr minDist = std::numeric_limits<float>::max();
    for (const Template* i = templates.data(); i < (templates.data() + templates.num_elements()); i++) {
        SixDOF sixDOF = i->sixDOF;

    }
    return templates.origin()->sixDOF;
}
