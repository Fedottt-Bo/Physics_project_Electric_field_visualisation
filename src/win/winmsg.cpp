/* FILE NAME   : 'winmsg.cpp'
 * PURPOSE     : WINAPI module.
 *               Message crackers realization file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 22.07.2022.
 * NOTE        : Module namespace 'win'.
 */

#include <pch.h>

#include "win.h"

/* Project main namespace */
namespace win
{
  /* Mainloop start function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  VOID win::Run( VOID )
  {
    MSG msg;

    while (TRUE)
    {
      /* Check message at window message queue */
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        if (msg.message == WM_QUIT)
          break;
        else
        {
          /* Displatch message to window */
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      else
        Idle();
    }
  }

  /* WM_CREATE window message handle function.
   * ARGUMENTS:
   *   - structure with creation data:
   *       CREATESTRUCT *CS;
   * RETURNS:
   *   (BOOL) TRUE to continue creation window, FALSE to terminate.
   */
  BOOL win::OnCreate( CREATESTRUCT *CS )
  {
    SetTimer(hWnd, 30, 1, nullptr);

    return TRUE;
  } /* End of 'win::OnCreate' function */

  /* WM_CLOSE window message handle function.
   * ARGUMENTS: None.
   * RETURNS:
   *   (BOOL) Closing allow flag.
   */
  BOOL win::OnClose( VOID )
  {
    return IsInit ? Close() : TRUE;
  } /* End of 'win::OnClose' function */

  /* WM_SIZE window message handle function.
   * ARGUMENTS:
   *   - sizing flag (see SIZE_***, like SIZE_MAXIMIZED)
   *       UINT State;
   *   - new width and height of client area:
   *       INT W, H;
   * RETURNS: None.
   */
  VOID win::OnSize( UINT State, INT W, INT H )
  {
    this->W = W;
    this->H = H;

    Resize(IsSizeProcess);
  } /* End of 'win::OnSize' function */
   
  /* WM_ERASEBKGND window message handle function.
   * ARGUMENTS:
   *   - device context of client area:
   *       HDC hDC;
   * RETURNS:
   *   (BOOL) TRUE if background is erased, FALSE otherwise.
   */
  BOOL win::OnEraseBkgnd( HDC hDC )
  {
    Erase(hDC);

    return TRUE;
  } /* End of 'win::OnEraseBkgnd' function */

  /* WM_PAINT window message handle function.
   * ARGUMENTS:
   *   - window device context:
   *       HDC hDC;
   *   - paint message structure pointer:
   *       PAINTSTRUCT *PS;
   * RETURNS: None.
   */
  VOID win::OnPaint( HDC hDC, PAINTSTRUCT *PS )
  {
    Paint(hDC);
  } /* End of 'win::OnPaint' function */ 

  /* WM_ACTIVATE window message handle function.
   * ARGUMENTS:
   *   - reason (WA_CLICKACTIVE, WA_ACTIVE or WA_INACTIVE):
   *       UINT Reason;
   *   - handle of active window:
   *       HWND hWndActDeact;
   *   - minimized flag:
   *       BOOL IsMinimized;
   * RETURNS: None.
   */
  VOID win::OnActivate( UINT Reason, HWND hWndActDeact, BOOL IsMinimized )
  {
    IsActive = (Reason == WA_CLICKACTIVE || Reason == WA_ACTIVE);
  } /* End of 'win::OnActivate' function */

  /* WM_TIMER window message handle function.
   * ARGUMENTS:
   *   - specified the timer identifier.
   *       INT Id;
   * RETURNS: None.
   */
  VOID win::OnTimer( INT Id )
  {
    if (!IsInit)
    {
      KillTimer(hWnd, Id);
      SetTimer(hWnd, 47, 1, nullptr);

      IsInit = TRUE;

      Init();
      Resize(FALSE);
    }
    else
      Timer();
  } /* End of 'win::OnTimer' function */

  /* WM_MOUSEWHEEL window message handle function.
   * ARGUMENTS:
   *   - mouse window position:
   *       INT X, Y;
   *   - mouse wheel relative delta value:
   *       INT Z;
   *   - mouse keys bits (see MK_*** bits constants):
   *       UINT Keys;
   * RETURNS: None.
   */
  VOID win::OnMouseWheel( INT X, INT Y, INT Z, UINT Keys )
  {
    MouseWheel += Z;
  } /* End of 'win::OnMouseWheel' function */

  /* WM_EXITSIZEMOVE window message handle function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  VOID win::OnExitSizeMove( VOID )
  {
    IsSizeProcess = FALSE;
  } /* End of 'win::OnExitSizeMove' function */

  /* WM_ENTERSIZEMOVE window message handle function.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  VOID win::OnEnterSizeMove( VOID )
  {
    IsSizeProcess = TRUE;
  } /* End of 'win::OnEnterSizeMove' function */
} /* end of 'win' namespace */

 /* END OF 'winmsg.cpp' FILE */
