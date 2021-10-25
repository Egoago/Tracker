#include "DistanceTensor.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>
#include "../Misc/Constants.hpp"
#include "../Misc/Log.hpp"
#include "../Misc/ConfigParser.hpp"
#include "../Detectors/LSDDetector.hpp"
#include "../Detectors/CannyDetector.hpp"
#include <opencv2/highgui.hpp>  //TODO remove

using namespace std;
using namespace cv;
using namespace tr;

EdgeDetector* getEdgeDetector() {
    switch (strHash(ConfigParser::instance().getEntry<std::string>(CONFIG_SECTION_DCD3T, "edge detector", "lsd").c_str())) {
    case strHash("canny"): return new CannyDetector();
    case strHash("lsd"):
    default: return new LSDDetector();
    }
}

DistanceTensor::DistanceTensor(const float aspect) :
    height(uint(ConfigParser::instance().getEntry(CONFIG_SECTION_DCD3T, "resolution", 1024))),
    width(uint(height * aspect)),
    q(ConfigParser::instance().getEntry(CONFIG_SECTION_DCD3T, "orientation quantization", 60)),
    edgeDetector(getEdgeDetector()),
    maxCost(sqrtf((float)width * width + height * height)) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    buffers.allocate({ 2, //2 swap buffers
                       q,
                       height,
                       width}, maxCost);
    front = true;
    quantizedEdges = new std::vector<Edge<vec2f>>[q];
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

//TODO constexpr after log remove
inline double quantizedIndex(const double angle, const uint q) {
    static const constexpr double pi = double(EIGEN_PI);
#ifndef _DEBUG
    tr::Logger::warning("Remove boundary check from release");
#endif // !_DEBUG
    if (angle > pi || angle < 0.0f)
        tr::Logger::warning("angle out of bounds");
    return angle * (double)q / pi;
}

double DistanceTensor::round(const std::initializer_list<double>& indices) const {
    uint roundedIndices[3] = { 0u };
    uint i = 0;
    for (const double index : indices) {
        roundedIndices[i] = uint(std::round(index));
        i++;
    }
    return at(roundedIndices);
}

double DistanceTensor::interpolate(const std::initializer_list<double>& indices) const {
    uint interpolationOmitted = 0u;
    uint i = 0u;
    uint low[3]{0};
    double t[3]{0}, _t[3]{0};
    for (const double index : indices) {
        low[i] = uint(floor(index));
        interpolationOmitted |= (low[i] == uint(ceil(index))) << i;
        t[i] = index - low[i];
        _t[i] = 1.0 - t[i];
        i++;
    }
    double value = 0.0;
    uint interpolatedIndices[3] = { 0u };
    for (uint i = 0u; i < 8u; i++) {
        if (i & interpolationOmitted) break;
        double weight = 1.0;
        for (uint dim = 0u; dim < 3u; dim++) {
            const bool useHigher = i & (1u << dim);
            weight *= (useHigher) ? t[dim] : _t[dim];
            interpolatedIndices[dim] = low[dim];
            if (useHigher) interpolatedIndices[dim]++;
        }
        value += weight * at(interpolatedIndices);
    }
    return value;
}

double DistanceTensor::sample(const std::initializer_list<double>& indices) const {
    const static bool useInterpolation = ConfigParser::instance().getEntry(CONFIG_SECTION_DCD3T, "use interpolation", true);
    return (useInterpolation) ? interpolate(indices) : round(indices);
}

double tr::DistanceTensor::at(const uint indices[3]) const {
#ifndef _DEBUG
    Logger::warning("Remove DistanceTensor at from release");
#endif // !_DEBUG
    return double(buffers.at({ front,
                        indices[2] % q,
                        indices[1] % height,
                        indices[0] % width}));
}

double DistanceTensor::evaluate(const double coordinates[3], double partialDerivatives[3]) const {
    const double indices[3] = { coordinates[0] * (width - double(1)),  //u
                              coordinates[1] * (height - double(1)), //v
                              quantizedIndex(coordinates[2], q)};  //angle
    //NOTE numeric differentiation is best executed with ceres
    if (partialDerivatives != nullptr) {
        static const double numDiffStep = double(ConfigParser::instance().getEntry(CONFIG_SECTION_DCD3T, "numeric diff step size", 1e-5));
        static const double invDiffStep = double(1) / (double(2) * numDiffStep);
        partialDerivatives[0] = invDiffStep *
            (sample({ indices[0] + numDiffStep, indices[1], indices[2] }) -
             sample({ indices[0] - numDiffStep, indices[1], indices[2] }));
        partialDerivatives[1] = invDiffStep *
            (sample({ indices[0], indices[1] + numDiffStep, indices[2] }) -
             sample({ indices[0], indices[1] - numDiffStep, indices[2] }));
        partialDerivatives[2] = invDiffStep *
            (sample({ indices[0], indices[1], indices[2] + numDiffStep }) -
             sample({ indices[0], indices[1], indices[2] - numDiffStep }));
    }

    return sample({ indices[0], indices[1], indices[2] });
}

void DistanceTensor::setFrame(const cv::Mat& nextFrame) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    vector<Edge<vec2f>> edges;
    Mat frame = nextFrame;
    if (nextFrame.type() != CV_8U) {
        cvtColor(nextFrame, frame, COLOR_BGR2GRAY);
    }
    edgeDetector->detectEdges(frame, edges);
    //TODO remove logging
    std::vector<cv::Mat> copies{ frame,frame,frame };
    cv::merge(copies, nextFrame);
    for (const auto& edge : edges) {
        line(nextFrame,
            Point((int)std::round(edge.a.x() * nextFrame.cols),
                  (int)std::round(edge.a.y() * nextFrame.rows)),
            Point((int)std::round(edge.b.x() * nextFrame.cols),
                  (int)std::round(edge.b.y() * nextFrame.rows)),
            Scalar(255, 0, 0), 3, LINE_8);
    }

    distanceTransformFromEdges(edges);
    //Logger::drawFrame(height, width, &buffers.at({ front, 0, 0, 0 }), CV_32F, "trans");   //TODO remove logging
    directedDistanceTransform();
    //Logger::drawFrame(height, width, &buffers.at({ front, 0, 0, 0 }), CV_32F, "dist");    //TODO remove logging
    const static uint blurCount = uint(ConfigParser::instance().getEntry(CONFIG_SECTION_DCD3T, "blurring steps", 1));
    for (uint i = 0; i < blurCount; i++) {
        /*cv::Mat slice(height, q, CV_32F);
        cv::namedWindow("blur", cv::WINDOW_NORMAL);
        cv::resizeWindow("blur", cv::Size(slice.cols * 8, slice.rows / 4));
        for (uint x = 0; x < q; x++)
            for (uint y = 0; y < height; y++)
                slice.at<float>(y, x) = buffers.at({ front, x, y, 0 });
        cv::normalize(slice, slice, 0, 1, cv::NORM_MINMAX);*/
        gaussianBlur();
        /*cv::imshow("blur", slice);
        cv::waitKey(1);*/
    }
    //Logger::drawFrame(height, width, &buffers.at({ front, 0, 0, 0 }), CV_32F, "blur");    //TODO remove logging

    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void tr::DistanceTensor::distanceTransformFromEdges(const std::vector<Edge<vec2f>>& edges) {
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
    for (uint i = 0; i < q; i++)
        quantizedEdges[i].clear();
    for (const Edge<vec2f>& edge : edges)
        quantizedEdges[(uint)quantizedIndex(edge.orientation(), q)].push_back(edge);
    cv::Mat tmp = Mat((int)height, (int)width, CV_8U);
    for (uint i = 0; i < q; i++) {
        tmp = Scalar::all(255);
        for (const Edge<vec2f>& edge : quantizedEdges[i])
            line(tmp,
                Point((int)std::round(edge.a.x() * width),
                      (int)std::round(edge.a.y() * height)),
                Point((int)std::round(edge.b.x() * width),
                      (int)std::round(edge.b.y() * height)),
                Scalar(0), 1, LINE_8);
        //Logger::drawFrame(&tmp, "edge");    //TODO remove logging
        cv::Mat storage(height, width, CV_32F, &buffers.at({ front, i, 0, 0 }));
        if (quantizedEdges[i].size() > 0)
            cv::distanceTransform(tmp, storage, DIST_L2, DIST_MASK_PRECISE, CV_32F);
    }
    Logger::logProcess(__FUNCTION__);   //TODO remove logging
}

void DistanceTensor::directedDistanceTransform() {
    Logger::logProcess(__FUNCTION__);   //TODO remove loggingy
    const static float lambda = ConfigParser::instance().getEntry(CONFIG_SECTION_DCD3T, "lambda", 100.0f);
    const static float dirCost = lambda * float(EIGEN_PI)/q;
    const static ulong pixelCount = width * height;
    //TODO parallelization
    //TODO buffer swap
    float* costs = new float[q];
    float* const frameStart = &buffers.at({ front, 0, 0, 0 });
	for (ulong pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
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
    float* const frontStart = &buffers.at({ front, 0, 0, 0 });
    float* const backStart = &buffers.at({ !front, 0, 0, 0 });
    //TODO parallelization
    for (uint dir = 0; dir < q; dir++) {
        const uint prev = (dir == 0) ? q-1 : dir - 1;
        const uint next = (dir == q-1) ? 0 : dir + 1;
        for (uint pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
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
