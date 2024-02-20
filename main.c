#include <stdlib.h>
#include <windows.h>

int gNumVisible = 0;
HWND *gVisibleWnds = NULL;

VOID CALLBACK TmrProc(HWND hwnd, UINT uMsg, UINT idTmr, DWORD dwTime)
{
    for (int i = 0; i < gNumVisible; i++) {
        SendMessage(gVisibleWnds[i], WM_SETREDRAW, TRUE, 0);
    }
    PostQuitMessage(0);
}

BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam)
{
    static int idx = 0;
    if (lParam && gVisibleWnds && idx < gNumVisible && IsWindowVisible(hwnd)) {
        gVisibleWnds[idx++] = hwnd;
    } else if (!lParam && IsWindowVisible(hwnd)) {
        gNumVisible++;
    }
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    extern const char _binary_oe_bmp_start[];
    BITMAPFILEHEADER *bfh = (BITMAPFILEHEADER *)_binary_oe_bmp_start;
    BITMAPINFO *bmi = (BITMAPINFO *)(bfh + 1);
    void *ppv;
    HBITMAP hbm = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, &ppv, NULL, 0);
    SetDIBits(NULL, hbm, 0, bmi->bmiHeader.biHeight, (char *)bfh + bfh->bfOffBits, bmi, DIB_RGB_COLORS);

    HDC hdcMem = CreateCompatibleDC(NULL);
    SelectObject(hdcMem, hbm);

    EnumWindows(EnumWndProc, 0);
    gVisibleWnds = (HWND *)calloc(gNumVisible, sizeof(HWND));
    if (!gVisibleWnds) {
        return 1;
    }
    EnumWindows(EnumWndProc, 1);

    for (int i = 0; i < gNumVisible; i++) {
        HDC hdc = GetWindowDC(gVisibleWnds[i]);
        if (!hdc) {
            continue;
        }
        RECT r;
        GetWindowRect(gVisibleWnds[i], &r);
        StretchBlt(hdc, 0, 0, r.right - r.left, r.bottom - r.top, hdcMem, 0, 0, bmi->bmiHeader.biWidth,
                   bmi->bmiHeader.biHeight, SRCCOPY);
        SendMessage(gVisibleWnds[i], WM_SETREDRAW, FALSE, 0);
        ReleaseDC(gVisibleWnds[i], hdc);
    }

    SetTimer(NULL, 14888, 10000, (TIMERPROC)TmrProc);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteDC(hdcMem);
    DeleteObject(hbm);
    free(gVisibleWnds);

    return 0;
}