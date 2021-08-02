#include "DistanceTensor.h"
#include <opencv2/imgproc.hpp>
#include "Misc/Links.h"
#include "Detectors/LSDDetector.h"
#include "Detectors/CannyDetector.h"

using namespace std;
using namespace cv;
using namespace tr;

ConfigParser DistanceTensor::config(DCDT3_CONFIG_FILE);

constexpr inline int quantizedIndex(const float value, const unsigned int q) {
    const constexpr float pi = glm::pi<float>();
    const float d = pi / q;
    return (int)(value / d) % q;
}

unsigned int getQ(ConfigParser& config) {
    return std::stoi(config.getEntry("orientation quantization", "60"));
}

EdgeDetector* getEdgeDetector(ConfigParser& config) {
    switch (strHash(config.getEntry("edge detector", "lsd").c_str())) {
    case strHash("canny"): return new CannyDetector();
    case strHash("lsd"):
    default: return new LSDDetector();
    }
}

DistanceTensor::DistanceTensor(unsigned int width, unsigned int height)
    :q(getQ(config)),
     edgeDetector(getEdgeDetector(config)),
    dcdt3(buffer1),
    other(buffer2),
    maxCost(stof(config.getEntry("max cost", "3000.0"))) {
    buffer1.resize(q);
    buffer2.resize(q, Mat((int)height, (int)width, CV_32F));
    first = true;
    quantizedEdges.resize(q);
    tmp = Mat((int)height, (int)width, CV_8U);
    costs = new float[q];
}

void DistanceTensor::swapBuffers() {
     dcdt3 = (first) ? buffer2 : buffer1;
     other = (!first) ? buffer2 : buffer1;
     first = !first;
}

void DistanceTensor::setFrame(const cv::Mat& nextFrame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    vector<Edge<glm::vec2>> edges;
    edgeDetector->detectEdges(nextFrame, edges);
    for (auto& container : quantizedEdges)
        container.clear();
    for (Edge<glm::vec2>& edge : edges)
        quantizedEdges[quantizedIndex(getOrientation(edge), q)].push_back(edge);

    for (unsigned int i = 0; i < q; i++) {
        tmp = Scalar::all(255.0);
        for (Edge<glm::vec2>& edge : quantizedEdges[i]) {
            const Point A((int)edge.a.x, (int)edge.a.y), B((int)edge.b.x, (int)edge.b.y);
            line(tmp, A, B, Scalar(0.0), 1, FILLED, LINE_8);
        }
        distanceTransform(tmp, dcdt3[i], DIST_L2, DIST_MASK_PRECISE);
    }

    directedDistanceTransform();

    //TODO reintroduce blurring
    //gaussianBlur();
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

float tr::DistanceTensor::getDist(const glm::vec2 uv, const float angle) const {
    cv::Point pixel((int)((uv.x * (float)tmp.cols) + 0.5f),
        (int)((uv.y * (float)tmp.rows) + 0.5f));
    if (pixel.x < 0 || pixel.x >= tmp.cols ||
        pixel.y < 0 || pixel.y >= tmp.rows) return maxCost;

    return dcdt3[quantizedIndex(angle, q)].at<float>(pixel);
}

//TODO merge blur with ddt?
void DistanceTensor::gaussianBlur() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    //TODO parallelization
    for (int x = 0; x < dcdt3[0].cols; x++)
        for (int y = 0; y < dcdt3[0].rows; y++) {
            for (unsigned int dir = 0; dir < q; dir++) {
                const unsigned int prev = (dir == 0) ? q-1 : dir - 1;
                const unsigned int next = (dir == q-1) ? 0 : dir + 1;
                //TODO localization
                other[dir].at<float>(y, x) =
                    dcdt3[prev].at<float>(y, x) * 0.25f +
                    dcdt3[dir].at<float>(y, x) * 0.5f +
                    dcdt3[next].at<float>(y, x) * 0.25f;
            }
        }
    swapBuffers();
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void DistanceTensor::directedDistanceTransform() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static float lambda = stof(config.getEntry("lambda", "1.0"));
    const float dirCost = lambda*glm::pi<float>()/q;

    //TODO parallelization
    //TODO buffer swap
	for (int x = 0; x < dcdt3[0].cols; x++)
	for (int y = 0; y < dcdt3[0].rows; y++) {
		for (unsigned int i = 0; i < q; i++) {
			costs[i] = dcdt3[i].at<float>(y,x);
			if (costs[i] > maxCost)
				costs[i] = maxCost;
		}

		//forward pass
		if (costs[0] > costs[q - 1] + dirCost)
			costs[0] = costs[q - 1] + dirCost;
		for (unsigned int i = 1; i < q; i++)
			if (costs[i] > costs[i - 1] + dirCost)
				costs[i] = costs[i - 1] + dirCost;

		if (costs[0] > costs[q - 1] + dirCost)
			costs[0] = costs[q - 1] + dirCost;
		for (unsigned int i = 1; i < q; i++)
			if (costs[i] > costs[i - 1] + dirCost)
				costs[i] = costs[i - 1] + dirCost;
			else break;

		//backward pass
		if (costs[q - 1] > costs[0] + dirCost)
			costs[q - 1] = costs[0] + dirCost;
		for (unsigned int i = q - 1; i > 0; i--)
			if (costs[i - 1] > costs[i] + dirCost)
				costs[i - 1] = costs[i] + dirCost;

		if (costs[q - 1] > costs[0] + dirCost)
			costs[q - 1] = costs[0] + dirCost;
		for (unsigned int i = q - 1; i > 0; i--)
			if (costs[i - 1] > costs[i] + dirCost)
				costs[i - 1] = costs[i] + dirCost;
			else break;

		for (unsigned int i = 0; i < q; i++)
            dcdt3[i].at<float>(y,x) = costs[i];
	}
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}
