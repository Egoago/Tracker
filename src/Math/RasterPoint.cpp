#include "RasterPoint.hpp"
#include <glm/ext/vector_float4.hpp>
#include "Rotation.hpp"

using namespace tr;

bool renderPoint(const glm::vec3 p, const glm::mat4& mvp, glm::vec2& uv) {
	glm::vec4 pPos = mvp * glm::vec4(p, 1.0f);	//model -> NDC
	pPos /= pPos.w;								//Perspective divide
	pPos = (pPos + 1.0f) / 2.0f;				//NDC -> screen
	if (pPos.x < 0.0f || pPos.x >= 1.0f ||		//clip
		pPos.y < 0.0f || pPos.y >= 1.0f) {
		return false;
	}
	uv.x = pPos.x;
	uv.y = pPos.y;
	return true;
}

RasterPoint::RasterPoint() : 
	RasterPoint(glm::vec3(0), glm::vec3(0)) {}

#pragma warning(suppress : 26495)
RasterPoint::RasterPoint(const glm::vec3 pos, const glm::vec3 offsetPos) :
	pos(pos), offsetPos(offsetPos), uv(0), angle(0) {
}

bool tr::RasterPoint::render(const glm::mat4& mvp) {
	glm::vec2 x, ox;
	if (!renderPoint(pos, mvp, x)) return false;
	if (!renderPoint(offsetPos, mvp, ox)) return false;
	const float _angle = getOrientation(x - ox);
	if (isnan(_angle)) return false;
	uv = x;
	angle = _angle;
	return true;
}
