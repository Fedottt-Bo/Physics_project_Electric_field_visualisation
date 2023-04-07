/* FILE NAME   : 'ef_force_lines.cpp'
 * PURPOSE     : Physics module.
 *               Electric field force lines evaluation class implementation file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 07.04.2023.
 * NOTE        : Module namespace 'prj::phys'.
 */

#include <pch.h>

#include "ef_force_lines.h"

using namespace prj::phys;

/* Force evaluation function
 * ARGUMENTS:
 *    - Position:
 *        __m128d PosVec;
 * RETURNS:
 *   (__m128d) Force vector.
 */
__m128d __vectorcall ef_force_line::EvalForce( __m128d PosVec )
{
  __m128d Res = _mm_setzero_pd();

  for (const auto &Elm : Charges)
  {
    auto Dir = _mm_sub_pd(PosVec, _mm_load_pd((dbl *)&Elm.Coord));

    auto Len = _mm_mul_pd(Dir, Dir);
    Len = _mm_hadd_pd(Len, Len);

    dbl SqrLength;
    _mm_store_sd(&SqrLength, Len);

    auto DistCorr = _mm_mul_pd(_mm_mul_pd(Len, Len), Len);
    DistCorr = _mm_sqrt_pd(DistCorr);

    auto Val = _mm_mul_pd(_mm_set1_pd(Elm.Charge), Dir);
    Val = _mm_div_pd(Val, DistCorr);

    Res = _mm_add_pd(Res, Val);
  }

  return Res;
} /* End of 'ef_force_line::EvalForce' function */

/* Next point evaluation function.
 * RETURNS:
 *   (coordf) Next coordinate.
 */
coordf ef_force_line::Next1( void )
{
  auto Pos {_mm_load_pd(this->Pos)};

  auto Offset = EvalForce(Pos);
  auto TmpOffset = _mm_mul_pd(Offset, Offset);
  Offset = _mm_div_pd(Offset, _mm_sqrt_pd(_mm_hadd_pd(TmpOffset, TmpOffset)));

  Pos = _mm_add_pd(Pos, _mm_mul_pd(Offset, _mm_set1_pd(0.30)));
  _mm_store_pd(this->Pos, Pos);

  coordf Tmp;
  _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

  return Tmp;
} /* End of 'ef_force_line::Next1' function */

/* Next point evaluation function.
 * RETURNS:
 *   (coordf) Next coordinate.
 */
coordf ef_force_line::Next2( void )
{
  auto Pos {_mm_load_pd(this->Pos)};

  if (Continue)
  {
    const auto HalfPack {_mm_load_pd(this->HalfPack)};
    const auto Rev3 {_mm_load_pd(this->Rev3)};
    const auto Rev6 {_mm_load_pd(this->Rev6)};

    auto Offset1 = EvalForceLen(Pos);

    auto Offset2 = EvalForceLen(_mm_fmadd_pd(Offset1, HalfPack, Pos));
    auto Offset3 = EvalForceLen(_mm_fmadd_pd(Offset2, HalfPack, Pos));
    auto Offset4 = EvalForceLen(_mm_add_pd(Offset3, Pos));

    auto Offset = _mm_fmadd_pd(Offset4, Rev6,
                               _mm_fmadd_pd(Offset3, Rev3,
                                            _mm_fmadd_pd(Offset2, Rev3,
                                                         _mm_mul_pd(Offset1, Rev6))));

    Pos = _mm_add_pd(Offset, Pos);

    for (const auto &Elm : Charges)
    {
      if ((*(UINT64 *)&Elm.Charge) & (1ui64 << 63))
      {
        auto Dir = _mm_sub_pd(Pos, _mm_load_pd((dbl *)&Elm.Coord));

        auto Len = _mm_mul_pd(Dir, Dir);
        Len = _mm_hadd_pd(Len, Len);

        auto Size = _mm_set_sd(Elm.Size * 2.0);
        Size = _mm_mul_sd(Size, Size);

        if (_mm_comile_sd(Len, Size))
        {
          Continue = false;
          Pos = _mm_load_pd((dbl *)&Elm.Coord);
          break;
        }
      }
    }

    _mm_store_pd(this->Pos, Pos);
  }

  coordf Tmp;
  _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

  return Tmp;
} /* End of 'ef_force_line::Next2' function */

/* Next point evaluation function.
 * RETURNS:
 *   (coordf) Next coordinate.
 */
coordf ef_force_line::Next3( void )
{
  auto Pos {_mm_load_pd(this->Pos)};

  if (Continue)
  {
    const auto LengthPack {_mm_load_pd(this->LengthPack)};
    const auto HalfPack {_mm_load_pd(this->HalfPack)};
    const auto Rev3 {_mm_load_pd(this->Rev3)};
    const auto Rev6 {_mm_load_pd(this->Rev6)};

    auto Offset1 = EvalForceNormLen(Pos);

    auto Offset2 = EvalForceNormLen(_mm_fmadd_pd(Offset1, HalfPack, Pos));
    auto Offset3 = EvalForceNormLen(_mm_fmadd_pd(Offset2, HalfPack, Pos));
    auto Offset4 = EvalForceNormLen(_mm_add_pd(Offset3, Pos));

    auto Offset = _mm_fmadd_pd(Offset4, Rev6,
                               _mm_fmadd_pd(Offset3, Rev3,
                                            _mm_fmadd_pd(Offset2, Rev3,
                                                         _mm_mul_pd(Offset1, Rev6))));

    auto OffsetSqr = _mm_mul_pd(Offset, Offset);
    Offset = _mm_div_pd(_mm_mul_pd(Offset, LengthPack), _mm_sqrt_pd(_mm_hadd_pd(OffsetSqr, OffsetSqr)));

    Pos = _mm_add_pd(Offset, Pos);

    for (const auto &Elm : Charges)
    {
      if ((*(UINT64 *)&Elm.Charge) & (1ui64 << 63))
      {
        auto Dir = _mm_sub_pd(Pos, _mm_load_pd((dbl *)&Elm.Coord));

        auto Len = _mm_mul_pd(Dir, Dir);
        Len = _mm_hadd_pd(Len, Len);

        auto Size = _mm_set_sd(Elm.Size * 2.0);
        Size = _mm_mul_sd(Size, Size);

        if (_mm_comile_sd(Len, Size))
        {
          Continue = false;
          Pos = _mm_load_pd((dbl *)&Elm.Coord);
          break;
        }
      }
    }

    _mm_store_pd(this->Pos, Pos);
  }

  coordf Tmp;
  _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

  return Tmp;
} /* End of 'ef_force_line::Next3' function */

/* Next point evaluation function.
 * RETURNS:
 *   (coordf) Next coordinate.
 */
coordf ef_force_line::Next4( void )
{
  auto Pos {_mm_load_pd(this->Pos)};

  if (Continue)
  {
    const auto HalfPack {_mm_load_pd(this->HalfPack)};
    const auto Rev3 {_mm_load_pd(this->Rev3)};
    const auto Rev6 {_mm_load_pd(this->Rev6)};

    auto Offset1 = EvalForceNormLen(Pos);

    auto Offset2 = EvalForceNormLen(_mm_fmadd_pd(Offset1, HalfPack, Pos));
    auto Offset3 = EvalForceNormLen(_mm_fmadd_pd(Offset2, HalfPack, Pos));
    auto Offset4 = EvalForceNormLen(_mm_add_pd(Offset3, Pos));

    auto Offset = _mm_fmadd_pd(Offset4, Rev6,
                               _mm_fmadd_pd(Offset3, Rev3,
                                            _mm_fmadd_pd(Offset2, Rev3,
                                                         _mm_mul_pd(Offset1, Rev6))));

    Pos = _mm_add_pd(Offset, Pos);

    for (const auto &Elm : Charges)
    {
      if ((*(UINT64 *)&Elm.Charge) & (1ui64 << 63))
      {
        auto Dir = _mm_sub_pd(Pos, _mm_load_pd((dbl *)&Elm.Coord));

        auto Len = _mm_mul_pd(Dir, Dir);
        Len = _mm_hadd_pd(Len, Len);

        auto Size = _mm_set_sd(Elm.Size * 2.0);
        Size = _mm_mul_sd(Size, Size);

        if (_mm_comile_sd(Len, Size))
        {
          Continue = false;
          Pos = _mm_load_pd((dbl *)&Elm.Coord);
          break;
        }
      }
    }

    _mm_store_pd(this->Pos, Pos);
  }

  coordf Tmp;
  _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

  return Tmp;
} /* End of 'ef_force_line::Next4' function */

/* END OF 'ef_force_lines.cpp' FILE */
