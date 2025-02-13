#ifndef UNICODE
#define UNICODE
#endif

#if _DEBUG
// enable mem leak detection
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <cstdlib>
#include <windows.h>
#include <thread>
#include <chrono>
#include <atomic>

struct StateInfo {

};

std::atomic_bool g_running{ true };
HWND g_hwnd = nullptr;

LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WinThread(HINSTANCE hInstance, int nCmdShow, StateInfo* pState);

int __stdcall wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int nCmdShow) {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    StateInfo* pState = new StateInfo();
    if (pState == NULL) return 0;

    std::thread winThread(WinThread, hInstance, nCmdShow, pState);

    int rectX = 50;
    int rectY = 50;
    const int rectW = 50;
    const int rectH = 50;
    const int rectSpeed = 10;

    while (g_running) {
        
        // input handling
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) rectX -= rectSpeed;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) rectX += rectSpeed;
        if (GetAsyncKeyState(VK_UP) & 0x8000) rectY -= rectSpeed;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) rectY += rectSpeed;
        
        // drawing
        if (g_hwnd) {
            HDC hdc = GetDC(g_hwnd);
            if (hdc) {
                RECT clientRect;
                GetClientRect(g_hwnd, &clientRect);
                FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 3));

                RECT rect = { rectX, rectY, rectX + rectW, rectY + rectH };
                HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
                FillRect(hdc, &rect, brush);
                DeleteObject(brush);

                ReleaseDC(g_hwnd, hdc);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    // cleanup
    winThread.join();
    delete pState;
    return 0;
}

void WinThread(HINSTANCE hInstance, int nCmdShow, StateInfo* pState) {
    const wchar_t* CLASS_NAME = L"WindOwO";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    g_hwnd = CreateWindowExW(0, CLASS_NAME, L"WindOwO", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
        CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, pState);

    if (g_hwnd == NULL) {
        g_running = false;
        return;
    }

    ShowWindow(g_hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageW(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    g_running = false;
    return;
}

LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY: 
        PostQuitMessage(0);
        return 0;
    
    case WM_PAINT: 
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
    
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}