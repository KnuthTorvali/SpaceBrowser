#include <windows.h>
#include <gdiplus.h>
#include <tchar.h>
#include <string>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HICON LoadIconFromPNG(const wchar_t* szFileName) {
    Bitmap* bitmap = Bitmap::FromFile(szFileName);
    if (!bitmap) return NULL;

    HICON hIcon = NULL;
    bitmap->GetHICON(&hIcon);
    delete bitmap;
    return hIcon;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    static wchar_t szAppName[] = L"SpaceBrowser";
    WNDCLASS wndclass = {};
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;

    wndclass.hIcon = LoadIconFromPNG(L"logo.png");

    if (!wndclass.hIcon) {
        wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass)) {
        return EXIT_FAILURE;
    }

    HWND hWnd = CreateWindow(
        szAppName,
        L"Space Browser",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1750, 1000,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hStatic;
    static HFONT hFont;
    static Image* image = nullptr;
    static wstring buffer;

    switch (message)
    {
    case WM_CREATE: {
        image = new Image(L"Background.png");

        hFont = CreateFont(
            28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            HANGUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,
            L"¸¼Àº °íµñ"
        );

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        int barWidth = 800;
        int barHeight = 60;
        int barX = (clientRect.right - barWidth) / 2;
        int barY = (clientRect.bottom - barHeight) / 2;

        hStatic = CreateWindowEx(0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
            barX + 10, barY + 10, barWidth - 20, barHeight - 20,
            hWnd, NULL, GetModuleHandle(NULL), NULL);

        SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);

        return 0;
    }
    case WM_CHAR: {
        if (wParam >= 32) {
            buffer += (wchar_t)wParam;
        }

        SetWindowText(hStatic, buffer.c_str());

        InvalidateRect(hStatic, NULL, TRUE);
        return 0;
    }
    case WM_KEYDOWN: {
        if (wParam == VK_BACK && !buffer.empty()) {
            buffer.pop_back();
            SetWindowText(hStatic, buffer.c_str());

            InvalidateRect(hStatic, NULL, TRUE);
        }
        return 0;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(255, 255, 255));
        SetBkMode(hdcStatic, TRANSPARENT);

        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        Graphics graphics(hdc);
        if (image) {
            int x = 0;
            int y = 0;
            int width = image->GetWidth();
            int height = image->GetHeight();
            graphics.DrawImage(image, x, y, width, height);
        }

        SolidBrush brush(Color(100, 255, 255, 255));

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);

        int barWidth = 800;
        int barHeight = 60;
        int barX = (clientRect.right - barWidth) / 2;
        int barY = (clientRect.bottom - barHeight) / 2;

        GraphicsPath path;
        path.AddArc(barX, barY, barHeight, barHeight, 90, 180);
        path.AddLine(barX + barHeight / 2, barY, barX + barWidth - barHeight / 2, barY);
        path.AddArc(barX + barWidth - barHeight, barY, barHeight, barHeight, 270, 180);
        path.AddLine(barX + barWidth - barHeight / 2, barY + barHeight, barX + barHeight / 2, barY + barHeight);
        path.CloseFigure();

        graphics.FillPath(&brush, &path);

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: {
        return 1;
    }
    case WM_DESTROY:
        if (hFont) {
            DeleteObject(hFont);
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
