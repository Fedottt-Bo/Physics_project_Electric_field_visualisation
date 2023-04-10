/* FILE NAME   : 'anim.cpp'
 * PURPOSE     : Animation module implementation file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 10.04.2023.
 * NOTE        : Module namespace 'prj'.
 */

#include <pch.h>

#include "anim.h"
#include "utility/images/image_save.hpp"

/* Project namespace */
namespace prj
{
  /* Animation default constructor */
  anim::anim( void )
  {
    /* Get time frequency */
    QueryPerformanceFrequency((LARGE_INTEGER *)&RenderTimeFreq);

    /* Configure threads pool */
    ThreadsPool.SetFunction([this]( thread_data *Data ) -> bool
      {
        if (Data->LineData->size() >= Data->LineData->capacity())
          return true;

        Data->LineData->emplace_back(Data->LineEval.Next3());

        if (Data->LineEval.Continue)
        {
          ThreadsDataUpdated = true;
          return false;
        }
        else
          return true;
      });

    /* Launch window */
    win::Create("Very cool animation class for physics.", "Electric Field Visualization");
    win::Run();
  } /* End of constructor */

  /* Animation default destructor */
  anim::~anim( void )
  {
    // Nothing for manual delete
  } /* End of destructor */

  /* Window creation callback.
   * ARGUMENTS: None.
   * RETURNS: None.
   */
  void anim::Init( void )
  {
    Renderer.Init(hWnd, hInstance);
    Resize(FALSE);

    WasInit = true;
  } /* End of 'anim::Init' function */

  /* Resize callback.
   * ARGUMENTS:
   *   - Window resize process flag:
   *       BOOL IsSizing;
   * RETURNS: None.
   */
  void anim::Resize( BOOL )
  {
    if (!WasInit)
      return;

    Renderer.Resize(W, H);

    /* Reeval logical frame */
    {
      const dbl
        MidX {(Left + Right) * 0.5},
        MidY {(Top + Bottom) * 0.5};
      const dbl
        NewW {((Right - Left) * win::W * 0.5) / OldW},
        NewH {((Top - Bottom) * win::H * 0.5) / OldH};

      Left = MidX - NewW, Right = MidX + NewW;
      Bottom = MidY - NewH, Top = MidY + NewH;

      OldW = win::W, OldH = win::H;
    }
  } /* End of 'anim::Resize' function */

  /* Window closing callback.
   * ARGUMENTS: None.
   * RETURNS:
   *   (BOOL) Window closing allow flag.
   */
  BOOL anim::Close( void )
  {
    WasInit = FALSE;

    return TRUE;
  } /* End of 'anim::Close' function */

  /* Scene clearing function */
  void anim::ClearScene( void )
  {
    /* Stop all threads */
    ThreadsPool.Terminate();
    ThreadsDataUpdated = TRUE;

    /* Clear charges pool */
    Charges.clear();
  } /* End of 'anim::ClearScene' function */

  /* Charge adding function
   * ARGUMENTS:
   *   - Charge position:
   *       coordd Coord;
   */
  void anim::AddCharge( coordd Coord )
  {
    Charges.push_back({Coord, MinCharge, MinChargeSize});
    SelectedCharge = &Charges.back();
    InputState = input_state::Charge;

    SetReevaluation();
  } /* End of 'anim::AddCharge' function */

  /* Charge at position selecting
   * ARGUMENTS:
   *   - Charge position:
   *       coordd Coord;
   * RETURNS:
   *   (bool) true if found.
   */
  bool anim::SelectCharge( coordd Coord )
  {
    SelectedCharge = nullptr;

    for (auto &Elm : Charges)
    {
      const auto Size = Elm.Size;
      const dbl
        DX {Elm.Coord.X - Coord.X},
        DY {Elm.Coord.Y - Coord.Y};

      if (DX * DX + DY * DY <= Size)
      {
        SelectedCharge = &Elm;
        break;
      }
    }

    bool Found = SelectedCharge != nullptr;

    InputState = Found ? input_state::Charge : input_state::None;
    return Found;
  }

  /* Charge at position selecting, otherwise adding function
   * ARGUMENTS:
   *   - Charge position:
   *       coordd Coord;
   */
  void anim::SelectAddCharge( coordd Coord )
  {
    if (!SelectCharge(Coord))
      AddCharge(Coord);
  } /* End of 'anim::SelectAddCharge' function */

  /* Reevaluation setting function */
  void anim::SetReevaluation( void )
  {
    /* If not setted before */
    if (!Reeval)
    {
      Reeval = TRUE;

      // Some actions
      ThreadsPool.Terminate();
    }
  } /* End of 'anim::SetReevaluation' function */

  /* Animation state responce function */
  void anim::Responce( void )
  {
    if (!WasInit)
      return;

    /* Check window client area */
    BOOL IsClientArea {false};
    {
      POINT pt;
      GetCursorPos(&pt);

      auto HT = DefWindowProcA(win::hWnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
      if (HT == HTCLIENT)
        IsClientArea = true;
    }

    Input.InputResponce();

    if (Input.Keys[VK_MENU] && Input.KeysClick['\r'])
      FlipFullScreen();

    dbl
      MX {Left + ((Right - Left) * Input.Mx) / win::W},
      MY {Bottom + ((Top - Bottom) * Input.My) / win::H};

    switch (InputState)
    {
    case prj::anim::input_state::None:
      if (IsClientArea)
      {
        if (Input.KeysClick[VK_LBUTTON])
        {
          if (Input.Keys[VK_CONTROL])
            SelectAddCharge({MX, MY});
          else
            SelectCharge({MX, MY});

          break;
        }

        if (Input.KeysClick[VK_RBUTTON])
        {
          InputState = input_state::Move;
          break;
        }
      }

      break;
    case prj::anim::input_state::Move:
      if (Input.KeysUnclick[VK_RBUTTON])
        InputState = input_state::None;
      break;
    case prj::anim::input_state::Charge:
      if (Input.KeysUnclick[VK_LBUTTON])
      {
        SelectedCharge = nullptr;
        InputState = input_state::None;
      }
      break;
    default:
      break;
    }

    switch (InputState)
    {
    case prj::anim::input_state::Move:
      if (Input.Mdx != 0 || Input.Mdy != 0)
      {
        auto XOff {Input.Mdx * (Left - Right) / W};
        auto YOff {Input.Mdy * (Bottom - Top) / H};

        Left += XOff;
        Right += XOff;
        Top += YOff;
        Bottom += YOff;

        Redraw = true;
      }

      [[fallthrough]];
    case prj::anim::input_state::None:
      if (((Input.Mdz > 0 &&
            Right - Left > 10 &&
            Top - Bottom > 10) ||
           (Input.Mdz < 0 &&
            Right - Left < 1000 &&
            Top - Bottom < 1000)) && IsClientArea)
      {
        auto XPosOff {Right - MX};
        auto XNegOff {MX - Left};
        auto YNegOff {MY - Top};
        auto YPosOff {Bottom - MY};

        auto Scale {pow(0.8, Input.Mdz)};

        XPosOff *= Scale;
        XNegOff *= Scale;
        YNegOff *= Scale;
        YPosOff *= Scale;

        Right = MX + XPosOff;
        Left = MX - XNegOff;
        Top = MY - YNegOff;
        Bottom = MY + YPosOff;

        Redraw = true;
      }

      break;
    case prj::anim::input_state::Charge:
      if (Input.KeysClick[VK_DELETE])
      {
        Charges.remove_if([&]( const phys::charge &Ref ) -> bool { return &Ref == SelectedCharge; });

        SelectedCharge = nullptr;
        InputState = input_state::None;

        Reeval = TRUE;
        break;
      }

      if (Input.Mdz)
      {
        auto Old {SelectedCharge->Charge}, New {Old + Input.Mdz * 0.25};
        const auto Min {std::copysign(MinCharge, Old)};

        if (std::signbit(Old) != std::signbit(New - Min))
          New = -Min;

        if (Old != New)
        {
          SelectedCharge->Charge = New;
          SelectedCharge->Size = pow(abs(New), SizePow) * SizeCoeff;

          Reeval = TRUE;
        }
      }

      if (MX != SelectedCharge->Coord.X ||
          MY != SelectedCharge->Coord.Y)
      {
        if (Input.Keys['A'])
          SelectedCharge->Coord = {round(MX * 1.) * 1.,
                                   round(MY * 1.) * 1.};
        else
          SelectedCharge->Coord = {MX, MY};

        Reeval = TRUE;
      }
      break;
    default:
      break;
    }

    if (Reeval)
    {
      /* Stop all threads */
      ThreadsPool.Terminate();

      /* Init lines */
      for (auto &Elm : Charges)
      {
        Elm.Lines.clear();

        if (Elm.Charge < 0)
          continue;

        dbl CntF {round(LinesPerCharge * abs(Elm.Charge))};
        size_t Cnt {(size_t)CntF};
        Elm.Lines.reserve(Cnt);

        for (int i = 0; i < Cnt; i++)
        {
          dbl Angle {(M_PI * (i << 1)) / CntF};
          coordd Base {Elm.Coord.X + Elm.Size * cos(Angle) * 2.0,
                       Elm.Coord.Y + Elm.Size * sin(Angle) * 2.0};

          auto &Line {Elm.Lines.emplace_back()};

          Line.reserve(LineEvalLength);
          Line.push_back(coordf {(flt)Elm.Coord.X, (flt)Elm.Coord.Y});
          Line.push_back(coordf {(flt)Base.X, (flt)Base.Y});

          ThreadsPool.AddTask(phys::ef_force_line {Base, LineLengthCoeff, Charges}, &Line);
        }
      }

      /* Start threads */
      ThreadsPool.Run();

      ThreadsDataUpdated = true;
      Redraw = true;
    }

    Reeval = FALSE;

    /* Not more 24 frames per second can be rendered */
    UINT64 Time;
    QueryPerformanceCounter((LARGE_INTEGER *)&Time);

    Redraw = Redraw || ThreadsDataUpdated;

    if (24 * (Time - RenderTime) >= RenderTimeFreq)
    {
      Render();

      Redraw = FALSE;
      RenderTime = Time;
    }
  } /* End of 'anim::Responce' function */

  /* Render function */
  void anim::Render( void )
  {
    if (!WasInit)
      return;

    /* Update lines data */
    if (ThreadsDataUpdated)
    {
      ThreadsDataUpdated = false;

      std::vector<std::pair<const coordf *, size_t>> Lines;

      for (const auto &ChargeData : Charges)
        for (auto &Line : ChargeData.Lines)
          Lines.emplace_back(Line.data(), Line.size());

      /* Send update */
      Renderer.UpdateData(Lines, true);
    }

    std::vector<std::pair<coordf, std::pair<flt, flt>>> ChargesBulk {};

    ChargesBulk.reserve(Charges.size());
    for (const auto &ChargeData : Charges)
    {
      const auto &Pos {ChargeData.Coord};
      const auto &Size {ChargeData.Size};
      const auto &Charge {ChargeData.Charge};

      ChargesBulk.emplace_back(coordf {(flt)Pos.X, (flt)Pos.Y}, std::make_pair(Charge, Size));
    }

    std::pair<std::pair<coordf, std::pair<flt, flt>> *, size_t>
      TmpCharges {ChargesBulk.data(), ChargesBulk.size()};

    /* Call renderer */
    Renderer.Render(TmpCharges, {(flt)Left, (flt)Top}, {(flt)Right, (flt)Bottom});
  } /* End of 'anim::Render' function */

  /* Background erasion callback.
   * ARGUMENTS:
   *   - Draw context:
   *       HDC hDC;
   * RETURNS: None.
   */
  void anim::Erase( HDC hDC )
  {
    // No action - all drawing in render
  } /* End of 'anim::Erase' function */

  /* Paint callback.
   * ARGUMENTS:
   *   - Draw context:
   *       HDC hDC;
   * RETURNS: None.
   */
  void anim::Paint( HDC hDC )
  {
    // No action - all drawing in render
  } /* End of 'anim::Paint' function */

  /* Values updating function */
  void anim::eval_settings::Apply( void )
  {
    if (Anim->LinesPerCharge != LinesPerCharge)
      Anim->LinesPerCharge = LinesPerCharge, Anim->SetReevaluation();

    if (Anim->LineLengthCoeff != LineLengthCoeff)
      Anim->LineLengthCoeff = LineLengthCoeff, Anim->SetReevaluation();

    if (Anim->LineEvalLength != LineEvalLength)
      Anim->LineEvalLength = LineEvalLength, Anim->SetReevaluation();
  } /* End of 'anim::eval_settings::Apply' function */

  /* Dialog window process functions custom data external storage */
  std::map<HWND, VOID *> DialogsDataMap {};

  /* WM_COMMAND window message handle function.
   * ARGUMENTS:
   *   - Menu button ID:
   *       UINT Id;
   * RETURNS: None.
   */
  void anim::OnMenuButton( UINT Id )
  {
    /* Check window is active */
    if (!win::IsActive)
      return;

    /* Switch on button index */
    switch (Id)
    {
    case IDM_MAIN_MENU_EXIT:
      PostQuitMessage(0);
      return;
    case IDM_MAIN_MENU_SETTINGS:
    {
      eval_settings Settings {*this};
      InputState = input_state::Dialog;
      DialogBoxParamA(hInstance, (const CHAR *)IDD_DIALOG_SETTINGS, hWnd,
                      (DLGPROC)[]( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam ) -> INT_PTR
                      {
                        switch (Msg)
                        {
                        case WM_INITDIALOG:
                          DialogsDataMap[hWnd] = (VOID *)lParam;
                          SetDlgItemTextA(hWnd, IDC_EDIT_LINES_COUNT,
                                          std::to_string(((eval_settings *)lParam)->LinesPerCharge).c_str());
                          SetDlgItemTextA(hWnd, IDC_EDIT_LINES_LENGTH,
                                          std::to_string(((eval_settings *)lParam)->LineLengthCoeff).c_str());
                          SetDlgItemTextA(hWnd, IDC_EDIT_LINE_SEGM_COUNT,
                                          std::to_string(((eval_settings *)lParam)->LineEvalLength).c_str());
                          break;
                        case WM_CLOSE:
                          EndDialog(hWnd, 1);
                          break;
                        case WM_DESTROY:
                          DialogsDataMap.erase(hWnd);
                          break;
                        case WM_COMMAND:
                          switch (wParam)
                          {
                          case IDOK:
                          {
                            char Buf[0x40];

                            auto symbols = GetDlgItemTextA(hWnd, IDC_EDIT_LINES_COUNT, Buf, sizeof (Buf) - 1); Buf[symbols] = 0;

                            if (symbols > 0 && symbols < sizeof (Buf))
                            {
                              dbl NewVal = 0;
                              if (sscanf(Buf, "%lf", &NewVal) == 1 && NewVal > 0)
                              {
                                NewVal = std::clamp(NewVal, 0.25, 120.0);
                                ((anim::eval_settings *)DialogsDataMap[hWnd])->LinesPerCharge = NewVal;
                              }
                            }

                            symbols = GetDlgItemTextA(hWnd, IDC_EDIT_LINES_LENGTH, Buf, sizeof (Buf) - 1); Buf[symbols] = 0;

                            if (symbols > 0 && symbols < sizeof (Buf))
                            {
                              Buf[symbols] = 0;
                              dbl NewVal = 0;
                              if (sscanf(Buf, "%lf", &NewVal) == 1 && NewVal > 0)
                              {
                                NewVal = std::clamp(NewVal, 0.001, 10.0);
                                ((anim::eval_settings *)DialogsDataMap[hWnd])->LineLengthCoeff = NewVal;
                              }
                            }

                            symbols = GetDlgItemTextA(hWnd, IDC_EDIT_LINE_SEGM_COUNT, Buf, sizeof (Buf) - 1); Buf[symbols] = 0;

                            if (symbols > 0 && symbols < sizeof (Buf))
                            {
                              Buf[symbols] = 0;
                              size_t NewVal = 0;

                              if (sscanf(Buf, "%llu", &NewVal) == 1 && NewVal > 0)
                              {
                                NewVal = std::clamp<size_t>(NewVal, 1, 20'000);
                                ((anim::eval_settings *)DialogsDataMap[hWnd])->LineEvalLength = NewVal;
                              }
                            }
                          }
                            ((anim::eval_settings *)DialogsDataMap[hWnd])->Apply();
                            EndDialog(hWnd, 0);
                            break;
                          case IDC_EDIT_LINES_COUNT:
                            break;
                          }
                          break;
                        }

                        return 0;
                      },
                      (LPARAM)&Settings);
    }
      InputState = input_state::None;
      return;
    case IDM_MAIN_MENU_HELP:
      InputState = input_state::Dialog;
      DialogBoxParamA(hInstance, (const CHAR *)IDD_DIALOG_HELP, hWnd,
                      (DLGPROC)[]( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam ) -> INT_PTR
                      {
                        switch (Msg)
                        {
                        case WM_INITDIALOG:
                        {
                          /* Set heading */
                          std::string HeaderText =
                            "Made by Fedor Borodulin 10-1, PML30\n"
                            "Version from \"" __DATE__ "\"";
                          SetDlgItemTextA(hWnd, IDC_HELP_AUTHOR_TEXT, HeaderText.c_str());


                          /* Set help text (more than 255 symbols, cannot be stored as static in dialog) */
                          SetDlgItemTextA(hWnd, IDC_HELP_CONTROLS,
                                          "Controls (basic):\n"
                                          "  - Mouse Wheel - Zoom.\n"
                                          "  - Right Mouse Button - move 'camera'.\n"
                                          "  - Left Mouse Button - select charge.\n"
                                          "  - Ctrl + Left Mouse Button - select charge or add new.\n"
                                          "\nControls (charge selected):\n"
                                          "  - Moving mouse - move charge.\n"
                                          "  - Mouse wheel - charge value.\n"
                                          "  - Delete or backspace - delete charge.\n"
                                          "  - Hold 'A' - coordinates align.");
                        }
                          break;
                        case WM_CLOSE:
                          EndDialog(hWnd, 0);
                          break;
                        case WM_DESTROY:
                          break;
                        case WM_COMMAND:
                          switch (wParam)
                          {
                          case IDOK:
                            EndDialog(hWnd, 0);
                            break;
                          }
                          break;
                        }

                        return 0;
                      }, 0);
      InputState = input_state::None;
      return;
    case ID_SCENE_LOAD:
      ClearScene();

      [[fallthrough]];
    case ID_SCENE_LOADADD:
    {
      InputState = input_state::Dialog;

      CHAR FileNameBuf[0x400] {};
      OPENFILENAME FileName {sizeof (OPENFILENAME), hWnd, hInstance};
      FileName.lpstrFile = FileNameBuf;
      FileName.nMaxFile = (sizeof (FileNameBuf) / sizeof (*FileNameBuf)) - 1;
      FileName.lpstrFilter = "All Files\0*.*\0\0";

      if (GetOpenFileName(&FileName))
      {
        std::ifstream File {FileName.lpstrFile};
        bool WasError = false;

        int Cnt;
        if (File >> Cnt && Cnt < 2000)
        {
          dbl Charge, X, Y;
          CHAR Line[0x100] {};
          File.get();

          while (Cnt > 0 && File.getline(Line, sizeof (Line)))
          {
            Cnt--;

            if (sscanf(Line, "charge=%lf coord=%lf, %lf", &Charge, &X, &Y) != 3)
            {
              WasError = true;
              break;
            }

            if (abs(Charge) < MinCharge)
              Charge = std::copysign(MinCharge, Charge);

            Charges.push_back({coordd {X, Y}, Charge, pow(abs(Charge), SizePow) * SizeCoeff});
          }
        }
        else
          WasError = true;

        if (WasError || Cnt != 0)
        {
          ClearScene();
          MessageBoxA(hWnd, "Error while loading file!", "Error", MB_OK);
        }
        else
          SetReevaluation();

      }

      InputState = input_state::None;
    }
      return;
    case ID_SCENE_SAVE:
      if (Charges.size() != 0)
      {
        InputState = input_state::Dialog;

        CHAR FileNameBuf[0x400] {};
        OPENFILENAME FileName {sizeof (OPENFILENAME), hWnd, hInstance};
        FileName.lpstrFile = FileNameBuf;
        FileName.nMaxFile = (sizeof (FileNameBuf) / sizeof (*FileNameBuf)) - 1;
        FileName.lpstrFilter = "All Files\0*.*\0\0";

        if (GetSaveFileName(&FileName))
        {
          std::ofstream File {FileName.lpstrFile};

          File << Charges.size() << '\n';

          for (auto &Elm : Charges)
            File << "charge=" << Elm.Charge << " coord=" << Elm.Coord.X << ", " << Elm.Coord.Y << '\n';
        }

        InputState = input_state::None;
      }
      return;
    case ID_SCENE_SCREENSHOT:
      {
        InputState = input_state::Dialog;

        CHAR FileNameBuf[0x400] {};
        OPENFILENAME FileName {sizeof (OPENFILENAME), hWnd, hInstance};
        FileName.lpstrFile = FileNameBuf;
        FileName.nMaxFile = (sizeof (FileNameBuf) / sizeof (*FileNameBuf)) - 1;
        FileName.lpstrFilter = "Portalble Network Graphic\0*.png*\0\0";
        FileName.lpstrDefExt = "png";

        if (GetSaveFileName(&FileName))
        {
          /* Create Device Context */
          HDC hDC = GetDC(NULL);
          HDC hMemDC = CreateCompatibleDC(hDC);

          /* Create temporary bitmap */
          HBITMAP hBm;
          DWORD *Pixels;

          BITMAPINFO Info {};
          Info.bmiHeader.biSize = sizeof (Info.bmiHeader);

          RECT Size;
          GetClientRect(win::hWnd, &Size);
          Info.bmiHeader.biWidth = abs(Size.right - Size.left);
          Info.bmiHeader.biHeight = -abs(Size.bottom - Size.top);

          Info.bmiHeader.biBitCount = 32;
          Info.bmiHeader.biPlanes = 1;

          hBm = CreateDIBSection(hMemDC, &Info, 0, (VOID **)&Pixels, NULL, 0);

          SelectObject(hMemDC, hBm);

          PrintWindow(win::hWnd, hMemDC, PW_CLIENTONLY);

          try 
          {
            img::SaveAsPng(FileNameBuf, Info.bmiHeader.biWidth, -Info.bmiHeader.biHeight, Pixels);
          }
          catch ( std::runtime_error & ) {}

          DeleteDC(hMemDC);
          DeleteObject(hBm);
          ReleaseDC(NULL, hDC);
        }

        InputState = input_state::None;
      }
      return;
    case ID_SCENE_CLEAR:
      ClearScene();
      return;
    }
  } /* End of 'anim::OnMenuButton' function */
} /* end of 'prj' namespace */

/* END OF 'anim.cpp' FILE */
