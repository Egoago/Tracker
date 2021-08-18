#pragma once
#include "Estimator.hpp"
#include "../Misc/Base.hpp"

namespace tr {
	class DirectEstimator : public Estimator {
	public:
		DirectEstimator(const uint candidateCount, const Tensor<Template>& templates) :
			Estimator(candidateCount, templates) {}
		virtual const std::vector<const Template*> estimate(const DistanceTensor& dcd3t) override;
	};
}

