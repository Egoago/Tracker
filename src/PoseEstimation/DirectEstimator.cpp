#include "DirectEstimator.hpp"

using namespace tr;

std::vector<Template*> tr::DirectEstimator::estimate(const DistanceTensor& dcd3t) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    std::vector<Template*> candidates;
    real* distances = new real[candidateCount] {0.0f};
    uint c = 0;
    for (Template* temp = templates.begin(); temp < templates.end(); temp++, c++) {
        const uint pixelCount = (uint)temp->rasterPoints.size();
        if (pixelCount < 20) {
            //Logger::warning(std::to_string(c++) + ":\tpixels " + std::to_string(pixelCount));
            continue;
        }
        //TODO implement non naive way/paralellization/OpenMP
        real distance = 0.0f;
        //TODO generalize with custom loss functions as layer
        for (uint i = 0; i < pixelCount; i++) {
            const real indices[3] = {
                real(temp->rasterPoints[i].indexData[0]),
                real(temp->rasterPoints[i].indexData[1]),
                real(temp->rasterPoints[i].indexData[2])};
            const real value = dcd3t.evaluate(indices);
            distance += value* value;
        }
        distance /= (real)pixelCount;
        if (pixelCount <= 20)
            distance = 1000000000000.0f;
        else if (pixelCount < 80)
            distance /= (pixelCount - 20) / 60.0f;
        //distance /= 1.0f + std::expf(-(float)pixelCount / 20.0f + 2.5f);
        /*Logger::log(std::to_string(c) + ":\tpixels " + std::to_string(pixelCount)
                    + "\tloss " + std::to_string(distance));*/
        if (candidates.size() < candidateCount) {
            distances[candidates.size()] = distance;
            candidates.push_back(temp);
        }
        else if (distances[candidateCount-1] > distance)
        {
            uint index = 0;
            for (index = 0; index < candidateCount; index++)
                if (distances[index] > distance) break;
            for (uint backIndex = candidateCount - 1; backIndex > index; backIndex--)
                distances[backIndex] = distances[backIndex - 1];
            distances[index] = distance;

            candidates.insert(candidates.begin() + index, temp);
            candidates.pop_back();
        }
    }
    for (uint i = 0; i < candidateCount; i++)
        Logger::log(std::to_string(i + 1) + ". candidate: " + std::to_string(distances[i]));
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    return candidates;
}