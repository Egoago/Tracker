#pragma once
#include <vector>
#include <limits>
#include "../Math/Loss.hpp"
#include "../Math/Tensor.hpp"
#include "../Math/Template.hpp"
#include "../Misc/Base.hpp"
#include "DistanceTensor.hpp"

namespace tr {
	class Estimator {
	private:
		std::unique_ptr<Loss<double>> lossFunction;
	protected:
		const uint candidateCount;
		const Tensor<Template>& templates;
		double getDistance(const Template& temp, const DistanceTensor& dcd3t, double maxDistance = std::numeric_limits<double>::max());
	public:
		Estimator(const Tensor<Template>& templates);
		//TODO outCandidates Template pointer/ref
		virtual const std::vector<const Template*> estimate(const DistanceTensor& distanceTensor) = 0;
	};
}