#pragma once
#include "Estimator.hpp"
#include <vector>

namespace tr {
	class SmartEstimator : public Estimator {
	private:
		uint chunkSize;
	public:
		SmartEstimator(const Tensor<Template>& templates);
		virtual const std::vector<const Template*> estimate(const DistanceTensor& dcd3t) override;
	};
}
