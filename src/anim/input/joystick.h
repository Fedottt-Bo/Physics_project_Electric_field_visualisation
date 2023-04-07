/* FILE NAME   : 'joystick.h'
 * PURPOSE     : Input system joystick part handle file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 27.10.2022.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __joystick_h__
#define __joystick_h__

#include <def.h>

/* Joystick API */
#include <mmsystem.h>
#pragma comment(lib, "winmm")

/* Project namespace */
namespace prj
{
  /* Joystick input class */
  class joystick
  {
  public:
    /* Buttons */
    BYTE JBut[32] {}, JButOld[32] {}, JButClick[32] {};

    /* Auxilary value */
    INT JPov {0};

    /* Dimentions */
    dbl JX {0.0}, JY {0.0}, JZ {0.0}, JR {0.0};

    /* Default constructor */
    joystick( VOID ) {}

    /* Input responce function */
    VOID JoystickResponce( VOID )
    {
      /* Check joysticks count is >= 1 */
      if (joyGetNumDevs() > 0)
      {
        JOYCAPS jc;

        if (joyGetDevCaps(JOYSTICKID1, &jc, sizeof(jc)) == JOYERR_NOERROR)
        {
          JOYINFOEX ji;

          ji.dwSize = sizeof(ji);
          ji.dwFlags = JOY_RETURNALL;

          if (joyGetPosEx(JOYSTICKID1, &ji) == JOYERR_NOERROR)
          {
            for (INT i = 0; i < 32; i++)
            {
              JBut[i] = (ji.dwButtons >> i) & 1;
              JButClick[i] = JBut[i] & !JButOld[i];
              JButOld[i] = JBut[i];
            }

            JX = (2.0 * (ji.dwXpos - jc.wXmin) / (jc.wXmax - jc.wXmin) - 1);
            JY = (2.0 * (ji.dwYpos - jc.wYmin) / (jc.wYmax - jc.wYmin) - 1);
            JZ = (2.0 * (ji.dwZpos - jc.wZmin) / (jc.wZmax - jc.wZmin) - 1);
            JR = (2.0 * (ji.dwRpos - jc.wRmin) / (jc.wRmax - jc.wRmin) - 1);

            JPov = ji.dwPOV == 0xFFFF ? -1 : ji.dwPOV / 4500;
          }
        }
      }
    } /* End of 'JoystickResponce' function */
  }; /* end of 'joystick' class */
} /* end of 'prj' namespace */

#endif /* __joystick_h__ */

 /* END OF 'joystick.h' FILE */
