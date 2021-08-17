#pragma once
#include "Registrator.hpp"

namespace tr {
	class CeresRegistrator : public Registrator {
	public:
		CeresRegistrator(const mat4d& P);
		virtual SixDOF registrate(const DistanceTensor& distanceTensor, const Template* candidate) override;
	};
}