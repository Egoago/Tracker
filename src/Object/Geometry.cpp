#include "Geometry.hpp"
#include "../Misc/Log.hpp"
#include <string>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Links.hpp"

using namespace std;
using namespace tr;

static ConfigParser config(GEO_CONFIG_FILE);

template <class T>
uint find(const T* array, const T& value, const uint size) {
    for (uint i = 0; i < size; i++)
        if (array[i] == value)
            return i;
    return size;
}

void Geometry::generate() {
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    //TODO implement soring based pairing with orientation alignement
    //TODO implement a more sohpisticated algorithm
    //like leave out edges on same triangle
    //or match triangles
    const uint indexCount = (uint)indices.size();
    auto tempNormals = make_unique<vec3f[]>(indexCount / 2u);
    std::vector<vec3f> buffer(indexCount);
    lowEdgeVertices.reserve(indexCount / 2u);
    highEdgeVertices.reserve(indexCount / 2u);
    lowEdgeCurvatures.reserve(indexCount / 2u);
    highEdgeCurvatures.reserve(indexCount / 2u);

    uint pairCount = 0;
    uint singleCount = 0;
    uint pos = 0u;
    for (uint i = 0; i < indexCount/3u; i++) {
        for (uint k = 0; k < 3; k++) {
            const vec3f a = vertices.at(indices.at(3 * i + k));
            const vec3f b = vertices.at(indices.at(3 * i + (1 + k) % 3));
            const vec3f normal = normals.at(indices.at(3 * i + k));
            //find matching edge
            for (pos = 0u; pos < singleCount; pos++)
                if (buffer.at(pos * 2) == a && buffer.at(pos * 2 + 1) == b ||
                    buffer.at(pos * 2) == b && buffer.at(pos * 2 + 1) == a)
                    break;
            if (pos == singleCount) {
                buffer.at(pos * 2) = a;
                buffer.at(pos * 2 + 1) = b;
                tempNormals[pos] = normal;
                singleCount++;
            }
            else {
                float curvature = angle(normal, tempNormals[pos]);
                if (pos != pairCount) {
                    buffer.at(pos * 2) = buffer.at(pairCount * 2);
                    buffer.at(pos * 2 + 1) = buffer.at(pairCount * 2 + 1);
                    buffer.at(pairCount * 2) = a;
                    buffer.at(pairCount * 2 + 1) = b;
                    tempNormals[pos] = tempNormals[pairCount];
                }
                singleCount--;
                pairCount++;
            }
        }
    }
    //generating outliner edges
    std::vector<vec3f> lowEdges, highEdges, lowDirections, highDirections;
    const float lowThreshold = radian(config.getEntry("low threshold", 1e-3f));
    const float highThreshold = radian(config.getEntry("high threshold", 30.0f));
    for (uint i = 0; i < pairCount; i++) {
        float curvature = geometry.edgeCurvatures[i];
        if (curvature > lowThreshold) {
            vec3f a = geometry.edgeVertices[2 * i];
            vec3f b = geometry.edgeVertices[2 * i + 1];
            lowEdges.push_back(a);
            lowEdges.push_back(b);
            vec3f dir = -(a - b).matrix().normalized();
            //opposite directions are the same
            if (vec3f(0.9f, 0.65f, 0.56f).matrix().normalized().dot(dir.matrix()) < 0.0f)
                dir = -dir;
            lowDirections.push_back(dir);
            lowDirections.push_back(dir);
            if (curvature > highThreshold) {
                highEdges.push_back(a);
                highEdges.push_back(b);
                highDirections.push_back(dir);
                highDirections.push_back(dir);
            }
        }
    }

    Logger::log("edges: " + std::to_string(indexCount));
    Logger::log("pairs: " + std::to_string(pairCount));
    Logger::log("missed: " + std::to_string(singleCount));
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
}