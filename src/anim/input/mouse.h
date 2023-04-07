/* FILE NAME   : 'mouse.h'
 * PURPOSE     : Input system mouse part handle file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 26.03.2023.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __mouse_h__
#define __mouse_h__

#include <def.h>

/* Project namespace */
namespace prj
{
  /* Mouse input class */
  class mouse
  {
  private:
    /* Window handle */
    HWND &hWnd;

    /* Mouse wheel acumulator handle */
    INT &MouseWheel;

  public:
    /* Mouse move parameters */
    dbl
      Mx {0.0}, My {0.0}, Mz {0.0},
      Mdx {0.0}, Mdy {0.0}, Mdz {0.0};

    /* Default constructor */
    mouse( HWND &hWnd, INT & MouseWheel ) :
      hWnd(hWnd), MouseWheel(MouseWheel)
    { }

    /* Responce function */
    VOID MouseResponce( VOID )
    {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(hWnd, &pt);

      Mdx = pt.x - Mx;
      Mdy = pt.y - My;

      Mx += Mdx;
      My += Mdy;

      Mdz = MouseWheel / 120.0;
      Mz += Mdz;
      MouseWheel = 0;
    } /* End fo 'MouseResponce' function */
  }; /* end of 'mouse' class */
} /* end of 'prj' namespace */

#endif /* __mouse_h__ */

 /* END OF 'mouse.h' FILE */
