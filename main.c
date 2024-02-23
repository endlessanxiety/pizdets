#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <windows.h>

int gNumVisible = 0;
HWND *gVisibleWnds = NULL;

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

void rot2(POINT *p, double a)
{
    LONG x_old = p->x;
    p->x = cos(a) * p->x - sin(a) * p->y;
    p->y = sin(a) * x_old + cos(a) * p->y;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    extern const char _binary_sw_bmp_start[];
    BITMAPFILEHEADER *bfh = (BITMAPFILEHEADER *)_binary_sw_bmp_start;
    BITMAPINFO *bmi = (BITMAPINFO *)(bfh + 1);
    void *ppv;
    HBITMAP hbm = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, &ppv, NULL, 0);
    SetDIBits(NULL, hbm, 0, bmi->bmiHeader.biHeight, (char *)bfh + bfh->bfOffBits, bmi, DIB_RGB_COLORS);
    LONG w = bmi->bmiHeader.biWidth;
    LONG h = bmi->bmiHeader.biHeight;

    HDC hdcMem = CreateCompatibleDC(NULL);
    SelectObject(hdcMem, hbm);

    HDC hdcRot = CreateCompatibleDC(NULL);
    void *bmBits = malloc(w * h * 4);
    if (!bmBits) {
        return 1;
    }
    HBITMAP hbmRot = CreateBitmap(w, h, 1, 32, bmBits);
    SelectObject(hdcRot, hbmRot);

    HBRUSH hbrBg = CreateSolidBrush(RGB(0, 0, 0));

    EnumWindows(EnumWndProc, 0);
    gVisibleWnds = (HWND *)calloc(gNumVisible, sizeof(HWND));
    if (!gVisibleWnds) {
        return 1;
    }
    EnumWindows(EnumWndProc, 1);

    double rotAng = 0.0;

    for (int t = 0; t < 5 * 60000; t += 20) {
        POINT p[3];
        p[0].x = -w / 2;
        p[0].y = h / 2;
        p[1].x = -p[0].x;
        p[1].y = p[0].y;
        p[2].x = p[0].x;
        p[2].y = -p[0].y;

        for (int i = 0; i < 3; i++) {
            rot2(&p[i], rotAng);
            p[i].x += w / 2;
            p[i].y = h / 2 - p[i].y;
        }

        rotAng += M_PI / 30;

        RECT r = {.left = 0, .top = 0, .right = w, .bottom = h};
        FillRect(hdcRot, &r, hbrBg);
        PlgBlt(hdcRot, p, hdcMem, 0, 0, w, h, NULL, 0, 0);

        for (int i = 0; i < gNumVisible; i++) {
            HDC hdc = GetWindowDC(gVisibleWnds[i]);
            if (!hdc) {
                continue;
            }
            GetWindowRect(gVisibleWnds[i], &r);
            r.right -= r.left;
            r.bottom -= r.top;
            r.left = r.top = 0;

#define MIN(a, b) ((a) < (b) ? (a) : (b))

            LONG sqSide = MIN(r.right, r.bottom);
            StretchBlt(hdc, (r.right - sqSide) / 2, (r.bottom - sqSide) / 2, sqSide, sqSide, hdcRot, 0, 0, w, h,
                       SRCCOPY);
            RECT fr;
            fr.left = fr.top = 0;
            if (r.right > r.bottom) {
                fr.right = (r.right - sqSide) / 2;
                fr.bottom = sqSide;
            } else {
                fr.right = sqSide;
                fr.bottom = (r.bottom - sqSide) / 2;
            }
            FillRect(hdc, &fr, hbrBg);
            if (r.right > r.bottom) {
                fr.left = fr.right + sqSide;
                fr.right = r.right;
            } else {
                fr.top = fr.bottom + sqSide;
                fr.bottom = r.bottom;
            }
            FillRect(hdc, &fr, hbrBg);
            ReleaseDC(gVisibleWnds[i], hdc);
        }

        Sleep(20);
    }

    DeleteDC(hdcMem);
    DeleteObject(hbm);
    DeleteObject(hbmRot);
    DeleteObject(hbrBg);
    free(gVisibleWnds);
    free(bmBits);

    return 0;
}