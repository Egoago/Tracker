#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <uEye.h>
#include <tchar.h>
#include <conio.h>

using namespace cv;
using namespace std;

HWND CreateDisplayWindow(int, int);
LRESULT WINAPI ConsoleDispWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(int argc, char** argv) {
    HIDS hCam = 0;
    int DisplayWidth = 1280;
    int DisplayHeight = 960;
    HWND hWndDisplay = CreateDisplayWindow(DisplayWidth, DisplayHeight);
    int nRet = is_InitCamera(&hCam, hWndDisplay);
    if (nRet != IS_SUCCESS)
    {
        cout << "Camera not found" << endl;
        exit(1);
    }
    else
    {
        cout << "Camera initialisation was successful!" << endl << endl;
    }
    SENSORINFO sInfo;
    nRet = is_GetSensorInfo(hCam, &sInfo);
    if (nRet == IS_SUCCESS)
    {
        cout << "Cameramodel: \t\t" << sInfo.strSensorName << endl;
        cout << "Maximum image width: \t" << sInfo.nMaxWidth << endl;
        cout << "Maximum image height: \t" << sInfo.nMaxHeight << endl;
    }
    int MaxImageSizeX = sInfo.nMaxWidth;
    int MaxImageSizeY = sInfo.nMaxHeight;
    DisplayWidth = MaxImageSizeX;
    DisplayHeight = MaxImageSizeY;
    int n = 1;
    do {
        DisplayWidth = MaxImageSizeX / n;
        n++;
    } while (DisplayWidth >= 1280);
    DisplayHeight = MaxImageSizeY / (n - 1);

    int nColorMode;
    int nBitsPerPixel;
    cout << "Color mode:";
    if (sInfo.nColorMode == IS_COLORMODE_BAYER)
    {
        // for color camera models use RGB24 mode
        cout << "BAYER" << endl;
        nColorMode = IS_CM_BGR8_PACKED;
        nBitsPerPixel = 24;
    }
    else if (sInfo.nColorMode == IS_COLORMODE_CBYCRY)
    {
        // for CBYCRY camera models use RGB32 mode
        cout << "CBYCRY" << endl;
        nColorMode = IS_CM_BGRA8_PACKED;
        nBitsPerPixel = 32;
    }
    else
    {
        // for monochrome camera models use Y8 mode
        cout << "MONO" << endl;
        nColorMode = IS_CM_MONO8;
        nBitsPerPixel = 8;
    }
    // Sets the color mode to be used when image data are saved or displayed by the graphics card
    is_SetColorMode(hCam, nColorMode);


    // allocates an image memory for an image, activates it and sets the way in which the images will be displayed on the screen
    int nMemoryId;
    char* pcImageMemory;
    is_AllocImageMem(hCam, MaxImageSizeX, MaxImageSizeY, nBitsPerPixel, &pcImageMemory, &nMemoryId);
    is_SetImageMem(hCam, pcImageMemory, nMemoryId);
    is_SetDisplayMode(hCam, IS_SET_DM_DIB);

    // Configuration of the display window
    SetWindowPos(hWndDisplay, NULL, 0, 50, DisplayWidth, DisplayHeight, SWP_NOZORDER);
    RECT rc;
    GetClientRect(hWndDisplay, &rc);
    SetWindowPos(hWndDisplay, NULL, 0, 50, 2 * DisplayWidth - (rc.right - rc.left), 2 * DisplayHeight - (rc.bottom - rc.top), SWP_NOZORDER);
    ShowWindow(hWndDisplay, SW_SHOW);

    while (1)
    {
        is_FreezeVideo(hCam, IS_WAIT);
        is_RenderBitmap(hCam, nMemoryId, hWndDisplay, IS_RENDER_FIT_TO_WINDOW);
        if (_kbhit())
        {
            int key = _getch();
            if ((key == 'q') || (key == 'Q'))
                break;
        }
        MSG msg;
        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        DispatchMessage(&msg);
    }

    // Releases an image memory that was allocated
    nRet = is_FreeImageMem(hCam, pcImageMemory, nMemoryId);
    if (nRet == IS_SUCCESS)
    {
        cout << "The allocated memory has been successfully released!" << endl;
    }

    // Disables the hCam camera handle and releases the data structures and memory areas taken up by the uEye camera
    nRet = is_ExitCamera(hCam);
    if (nRet == IS_SUCCESS)
    {
        cout << "The camera has been successfully logged off!" << endl << endl;
    }

    system("pause");
    return 0;

}

//////////////////////////////////////////////////////   Windows Code   //////////////////////////////////////////////////////
HWND CreateDisplayWindow(int width = 1280, int height = 960)
{
    WNDCLASSEX wcx;
    HMODULE hInstance = GetModuleHandle(NULL);
    if (GetClassInfoEx(hInstance, _T("ConsoleDispClass"), &wcx) == 0)
    {
        // Fill in the window class structure with parameters that describe the main window.
        wcx.cbSize = sizeof(wcx);                 // size of structure
        wcx.style = CS_HREDRAW | CS_NOCLOSE | CS_SAVEBITS | CS_VREDRAW | WS_OVERLAPPED;
        wcx.lpfnWndProc = ConsoleDispWndProc;     // points to window procedure
        wcx.cbClsExtra = 0;                       // no extra class memory
        wcx.cbWndExtra = 0;                       // no extra window memory
        wcx.hInstance = hInstance;                // handle to instance
        wcx.hIcon = NULL;                         // no icon
        wcx.hCursor = NULL;                       // no cursor
        wcx.lpszMenuName = NULL;                  // name of menu resource
        wcx.lpszClassName = _T("ConsoleDispClass"); // name of window class
        wcx.hIconSm = NULL;                       // small class icon
        wcx.hbrBackground = NULL;

        // Register the window class.
        ATOM atom = RegisterClassEx(&wcx);

        if (atom != NULL)
        {
            // create our display window
            HWND hWndDisplay = CreateWindow(_T("ConsoleDispClass"), // name of window class
                L"uEye",        // title-bar string
                WS_OVERLAPPEDWINDOW,        // top-level window
                CW_USEDEFAULT,              // default horizontal position
                CW_USEDEFAULT,              // default vertical position
                width,               // default width
                height,              // default height
                (HWND)NULL,                 // no owner window
                (HMENU)NULL,                // use class menu
                hInstance,                  // handle to application instance
                (LPVOID)NULL);              // no window-creation data

            if (!hWndDisplay)
            {
                DWORD a = GetLastError();
                LPVOID lpMsgBuf;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS, NULL, a, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf, 0, NULL);

                // Free the buffer.
                LocalFree(lpMsgBuf);
                ::UnregisterClass(_T("ConsoleDispClass"), hInstance);
                return NULL;
            }
            else return hWndDisplay;
        }
    }
    return NULL;
}

LRESULT WINAPI ConsoleDispWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    RECT rc;

    switch (msg)
    {
    case WM_CREATE:
        // Initialize the window.
        return 0;

    case WM_ERASEBKGND:
        if (GetUpdateRect(hwnd, &rc, FALSE))
        {
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
        }
        return 1;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}