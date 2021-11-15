#include "Estimator.hpp"
#include "../../Math/Mean.hpp"
#include "../../Misc/ConfigParser.hpp"
#include "../../Misc/Constants.hpp"
#include <thread>

using namespace tr;

Loss<double>* getLossFunction() {
	switch (strHash(ConfigParser::instance().getEntry<std::string>(CONFIG_SECTION_ESTIMATION, "loss function", "mse").c_str())) {
	case strHash("huber"): return new Huber<double>(ConfigParser::instance().getEntry(CONFIG_SECTION_ESTIMATION, "huber cutoff", 1.0));
	case strHash("mae"): return new MAE<double>();
	case strHash("mse"): return new MSE<double>();
	default: 
		Logger::warning("Not implemented loss function");
		return new MSE<double>();
	}
}

double tr::Estimator::getDistance(const Template& temp, const DistanceTensor& dcd3t, double maxDistance) {
	const uint pixelCount = (uint)temp.rasterPoints.size();
	if (pixelCount < 20) {
		Logger::warning("Too few pixels at estimation (" + std::to_string(pixelCount) + ")");
		return std::numeric_limits<double>::max();
	}
	std::unique_ptr<Loss<double>> loss(lossFunction->clone());	// for parallelization
	loss->reset(pixelCount);
	for (uint i = 0; i < pixelCount; i++) {
		const double indices[3] = {
			double(temp.rasterPoints[i].indexData[0]),
			double(temp.rasterPoints[i].indexData[1]),
			double(temp.rasterPoints[i].indexData[2]) };
		const double error = dcd3t.evaluate(indices);
		loss->addError(error);
	}
	double distance = loss->loss();
	//distance *= -temp.sixDOF.position.z();
	return distance;
}

tr::Estimator::Estimator(const Tensor<Template>& templates) :
	templates(templates),
	lossFunction(getLossFunction()),
	candidateCount(ConfigParser::instance().getEntry(CONFIG_SECTION_ESTIMATION,
													 "candidate count",
													 std::thread::hardware_concurrency())) {}
