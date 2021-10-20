#include "Geometry.hpp"
#include "../Misc/Log.hpp"
#include <string>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Constants.hpp"
#include "../Math/Edge.hpp"
#include <limits>
#include <execution>

using namespace std;
using namespace tr;

template <class T>
uint find(const T* array, const T& value, const uint size) {
    for (uint i = 0; i < size; i++)
        if (array[i] == value)
            return i;
    return size;
}

vec3f getCenter(const std::vector<vec3f>& vertices) {
    constexpr const float minFloat = numeric_limits<float>::min();
    constexpr const float maxFloat = numeric_limits<float>::max();
    float boundaries[6] = { maxFloat, minFloat,     //x
                            maxFloat, minFloat,     //y
                            maxFloat, minFloat };   //z
    for (const auto& vertex : vertices) {
        if (vertex.x() < boundaries[0])
            boundaries[0] = vertex.x();
        if (vertex.x() > boundaries[1])
            boundaries[1] = vertex.x();
        if (vertex.y() < boundaries[2])
            boundaries[2] = vertex.y();
        if (vertex.y() > boundaries[3])
            boundaries[3] = vertex.y();
        if (vertex.z() < boundaries[4])
            boundaries[4] = vertex.z();
        if (vertex.z() > boundaries[5])
            boundaries[5] = vertex.z();
    }
    return vec3f((boundaries[0] + boundaries[1]) / 2.0f,
                 (boundaries[2] + boundaries[3]) / 2.0f,
                 (boundaries[4] + boundaries[5]) / 2.0f);
}

float getRadius(const std::vector<vec3f>& vertices) {
    float radius = 0.0f;
    for (const auto& vertex : vertices) {
        const float length = vertex.matrix().norm();
        if (length > radius) radius = length;
    }
    return radius;
}

void Geometry::generate() {
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
    //===== Scaling =======
    centerOffset = -getCenter(vertices);
    std::for_each(
        std::execution::par_unseq,
        vertices.begin(),
        vertices.end(),
        [&centerOffset=centerOffset](auto& vertex)
        {
            vertex += centerOffset;
        });
    boundingRadius = getRadius(vertices);
    //TODO implement soring based pairing with orientation alignement
    //TODO implement a more sohpisticated algorithm
    //like leave out edges on same triangle
    //or match triangles
    const uint indexCount = (uint)indices.size();
    std::vector<vec3f> tempNormals;
    std::vector<Edge<>> tempEdges;
    tempNormals.reserve(indexCount / 2u);
    tempEdges.reserve(indexCount / 2u);
    edges.reserve(indexCount / 2u * 4u);
    highEdgeIndices.reserve(indexCount / 2u);
    lowEdgeIndices.reserve(indexCount / 2u);

    uint vertexCount = 0u;
    const float highThreshold = radian(ConfigParser::instance().getEntry(CONFIG_SECTION_GEOMETRY, "high threshold", 30.0f));
    const float lowThreshold = radian(ConfigParser::instance().getEntry(CONFIG_SECTION_GEOMETRY, "low threshold", 1e-3f));
    for (uint i = 0u; i < indexCount/3u; i++) { //for every triangle in a strip
        for (uint k = 0u; k < 3u; k++) {        //for every vertex in that triangle
            const Edge<> edge(vertices[indices[3u * i + k]],
                              vertices[indices[3u * i + (1u + k) % 3u]]);
            const vec3f normal = normals[indices[3u * i + k]];
            //find matching edge
            auto it = std::find(tempEdges.begin(), tempEdges.end(), edge);
            if(it == tempEdges.end()) {
#ifdef _DEBUG
                if (tempEdges.size() == indexCount / 2u)
                    Logger::error("missed edge");
#endif // !_DEBUG
                tempEdges.emplace_back(edge);
                tempNormals.emplace_back(normal);
            }
            else {
                const int index = (int)(it - tempEdges.begin());
                float curvature = angle(normal, tempNormals[index]);
                if (it != tempEdges.end()-1) {
                    tempEdges[index] = tempEdges.back();
                    tempNormals[index] = tempNormals.back();
                }
                tempEdges.pop_back();
                tempNormals.pop_back();
                if (curvature > lowThreshold) {
                    const vec3f dir = edge.direction();
                    edges.emplace_back(edge.a);
                    edges.emplace_back(dir);
                    edges.emplace_back(edge.b);
                    edges.emplace_back(dir);
                    if (curvature > highThreshold) {
                        highEdgeIndices.emplace_back(vertexCount++);
                        highEdgeIndices.emplace_back(vertexCount++);
                    }
                    else {
                        lowEdgeIndices.emplace_back(vertexCount++);
                        lowEdgeIndices.emplace_back(vertexCount++);
                    }
                }
            }
        }
    }
    edges.shrink_to_fit();
    highEdgeIndices.shrink_to_fit();
    lowEdgeIndices.shrink_to_fit();

    Logger::log("vertices: " + std::to_string(vertices.size()));
    Logger::log("edges: " + std::to_string(edges.size()/4));
    Logger::log("high indices: " + std::to_string(highEdgeIndices.size()));
    Logger::log("low indices: " + std::to_string(lowEdgeIndices.size()));
    Logger::log("mesh indices: " + std::to_string(indices.size()));
    Logger::log("missed: " + std::to_string(tempEdges.size()));
    tr::Logger::logProcess(__FUNCTION__);   //TODO remove logging
}