#include "SixDOF.hpp"
#include <glm/ext/matrix_transform.hpp>

using namespace tr;

SixDOF::SixDOF(glm::vec<3, real> position,
	glm::vec<3, real> orientation) : position(position), orientation(orientation) {}

glm::mat4 SixDOF::getModelTransformMatrix() const {
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::translate(M, glm::vec3(position.x, position.y, position.z));
	M = glm::rotate(M, orientation.p, glm::vec3(1.0f, 0.0f, 0.0f));
	M = glm::rotate(M, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::rotate(M, orientation.r, glm::vec3(0.0f, 0.0f, 1.0f));
	return M;
}

std::ostream& tr::operator<<(std::ostream& ost, const SixDOF& sDOF) {
	ost << "pos: "
		<< sDOF.position.x << ' '
		<< sDOF.position.y << ' '
		<< sDOF.position.z << ' '
		<< "ori: "
		<< sDOF.orientation.y << ' '
		<< sDOF.orientation.p << ' '
		<< sDOF.orientation.r << std::endl;
	return ost;
}
