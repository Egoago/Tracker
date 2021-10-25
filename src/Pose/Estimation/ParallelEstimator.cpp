#include "ParallelEstimator.hpp"
#include <limits>

using namespace tr;

const std::vector<const Template*> tr::ParallelEstimator::estimate(const DistanceTensor& dcd3t) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    std::vector<const Template*> candidates(candidateCount, nullptr);
    double max;
    uint id;
#pragma omp parallel num_threads(candidateCount) private(id, max)
    {
        max = std::numeric_limits<double>::max();
        id = omp_get_thread_num();
#pragma omp for
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
