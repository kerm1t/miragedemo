// demo.cpp : our fresh new app fro mirage demo end of 2022
//

#include "framework.h"
#include "demo.h"
#include <string>
#include "shellapi.h" // drag & drop
#include <algorithm> // transform()
#include <cctype>  // tolower()


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define MAX_LOADSTRING 100
#define STR_ENDS_WITH(S, E) (strcmp(S + strlen(S) - (sizeof(E)-1), E) == 0)
#define USE_BITBLT

// I) background image
unsigned char *img;
int w, h, channels;
unsigned char* dibdata; // that's what we need to bitblt!!
HDC hdcMem;

// II) sprite(s)
struct sprite
{
  unsigned char *img; // stbi
  int w, h, channels;
  unsigned char* dibdata; // that's what we need to bitblt!!
  HDC hdcMem; // 
};
sprite owl;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int load_img(std::string filename)
{
  transform(filename.begin(), filename.end(), filename.begin(), std::tolower);
  if (STR_ENDS_WITH(filename.c_str(), ".jpg") ||
      STR_ENDS_WITH(filename.c_str(), ".jpeg") || 
    STR_ENDS_WITH(filename.c_str(), ".png")) {
    if (!stbi_info(filename.c_str(), &w, &h, &channels)) {
      printf("Couldn't read header %s\n", filename.c_str());
      return 0;
    }
    else {
      img = stbi_load(filename.c_str(), &w, &h, &channels, 0);
      if (img == NULL)
      {
        printf("Error reading %s\n", filename.c_str());
      }
      else
      {
        // the below conversions work great with jpg, partially with png and not good with gif (vertical green stripes)
        // --> check stbi_image
#ifdef USE_BITBLT
        // (1) RGB (stbimage) to BGR (DIBSection)
//          https://stackoverflow.com/questions/34033876/how-to-change-rgb-to-bgr
        unsigned char tmp;
        int raw_image_size = w * h * channels;
        for (int i = 0; i < raw_image_size; i += 3)
        {
          // swap R and B; raw_image[i + 1] is G, so it stays where it is.
          tmp = img[i + 0];
          img[i + 0] = img[i + 2];
          img[i + 2] = tmp;
        }


        // (2) copy image to DIBsection, in order to bitblt it later
        //     seems CreateCompatibleBitmap() could be even faster, but what the heck
        unsigned char* p, *q;
        int IMGRowBytes, DIBRowBytes;

//        https://stackoverflow.com/questions/7502588/createcompatiblebitmap-and-createdibsection-memory-dcs
//        https://libredd.it/pqiq24

        BITMAPINFO bi;
        memset(&bi, 0, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = w;
        bi.bmiHeader.biHeight = -h; // upside down
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = channels * 8; // assumed to be 24 bits
        bi.bmiHeader.biCompression = BI_RGB;
#define DIB_HSECTION NULL // system will alloc memory
        HBITMAP m_hBmp = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, (void**)&dibdata, DIB_HSECTION, NULL);
        printf("DIB Handle=%pn", m_hBmp);

        DIBRowBytes = IMGRowBytes = w * channels;
        while (DIBRowBytes & 3) ++DIBRowBytes;  // DIB needs 4n bytes/row

        p = dibdata;
        q = img;

        for (int y = 0; y < h; ++y) {
          memcpy(p, q, IMGRowBytes);
          p += DIBRowBytes;
          q += IMGRowBytes;
        }
        HBITMAP m_hOldBmp = (HBITMAP)SelectObject(hdcMem, m_hBmp);
        DeleteObject(m_hBmp);
        DeleteObject(m_hOldBmp);
#endif

        // ...don't forget to free all mem!
      }
    }
  }
  return 1;
}


// this fct() is more generalized as load_img, as it returns the img, thus can load more sprites/imgs
int load_sprite(std::string filename, sprite &spr)
{
  transform(filename.begin(), filename.end(), filename.begin(), std::tolower);
  if (STR_ENDS_WITH(filename.c_str(), ".jpg") ||
    STR_ENDS_WITH(filename.c_str(), ".jpeg") ||
    STR_ENDS_WITH(filename.c_str(), ".png")) {
    if (!stbi_info(filename.c_str(), &spr.w, &spr.h, &spr.channels)) {
      printf("Couldn't read header %s\n", filename.c_str());
      return 0;
    }
    else {
      spr.img = stbi_load(filename.c_str(), &spr.w, &spr.h, &spr.channels, 0);
      if (spr.img == NULL)
      {
        printf("Error reading %s\n", filename.c_str());
      }
      else
      {
        // the below conversions work great with jpg, partially with png and not good with gif (vertical green stripes)
        // --> check stbi_image
#ifdef USE_BITBLT
        // (1) RGB (stbimage) to BGR (DIBSection)
//          https://stackoverflow.com/questions/34033876/how-to-change-rgb-to-bgr
        unsigned char tmp;
        int raw_image_size = spr.w * spr.h * spr.channels;
        for (int i = 0; i < raw_image_size; i += 3)
        {
          // swap R and B; raw_image[i + 1] is G, so it stays where it is.
          tmp = spr.img[i + 0];
          spr.img[i + 0] = spr.img[i + 2];
          spr.img[i + 2] = tmp;
        }


        // (2) copy image to DIBsection, in order to bitblt it later
        //     seems CreateCompatibleBitmap() could be even faster, but what the heck
        unsigned char* p, *q;
        int IMGRowBytes, DIBRowBytes;

        //        https://stackoverflow.com/questions/7502588/createcompatiblebitmap-and-createdibsection-memory-dcs
        //        https://libredd.it/pqiq24

        BITMAPINFO bi;
        memset(&bi, 0, sizeof(bi));
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = spr.w;
        bi.bmiHeader.biHeight = -spr.h; // upside down
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = spr.channels * 8; // assumed to be 24 bits
        bi.bmiHeader.biCompression = BI_RGB;
        HBITMAP m_hBmp = CreateDIBSection(spr.hdcMem, &bi, DIB_RGB_COLORS, (void**)&spr.dibdata, NULL, NULL);
        printf("DIB Handle=%pn", m_hBmp);

        DIBRowBytes = IMGRowBytes = spr.w * spr.channels;
        while (DIBRowBytes & 3) ++DIBRowBytes;  // DIB needs 4n bytes/row

        p = spr.dibdata;
        q = spr.img;

        for (int y = 0; y < spr.h; ++y) {
          memcpy(p, q, IMGRowBytes);
          p += DIBRowBytes;
          q += IMGRowBytes;
        }
        HBITMAP m_hOldBmp = (HBITMAP)SelectObject(spr.hdcMem, m_hBmp);
        DeleteObject(m_hBmp);
        DeleteObject(m_hOldBmp);
#endif

        // ...don't forget to free all mem!
      }
    }
  }
  return 1;
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FOTOALBUM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FOTOALBUM));

    MSG msg;

    HDC hDC = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hDC);
    std::string filename = "c:\\git\\s-l1600.jpg";
    load_img(filename);

    owl.hdcMem = CreateCompatibleDC(hDC);
//    load_sprite("C:\\GIT\\miragedemo\\fotoalbum\\img\\owl1.png",owl);
    load_sprite("..\\img\\owl1.png", owl);

    // (3) trigger WM_PAINT event
    GetMessage(&msg, nullptr, 0, 0);
    RedrawWindow(msg.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    ReleaseDC(NULL, hDC);
    ReleaseDC(NULL, hdcMem); // is this needed? as not created with GetDC
    DeleteDC(hDC);
    DeleteDC(hdcMem);
    DeleteDC(owl.hdcMem);
    // delete dibsection ?

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FOTOALBUM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FOTOALBUM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        DragAcceptFiles(hWnd, TRUE); // drag n drop, yeah!
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

#ifdef USE_BITBLT
            if (img != NULL) BitBlt(hdc, 0, 0, 1600, 617, hdcMem, 0, 0, SRCCOPY);
            // SRCPAINT,
            // MERGECOPY
            // http://www.winprog.org/tutorial/transparency.html
            if (owl.img != NULL) BitBlt(hdc, 200, 200, owl.w, owl.h, owl.hdcMem, 0, 0, SRCPAINT); // paint "sprite"
///            if (owl.img != NULL) TransparentBlt(hdc, 200, 200, owl.w, owl.h, owl.hdcMem, 0, 0, 0, 0, 0); // paint "sprite"
#endif
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DROPFILES:
        char filename[MAX_PATH];
        DragQueryFileA((HDROP)wParam, 0, filename, MAX_PATH); // use file >> 0 << in list of dragged files
        load_img(filename);
        RedrawWindow(hWnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
        DragFinish((HDROP)wParam);
        break;
    case WM_DESTROY:
        DragAcceptFiles(hWnd, FALSE); // prevents bogus drop messages after we are gone ;-/ ??
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
