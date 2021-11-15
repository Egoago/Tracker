#pragma once
#include "Registrator.hpp"

namespace tr {
	class CeresRegistrator : public Registrator {
	public:
		CeresRegistrator(const mat4d& P);
		virtual const Registration registrate(const DistanceTensor& distanceTensor, const Template* candidate) const override;
		virtual Registrator* clone() const;
	};
}