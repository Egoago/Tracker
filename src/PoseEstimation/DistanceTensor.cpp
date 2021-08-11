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
#include "../Math/Interpolator.hpp"

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
    buffers.allocate({ 5, //2 swap buffers for dist value and 3 derivatives
                       q,
                       height,
                       width});
    front = true;
    quantizedEdges = new std::vector<Edge<glm::vec2>>[q];
    tmp = Mat((int)height, (int)width, CV_8U);
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

//TODO constexpr after log remove
inline float quantizedIndex(const float angle, const uint q) {
    static const constexpr float pi = glm::pi<float>();
#ifndef _DEBUG
    tr::Logger::warning("Remove boundary check from release");
#endif // !_DEBUG
    if (angle > pi || angle < 0.0f)
        tr::Logger::warning("angle out of bounds");
    return angle * (float)q / pi;
}

bool DistanceTensor::checkIndices(const std::vector<real>& indices) const {
#ifndef _DEBUG
    tr::Logger::warning("Remove boundary check from release");
#endif // !_DEBUG
    if (indices[0] < 0 || indices[0] >= width ||
        indices[1] < 0 || indices[1] >= height ||
        indices[2] < 0 || indices[2] >= q) {
        Logger::warning("Distance tensor out of bounds");
        return maxCost;
    }
    return true;
}

float DistanceTensor::at(const float indices[3], double partialDerivatives[3]) const {
    static Interpolator<float> interpolator(3);
    interpolator.interpolate({indices[0] * (width - 1.0f),      //u
                              indices[1] * (height - 1.0f),     //v
                              quantizedIndex(indices[2], q)});  //angle
    
    if (partialDerivatives != nullptr) {
        for(uint i = 0; i < 3; i++)
            partialDerivatives[i] =
                interpolator.execute<real>(
                    [&](const std::vector<real>& indices) {
                        checkIndices(indices);
                        return buffers.at({
                            2u+i,
                            (uint)indices[2] % q,
                            (uint)indices[1] % height,
                            (uint)indices[0] % width });});
    }
    return interpolator.execute<real>(
        [&](const std::vector<real>& indices) {
            checkIndices(indices);
            return buffers.at({ front,
                        (uint)indices[2] % q,
                        (uint)indices[1] % height,
                        (uint)indices[0] % width });});
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

    gaussianBlur();

    calculateDerivatives();
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
        cv::Mat storage(height, width, CV_32F, &buffers.at({ front, i, 0, 0 }));
        cv::distanceTransform(tmp, storage, DIST_L2, DIST_MASK_PRECISE, CV_32F);
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void DistanceTensor::directedDistanceTransform() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static float lambda = stof(config.getEntry("lambda", "100.0"));
    const static float dirCost = lambda*glm::pi<float>()/q;
    const static uint pixelCount = width * height;
    //TODO parallelization
    //TODO buffer swap
    float* costs = new float[q];
    float* const frameStart = &buffers.at({ front, 0, 0, 0 });
	for (uint pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
		for (uint i = 0; i < q; i++) {
			costs[i] = *(frameStart + (i * pixelCount + pixelIndex));
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
            *(frameStart + (i * pixelCount + pixelIndex)) = costs[i];
	}
    delete[] costs;
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void DistanceTensor::gaussianBlur() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static uint pixelCount = width * height;
    float* const frontStart = &buffers.at({ front, 0, 0, 0 });
    float* const backStart = &buffers.at({ !front, 0, 0, 0 });
    //TODO parallelization
    for (uint pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
            for (uint dir = 0; dir < q; dir++) {
                const uint prev = (dir == 0) ? q-1 : dir - 1;
                const uint next = (dir == q-1) ? 0 : dir + 1;
                //TODO localization
                *(backStart + (dir * pixelCount + pixelIndex)) =
                    0.25f * (*(frontStart + (prev * pixelCount + pixelIndex))) +
                    0.5f  * (*(frontStart + (dir  * pixelCount + pixelIndex))) +
                    0.25f * (*(frontStart + (next * pixelCount + pixelIndex)));
            }
        }
    front = !front; //swap buffers
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

//TODO merge ddt, blur, der? HINT: Same for structure
void DistanceTensor::calculateDerivatives() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static uint pixelCount = width * height;
    static float* const distStart = &buffers.at({ front,0,0,0 });
    static float* const xDerStart = &buffers.at({ 2,0,0,0 });
    static float* const yDerStart = xDerStart + pixelCount*q;
    static float* const angleDerStart = yDerStart + pixelCount*q;

    //x/y derivateives
    Logger::logProcess("x, y derivatives");   //TODO remove logging
    for (uint i = 0u; i < q; i++) { //TODO parallelization
        const uint offset = i * pixelCount;
        cv::Mat distWrapper(height, width, CV_32F, distStart + offset);
        cv::Mat xWrapper(height, width, CV_32F, xDerStart + offset);
        cv::Mat yWrapper(height, width, CV_32F, yDerStart + offset);
        cv::Sobel(distWrapper, xWrapper, CV_32F, 1, 0, -1);
        cv::Sobel(distWrapper, yWrapper, CV_32F, 0, 1, -1);
    }
    Logger::logProcess("x, y derivatives");   //TODO remove logging
    //angle derivatives
    Logger::logProcess("angle derivatives");   //TODO remove logging
    uint angle = 0, prevAngle = q - 1, nextAngle = 1;
    while ( angle < q ) {
        for (uint pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {  //TODO parallelization
            *(angleDerStart + (angle * pixelCount + pixelIndex)) = 0.5f *
                (*(distStart + (nextAngle * pixelCount + pixelIndex)) -
                 *(distStart + (prevAngle * pixelCount + pixelIndex)));
        }
        angle++;
        prevAngle = angle - 1;
        nextAngle = (angle + 1) % q;
    }
    Logger::logProcess("angle derivatives");   //TODO remove logging
    //TODO remove logging
    /*for (uint i = 0u; i < q; i++) {
        const uint offset = i * pixelCount;
        cv::Mat distWrapper(height, width, CV_32F, distStart + offset);
        cv::Mat xWrapper(height, width, CV_32F, xDerStart + offset);
        cv::Mat yWrapper(height, width, CV_32F, yDerStart + offset);
        cv::Mat angleWrapper(height, width, CV_32F, angleDerStart + offset);
        Mat tmp;
        distWrapper.convertTo(tmp, CV_32F, 2.0f / maxCost);
        cv::imshow("dist", tmp);
        xWrapper.convertTo(tmp, CV_32F, 2.0f / maxCost);
        cv::imshow("x", tmp);
        yWrapper.convertTo(tmp, CV_32F, 2.0f / maxCost);
        cv::imshow("y", tmp);
        angleWrapper.convertTo(tmp, CV_32F, 10.0f / maxCost);
        cv::imshow("angle", tmp);
        cv::waitKey(50000000);
    
    }*/
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}
