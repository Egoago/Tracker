#include "Geometry.hpp"
#include "../Misc/Log.hpp"
#include <string>

using namespace std;
using namespace tr;

template <class T>
uint find(const T* array, const T& value, const uint size) {
    for (uint i = 0; i < size; i++)
        if (array[i] == value)
            return i;
    return size;
}

void Geometry::detectEdgePairs() {
    //TODO implement soring based pairing with orientation alignement
    //TODO implement a more sohpisticated algorithm
    //like leave out edges on same triangle
    //or match triangles

    auto tempNormals = make_unique<vec3f[]>(getIndexCount() / 2);
    edgeVertices.resize(getIndexCount());
    edgeCurvatures.resize(getIndexCount() / 2);

    uint pairCount = 0;
    uint singleCount = 0;
    for (uint i = 0; i < getFaceCount(); i++) {
        for (uint k = 0; k < 3; k++) {
            const vec3f a = vertices.at(indices.at(3 * i + k));
            const vec3f b = vertices.at(indices.at(3 * i + (1 + k) % 3));
            const vec3f normal = normals.at(indices.at(3 * i + k));
            //find matching edge
            uint pos = pairCount;
            while (pos < singleCount + pairCount) {
                if (edgeVertices.at(pos * 2) == a && edgeVertices.at(pos * 2 + 1) == b ||
                    edgeVertices.at(pos * 2) == b && edgeVertices.at(pos * 2 + 1) == a)
                    break;
                pos++;
            }
            if (pos == singleCount + pairCount) {
                edgeVertices.at(pos * 2) = a;
                edgeVertices.at(pos * 2 + 1) = b;
                tempNormals[pos] = normal;
                singleCount++;
            }
            else {
                edgeCurvatures.at(pairCount) = angle(normal, tempNormals[pos]);
                if (pos != pairCount) {
                    edgeVertices.at(pos * 2) = edgeVertices.at(pairCount * 2);
                    edgeVertices.at(pos * 2 + 1) = edgeVertices.at(pairCount * 2 + 1);
                    edgeVertices.at(pairCount * 2) = a;
                    edgeVertices.at(pairCount * 2 + 1) = b;
                    tempNormals[pos] = tempNormals[pairCount];
                }
                singleCount--;
                pairCount++;
            }
        }
    }
    Logger::log("edges: " + std::to_string(getIndexCount()));
    Logger::log("pairs: " + std::to_string(pairCount));
    Logger::log("missed: " + std::to_string(singleCount));
}