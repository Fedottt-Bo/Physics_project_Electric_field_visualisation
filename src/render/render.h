/* FILE NAME   : 'render.h'
 * PURPOSE     : Render module header file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 07.04.2023.
 * NOTE        : Module namespace 'prj'.
 */

#ifndef __render_h__
#define __render_h__

/* Direct2D headers */
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

/* Smart pointers */
#include <wrl.h>
using Microsoft::WRL::ComPtr;

/* Project namespace */
namespace prj
{
  /* Render class */
  class render
  {
  private:
    /* Window handle */
    HWND hWnd {};

    /* Direct2D factories */
    ComPtr<ID2D1Factory> Factory {};
    ComPtr<IDWriteFactory> WriteFactory {};

    /* Render target */
    ComPtr<ID2D1HwndRenderTarget> RenderTarget {};

    /* First resize flag */
    BOOL FirstResize {true};

    /* Color brushes */
    ComPtr<ID2D1SolidColorBrush>
      ColorBrushLines {},
      ColorBrushLineDirs {},
      ColorBrushPosCharge {},
      ColorBrushNegCharge {};

    /* Text font */
    ComPtr<IDWriteTextFormat> TextFmt {};
    flt FontSize = 102.f / GetDpiForSystem();

    /* Lines geometry store */
    ComPtr<ID2D1PathGeometry> LinesGeom {}, LineDirsGeom {};

    /* Screen size */
    INT Width {1}, Height {1};

  public:
    /* Default constructor */
    render( void );

    /* Default destructor */
    ~render( void )
    {
      // No manual deletion
    } /* End of destructor */

    /* Initialization function
     * ARGUMENTS:
     *   - Window handle:
     *       HWND hWnd;
     *   - Instance handle (default: GetModuleHandleA(NULL)):
     *       HINSTANCE hInst;
     */
    void Init( HWND hWnd, HINSTANCE hInst = GetModuleHandleA(NULL) )
    {
      /* Just store window handle */
      this->hWnd = hWnd;
    } /* End of 'Init' function */

    /* Resize callback
     * ARGUMENTS:
     *   - Size:
     *       INT W, H;
     */
    void Resize( INT W, INT H );

    /* Lines data update function.
     * ARGUMENTS:
     *   - Lines data:
     *       const std::vector<std::pair<const coordf *, size_t>> &Lines;
     *   - Directions drawing flag (default: false):
     *       bool DrawDirs;
     */
    void UpdateData( const std::vector<std::pair<const coordf *, size_t>> &Lines, bool DrawDirs = false );

    /* Render function
     * ARGUMENTS:
     *   - Charges positions and sizes:
     *       const std::pair<std::pair<coordf, std::pair<flt, flt>> *, size_t> &Charges;
     *   - Logical screen coordinates:
     *       const std::pair<flt, flt> &LeftTop, &RightBottom;
     */
    void Render( const std::pair<std::pair<coordf, std::pair<flt, flt>> *, size_t> &Charges,
                 const std::pair<flt, flt> &LeftTop,
                 const std::pair<flt, flt> &RightBottom );
  };
}

#endif /* __render_h__ */

/* END OF 'render.h' FILE */
