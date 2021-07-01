#pragma once
#include "Camera.h"

class uEyeCamera : public Camera
{
private:
	int id;
	unsigned long hCam = 0;
	char* pcImageMemory;
public:
	uEyeCamera();
	~uEyeCamera();
	virtual char* getNextFrame();
	virtual double getFPS() const;
};