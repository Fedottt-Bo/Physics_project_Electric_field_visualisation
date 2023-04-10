/* FILE NAME   : 'image_def.h'
 * PURPOSE     : Images work module.
 *               Images common definitions handle file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 10.04.2023.
 * NOTE        : Module namespace 'prj::img'.
 */

#ifndef __image_def_h__
#define __image_def_h__

#include "def.h"

/* WIC includes */
#include <wincodec.h>
#include <wincodecsdk.h>
#include <Shlwapi.h>

/* Project namespace // Images module */
namespace prj::img
{
  /* WIC resources storage class */
  class wic_factory
  {
  public:
    /* WIC component factory */
    ComPtr<IWICImagingFactory> Factory;

  private:
    /* Single tone private constructor */
    wic_factory( void )
    {
      /* Create factory */
      CoInitialize(NULL);
      CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_ALL, IID_PPV_ARGS(&Factory));
    } /* End of constructor */

  public:
    /* Instance getting function */
    static wic_factory &Get( void )
    {
      static wic_factory Inst {};

      return Inst;
    } /* End of 'Get' function */

    /* -> operator useful overloading */
    IWICImagingFactory *operator->( void )
    {
      return Factory.Get();
    } /* End of '->' operator */
  }; /* end of 'wic_factory' class */
} /* end of 'prj::img' namespace */

#endif /* __image_def_h__ */

/* END OF 'image_def.h' FILE */
