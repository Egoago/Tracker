#include "SixDOF.hpp"
#include "EigenTransform.hpp"

using namespace tr;

SixDOF::SixDOF(vec3f position,
	vec3f orientation) : position(position), orientation(orientation) {}

mat4f SixDOF::getModelTransformMatrix() const {
	mat4f M = mat4f::Identity();
	M *= tr::translate(position);
	M *= tr::rotate(orientation);
	return M;
}

std::ostream& tr::operator<<(std::ostream& ost, const SixDOF& sDOF) {
	ost << "pos:\t"
		<< sDOF.position.x() << '\t'
		<< sDOF.position.y() << '\t'
		<< sDOF.position.z() << '\t'
		<< "ori:\t"
		<< sDOF.orientation.x() << '\t'
		<< sDOF.orientation.y() << '\t'
		<< sDOF.orientation.z() << std::endl;
	return ost;
}
