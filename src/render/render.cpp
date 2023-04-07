/* FILE NAME   : 'render.cpp'
 * PURPOSE     : Render module implementation file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 30.04.2023.
 * NOTE        : Module namespace 'prj'.
 */

#include <pch.h>

#include "render.h"

/* Direct2D libraries */
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

/* Project namespace */
namespace prj
{
  /* Default constructor */
  render::render( void )
  {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(Factory.GetAddressOf()));
  
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(*WriteFactory.Get()), (IUnknown **)WriteFactory.GetAddressOf());
  
    WriteFactory->CreateTextFormat(L"Consolas", nullptr, DWRITE_FONT_WEIGHT_REGULAR,
                                   DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_SEMI_CONDENSED,
                                   FontSize, L"", TextFmt.GetAddressOf());
    TextFmt->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    TextFmt->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
  } /* End of constructor */
  
  /* Resize callback
   * ARGUMENTS:
   *   - Size:
   *       INT W, H;
   */
  void render::Resize( INT W, INT H )
  {
    Width = W, Height = H;
  
    if (RenderTarget)
      RenderTarget->Release();
  
    D2D1_RENDER_TARGET_PROPERTIES Info
    {
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      {DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED},
      0.f, 0.f,
      D2D1_RENDER_TARGET_USAGE_NONE,
      D2D1_FEATURE_LEVEL_DEFAULT,
    };
    D2D1_HWND_RENDER_TARGET_PROPERTIES Info2
    {
      hWnd,
      D2D1_SIZE_U {(UINT32)W, (UINT32)H},
      D2D1_PRESENT_OPTIONS_NONE
    };
  
    Factory->CreateHwndRenderTarget(&Info, &Info2, &RenderTarget);
  
    /* Create some resources once */
    if (FirstResize)
    {
      FirstResize = false;
  
      RenderTarget->CreateSolidColorBrush(D2D1_COLOR_F {1.f, 1.f, 0.f, 1.f}, ColorBrushLines.GetAddressOf());
      RenderTarget->CreateSolidColorBrush(D2D1_COLOR_F {0.f, 1.f, 0.f, 1.f}, ColorBrushLineDirs.GetAddressOf());
      RenderTarget->CreateSolidColorBrush(D2D1_COLOR_F {1.f, 0.f, 0.f, 1.f}, ColorBrushPosCharge.GetAddressOf());
      RenderTarget->CreateSolidColorBrush(D2D1_COLOR_F {0.f, 0.f, 1.f, 1.f}, ColorBrushNegCharge.GetAddressOf());
    }
  } /* End of 'render::Resize' function */
  
  /* Lines data update function.
   * ARGUMENTS:
   *   - Lines data:
   *       const std::vector<std::pair<const coordf *, size_t>> &Lines;
   *   - Directions drawing flag (default: false):
   *       bool DrawDirs;
   */
  void render::UpdateData( const std::vector<std::pair<const coordf *, size_t>> &Lines, bool DrawDirs )
  {
    Factory->CreatePathGeometry(LinesGeom.ReleaseAndGetAddressOf());
  
    LineDirsGeom.Reset();
    if (DrawDirs)
      Factory->CreatePathGeometry(LineDirsGeom.GetAddressOf());
  
    {
      ComPtr<ID2D1GeometrySink> LinesSink {}, LineDirsSink {};

      LinesGeom->Open(LinesSink.GetAddressOf());
      if (DrawDirs)
        LineDirsGeom->Open(LineDirsSink.GetAddressOf());
  
      for (auto &Elm : Lines)
      {
        if (Elm.first == nullptr || Elm.second < 2)
          continue;
  
        LinesSink->BeginFigure(*(D2D1_POINT_2F *)Elm.first, D2D1_FIGURE_BEGIN_HOLLOW);
        LinesSink->AddQuadraticBeziers((D2D1_QUADRATIC_BEZIER_SEGMENT *)(Elm.first + 1), (Elm.second - 1) >> 1);
        if ((Elm.second & 1) == 0)
          LinesSink->AddLine(((D2D1_POINT_2F *)Elm.first)[Elm.second - 1]);
  
        LinesSink->EndFigure(D2D1_FIGURE_END_OPEN);
  
        if (DrawDirs)
        {
          const size_t DirFreq {std::clamp<size_t>(roundf(powf(Elm.second, .8f)), 2, 50)};
  
          for (size_t i {(DirFreq >> 1) + 1}; i < (Elm.second - 2); i += DirFreq)
          {
            __m128 Points {_mm_load_ps((flt *)(Elm.first + i - 1))};
            auto Dir = _mm_permute_ps(Points, 0b11'01'10'00);
            Dir = _mm_hsub_ps(Dir, Dir);
  
            auto DirLen = _mm_mul_ps(Dir, Dir);
            DirLen = _mm_hadd_ps(DirLen, DirLen);
            DirLen = _mm_rsqrt_ps(DirLen);
  
            Dir = _mm_mul_ps(Dir, DirLen);
            Dir = _mm_mul_ps(Dir, _mm_setr_ps(.6f, .6f, .2f, .2f));
  
            auto Point0 {_mm_permute_ps(Points, 0b01'00'01'00)};
            Point0 = _mm_add_ps(Point0, _mm_permute_ps(Dir, 0b01'00'01'00));
  
            Point0 = _mm_addsub_ps(_mm_permute_ps(Point0, 0b10'11'01'00),
                                   _mm_permute_ps(Dir, 0b11'10'10'11));
            Point0 = _mm_permute_ps(Point0, 0b10'11'01'00);
  
            coordf Coords[2];
            _mm_store_ps((flt *)Coords, Point0);
  
            LineDirsSink->BeginFigure({Elm.first[i - 1].X * .8f + Elm.first[i].X * .2f,
                                       Elm.first[i - 1].Y * .8f + Elm.first[i].Y * .2f}, D2D1_FIGURE_BEGIN_FILLED);
            LineDirsSink->AddLine({Coords[0].X, Coords[0].Y});
            LineDirsSink->AddLine({Coords[1].X, Coords[1].Y});
            LineDirsSink->EndFigure(D2D1_FIGURE_END_CLOSED);
          }
        }
      }

      LinesSink->Close();
      if (DrawDirs)
        LineDirsSink->Close();
    }
  } /* End of 'render::UpdateData' function */
  
  /* Render function
   * ARGUMENTS:
   *   - Charges positions and sizes:
   *       const std::pair<std::pair<coordf, std::pair<flt, flt>> *, size_t> &Charges;
   *   - Logical screen coordinates:
   *       const std::pair<flt, flt> &LeftTop, &RightBottom;
   */
  void render::Render( const std::pair<std::pair<coordf, std::pair<flt, flt>> *, size_t> &Charges,
                       const std::pair<flt, flt> &LeftTop,
                       const std::pair<flt, flt> &RightBottom )
  {
    if (RenderTarget == nullptr)
      return;
  
    RenderTarget->BeginDraw();
    RenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    RenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
  
    D2D1_MATRIX_3X2_F GeomTransform {}, TextTransform {};
    {
      const auto [W, H] {RenderTarget->GetSize()};
      auto [Left, Top] {LeftTop};
      auto [Right, Bottom] {RightBottom};
  
      Right -= Left;
      Top -= Bottom;
  
      GeomTransform.m11 = (flt)Width / Right;
      GeomTransform.m22 = (flt)Height / Top;
      GeomTransform.dx = -Left * GeomTransform.m11;
      GeomTransform.dy = -Bottom * GeomTransform.m22;
  
      TextTransform.m11 = W / Right;
      TextTransform.m22 = H / Top;
      TextTransform.dx = -Left * TextTransform.m11;
      TextTransform.dy = -Bottom * TextTransform.m22;
    }
  
    RenderTarget->SetTransform(GeomTransform);
    RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
  
    /* Draw lines */
    if (LinesGeom)
      RenderTarget->DrawGeometry(LinesGeom.Get(), ColorBrushLines.Get(), .13f);
  
    if (LineDirsGeom)
      RenderTarget->FillGeometry(LineDirsGeom.Get(), ColorBrushLineDirs.Get());
  
    /* Draw charges */
    {
      for (size_t i = 0; i < Charges.second; i++)
      {
        auto [X, Y] {Charges.first[i].first};
        auto [Charge, Size] {Charges.first[i].second};
  
        auto Brush {(Charge > 0) ? ColorBrushPosCharge : ColorBrushNegCharge};
  
        RenderTarget->SetTransform(GeomTransform);
        RenderTarget->FillEllipse({{X, Y}, Size, Size}, Brush.Get());
  
        auto TmpStr {std::to_wstring(Charge)};
        TmpStr = TmpStr.substr(0, TmpStr.find('.') + 2 + 1);
  
        RenderTarget->SetTransform(TextTransform);
        RenderTarget->DrawTextA(TmpStr.c_str(), TmpStr.size(), TextFmt.Get(),
                                {X - 8, Y - Size - 1, X + 8, Y - Size}, Brush.Get());
      }
    }
  
    RenderTarget->EndDraw();
  } /* End of 'render::Render' function */
} /* end of 'prj' namespace */

/* END OF 'render.cpp' FILE */
