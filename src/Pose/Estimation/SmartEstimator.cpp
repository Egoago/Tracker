#include "SmartEstimator.hpp"
#include <limits>
#include <omp.h>
#include "../../Misc/ConfigParser.hpp"
#include "../../Misc/Constants.hpp"

using namespace tr;

tr::SmartEstimator::SmartEstimator(const Tensor<Template>& templates) : Estimator(templates) {
    chunkSize = ConfigParser::instance().getEntry(CONFIG_SECTION_ESTIMATION, "chunk size", 30u);
}

const std::vector<const Template*> tr::SmartEstimator::estimate(const DistanceTensor& dcd3t) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    std::vector<const Template*> candidates(candidateCount, nullptr);
    double max;
    uint id;
    //Logger::log("Processors: " + std::to_string(omp_get_num_procs()));
    //Logger::log("Max threads: " + std::to_string(omp_get_max_threads()));
#pragma omp parallel num_threads(candidateCount) private(id, max) shared(candidates)
    {
        max = std::numeric_limits<double>::max();
        id = omp_get_thread_num();
#pragma omp for nowait schedule(dynamic, chunkSize)
        for (int i = 0; i < int(templates.getSize()); i++) {
            const Template* temp = templates.begin() + i;
            if (temp->rasterPoints.size() >= 20) {
                const double distance = getDistance(*temp, dcd3t);
                if (distance < max) {
                    max = distance;
                    candidates[id] = temp;
                }
            }
        }
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return candidates;
}