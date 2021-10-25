#pragma once
#include "Estimator.hpp"

namespace tr {
	class DirectEstimator : public Estimator {
	public:
		DirectEstimator(const Tensor<Template>& templates) : Estimator(templates) {}
		virtual const std::vector<const Template*> estimate(const DistanceTensor& dcd3t) override;
	};
}

