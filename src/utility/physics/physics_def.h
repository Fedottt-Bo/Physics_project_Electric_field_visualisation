/* FILE NAME   : 'physics_def.h'
 * PURPOSE     : Physics module.
 *               Basic definitions handle file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 07.04.2023.
 * NOTE        : Module namespace 'prj::phys'.
 */

#ifndef __physics_def_h__
#define __physics_def_h__

#include <def.h>

/* Project namespace // Physics module */
namespace prj::phys
{
  /* Electric charge data structure */
  struct charge
  {
    coordd Coord;
    dbl Charge, Size;
    std::vector<std::vector<coordf>> Lines {};
  }; /* end of 'charge' structure */
} /* end of 'prj::phys' namespace */

#endif /* __physics_def_h__ */

/* END OF 'physics_def.h' FILE */
