#include "Geometry.h"
#include "Coordinates.h"

void Geometry::detectEdgePairs() {
    //TODO implement soring based pairing with orientation alignement
    std::vector<Edge<>> pairs(getIndexCount());
    const unsigned int* indices = getIndices();
    const glm::vec3* vertices = getVertices();
    //TODO implement a more sohpisticated algorithm
    //like leave out edges on same triangle
    //or match triangles
    unsigned int pairCount = 0, singleCount = 0;
    for (unsigned int i = 0; i < getFaceCount(); i++) {
        for (unsigned int k = 0; k < 3; k++) {
            Edge<> edge = Edge<>(vertices[indices[3 * i + k]],    //1.
                vertices[indices[3 * i + (1 + k) % 3]]);
            bool foundPair = false;
            for (unsigned int l = pairCount; l < (singleCount + pairCount); l++) {//2.
                if (pairs[2 * l] == edge ||                                 //3.
                    pairs[2 * l] == edge.flip()) {
                    pairs[2 * pairCount + 1] = pairs[2 * l];                //4.
                    pairs[2 * l] = pairs[2 * pairCount];                    //5.
                    pairs[2 * pairCount] = edge;                            //6.
                    pairCount++;                                            //7.
                    singleCount--;
                    foundPair = true;
                    break;
                }
            }
            if (!foundPair) {
                pairs[2 * (singleCount + pairCount)] = edge;
                singleCount++;
            }
        }
    }
    std::cout << "edges: " << getIndexCount() << std::endl;
    std::cout << "pairs: " << pairCount << std::endl;
    std::cout << "missed: " << singleCount << std::endl;
    for (auto &edge : pairs) {
        edgeVertices.push_back(edge.a);
        edgeVertices.push_back(edge.b);
    }
}