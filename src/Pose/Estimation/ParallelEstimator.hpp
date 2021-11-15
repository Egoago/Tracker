#pragma once
#include "Estimator.hpp"
#include <vector>
#include <omp.h>

namespace tr {
	class ParallelEstimator : public Estimator {
	public:
		ParallelEstimator(const Tensor<Template>& templates) : Estimator(templates) { }
		virtual const std::vector<const Template*> estimate(const DistanceTensor& dcd3t) override;
	};
}

