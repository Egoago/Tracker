#pragma once

class Camera
{
protected:
	int width, height;
	int nBitsPerPixel;
public:
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	int getPixelSize() const { return nBitsPerPixel / 8; }
	virtual char* getNextFrame() = 0;
	virtual double getFPS() const = 0;
};

