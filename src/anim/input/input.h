/* FILE NAME   : 'input.h'
 * PURPOSE     : Input system collection handle file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 27.10.2022.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __input_h__
#define __input_h__

#include <def.h>

/* Project namespace */
namespace prj
{
  /* Input collection class forward declaration */
  class input;
} /* end of 'prj' namespace */

#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"

/* Project namespace */
namespace prj
{
  /* Input collection class */
  class input : public keyboard, public mouse, public joystick
  {
  public:
    /* Default constructor
     * ARGUMENTS:
     *   - Window handle reference:
     *       HWND &hWnd;
     *   - Mouse wheel accumulator reference:
     *       INT &MouseWheel;
     */
    input( HWND &hWnd, INT &MouseWheel ) :
      keyboard {}, mouse {hWnd, MouseWheel}, joystick {}
    { }

    /* Whole input responce function */
    VOID InputResponce( VOID )
    {
      KeyboardResponce();
      MouseResponce();
      JoystickResponce();
    } /* End of 'InputResponce' function */
  }; /* end of 'input' class */
} /* end of 'prj' namespace */

#endif /* __input_h_ */

 /* END OF 'input.h' FILE */
