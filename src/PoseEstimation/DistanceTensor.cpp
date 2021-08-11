#include "DistanceTensor.hpp"
#include <opencv2/imgproc.hpp>
#include <initializer_list>
#include <glm/ext/scalar_constants.hpp>
#include "../Misc/Links.hpp"
#include "../Misc/Log.hpp"
#include "../Detectors/LSDDetector.hpp"
#include "../Detectors/CannyDetector.hpp"
//TODO remove logging
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;
using namespace tr;

ConfigParser DistanceTensor::config(DCDT3_CONFIG_FILE);

uint getQ(ConfigParser& config) {
    return std::stoi(config.getEntry("orientation quantization", "60"));
}

EdgeDetector* getEdgeDetector(ConfigParser& config) {
    switch (strHash(config.getEntry("edge detector", "lsd").c_str())) {
    case strHash("canny"): return new CannyDetector();
    case strHash("lsd"):
    default: return new LSDDetector();
    }
}

DistanceTensor::DistanceTensor(uint width, uint height) :
    q(getQ(config)),
    width(width),
    height(height),
    edgeDetector(getEdgeDetector(config)),
    maxCost(std::sqrtf((float)width*width+height*height)) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    buffers = new cv::Mat*[2];
    for (int buffer = 0; buffer < 2; buffer++) {
        buffers[buffer] = new cv::Mat[q];
        for (uint i = 0; i < q; i++)
            buffers[buffer][i].create((int)height, (int)width, CV_32F);
    }
    front = true;
    quantizedEdges = new std::vector<Edge<glm::vec2>>[q];
    tmp = Mat((int)height, (int)width, CV_8U);
    costs = new float[q];
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void DistanceTensor::setFrame(const cv::Mat& nextFrame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    vector<Edge<glm::vec2>> edges;
    Mat frame = nextFrame;
    if (nextFrame.type() != CV_8U) {
        cvtColor(nextFrame, frame, COLOR_BGR2GRAY);
    }
    edgeDetector->detectEdges(frame, edges);

    distanceTransformFromEdges(edges);
    
    directedDistanceTransform();

    //gaussianBlur();
    //TODO remove logging
   /* while(1)
    for (uint i = 0; i < q; i++) {
        tmp = Scalar::all(255);
        for (Edge<glm::vec2>& edge : quantizedEdges[i]) {
            const Point A((int)std::round(edge.a.x),
                (int)std::round(edge.a.y)),
                B((int)std::round(edge.b.x),
                    (int)std::round(edge.b.y));
            line(tmp, A, B, Scalar(0), 1, LINE_8);
        }
        cv::imshow("detected lines", tmp);
        cv::waitKey(1);

        buffers[front][i].convertTo(tmp, CV_32F, 2.0f / maxCost);
        cv::imshow("dist transform", tmp);
        cv::waitKey(1);
        Logger::log( std::to_string(i)+ ". quanized edges");
        cv::waitKey(10000000);
    }*/

    //TODO reintroduce blurring
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void tr::DistanceTensor::distanceTransformFromEdges(const std::vector<Edge<glm::vec2>>& edges) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    for (uint i = 0; i < q; i++)
        quantizedEdges[i].clear();
    for (const Edge<glm::vec2>& edge : edges)
        quantizedEdges[(uint)quantizedIndex(edge.orientation(), q)].push_back(edge);

    for (uint i = 0; i < q; i++) {
        tmp = Scalar::all(255);
        for (Edge<glm::vec2>& edge : quantizedEdges[i]) {
            const Point A((int)std::round(edge.a.x),
                (int)std::round(edge.a.y)),
                B((int)std::round(edge.b.x),
                    (int)std::round(edge.b.y));
            line(tmp, A, B, Scalar(0), 1, LINE_8);
        }
        cv::distanceTransform(tmp, buffers[front][i], DIST_L2, DIST_MASK_PRECISE, CV_32F);
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

std::shared_ptr<uint[]> getIndices(const cv::Mat& buffer) {
    const uint width = buffer.cols;
    const uint height = buffer.rows;
    uint stride = (uint)buffer.step1();
    std::shared_ptr<uint[]> indices(new uint[width * height]);
    for (uint y = 0; y < height; y++) {
        for (uint x = 0; x < width; x++)
            indices[width * y + x] = stride * y + x;
        }
    return indices;
}

void DistanceTensor::directedDistanceTransform() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static float lambda = stof(config.getEntry("lambda", "100.0"));
    const static float dirCost = lambda*glm::pi<float>()/q;
    const static auto indices = getIndices(buffers[front][0]);
    //TODO parallelization
    //TODO buffer swap
	for (uint p = 0; p < width * height; p++) {
        const uint index = indices[p];
		for (uint i = 0; i < q; i++) {
			costs[i] = ((float*)buffers[front][i].data)[index];
			if (costs[i] > maxCost)
				costs[i] = maxCost;
		}

		//forward pass
		if (costs[0] > costs[q - 1] + dirCost)
			costs[0] = costs[q - 1] + dirCost;
		for (uint i = 1; i < q; i++)
			if (costs[i] > costs[i - 1] + dirCost)
				costs[i] = costs[i - 1] + dirCost;

		if (costs[0] > costs[q - 1] + dirCost)
			costs[0] = costs[q - 1] + dirCost;
		for (uint i = 1; i < q; i++)
			if (costs[i] > costs[i - 1] + dirCost)
				costs[i] = costs[i - 1] + dirCost;
			else break;

		//backward pass
		if (costs[q - 1] > costs[0] + dirCost)
			costs[q - 1] = costs[0] + dirCost;
		for (uint i = q - 1; i > 0; i--)
			if (costs[i - 1] > costs[i] + dirCost)
				costs[i - 1] = costs[i] + dirCost;

		if (costs[q - 1] > costs[0] + dirCost)
			costs[q - 1] = costs[0] + dirCost;
		for (uint i = q - 1; i > 0; i--)
			if (costs[i - 1] > costs[i] + dirCost)
				costs[i - 1] = costs[i] + dirCost;
			else break;

		for (uint i = 0; i < q; i++)
            ((float*)buffers[front][i].data)[index] = costs[i];
	}
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

//TODO merge blur with ddt?
void DistanceTensor::gaussianBlur() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    uint stride = (uint)buffers[front][0].step1();
    const static auto indices = getIndices(buffers[front][0]);
    //TODO parallelization
    for (uint p = 0; p < width * height; p++) {
            const uint pixelIndex = indices[p];
            for (uint dir = 0; dir < q; dir++) {
                const uint prev = (dir == 0) ? q-1 : dir - 1;
                const uint next = (dir == q-1) ? 0 : dir + 1;
                //TODO localization
                ((float*)buffers[!front][dir].data)[pixelIndex] =
                    ((float*)buffers[front][prev].data)[pixelIndex] * 0.25f +
                    ((float*)buffers[front][dir].data)[pixelIndex] * 0.5f +
                    ((float*)buffers[front][next].data)[pixelIndex] * 0.25f;
            }
        }
    front = !front; //swap buffers
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}
