#include "Template.hpp"
#include "../Misc/Log.hpp"

using namespace tr;

std::ostream& tr::operator<<(std::ostream& ost, const Template& tmp) {
	ost << "points: " << tmp.rasterPoints.size() << std::endl
		<< "6DOF: " << tmp.sixDOF;
	return ost;
}

std::ostream& tr::operator<<(std::ostream& out, Bits<Template&> o) {
	out << bits(o.t.sixDOF)
		<< bits(o.t.rasterPoints);
	return out;
}

std::istream& tr::operator>>(std::istream& ins, Bits<Template&> o) {
	ins >> bits(o.t.sixDOF)
		>> bits(o.t.rasterPoints);
	return ins;
}