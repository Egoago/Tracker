#pragma once
#include <initializer_list>
#include <vector>
#include <functional>	

namespace tr {
	template<typename IndexType = float>
	class Interpolator {
		IndexType *low, *high;
		double *t, *_t;
		char *buffer;
		unsigned int interpolationOmitted;	//bitmask
		unsigned int maxEvaluations, iDim;
	public:
		Interpolator(const unsigned int iDim);
		inline ~Interpolator() { delete[] buffer;  };
		void interpolate(std::initializer_list<IndexType> indices);

		template<typename ResultType, typename Function>
		ResultType execute(const Function& lambda);
	};

	template<typename IndexType>
	inline Interpolator<IndexType>::Interpolator(const unsigned int iDim) :
		iDim(iDim), maxEvaluations(0u), interpolationOmitted(0u) {
		buffer = new char[2u * iDim * sizeof(IndexType) + 2u * iDim * sizeof(double)];
		low = (IndexType*)buffer;
		high = &low[iDim];
		t = (double*)(buffer + 2u * iDim * sizeof(IndexType));
		_t = &t[iDim];
	}

	template<typename IndexType>
	inline void Interpolator<IndexType>::interpolate(std::initializer_list<IndexType> indices) {
		maxEvaluations = 1u;
		interpolationOmitted = 0u;
		unsigned int i = 0u;
		for (const IndexType index : indices) {
			maxEvaluations *= 2u;
			low[i] = floor(index);
			high[i] = ceil(index);
			interpolationOmitted |= (low[i] == high[i]) << i;
			t[i] = index - low[i];
			_t[i] = 1.0 - t[i];
			i++;
		}
	}
	template<typename IndexType>
	template<typename ResultType, typename Function>
	inline ResultType Interpolator<IndexType>::execute(const Function& lambda) {
		ResultType value = ResultType(0);
		std::vector<IndexType> index(iDim);
		int c = 0;
		for (unsigned int i = 0; i < maxEvaluations; i++) {
			if (i & interpolationOmitted) break;
			c++;
			double weight = 1.0;
			for (unsigned int dim = 0; dim < iDim; dim++) {
				const bool useHigher = i & (1u << dim);
				weight *= (useHigher) ? t[dim] : _t[dim];
				index[dim] = (useHigher) ? high[dim] : low[dim];
			}
			value += lambda(index) * (ResultType)weight;
		}
		return value;
	}
}

