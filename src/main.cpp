#ifndef UNICODE
#define UNICODE
#endif

#if _DEBUG
// enable mem leak detection
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

#include <cstdlib>
#include <thread>
#include <chrono>
#include <atomic>

struct StateInfo {
    std::atomic_bool running{ true };
    HWND hwnd = nullptr;
    // d2d objects
    ID2D1Factory* pD2DFactory = nullptr;
    ID2D1HwndRenderTarget* pRenderTarget = nullptr;
    // drawing state
    D2D1_ELLIPSE* ellipse = nullptr;
};

LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void WndThread(HINSTANCE hInstance, int nCmdShow, StateInfo* pState);

int __stdcall wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int nCmdShow) {
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    StateInfo* pState = new StateInfo();
    if (!pState) return 0;

    std::thread winThread(WndThread, hInstance, nCmdShow, pState);

    pState->ellipse = new D2D1_ELLIPSE{};
    pState->ellipse->point = D2D1_POINT_2F{ 100, 100 };
    pState->ellipse->radiusX = 20;
    pState->ellipse->radiusY = 20;

    // main loop
    while (pState->running) {
        


        if (pState->hwnd) InvalidateRect(pState->hwnd, nullptr, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    // cleanup
    winThread.join();
    delete pState->ellipse;
    delete pState;
    return 0;
}

void WndThread(HINSTANCE hInstance, int nCmdShow, StateInfo* pState) {

    // init window
    const wchar_t* className = L"WindOwO";  
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    RegisterClassW(&wc);

    pState->hwnd = CreateWindowExW(0, className, L"WindOwO", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
        CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, pState);

    if (pState->hwnd == NULL) {
        pState->running = false;
        return;
    }

    ShowWindow(pState->hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageW(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    pState->running = false;
}

LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    StateInfo* pState = reinterpret_cast<StateInfo*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg) {
    case WM_CREATE: {
        LPCREATESTRUCT pCreate = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pState = reinterpret_cast<StateInfo*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pState));

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pState->pD2DFactory);
        if (FAILED(hr)) return -1;

        RECT rc;
        GetClientRect(hwnd, &rc);
        D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

        hr = pState->pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, size),
            &pState->pRenderTarget);
        if (FAILED(hr)) return -1;

        return 0;
    }

    case WM_SIZE: {
        if (pState && pState->pRenderTarget) {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            D2D1_SIZE_U size = D2D1::SizeU(width, height);
            pState->pRenderTarget->Resize(size);
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        if (pState && pState->pRenderTarget) {
            pState->pRenderTarget->BeginDraw();
            pState->pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));



            HRESULT hr = pState->pRenderTarget->EndDraw();
            if (hr == D2DERR_RECREATE_TARGET) {
                // handle device loss and recreate render target
            }
        }
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY: {
        // cleanup
        if (pState) {
            if (pState->pRenderTarget) {
                pState->pRenderTarget->Release();
                pState->pRenderTarget = nullptr;
            }
            if (pState->pD2DFactory) {
                pState->pD2DFactory->Release();
                pState->pD2DFactory = nullptr;
            }
        }
        PostQuitMessage(0);
        return 0;
    }

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}