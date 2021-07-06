#include "ModelEdgeDetector.h"
#include <iostream>

struct DirectedEdge {
    Vertex a, b;
    DirectedEdge(Vertex a = Vertex(), Vertex b = Vertex()) : a(a), b(b) {}

    DirectedEdge& flip() {
        Vertex c = a;
        a = b;
        b = c;
        return *this;
    }

    bool operator==(const DirectedEdge& other) {
        return (a == other.a) && (b == other.b);
    }
};

std::vector<DirectedEdge> detectEdgePairs(Geometry& geometry) {
    std::vector<DirectedEdge> pairs(geometry.getIndecesCount());
    unsigned int* indices = (unsigned int*)geometry.getIndices();
    Vertex* vertices = (Vertex*)geometry.getVertices();

    size_t pairCount = 0, singleCount = 0;
    for (size_t i = 0; i < geometry.getIndecesCount()/3; i++) {
        for (size_t k = 0; k < 3; k++) {
            DirectedEdge edge = DirectedEdge(vertices[indices[3*i + k]],    //1.
                                             vertices[indices[3*i + (1+k)%3]]);
            bool foundPair = false;
            for (size_t l = pairCount; l < (singleCount + pairCount); l++) {//2.
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
    std::cout << "edges: " << geometry.getIndecesCount() << std::endl;
    std::cout << "pairs: " << pairCount << std::endl;
    std::cout << "missed: " << singleCount << std::endl;
    return pairs;
}

std::vector<Edge> ModelEdgeDetector::detectOutlinerEdges(Geometry& geometry)
{
    std::vector<DirectedEdge> edgePairs = detectEdgePairs(geometry);
    for (size_t i = 0; i < edgePairs.size()/2; i++)
    {
        std::cout
            << edgePairs[i * 2].a.normal.x << " "
            << edgePairs[i * 2].a.normal.y << " "
            << edgePairs[i * 2].a.normal.z << " "
            << edgePairs[i * 2].b.normal.x << " "
            << edgePairs[i * 2].b.normal.y << " "
            << edgePairs[i * 2].b.normal.z << " "
            << std::endl
            << edgePairs[i * 2 + 1].a.normal.x << " "
            << edgePairs[i * 2 + 1].a.normal.y << " "
            << edgePairs[i * 2 + 1].a.normal.z << " "
            << edgePairs[i * 2 + 1].b.normal.x << " "
            << edgePairs[i * 2 + 1].b.normal.y << " "
            << edgePairs[i * 2 + 1].b.normal.z << " "
            << std::endl << "====================" << std::endl;
    }
    //TODO implement a more sohpisticated algorithm
    //like leave out edges on same triangle
    //or match triangles
    
    return std::vector<Edge>();
}
