#include "Geometry.hpp"
#include "../Misc/Log.hpp"
#include <string>
#include "../Misc/ConfigParser.hpp"
#include "../Misc/Links.hpp"
#include "../Math/Edge.hpp"

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
    std::vector<vec3f> tempNormals;
    std::vector<Edge<>> tempEdges;
    tempNormals.reserve(indexCount / 2u);
    tempEdges.reserve(indexCount / 2u);
    edges.reserve(indexCount / 2u * 4u);
    highEdgeIndices.reserve(indexCount / 2u);
    lowEdgeIndices.reserve(indexCount / 2u);

    uint vertexCount = 0u;
    const float highThreshold = radian(config.getEntry("high threshold", 30.0f));
    const float lowThreshold = radian(config.getEntry("low threshold", 1e-3f));
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