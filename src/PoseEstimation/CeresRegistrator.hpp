#pragma once
#include "Registrator.hpp"

namespace tr {
	class CeresRegistrator : public Registrator {
	public:
		CeresRegistrator(const emat4& P) : Registrator(P) {}
		virtual SixDOF registrate(const DistanceTensor& distanceTensor, const Template* candidate) override;
	};
}

