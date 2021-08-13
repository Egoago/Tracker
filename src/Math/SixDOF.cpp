#include "SixDOF.hpp"
#include <glm/ext/matrix_transform.hpp>

using namespace tr;

SixDOF::SixDOF(glm::vec3 position,
	glm::vec3 orientation) : position(position), orientation(orientation) {}

glm::mat4 SixDOF::getModelTransformMatrix() const {
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::translate(M, glm::vec3(position.x, position.y, position.z));
	M = glm::rotate(M, orientation.p, glm::vec3(1.0f, 0.0f, 0.0f));
	M = glm::rotate(M, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	M = glm::rotate(M, orientation.r, glm::vec3(0.0f, 0.0f, 1.0f));
	return M;
}

std::ostream& tr::operator<<(std::ostream& ost, const SixDOF& sDOF) {
	ost << "pos:\t"
		<< sDOF.position.x << '\t'
		<< sDOF.position.y << '\t'
		<< sDOF.position.z << '\t'
		<< "ori:\t"
		<< sDOF.orientation.y << '\t'
		<< sDOF.orientation.p << '\t'
		<< sDOF.orientation.r << std::endl;
	return ost;
}
