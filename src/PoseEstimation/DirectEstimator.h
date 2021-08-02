#pragma once
#include "Estimator.h"

namespace tr {
	class DirectEstimator : public Estimator {
	public:
		DirectEstimator(unsigned int candidateCount) : Estimator(candidateCount) {}
		virtual std::vector<Template*> estimate(const DistanceTensor& dcd3t) override;
	};
}

