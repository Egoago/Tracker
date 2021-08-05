#pragma once

namespace tr {
	class Loss {
	public:
		virtual float loss(const float loss) = 0;
		virtual float der(const float loss) = 0;
	};
}
