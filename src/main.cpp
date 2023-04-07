/* FILE NAME   : 'main.cpp'
 * PURPOSE     : Entry point file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 27.04.2023.
 */

#include <pch.h>

/* The main program function.
 * ARGUMENTS:
 *   - handle of application instance:
 *       HINSTANCE hInstance;
 *   - dummy handle of previous application instance (not used):
 *       HINSTANCE hPrevInstance;
 *   - command line string:
 *       LPSTR CmdLine;
 *   - show window command parameter (see SW_***):
 *       INT CmdShow;
 * RETURNS:
 *   (INT) Error level for operation system (0 for success).
 */
INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR CmdLine, INT ShowCmd )
{
  /* Debug console */
#ifdef _DEBUG
  AllocConsole();
  SetConsoleTitle("Physics Console");
  EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE,
                 MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

  //freopen("CONERR$", "w", stderr);
  freopen("CONOUT$", "w", stdout);
  freopen("CONIN$", "r", stdin);

  system("@chcp 1251");
  system("cls");
#endif /* !_DEBUG */

  /* Animation (auto run) */
  prj::anim Anim {};

  return 0;
}
