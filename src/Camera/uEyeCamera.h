#pragma once
#include "Camera.h"

class UEyeCamera : public Camera
{
private:
	int id;
	unsigned long hCam = 0;
	char* pcImageMemory;
public:
	UEyeCamera();
	~UEyeCamera();
	virtual char* getNextFrame();
	virtual double getFPS() const;
	virtual int getFormat() const;
};