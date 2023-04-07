/* FILE NAME   : 'keyboard.h'
 * PURPOSE     : Input system keyboard part handle file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 27.10.2022.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __keyboard_h_
#define __keyboard_h_

#include <def.h>

namespace prj
{
  /* Keyboard input class */
  class keyboard
  {
  public:
    /* Keys arrays */
    BYTE Keys[0x100] {};
    BYTE KeysOld[0x100] {};
    BYTE KeysClick[0x100] {};
    BYTE KeysUnclick[0x100] {};

    /* Default constructor */
    keyboard( VOID )
    {
    } /* End of constructor */

    /* Responce function */
    VOID KeyboardResponce( VOID )
    {
      GetKeyboardState(Keys);

      for (INT i = 0; i < 0x100; i += 1)
      {
        Keys[i] >>= 7;
        KeysClick[i] = Keys[i] && !KeysOld[i];
        KeysUnclick[i] = !Keys[i] && KeysOld[i];
        KeysOld[i] = Keys[i];
      }
    } /* End of 'KeyboardResponce' function */
  }; /* end of 'keyboard' class */
} /* end of 'prj' namespace */

#endif /* __keyboard_h_ */

 /* END OF 'keyboard.h' FILE */
