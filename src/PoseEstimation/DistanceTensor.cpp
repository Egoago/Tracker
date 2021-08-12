#include "DistanceTensor.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>
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
    maxCost(std::sqrt((real)width*width+height*height)) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    buffers.allocate({ 2, //2 swap buffers
                       q,
                       height,
                       width});
    front = true;
    quantizedEdges = new std::vector<Edge<glm::vec2>>[q];
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

//TODO constexpr after log remove
inline tr::real quantizedIndex(const tr::real angle, const uint q) {
    static const constexpr tr::real pi = glm::pi<tr::real>();
#ifndef _DEBUG
    tr::Logger::warning("Remove boundary check from release");
#endif // !_DEBUG
    if (angle > pi || angle < 0.0f)
        tr::Logger::warning("angle out of bounds");
    return angle * (tr::real)q / pi;
}

bool DistanceTensor::checkIndices(const uint indices[3]) const {
#ifndef _DEBUG
    tr::Logger::warning("Remove boundary check from release");
#endif // !_DEBUG
    if (indices[0] < 0 || indices[0] >= width ||
        indices[1] < 0 || indices[1] >= height ||
        indices[2] < 0 || indices[2] > q) {
        Logger::warning("Distance tensor out of bounds");
        return false;
    }
    return true;
}

tr::real DistanceTensor::interpolate(const std::initializer_list<real>& indices) const {
    uint interpolationOmitted = 0u;
    uint i = 0u;
    uint low[3]{0}, high[3]{0};
    real t[3]{0}, _t[3]{0};
    for (const real index : indices) {
        low[i] = uint(floor(index));
        high[i] = uint(ceil(index));
        interpolationOmitted |= (low[i] == high[i]) << i;
        t[i] = index - low[i];
        _t[i] = real(1) - t[i];
        i++;
    }
    real value = real(0);
    uint interpolatedIndices[3] = { 0u };
    for (uint i = 0u; i < 8u; i++) {
        if (i & interpolationOmitted) break;
        real weight = real(1);
        for (uint dim = 0u; dim < 3u; dim++) {
            const bool useHigher = i & (1u << dim);
            weight *= (useHigher) ? t[dim] : _t[dim];
            interpolatedIndices[dim] = (useHigher) ? high[dim] : low[dim];
        }
        value += weight * at(interpolatedIndices);
    }
    return value;
}

tr::real tr::DistanceTensor::at(const uint indices[3]) const {
#ifndef DEBUG
    Logger::warning("Remove DistanceTensor at from release");
#endif // !DEBUG
    return buffers.at({ front,
                        indices[2] % q,
                        indices[1] % height,
                        indices[0] % width});
}

tr::real DistanceTensor::Evaluate(const real coordinates[3], real partialDerivatives[3]) const {
    const real indices[3] = { coordinates[0] * (width - real(1)),  //u
                              coordinates[1] * (height - real(1)), //v
                              quantizedIndex(coordinates[2], q)};  //angle

    if (partialDerivatives != nullptr) {
        static const real numDiffStep = real(stod(config.getEntry("numeric diff step size", "1e-3")));
        static const real invDiffStep = real(1) / (real(2) * numDiffStep);
        partialDerivatives[0] = invDiffStep *
            interpolate({ indices[0] + numDiffStep, indices[1], indices[2] }) -
            interpolate({ indices[0] - numDiffStep, indices[1], indices[2] });
        partialDerivatives[1] = invDiffStep *
            interpolate({ indices[0], indices[1] + numDiffStep, indices[2] }) -
            interpolate({ indices[0], indices[1] - numDiffStep, indices[2] });
        partialDerivatives[2] = invDiffStep *
            interpolate({ indices[0], indices[1], indices[2] + numDiffStep }) -
            interpolate({ indices[0], indices[1], indices[2] - numDiffStep });
    }

    return interpolate({ indices[0], indices[1], indices[2] });
}

void DistanceTensor::setFrame(const cv::Mat& nextFrame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    vector<Edge<glm::vec2>> edges;
    Mat frame = nextFrame;
    if (nextFrame.type() != CV_8U) {
        cvtColor(nextFrame, frame, COLOR_BGR2GRAY);
    }
    edgeDetector->detectEdges(frame, edges);
    //TODO remove logging
    std::vector<cv::Mat> copies{ frame,frame,frame };
    cv::merge(copies, frame);
    for (const auto& edge : edges) {
        Point A((int)edge.a.x, (int)edge.a.y),
            B((int)edge.b.x, (int)edge.b.y);
        line(nextFrame, A, B, Scalar(255, 0, 0), 1, LINE_8);
    }

    distanceTransformFromEdges(edges);
    
    directedDistanceTransform();

    gaussianBlur();

    //calculateDerivatives();
    //TODO remove logging
    //while(1)
    for (uint i = 0; i < q; i++) {
        cv::Mat tmp = Mat((int)height, (int)width, CV_8U, cv::Scalar(255));
        for (Edge<glm::vec2>& edge : quantizedEdges[i]) {
            const Point A((int)std::round(edge.a.x),
                (int)std::round(edge.a.y)),
                B((int)std::round(edge.b.x),
                    (int)std::round(edge.b.y));
            line(tmp, A, B, Scalar(0), 1, LINE_8);
        }
        cv::Mat wrapper((int)height, (int)width, realCV, &buffers.at({front, i, 0, 0}));
        cv::imshow("detected lines", tmp);
        cv::imshow("dist transform", wrapper);
        cv::waitKey(10000000);
    }

    //TODO reintroduce blurring
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void tr::DistanceTensor::distanceTransformFromEdges(const std::vector<Edge<glm::vec2>>& edges) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    for (uint i = 0; i < q; i++)
        quantizedEdges[i].clear();
    for (const Edge<glm::vec2>& edge : edges)
        quantizedEdges[(uint)quantizedIndex(edge.orientation(), q)].push_back(edge);
    cv::Mat tmp = Mat((int)height, (int)width, CV_8U);
    for (uint i = 0; i < q; i++) {
        tmp = Scalar::all(255);
        for (Edge<glm::vec2>& edge : quantizedEdges[i])
            line(tmp,
                Point((int)std::round(edge.a.x),
                      (int)std::round(edge.a.y)),
                Point((int)std::round(edge.b.x),
                      (int)std::round(edge.b.y)),
                Scalar(0), 1, LINE_8);
        cv::Mat storage(height, width, realCV, &buffers.at({ front, i, 0, 0 }));
        cv::distanceTransform(tmp, storage, DIST_L2, DIST_MASK_PRECISE, realCV);
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void DistanceTensor::directedDistanceTransform() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static real lambda = real(stod(config.getEntry("lambda", "100.0")));
    const static real dirCost = lambda*glm::pi<real>()/q;
    const static uint pixelCount = width * height;
    //TODO parallelization
    //TODO buffer swap
    real* costs = new real[q];
    real* const frameStart = &buffers.at({ front, 0, 0, 0 });
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

//TODO merge ddt, blur? HINT: Same for structure
void DistanceTensor::gaussianBlur() {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    const static uint pixelCount = width * height;
    real* const frontStart = &buffers.at({ front, 0, 0, 0 });
    real* const backStart = &buffers.at({ !front, 0, 0, 0 });
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

//TODO remove derivative precomputing
//void DistanceTensor::calculateDerivatives() {
//    //Logger::logProcess(__FUNCTION__);   //TODO remove logging
//    //Logger::logProcess("init");   //TODO remove logging
//    //const static uint pixelCount = width * height;
//    //static float* const distStart = &buffers.at({ front,0,0,0 });
//    //static float* const xDerStart = &buffers.at({ 2,0,0,0 });
//    //static float* const yDerStart = xDerStart + pixelCount*q;
//    //static float* const angleDerStart = yDerStart + pixelCount*q;
//    //Logger::logProcess("init");   //TODO remove logging
//
//    ////x/y derivateives
//    //Logger::logProcess("x, y derivatives");   //TODO remove logging
//    //for (uint i = 0u; i < q; i++) { //TODO parallelization
//    //    const uint offset = i * pixelCount;
//    //    cv::Mat distWrapper(height, width, CV_32F, distStart + offset);
//    //    cv::Mat xWrapper(height, width, CV_32F, xDerStart + offset);
//    //    cv::Mat yWrapper(height, width, CV_32F, yDerStart + offset);
//    //    cv::Sobel(distWrapper, xWrapper, CV_32F, 1, 0, -1);
//    //    cv::Sobel(distWrapper, yWrapper, CV_32F, 0, 1, -1);
//    //}
//    //Logger::logProcess("x, y derivatives");   //TODO remove logging
//    ////angle derivatives
//    //Logger::logProcess("angle derivatives");   //TODO remove logging
//    //uint angle = 0, prevAngle = q - 1, nextAngle = 1;
//    //while ( angle < q ) {
//    //    for (uint pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {  //TODO parallelization
//    //        *(angleDerStart + (angle * pixelCount + pixelIndex)) = 0.5f *
//    //            (*(distStart + (nextAngle * pixelCount + pixelIndex)) -
//    //             *(distStart + (prevAngle * pixelCount + pixelIndex)));
//    //    }
//    //    angle++;
//    //    prevAngle = angle - 1;
//    //    nextAngle = (angle + 1) % q;
//    //}
//    //Logger::logProcess("angle derivatives");   //TODO remove logging
//    ////TODO remove logging
//    ///*for (uint i = 0u; i < q; i++) {
//    //    const uint offset = i * pixelCount;
//    //    cv::Mat distWrapper(height, width, CV_32F, distStart + offset);
//    //    cv::Mat xWrapper(height, width, CV_32F, xDerStart + offset);
//    //    cv::Mat yWrapper(height, width, CV_32F, yDerStart + offset);
//    //    cv::Mat angleWrapper(height, width, CV_32F, angleDerStart + offset);
//    //    Mat tmp;
//    //    distWrapper.convertTo(tmp, CV_32F, 2.0f / maxCost);
//    //    cv::imshow("dist", tmp);
//    //    xWrapper.convertTo(tmp, CV_32F, 2.0f / maxCost);
//    //    cv::imshow("x", tmp);
//    //    yWrapper.convertTo(tmp, CV_32F, 2.0f / maxCost);
//    //    cv::imshow("y", tmp);
//    //    angleWrapper.convertTo(tmp, CV_32F, 10.0f / maxCost);
//    //    cv::imshow("angle", tmp);
//    //    cv::waitKey(50000000);
//    //
//    //}*/
//    //Logger::logProcess(__FUNCTION__);   //TODO remove logging
//}
