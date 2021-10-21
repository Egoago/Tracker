#pragma once
#include <vector>
#include "../Misc/Base.hpp"

namespace tr {
	template<typename T>
	class Loss {
	private:
		std::vector<T> errors;
	public:
		inline void addError(T error) { errors.push_back(error); }
		inline void reset(uint count = 0u) { errors.clear(); errors.reserve(count); }
		virtual Loss* clone() = 0;
		virtual T loss(const std::vector<T>& errors) = 0;
		inline T loss() { return loss(errors); }
	};
}