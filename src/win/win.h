/* FILE NAME   : 'win.h'
 * PURPOSE     : WINAPI module.
 *               Module handle file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 10.04.2023.
 * NOTE        : Module namespace 'win'.
 */

#ifndef __win_h__
#define __win_h__

#include <def.h>

/* Project main namespace */
namespace win
{
  /* Window handle class */
  class win
  {
  protected:
    HWND hWnd;             /* window handle */
    HMENU hMainMenu;       /* window menu handle */
    HACCEL hMainMenuAccel; /* Main menu shortcuts accelerator */
    INT W, H;              /* window size */
    HINSTANCE hInstance;   /* application handle */
 
  private:

    /* Window handle function.
     * ARGUMENTS:
     *   - window handle:
     *      HWND hWnd;
     *   - message type (see WM_***):
     *      UINT Msg;
     *   - message 'word' parameter:
     *      WPARAM wParam;
     *   - message 'long' parameter:
     *      LPARAM lParam;
     * RETURNS:
     *   (LRESULT) message depende return value.
     */
    static LRESULT CALLBACK WinFunc( HWND hWnd, UINT Msg,
                                     WPARAM wParam, LPARAM lParam );

    /* Size/moving process flag */
    BOOL IsSizeProcess;

    /* WM_CREATE window message handle function.
      * ARGUMENTS:
      *   - structure with creation data:
      *       CREATESTRUCT *CS;
      * RETURNS:
      *   (BOOL) TRUE to continue creation window, FALSE to terminate.
      */
    virtual BOOL OnCreate( CREATESTRUCT* CS );

    /* WM_SIZE window message handle function.
     * ARGUMENTS:
     *   - sizing flag (see SIZE_***, like SIZE_MAXIMIZED)
     *       UINT State;
     *   - new width and height of client area:
     *       INT W, H;
     * RETURNS: None.
     */
    virtual VOID OnSize( UINT State, INT W, INT H );

    /* WM_CLOSE window message handle function.
     * ARGUMENTS: None.
     * RETURNS:
     *   (BOOL) Closing allow flag.
     */
    virtual BOOL OnClose( VOID );

    /* WM_DESTROY window message handle function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID OnDestroy( VOID ) {}

    /* WM_ERASEBKGND window message handle function.
     * ARGUMENTS:
     *   - device context of client area:
     *       HDC hDC;
     * RETURNS:
     *   (BOOL) TRUE if background is erased, FALSE otherwise.
     */
    virtual BOOL OnEraseBkgnd( HDC hDC );

    /* WM_PAINT window message handle function.
     * ARGUMENTS:
     *   - device context of client area:
     *       HDC hDC;
     *   - paint message structure pointer:
     *       PAINTSTRUCT *PS;
     * RETURNS: None.
     */
    virtual VOID OnPaint( HDC hDC, PAINTSTRUCT *PS );

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
    virtual VOID OnActivate( UINT Reason, HWND hWndActDeact, BOOL IsMinimized );

    /* WM_MOUSEWHEEL window message handle function.
     * ARGUMENTS:
     *   - mouse window position:
     *       INT X, Y;
     *   - mouse wheel delta value (120 points per rotate):
     *       INT Z;
     *   - mouse keys bits (see MK_*** bits constants):
     *       UINT Keys;
     * RETURNS: None.
     */
    virtual VOID OnMouseWheel( INT X, INT Y, INT Z, UINT Keys );

    /* WM_TIMER window message handle function.
     * ARGUMENTS:
     *   - specified the timer identifier.
     *       INT Id;
     * RETURNS: None.
     */
    virtual VOID OnTimer( INT Id );

    /* WM_EXITSIZEMOVE window message handle function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID OnExitSizeMove( VOID );

    /* WM_ENTERSIZEMOVE window message handle function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID OnEnterSizeMove( VOID );

    /* WM_COMMAND window message handle function.
     * ARGUMENTS:
     *   - Menu button ID:
     *       UINT Id;
     * RETURNS: None.
     */
    virtual VOID OnMenuButton( UINT Id ) {}

  protected:
    /* Initialization flag */
    BOOL IsInit;

    /***
     * Virtual callbacks.
     ***/
    /* Window creation callback.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Init( VOID ) {}

    /* Window closing callback.
     * ARGUMENTS: None.
     * RETURNS:
     *   (BOOL) Window closing allow flag.
     */
    virtual BOOL Close( VOID ) { return TRUE; }

    /* Window destruction callback.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Destroyed( VOID ) {}

    /* Resize callback.
     * ARGUMENTS:
     *   - Window resize process flag:
     *       BOOL IsSizing;
     * RETURNS: None.
     */
    virtual VOID Resize( BOOL IsSizing ) {}

    /* Timer callback.
     * ARGUMENTS:
     *   - Timer index (default: 0):
     *       UINT Id;
     * RETURNS: None.
     */
    virtual VOID Timer( UINT Id = 0 ) {}

    /* Window activation callback.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Activate( VOID ) {}

    /* Idle task callback.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    virtual VOID Idle( VOID ) {}

    /* Background erasion callback.
     * ARGUMENTS:
     *   - Draw context:
     *       HDC hDC;
     * RETURNS: None.
     */
    virtual VOID Erase( HDC hDC ) {}

    /* Paint callback.
     * ARGUMENTS:
     *   - Draw context:
     *       HDC hDC;
     * RETURNS: None.
     */
    virtual VOID Paint( HDC hDC ) {}

    BOOL IsFullScreen;       /* Full screen state flag */
    RECT FullScreenSaveRect; /* Screen size before flip state */

    /* Full screen mode switch function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    VOID FlipFullScreen( VOID );

  public:
    INT MouseWheel; /* Mouse wheel agregation variable */
    BOOL IsActive;  /* Window active state flag */

    /* Class constructor
     * ARGUMENTS:
     *   - Application instance handle:
     *       HINSTANCE hInst;
     */
    win( HINSTANCE hInst = GetModuleHandle(NULL) );

    /* Window creating function.
     * ARGUMENTS:
     *   - Window class name and name:
     *       const CHAR *ClassName, const CHAR *WindowName;
     * RETURNS:
     *   (BOOL) Success flag.
     */
    BOOL Create( const CHAR *ClassName, const CHAR *WindowName );

    /* Class virtual destructor */
    virtual ~win( VOID );

    /* Mainloop start function.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    VOID Run( VOID );
  }; /* end of 'win' class */
} /* end of 'win' namespace */

#endif /* __win_h__ */

 /* END OF 'win.h' FILE */
