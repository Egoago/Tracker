#pragma once
#include "Estimator.hpp"

namespace tr {
	class DirectEstimator : public Estimator {
	public:
		DirectEstimator(const unsigned int candidateCount, Tensor<Template>& templates) :
			Estimator(candidateCount, templates) {}
		virtual std::vector<Template*> estimate(const DistanceTensor& dcd3t) override;
	};
}

