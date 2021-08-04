#pragma once

namespace tr
{
	class Camera
	{
	protected:
		int width, height;
		int nBitsPerPixel;
	public:
		int getWidth() const { return width; }
		virtual ~Camera() {}
		int getHeight() const { return height; }
		int getPixelSize() const { return nBitsPerPixel / 8; }
		virtual char* getNextFrame() = 0;
		virtual double getFPS() const = 0;
		virtual int getFormat() const = 0;
	};
}

