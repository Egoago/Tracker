#include "ParallelEstimator.hpp"
#include <limits>
#include <queue>
#include <utility>
#include "../../Misc/ConfigParser.hpp"

using namespace tr;

typedef std::pair<double, int> index;

const std::vector<const Template*> tr::ParallelEstimator::estimate(const DistanceTensor& dcd3t) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const int size = int(templates.getSize());
    const Template* start = templates.begin();
    double* distances = new double[size];
#pragma omp parallel for num_threads(omp_get_num_procs())
    for (int i = 0; i < size; i++)
        distances[i] = getDistance(*(start+i), dcd3t);
    std::priority_queue<index, std::vector<index>> indices;
    for (int i = 0; i < size; i++) {
        const float distance = distances[i];
        if (indices.size() < candidateCount)
            indices.push(std::make_pair(distance, i));
        else if (indices.top().first > distance){
            indices.pop();
            indices.push(std::make_pair(distance, i));
        }
    }
    delete[](distances);
    std::vector<const Template*> candidates;
    candidates.reserve(candidateCount);
    while (!indices.empty()) {
        candidates.push_back(start + indices.top().second);
        Logger::log(tr::string(indices.top().first));
        indices.pop();
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return candidates;
}
