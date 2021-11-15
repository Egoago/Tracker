#include "uEyeCamera.hpp"
#include <uEye.h>
#include <iostream>
#include "../Misc/Log.hpp"

using namespace tr;

int setColorMode(HIDS hCam, char colorMode) {
    int nColorMode;
    int nBitsPerPixel;
    if (colorMode == IS_COLORMODE_BAYER) {
        // for color camera models use RGB24 mode
        nColorMode = IS_CM_BGR8_PACKED;
        nBitsPerPixel = 24;
    }
    else if (colorMode == IS_COLORMODE_CBYCRY) {
        // for CBYCRY camera models use RGB32 mode
        nColorMode = IS_CM_BGRA8_PACKED;
        nBitsPerPixel = 32;
    }
    else {
        // for monochrome camera models use Y8 mode
        nColorMode = IS_CM_MONO8;
        nBitsPerPixel = 8;
    }
    // Sets the color mode to be used when image data are saved or displayed by the graphics card
    is_SetColorMode(hCam, nColorMode);
    return nBitsPerPixel;
}

UEyeCamera::UEyeCamera() {
    if (is_InitCamera(&hCam, NULL) != IS_SUCCESS)
        Logger::error("Camera not found");
    if (is_SetExternalTrigger(hCam, IS_SET_TRIGGER_OFF) != IS_SUCCESS)
        Logger::error("Could not disable external trigger mode on uEye camera");
    SENSORINFO sInfo;
    if (is_GetSensorInfo(hCam, &sInfo) != IS_SUCCESS)
        Logger::error("Could not read sensor info");
    width = sInfo.nMaxWidth;
    height = sInfo.nMaxHeight;
    nBitsPerPixel = setColorMode(hCam, sInfo.nColorMode);
    is_AllocImageMem(hCam, width, height, nBitsPerPixel, &pcImageMemory, &id);
    is_SetImageMem(hCam, pcImageMemory, id);
    is_SetDisplayMode(hCam, IS_SET_DM_DIB);
    load();
}

UEyeCamera::~UEyeCamera() {
    // Releases an image memory that was allocated
    is_FreeImageMem(hCam, pcImageMemory, id);
    // Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
    is_ExitCamera(hCam);
}

char* UEyeCamera::getNextFrameData() {
    is_FreezeVideo(hCam, IS_WAIT);
    return pcImageMemory;
}

double UEyeCamera::getFPS() const
{
    double fps = 0.0;
    is_GetFramesPerSecond(hCam, &fps);
    return fps;
}

int UEyeCamera::getFormat() const {
    return 0;
}
