/* FILE NAME   : 'anim.h'
 * PURPOSE     : Animation module header file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 25.04.2023.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __anim_h__
#define __anim_h__

#include <def.h>

#include "win/win.h"
#include "render/render.h"
#include "input/input.h"

/* Project namespace */
namespace prj
{
  /* Animation class */
  class anim : protected win::win
  {
  private:
    /* Logical frame data */
    dbl Left {-18}, Top {18}, Right {18}, Bottom {-18};
    INT OldW {1024}, OldH {1024};

    /* Initialization flag */
    bool WasInit = false;

    /* Minimal charge and charge visual size parameters */
    dbl
      MinCharge {1.0},
      SizePow {0.47},
      SizeCoeff {0.5},
      MinChargeSize {pow(MinCharge, SizePow) * SizeCoeff};

    /* Evaluations eval_settings */
    dbl LinesPerCharge {6}, LineLengthCoeff {0.18};
    size_t LineEvalLength {2'000};

    /* Scene clearing function */
    void ClearScene( void );

    /* Charge adding function
     * ARGUMENTS:
     *   - Charge position:
     *       coordd Coord;
     */
    void AddCharge( coordd Coord );

    /* Charge at position selecting, otherwise adding function
     * ARGUMENTS:
     *   - Charge position:
     *       coordd Coord;
     */
    void SelectAddCharge( coordd Coord );

    /* Reevaluation flag */
    BOOL Reeval = FALSE;

    /* Reevaluation setting function */
    void SetReevaluation( void );

    /* Interface for changing evaluations eval_settings from the outside */
    friend class eval_settings;
    class eval_settings
    {
    public:
      /* Animation pointer */
      anim *Anim;

      /* Settings copies */
      dbl LinesPerCharge;
      dbl LineLengthCoeff;
      size_t LineEvalLength;

      /* Default constructor */
      eval_settings( anim &Anim ) :
        Anim {&Anim},
        LinesPerCharge {Anim.LinesPerCharge},
        LineLengthCoeff {Anim.LineLengthCoeff},
        LineEvalLength {Anim.LineEvalLength}
      { }

      /* Values updating function */
      void Apply( void )
      {
        if (Anim->LinesPerCharge != LinesPerCharge)
          Anim->LinesPerCharge = LinesPerCharge, Anim->SetReevaluation();

        if (Anim->LineLengthCoeff != LineLengthCoeff)
          Anim->LineLengthCoeff = LineLengthCoeff, Anim->SetReevaluation();

        if (Anim->LineEvalLength != LineEvalLength)
          Anim->LineEvalLength = LineEvalLength, Anim->SetReevaluation();
      } /* End of 'Apply' function */
    }; /* end of 'eval_settings' class */

    /* Charge data structure */
    struct charge
    {
      coordd Coord;
      dbl Charge, Size;
      std::vector<std::vector<coordf>> Lines {};
    }; /* end of 'charge' structure */

    /* Charges pool */
    std::list<charge> Charges {};

    /* Current selected charge */
    charge *SelectedCharge {nullptr};

    /* Input actions state enum */
    enum class input_state : UINT
    {
      None,
      Move,
      Charge,
      Dialog
    } InputState {input_state::None};

    /* Input data field */
    input Input {win::hWnd, win::MouseWheel};

    /* Field line points sequence evaluator */
    friend class field_line;
    class field_line
    {
    private:
      /* Current position */
      __m128d Pos;

      /* Evaluation environment */
      const std::list<charge> &Charges;
      dbl Length;

      /* Auxilary packed data */
      const __m128d
        LengthPack {_mm_set1_pd(Length)},
        HalfPack {_mm_set1_pd(0.5)},
        Rev3 {_mm_set1_pd(1 / 3.0)},
        Rev6 {_mm_set1_pd(1 / 6.0)};

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
      inline __m128d __vectorcall EvalForce( __m128d PosVec )
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
      } /* End of 'EvalForce' function */

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

        return _mm_mul_pd(Force, LengthPack);
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

        return _mm_div_pd(_mm_mul_pd(Force, LengthPack), _mm_sqrt_pd(ForceLen));
      } /* End of 'EvalForceNormLen' function */

    public:
      field_line( const coordd &BasePos, const anim &Anim ) :
        Pos {_mm_load_pd((dbl *)&BasePos)},
        Charges {Anim.Charges},
        Length {Anim.LineLengthCoeff}
      {
      }

      /* Different implementations of next point getting function */
      /* Next point evaluation function.
       * RETURNS:
       *   (coordf) Next coordinate.
       */
      coordf Next1( void );

      /* Next point evaluation function.
       * RETURNS:
       *   (coordf) Next coordinate.
       */
      coordf Next2( void );

      /* Next point evaluation function.
       * RETURNS:
       *   (coordf) Next coordinate.
       */
      coordf Next3( void );

      /* Next point evaluation function.
       * RETURNS:
       *   (coordf) Next coordinate.
       */
      coordf Next4( void );
    };

    /* Line threaded evaluation data */
    struct trhead_data
    {
      field_line LineEval;
      std::vector<coordf> *LineData;
    }; /* end of 'trhead_data' structure */

    /* Single thread data */
    struct eval_trhead
    {
      std::vector<trhead_data *> Data;
      std::thread *Thread;
      bool Run {true};
    }; /* end of 'eval_trhead' structure */

    /* Threads pool */
    std::vector<trhead_data> ThreadsDataBulk;
    std::vector<eval_trhead> ThreadsPool;

    /* Lines data by threads update flag */
    bool ThreadsDataUpdated = false;

    /* Renderer class */
    render Renderer;

  public:
    /* Animation default constructor */
    anim( void );

    /* Animation default destructor */
    ~anim( void ) final;

    /* No copy/move constructor */
    anim( const anim & ) = delete;
    anim( anim && ) = delete;

  protected:
    /* Window creation callback.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Init( void ) final;

    /* Window closing callback.
     * ARGUMENTS: None.
     * RETURNS:
     *   (BOOL) Window closing allow flag.
     */
    BOOL Close( void ) final;

    /* Resize callback.
     * ARGUMENTS:
     *   - Window resize process flag:
     *       BOOL IsSizing;
     * RETURNS: None.
     */
    void Resize( BOOL ) final;

    /* Background erasion callback.
     * ARGUMENTS:
     *   - Draw context:
     *       HDC hDC;
     * RETURNS: None.
     */
    void Erase( HDC ) final;

    /* Paint callback.
     * ARGUMENTS:
     *   - Draw context:
     *       HDC hDC;
     * RETURNS: None.
     */
    void Paint( HDC ) final;

    /* Timer callback.
     * ARGUMENTS:
     *   - Timer index (default: 0):
     *       UINT Id;
     * RETURNS: None.
     */
    void Timer( UINT ) final { Responce(); }

    /* Idle task callback.
     * ARGUMENTS: None.
     * RETURNS: None.
     */
    void Idle( void ) final { Responce(); }

    /* WM_COMMAND window message handle function.
     * ARGUMENTS:
     *   - Menu button ID:
     *       UINT Id;
     * RETURNS: None.
     */
    void OnMenuButton( UINT Id ) final;

  private:
    /* Animation state responce function */
    void Responce( void );

    /* Render frequency control */
    UINT64 RenderTime = 0, RenderTimeFreq;

    /* Render function */
    void Render( void );
  }; /* end of 'anim' class */
} /* end of 'prj' namespace */

#endif /* __anim_h__ */

/* END OF 'anim.h' FILE */
