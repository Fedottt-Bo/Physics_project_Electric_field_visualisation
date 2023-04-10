/* FILE NAME   : 'ef_force_lines.h'
 * PURPOSE     : Physics module.
 *               Electric field force lines evaluation class handle file
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 10.04.2023.
 * NOTE        : Module namespace 'prj::phys'.
 */

#ifndef __ef_force_lines_h__
#define __ef_force_lines_h__

#include "physics_def.h"

/* Project namespace // Physics module */
namespace prj::phys
{
  /* Field line points sequence evaluator */
  class ef_force_line
  {
  private:
    /* Current position */
    dbl Pos[2];
  
    /* Evaluation environment */
    const std::list<charge> &Charges;
  
    /* Auxilary packed data */
    dbl
      LengthPack[2];

    const dbl
      HalfPack[2] {0.5, 0.5},
      Rev3[2] {1 / 3.0, 1 / 3.0},
      Rev6[2] {1 / 6.0, 1 / 6.0};
  
  public:
    /* Evaluations continuing flag */
    bool Continue {true};
  
  private:
    /* Force evaluation function
     * ARGUMENTS:
     *    - Position:
     *        __m128d PosVec;
     * RETURNS:
     *   (__m128d) Force vector.
     */
    __m128d __vectorcall EvalForce( __m128d PosVec );

    /* Normalized force evaluation function.
     * ARGUMENTS:
     *    - Position:
     *        __m128d PosVec;
     * RETURNS:
     *   (__m128d) Force vector.
     */
    inline __m128d __vectorcall EvalForceNorm( __m128d PosVec )
    {
      auto Force = EvalForce(PosVec);
  
      auto ForceLen = _mm_mul_pd(Force, Force);
      ForceLen = _mm_hadd_pd(ForceLen, ForceLen);
  
      return _mm_div_pd(Force, _mm_sqrt_pd(ForceLen));
    } /* End of 'EvalForceNorm' function */
  
    /* Length multiplied force evaluation function.
     * ARGUMENTS:
     *    - Position:
     *        __m128d PosVec;
     * RETURNS:
     *   (__m128d) Force vector.
     */
    inline __m128d __vectorcall EvalForceLen( __m128d PosVec )
    {
      auto Force = EvalForce(PosVec);
  
      return _mm_mul_pd(Force, _mm_load_pd(LengthPack));
    } /* End of 'EvalForceLen' function */
  
    /* Normalized and length multiplied force evaluation function.
     * ARGUMENTS:
     *    - Position:
     *        __m128d PosVec;
     * RETURNS:
     *   (__m128d) Force vector.
     */
    inline __m128d __vectorcall EvalForceNormLen( __m128d PosVec )
    {
      auto Force = EvalForce(PosVec);
  
      auto ForceLen = _mm_mul_pd(Force, Force);
      ForceLen = _mm_hadd_pd(ForceLen, ForceLen);
  
      return _mm_div_pd(_mm_mul_pd(Force, _mm_load_pd(LengthPack)), _mm_sqrt_pd(ForceLen));
    } /* End of 'EvalForceNormLen' function */

    /* Charges intersection check
     * ARGUMENTS:
     *   - Position:
     *       __m128d Pos;
     * RETURNS:
     *   (__m128d) New position.
     */
    inline __m128d __vectorcall CheckIntersection( __m128d Pos )
    {
      for (const auto &Elm : Charges)
      {
        if ((*(UINT64 *)&Elm.Charge) & (1ui64 << 63))
        {
          const auto ElmCoord {_mm_load_pd((dbl *)&Elm.Coord)};
          auto Dir = _mm_sub_pd(Pos, ElmCoord);

          auto Len = _mm_mul_pd(Dir, Dir);
          Len = _mm_hadd_pd(Len, Len);

          auto Size = _mm_set_sd(Elm.Size * 2.0);
          Size = _mm_mul_sd(Size, Size);

          if (_mm_comile_sd(Len, Size))
          {
            Continue = false;
            Pos = ElmCoord;

            break;
          }
        }
      }

      return Pos;
    } /* End of 'CheckIntersection' function */
  
  public:
    /* Default constructor
     * ARGUMENTS:
     *   - Start position:
     *       const coordd &BasePos;
     *   - Movement length coefficient:
     *       double LengthCoeff;
     *   - Charges pool:
     *        const std::list<charge> &ChargesPool;
     */
    ef_force_line( const coordd &BasePos, double LengthCoeff,
                const std::list<charge> &ChargesPool ) :
      Pos {BasePos.X, BasePos.Y},
      Charges {ChargesPool},
      LengthPack {LengthCoeff, LengthCoeff}
    { }
  
    /* Different implementations of next point getting function */
    /* Next point evaluation function.
     * Standard version.
     * RETURNS:
     *   (coordf) Next coordinate.
     */
    coordf Next1( void );
  
    /* Next point evaluation function.
     * Runge–Kutta method.
     * RETURNS:
     *   (coordf) Next coordinate.
     */
    coordf Next2( void );
  
    /* Next point evaluation function.
     * Runge–Kutta method with post-normalization.
     * RETURNS:
     *   (coordf) Next coordinate.
     */
    coordf Next3( void );
  
    /* Next point evaluation function.
     * Runge–Kutta method with forces normalization.
     * RETURNS:
     *   (coordf) Next coordinate.
     */
    coordf Next4( void );
  }; /* end of 'ef_force_line' class */
} /* end of 'prj::phys' namespace */

#endif /* __ef_force_lines_h__ */

/* END OF 'ef_force_lines.h' FILE */
