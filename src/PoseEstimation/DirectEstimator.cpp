#include "DirectEstimator.h"

using namespace tr;

std::vector<Template*> tr::DirectEstimator::estimate(const DistanceTensor& dcd3t) {
    std::vector<Template*> candidates;
    float* distances = new float[candidateCount] {0.0f};
    unsigned int c = 0;
    for (Template* temp = templates->begin(); temp < templates->end(); temp++, c++) {
        const unsigned int pixelCount = (unsigned int)temp->pos.size();
        if (pixelCount < 20) {
            //Logger::warning(std::to_string(c++) + ":\tpixels " + std::to_string(pixelCount));
            continue;
        }
        //TODO implement non naive way/paralellization/OpenMP
        float distance = 0.0f;
        for (unsigned int i = 0; i < pixelCount; i++) {
            distance += dcd3t.getDist(temp->uv[i], temp->angle[i]);
        }
        distance /= pixelCount;
        /*Logger::log(std::to_string(c) + ":\tpixels " + std::to_string(pixelCount)
                    + "\tloss " + std::to_string(distance));*/
        if (candidates.size() < candidateCount) {
            distances[candidates.size()] = distance;
            candidates.push_back(temp);
        }
        else if (distances[candidateCount-1] > distance)
        {
            unsigned int index = 0;
            for (index = 0; index < candidateCount; index++)
                if (distances[index] > distance) break;
            for (unsigned int backIndex = candidateCount - 1; backIndex > index; backIndex--)
                distances[backIndex] = distances[backIndex - 1];
            distances[index] = distance;

            candidates.insert(candidates.begin() + index, temp);
            candidates.pop_back();
        }
    }
    return candidates;
}