/* FILE NAME   : 'win.cpp'
 * PURPOSE     : WINAPI module.
 *               Main module functions.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 22.07.2022.
 * NOTE        : Module namespace 'win'.
 */

#include <pch.h>

/* Common constrols Win32 API component */
#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")

#include "win.h"

/* Project main namespace */
namespace win
{
  /* Class constructor
   * ARGUMENTS:
   *   - Application instance handle:
   *       HINSTANCE hInst;
   *   - Window class name and name:
   *       const CHAR *ClassName, const CHAR *WindowName
   */
  win::win( HINSTANCE hInst ) :
    hInstance(hInst), FullScreenSaveRect(), H(), W(), MouseWheel(0), IsFullScreen(FALSE),
    IsActive(TRUE), hWnd(NULL), IsInit(FALSE)
  {
  } /* End of constructor */

  /* Window creating function.
   * ARGUMENTS:
   *   - Window class name and name:
   *       const CHAR *ClassName, const CHAR *WindowName;
   * RETURNS:
   *   (BOOL) Success flag.
   */
  BOOL win::Create( const CHAR *ClassName, const CHAR *WindowName )
  {
    WNDCLASS wc;

    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(win *);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_SHIELD);
    wc.lpszMenuName = NULL;
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WinFunc;
    wc.lpszClassName = ClassName;

    if (!RegisterClass(&wc))
      exit(1);

    HMENU hMenu = LoadMenuA(hInstance, (const CHAR *)IDR_MAIN_MENU);
    hWnd = CreateWindowExA(0, ClassName, WindowName, WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                           nullptr, hMenu, hInstance, reinterpret_cast<VOID *>(this));

    ShowWindow(hWnd, 1);
    UpdateWindow(hWnd);

    return hWnd != nullptr;
  } /* End of 'Create' function */

  /* Class destructor (empty, virtual).
   */
  win::~win( VOID )
  {
  } /* End of destructor */

  /* WINAPI messages callback function.
   * ARGUMENTS:
   *   - Message owner window handle:
   *       HWND hWnd;
   *   - Message code:
   *       UINT Msg;
   *   - Dword and long pointer parameters:
   *       WPARAM wParam, LPARAM lParam;
   * RETURNS:
   *   (LRESULT) Message specific return code.
   */
  LRESULT CALLBACK win::WinFunc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
  {
    win *Win;
    HDC hDC;
    PAINTSTRUCT ps;
 
    switch (Msg)
    {
    case WM_GETMINMAXINFO:
    {
      auto MinMaxInfo {(MINMAXINFO *)lParam};
      MinMaxInfo->ptMaxTrackSize.y =
        GetSystemMetrics(SM_CYMAXTRACK) +
        GetSystemMetrics(SM_CYCAPTION) +
        GetSystemMetrics(SM_CYMENU) +
        GetSystemMetrics(SM_CYBORDER) * 2;

      MinMaxInfo->ptMinTrackSize = {240, 160};
    }
      return 0;
    case WM_CREATE:
      Win = reinterpret_cast<win *>(((CREATESTRUCT *)lParam)->lpCreateParams);
      Win->hWnd = hWnd;
      SetWindowLongPtr(hWnd, 0, (UINT_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
    default:
      Win = reinterpret_cast<win *>(GetWindowLongPtr(hWnd, 0));
      if (Win != nullptr)
        switch (Msg)
        {
        case WM_CREATE:
          return Win->OnCreate((CREATESTRUCT *)lParam) ? 0 : -1;
        case WM_SIZE:
          Win->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
          return 0;
        case WM_ERASEBKGND:
          return (LRESULT)Win->OnEraseBkgnd((HDC)wParam);
        case WM_PAINT:
          hDC = BeginPaint(hWnd, &ps);
          Win->OnPaint(hDC, &ps);
          EndPaint(hWnd, &ps);
          return 0;
        case WM_TIMER:
          Win->OnTimer((UINT)wParam);
          return 0;
        case WM_MOUSEWHEEL:
          Win->OnMouseWheel((INT)(SHORT)LOWORD(lParam),
                            (INT)(SHORT)HIWORD(lParam),
                            (INT)(SHORT)HIWORD(wParam),
                            (UINT)(SHORT)LOWORD(wParam));
          return 0;
        case WM_CLOSE:
          if (!Win->OnClose())
            return FALSE;
        case WM_DESTROY:
          Win->OnDestroy();
          PostQuitMessage(30);
          return 0;
        case WM_COMMAND:
          Win->OnMenuButton((UINT)LOWORD(wParam));
          return 0;
        case WM_ACTIVATE:
          Win->OnActivate((UINT)LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
          break;
        }
    }

    return DefWindowProc(hWnd, Msg, wParam, lParam);
  } /* End of 'WinFunc' function */

  /* Full screen mode switch function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  VOID win::FlipFullScreen( VOID )
  {
    if (!IsFullScreen)
    {
      HMONITOR hMon;
      MONITORINFOEX moninfo;
      RECT rc;
 
      IsFullScreen = TRUE;
 
      /* Save old window size and position */
      GetWindowRect(hWnd, &FullScreenSaveRect);
 
      /* Get closest monitor */
      hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
 
      /* Get monitor information */
      moninfo.cbSize = sizeof(moninfo);
      GetMonitorInfo(hMon, (MONITORINFO *)&moninfo);
 
      rc = moninfo.rcMonitor;
      AdjustWindowRect(&rc, GetWindowLong(hWnd, GWL_STYLE), FALSE);
 
      /* Expand window to full screen */
      SetWindowPos(hWnd, HWND_TOPMOST,
        rc.left, rc.top,
        rc.right - rc.left, rc.bottom - rc.top,
        SWP_NOOWNERZORDER);
    }
    else
    {
      IsFullScreen = FALSE;
 
      /* Restore window size and position */
      SetWindowPos(hWnd, HWND_NOTOPMOST,
        FullScreenSaveRect.left, FullScreenSaveRect.top,
        FullScreenSaveRect.right - FullScreenSaveRect.left, FullScreenSaveRect.bottom - FullScreenSaveRect.top,
        SWP_NOOWNERZORDER);
    }
  } /* End of 'FlipFullScreen' function */
} /* end of 'win' namespace */

 /* END OF 'win.cpp' FILE */
