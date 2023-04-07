/* FILE NAME   : 'anim.cpp'
 * PURPOSE     : Animation module implementation file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 25.04.2023.
 * NOTE        : Module namespace 'prj'.
 */

#include <pch.h>

#include <string>
#include <fstream>
#include <streambuf>

#include "anim.h"

#if 0 // defined(_DEBUG)
/* Debug timer very auxilary class */
class timer
{
private:
  dbl Freq;
  UINT64 Start, Prev;

public:
  timer( void )
  {
    UINT64 IFreq;
    QueryPerformanceFrequency((LARGE_INTEGER *)&IFreq), Freq = IFreq;

    QueryPerformanceCounter((LARGE_INTEGER *)&Start), Prev = Start;
  }

  void operator()( const std::string_view &Str = nullptr )
  {
    UINT64 Cur;
    QueryPerformanceCounter((LARGE_INTEGER *)&Cur);

    if (Str.empty())
      printf("Timer call: delta = %lf, global = %lf\n",
             (Cur - Prev) / Freq, (Cur - Start) / Freq);
    else
      printf("%s: delta = %lf, global = %lf\n", Str.data(),
             (Cur - Prev) / Freq, (Cur - Start) / Freq);
    Prev = Cur;
  }
};
#else
/* Empty debug timer class */
class timer
{
public:
  timer( void ) = default;
  void operator()( const std::string_view & = nullptr ) {}
};
#endif

/* Project namespace */
namespace prj
{
  /* Animation default constructor */
  anim::anim( void )
  {
    /* Get time frequency */
    QueryPerformanceFrequency((LARGE_INTEGER *)&RenderTimeFreq);

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
    for (auto &Elm : ThreadsPool)
    {
      Elm.Run = false;

      Elm.Thread->join();
      delete Elm.Thread;
    }

    ThreadsDataUpdated = TRUE;

    ThreadsPool.clear();
    ThreadsDataBulk.clear();
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

    Reeval = TRUE;
  } /* End of 'anim::AddCharge' function */

  /* Charge at position selecting, otherwise adding function
   * ARGUMENTS:
   *   - Charge position:
   *       coordd Coord;
   */
  void anim::SelectAddCharge( coordd Coord )
  {
    SelectedCharge = nullptr;
    InputState = input_state::Charge;

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

    if (SelectedCharge == nullptr)
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
    }
  } /* End of 'anim::SetReevaluation' function */

  /* Animation state responce function */
  void anim::Responce( void )
  {
    if (!WasInit)
      return;

    BOOL IsClientArea {hWnd == GetForegroundWindow() &&
                       DefWindowProcA(hWnd, WM_NCHITTEST, 0, MAKELPARAM(Input.Mx, Input.My)) == HTCLIENT};

    timer DebugTimer {};

    Input.InputResponce();

    if (Input.Keys[VK_MENU] && Input.KeysClick['\r'])
      FlipFullScreen();

    DebugTimer("Input");

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
          SelectAddCharge({MX, MY});

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

    DebugTimer("Selecting");

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
      }

      break;
    case prj::anim::input_state::Charge:
      if (Input.KeysClick[VK_DELETE])
      {
        Charges.remove_if([&]( const charge &Ref ) -> bool { return &Ref == SelectedCharge; });

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

    DebugTimer("Responce from input");

    if (Reeval)
    {
      /* Stop all threads */
      for (auto &Elm : ThreadsPool)
      {
        Elm.Run = false;

        Elm.Thread->join();
        delete Elm.Thread;
      }

      ThreadsPool.clear();
      ThreadsDataBulk.clear();

      /* Init lines */
      for (auto &Elm : Charges)
      {
        Elm.Lines.clear();

        if (Elm.Charge < 0)
          continue;

        Elm.Lines.resize((size_t)round(LinesPerCharge * abs(Elm.Charge)));

        for (int i = 0; i < Elm.Lines.size(); i++)
        {
          dbl Angle {(M_PI * (i << 1)) / (dbl)Elm.Lines.size()};
          coordd Base {Elm.Coord.X + Elm.Size * cos(Angle) * 2.0,
                       Elm.Coord.Y + Elm.Size * sin(Angle) * 2.0};

          Elm.Lines[i].reserve(LineEvalLength);
          Elm.Lines[i].push_back(coordf {(flt)Elm.Coord.X, (flt)Elm.Coord.Y});
          Elm.Lines[i].push_back(coordf {(flt)Base.X, (flt)Base.Y});

          ThreadsDataBulk.push_back({field_line {Base, *this}, Elm.Lines.data() + i});
        }
      }

      /* Start threads */
      {
        int Threads {(int)std::thread::hardware_concurrency() - 2};
        if (Threads < 2)
          Threads = 2;
        else if (Threads > ThreadsDataBulk.size())
          Threads = ThreadsDataBulk.size();
        dbl LinesPerThread {(dbl)ThreadsDataBulk.size() / Threads};

        const auto ThreadFunc {[this]( eval_trhead *Data )
          {
            bool WasReeval = true;

            while (Data->Run && WasReeval)
            {
              WasReeval = false;

              for (auto &Elm : Data->Data)
              {
                if (Elm->LineEval.Continue &&
                    Elm->LineData->size() < Elm->LineData->capacity())
                {
                  ThreadsDataUpdated = WasReeval = TRUE;

                  int i = 12;

                  do
                  {
                    Elm->LineData->emplace_back(Elm->LineEval.Next3());
                  } while (Elm->LineEval.Continue && i-- &&
                           Elm->LineData->size() < Elm->LineData->capacity());
                }
              }

              Sleep(0);
            }
          }};

        ThreadsPool.reserve(Threads);
        int lines = 0;
        for (int i = 0; i < Threads - 1; i++)
        {
          int cur_lines = ceil((i + 1) * LinesPerThread) - lines;

          auto &Data = ThreadsPool.emplace_back();

          for (int j = 0; j < cur_lines; j++)
            Data.Data.emplace_back(ThreadsDataBulk.data() + lines + j);

          Data.Thread = new std::thread {ThreadFunc, &Data};

          lines += cur_lines;
        }

        if (Threads > 0)
        {
          int cur_lines = ThreadsDataBulk.size() - lines;

          auto &Data = ThreadsPool.emplace_back();

          for (int j = 0; j < cur_lines; j++)
            Data.Data.emplace_back(ThreadsDataBulk.data() + lines + j);

          Data.Thread = new std::thread {ThreadFunc, &Data};

          lines += cur_lines;
        }
      }

      ThreadsDataUpdated = true;
    }

    Reeval = FALSE;
    DebugTimer("Reevaluation");

    /* Not more 24 frames per second can be rendered */
    UINT64 Time;
    QueryPerformanceCounter((LARGE_INTEGER *)&Time);

    if (24 * (Time - RenderTime) >= RenderTimeFreq)
    {
      Render();

      RenderTime = Time;
    }

    DebugTimer("Render");
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
    Renderer.Render(TmpCharges, {Left, Top}, {Right, Bottom});
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
                                          "  - Left Mouse Button - select charge or add new."
                                          "\n\nControls (charge selected):\n"
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
          Reeval = TRUE;
      }
    }
      return;
    case ID_SCENE_SAVE:
      if (Charges.size() != 0)
      {
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
      }
      return;
    case ID_SCENE_CLEAR:
      ClearScene();
      return;
    }
  } /* End of 'anim::OnMenuButton' function */

  /* Next point evaluation function.
   * RETURNS:
   *   (coordf) Next coordinate.
   */
  coordf anim::field_line::Next1( void )
  {
    auto Offset = EvalForce(Pos);
    auto TmpOffset = _mm_mul_pd(Offset, Offset);
    Offset = _mm_div_pd(Offset, _mm_sqrt_pd(_mm_hadd_pd(TmpOffset, TmpOffset)));

    Pos = _mm_add_pd(Pos, _mm_mul_pd(Offset, _mm_set1_pd(0.30)));

    coordf Tmp;
    _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

    return Tmp;
  } /* End of 'anim::field_line::Next1' function */

  /* Next point evaluation function.
   * RETURNS:
   *   (coordf) Next coordinate.
   */
  coordf anim::field_line::Next2( void )
  {
    if (Continue)
    {
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
    }

    coordf Tmp;
    _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

    return Tmp;
  } /* End of 'anim::field_line::Next2' function */

  /* Next point evaluation function.
   * RETURNS:
   *   (coordf) Next coordinate.
   */
  coordf anim::field_line::Next3( void )
  {
    if (Continue)
    {
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
    }

    coordf Tmp;
    _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

    return Tmp;
  } /* End of 'anim::field_line::Next3' function */

  /* Next point evaluation function.
   * RETURNS:
   *   (coordf) Next coordinate.
   */
  coordf anim::field_line::Next4( void )
  {
    if (Continue)
    {
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
    }

    coordf Tmp;
    _mm_store_sd((dbl *)&Tmp, _mm_castps_pd(_mm_cvtpd_ps(Pos)));

    return Tmp;
  } /* End of 'anim::field_line::Next4' function */
} /* end of 'prj' namespace */

/* END OF 'anim.cpp' FILE */
