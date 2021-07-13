#include "DCDT3Generator.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "Misc/Links.h"
#include "Detectors/LSDDetector.h"
#include "Detectors/CannyDetector.h"

using namespace std;
using namespace cv;

uint64_t constexpr mix(char m, uint64_t s) {
    return ((s << 7) + ~(s >> 3)) + ~m;
}

uint64_t constexpr my_hash(const char* m) {
    return (*m) ? mix(*m, my_hash(m + 1)) : 0;
}

ConfigParser DCDT3Generator::config(DCDT3_CONFIG_FILE);

size_t getQ(ConfigParser& config) {
    return std::stoi(config.getEntry("orientation quantization", "60"));
}

EdgeDetector* getEdgeDetector(ConfigParser& config) {
    switch (my_hash(config.getEntry("edge detector", "lsd").c_str())) {
    case my_hash("canny"): return new CannyDetector();
    case my_hash("lsd"):
    default: return new LSDDetector();
    }
}

DCDT3Generator::DCDT3Generator()
    :q(getQ(config)),
     edgeDetector(getEdgeDetector(config)) {
    config.save();
    dcdt3.resize(q);

    /*namedWindow("Original", WINDOW_NORMAL);
    resizeWindow("Original", 400, 300);
    moveWindow("Original", 50, 50);
    namedWindow("LSD", WINDOW_NORMAL);
    resizeWindow("LSD", 400, 300);
    moveWindow("LSD", 400 + 50, 50);*/
    namedWindow("Quantized", WINDOW_NORMAL);
    resizeWindow("Quantized", 400, 300);
    moveWindow("Quantized", 0, 50);
    namedWindow("Quantized 2", WINDOW_NORMAL);
    resizeWindow("Quantized 2", 400, 300);
    moveWindow("Quantized 2", 400, 50);
    namedWindow("Distance transform", WINDOW_NORMAL);
    resizeWindow("Distance transform", 400, 300);
    moveWindow("Distance transform", 800, 50);
    namedWindow("Distance transform 2", WINDOW_NORMAL);
    resizeWindow("Distance transform 2", 400, 300);
    moveWindow("Distance transform 2", 1200, 50);
    namedWindow("Directional Distance transform", WINDOW_NORMAL);
    resizeWindow("Directional Distance transform", 400, 300);
    moveWindow("Directional Distance transform", 400, 450);
    namedWindow("Directional Distance transform 2", WINDOW_NORMAL);
    resizeWindow("Directional Distance transform 2", 400, 300);
    moveWindow("Directional Distance transform 2", 800, 450);
}

int DCDT3Generator::quantizedIndex(float value) const {
    const constexpr float pi = glm::pi<float>();
    const float d = pi / q;
    return (int)(value / d) % q;
}

float DCDT3Generator::quantize(float value) const {
    const constexpr float pi = glm::pi<float>();
    const float d = pi / q;
    return quantizedIndex(value) * d + d/2.0f;
}

void DCDT3Generator::setFrame(cv::Mat& nextFrame)
{
    //TODO move temp holders to class fields
    //imshow("Original", nextFrame);

    vector<Edge<glm::vec2>> edges;
    edgeDetector->detectEdges(nextFrame, edges);

    vector<vector<Edge<glm::vec2>>> quantizedEdges(q);
    for (Edge<glm::vec2>& edge : edges)
        quantizedEdges[quantizedIndex(getOrientation(edge))].push_back(edge);
    Mat tmp(nextFrame.rows,nextFrame.cols, CV_8U);
    tmp = Scalar::all(0);
    for (Edge<glm::vec2>& edge : quantizedEdges[2]) {
        Point A((int)edge.a.x, (int)edge.a.y), B((int)edge.b.x, (int)edge.b.y);
        line(tmp, A, B, Scalar(255.0), 1, FILLED, LINE_8);
    }
    imshow("Quantized", tmp);
    tmp = Scalar::all(0);
    for (Edge<glm::vec2>& edge : quantizedEdges[6]) {
        Point A((int)edge.a.x, (int)edge.a.y), B((int)edge.b.x, (int)edge.b.y);
        line(tmp, A, B, Scalar(255.0), 1, FILLED, LINE_8);
    }
    imshow("Quantized 2", tmp);

    for (size_t i = 0; i < q; i++) {
        tmp = Scalar::all(255.0);
        for (Edge<glm::vec2>& edge : quantizedEdges[i]) {
            const Point A((int)edge.a.x, (int)edge.a.y), B((int)edge.b.x, (int)edge.b.y);
            line(tmp, A, B, Scalar(0.0), 1, FILLED, LINE_8);
        }
        distanceTransform(tmp, dcdt3[i], DIST_L2, DIST_MASK_PRECISE);
    }
    normalize(dcdt3[2], tmp, 0.0, 1.0, NORM_MINMAX);
    imshow("Distance transform", tmp);
    normalize(dcdt3[6], tmp, 0.0, 1.0, NORM_MINMAX);
    imshow("Distance transform 2", tmp);
    directedDistanceTransform();

    normalize(dcdt3[2], tmp, 0.0, 1.0, NORM_MINMAX);
    imshow("Directional Distance transform", tmp);
    normalize(dcdt3[6], tmp, 0.0, 1.0, NORM_MINMAX);
    imshow("Directional Distance transform 2", tmp);
    waitKey(1);

}

void DCDT3Generator::directedDistanceTransform() {
	float* costs;
	costs = new float[q];
    const float maxCost = 3000.0f;
    const float lambda = 100.0f;
    const float dirCost = lambda*glm::pi<float>()/q;

    //TODO parallelization
	for (int x = 0; x < dcdt3[0].cols; x++)
	for (int y = 0; y < dcdt3[0].rows; y++) {
		for (int i = 0; i < q; i++) {
			costs[i] = dcdt3[i].at<float>(y,x);
			if (costs[i] > maxCost)
				costs[i] = maxCost;
		}

		//forward pass
		if (costs[0] > costs[q - 1] + dirCost)
			costs[0] = costs[q - 1] + dirCost;
		for (int i = 1; i < q; i++)
			if (costs[i] > costs[i - 1] + dirCost)
				costs[i] = costs[i - 1] + dirCost;

		if (costs[0] > costs[q - 1] + dirCost)
			costs[0] = costs[q - 1] + dirCost;
		for (int i = 1; i < q; i++)
			if (costs[i] > costs[i - 1] + dirCost)
				costs[i] = costs[i - 1] + dirCost;
			else break;

		//backward pass
		if (costs[q - 1] > costs[0] + dirCost)
			costs[q - 1] = costs[0] + dirCost;
		for (int i = q - 1; i > 0; i--)
			if (costs[i - 1] > costs[i] + dirCost)
				costs[i - 1] = costs[i] + dirCost;

		if (costs[q - 1] > costs[0] + dirCost)
			costs[q - 1] = costs[0] + dirCost;
		for (int i = q - 1; i > 0; i--)
			if (costs[i - 1] > costs[i] + dirCost)
				costs[i - 1] = costs[i] + dirCost;
			else break;

		for (int i = 0; i < q; i++)
            dcdt3[i].at<float>(y,x) = costs[i];
	}

	delete[] costs;
}
