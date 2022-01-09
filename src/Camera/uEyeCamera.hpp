#pragma once
#include "Camera.hpp"

namespace tr
{
	class UEyeCamera : public Camera
	{
	private:
		int id;
		unsigned long hCam = 0;
		char* pcImageMemory;
	public:
		UEyeCamera();
		~UEyeCamera();
		virtual char* getNextFrameData() override;
		virtual double getFPS() const override;
		virtual int getFormat() const override;
	};
}