#pragma once
#include <vector>
#include "../Math/Tensor.hpp"
#include "../Math/Template.hpp"
#include "../Misc/Base.hpp"
#include "DistanceTensor.hpp"

namespace tr {
	class Estimator {
	protected:
		const uint candidateCount;
		Tensor<Template>& templates;
	public:
		Estimator(const uint candidateCount, Tensor<Template>& templates) :
			candidateCount(candidateCount), templates(templates) {}
		//TODO outCandidates Template pointer/ref
		virtual std::vector<Template*> estimate(const DistanceTensor& distanceTensor) = 0;
	};
}