#include "DistanceTensor.h"
#include <opencv2/imgproc.hpp>
#include "Misc/Links.h"
#include "Detectors/LSDDetector.h"
#include "Detectors/CannyDetector.h"
#include <opencv2/highgui.hpp>

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
    return std::stoi(config.getEntry("orientation quantization", "10"));
}

EdgeDetector* getEdgeDetector(ConfigParser& config) {
    switch (strHash(config.getEntry("edge detector", "lsd").c_str())) {
    case strHash("canny"): return new CannyDetector();
    case strHash("lsd"):
    default: return new LSDDetector();
    }
}

DistanceTensor::DistanceTensor(unsigned int width, unsigned int height) :
    q(getQ(config)),
    width(width),
    height(height),
    edgeDetector(getEdgeDetector(config)),
    maxCost(stof(config.getEntry("max cost", "3000.0"))) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    buffers = new cv::Mat*[2];
    for (int buffer = 0; buffer < 2; buffer++) {
        buffers[buffer] = new cv::Mat[q];
        for (unsigned int i = 0; i < q; i++)
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

    //TODO remove monitoring
    tmp = Scalar::all(255);
    for (auto& edge : edges) {
        const Point A((int)std::round(edge.a.x),
                      (int)std::round(edge.a.y)),
                    B((int)std::round(edge.b.x),
                      (int)std::round(edge.b.y));
        line(tmp, A, B, Scalar(0), 1, LINE_8);
    }
    cv::imshow("detected lines", tmp);

    for (unsigned int i = 0; i < q; i++)
        quantizedEdges[i].clear();
    for (Edge<glm::vec2>& edge : edges)
        quantizedEdges[quantizedIndex(getOrientation(edge), q)].push_back(edge);

    for (unsigned int i = 0; i < q; i++) {
        tmp = Scalar::all(255);
        for (Edge<glm::vec2>& edge : quantizedEdges[i]) {
            const Point A((int)std::round(edge.a.x),
                          (int)std::round(edge.a.y)),
                        B((int)std::round(edge.b.x),
                          (int)std::round(edge.b.y));
            line(tmp, A, B, Scalar(0), 1, LINE_8);
        }
        distanceTransform(tmp, buffers[front][i], DIST_L2, DIST_MASK_PRECISE, CV_32F);
    }
    gaussianBlur();
    buffers[front][q / 2].convertTo(buffers[front][q / 2], CV_32F, 1.0 / 2055.0);
    cv::imshow("dist transform", buffers[front][q/2]);
    cv::waitKey(100000);

    //directedDistanceTransform();

    //TODO reintroduce blurring
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

float tr::DistanceTensor::getDist(const glm::vec2 uv, const float angle) const {
    cv::Point pixel((int)((uv.x * (float)width) + 0.5f),
        (int)((uv.y * (float)height) + 0.5f));
    if (pixel.x < 0 || pixel.x >= (int)width ||
        pixel.y < 0 || pixel.y >= (int)height) return maxCost;

    return buffers[front][quantizedIndex(angle, q)].at<float>(pixel);
}

//TODO merge blur with ddt?
void DistanceTensor::gaussianBlur() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    //TODO parallelization
    for (unsigned int x = 0; x < width; x++)
        for (unsigned int y = 0; y < height; y++) {
            const cv::Point pixel(x, y);
            for (unsigned int dir = 0; dir < q; dir++) {
                const unsigned int prev = (dir == 0) ? q-1 : dir - 1;
                const unsigned int next = (dir == q-1) ? 0 : dir + 1;
                //TODO localization
                buffers[!front][dir].at<float>(pixel) =
                    buffers[front][prev].at<float>(pixel) * 0.25f +
                    buffers[front][dir].at<float>(pixel) * 0.5f +
                    buffers[front][next].at<float>(pixel) * 0.25f;
            }
        }
    front = !front; //swap buffers
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void DistanceTensor::directedDistanceTransform() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static float lambda = stof(config.getEntry("lambda", "1.0"));
    const float dirCost = lambda*glm::pi<float>()/q;

    //TODO parallelization
    //TODO buffer swap
	for (unsigned int x = 0; x < width; x++)
	for (unsigned int y = 0; y < height; y++) {
        const cv::Point pixel(x,y);
		for (unsigned int i = 0; i < q; i++) {
			costs[i] = buffers[front][i].at<float>(pixel);
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
            buffers[front][i].at<float>(pixel) = costs[i];
	}
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}
