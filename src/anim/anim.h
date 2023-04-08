/* FILE NAME   : 'anim.h'
 * PURPOSE     : Animation module header file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 07.04.2023.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __anim_h__
#define __anim_h__

#include <def.h>

#include "win/win.h"
#include "render/render.h"
#include "input/input.h"

#include "utility/physics/ef_force_lines.h"
#include "utility/threads_pool/threads_pool.hpp"

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

    /* Charge at position selecting
     * ARGUMENTS:
     *   - Charge position:
     *       coordd Coord;
     * RETURNS:
     *   (bool) true if found.
     */
    bool SelectCharge( coordd Coord );

    /* Charge at position selecting, otherwise adding function
     * ARGUMENTS:
     *   - Charge position:
     *       coordd Coord;
     */
    void SelectAddCharge( coordd Coord );

    /* Reevaluation and redrawing flags */
    BOOL Reeval {FALSE}, Redraw {TRUE};

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
      void Apply( void );
    }; /* end of 'eval_settings' class */

    /* Charges pool */
    std::list<phys::charge> Charges {};

    /* Current selected charge */
    phys::charge *SelectedCharge {nullptr};

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

    /* Line threaded evaluation data */
    struct thread_data
    {
      phys::ef_force_line LineEval;
      std::vector<coordf> *LineData;

      /* Default constructor */
      thread_data( void ) = default;

      /* Constructor from data */
      thread_data( phys::ef_force_line &&Line, std::vector<coordf> *LinePts ) :
        LineEval {Line}, LineData {LinePts}
      { }
    }; /* end of 'thread_data' structure */

    util::threads_pool<thread_data, 8> ThreadsPool;

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
