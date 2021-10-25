#include "DirectEstimator.hpp"

using namespace tr;

const std::vector<const Template*> tr::DirectEstimator::estimate(const DistanceTensor& dcd3t) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    std::vector<const Template*> candidates;
    candidates.reserve(candidateCount + 1u);
    std::vector<double> distances;
    distances.reserve(candidateCount + 1u);
    for (const Template* temp = templates.begin(); temp < templates.end(); temp++) {
        const uint pixelCount = (uint)temp->rasterPoints.size();
        if (pixelCount < 20) continue;
        const double distance = getDistance(*temp, dcd3t);
        if (candidates.size() < candidateCount) {
            distances.push_back(distance);
            candidates.push_back(temp);
        }
        else if (distances.back() > distance) {
            for (uint index = 0; index < candidateCount; index++)
                if (distances[index] > distance) {
                    distances.insert(distances.begin() + index, distance);
                    candidates.insert(candidates.begin() + index, temp);
                    distances.pop_back();
                    candidates.pop_back();
                    break;
                }
        }
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return candidates;
}