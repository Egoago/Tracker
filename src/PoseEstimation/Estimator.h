#pragma once
#include "../Math/Tensor.h"
#include "../Coordinates.h"
#include <vector>
#include "../DistanceTensor.h"

namespace tr {
	class Estimator
	{
	protected:
		const unsigned int candidateCount;
		Tensor<Template>& templates;
	public:
		Estimator(const unsigned int candidateCount, Tensor<Template>& templates) :
			candidateCount(candidateCount), templates(templates) {}
		//TODO outCandidates Template pointer/ref
		virtual std::vector<Template*> estimate(const DistanceTensor& dcd3t) = 0;
	};
}