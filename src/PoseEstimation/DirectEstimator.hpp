#pragma once
#include "Estimator.hpp"
#include "../Misc/Base.hpp"

namespace tr {
	class DirectEstimator : public Estimator {
	public:
		DirectEstimator(const uint candidateCount, Tensor<Template>& templates) :
			Estimator(candidateCount, templates) {}
		virtual std::vector<Template*> estimate(const DistanceTensor& dcd3t) override;
	};
}

