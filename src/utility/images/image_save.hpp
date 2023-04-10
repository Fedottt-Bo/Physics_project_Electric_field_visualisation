/* FILE NAME   : 'image_save.hpp'
 * PURPOSE     : Images work module.
 *               Images saving file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 10.04.2023.
 * NOTE        : Module namespace 'prj::img'.
 */

#ifndef __image_save_hpp__
#define __image_save_hpp__

#include "image_def.h"

/* Project namespace // Images module */
namespace prj::img
{
  /* Image (in format 32bpp - RGBA) saving as PNG function
   * ARGUMENTS:
   *   - Saving path:
   *       const std::filesystem::path &Path;
   *   - Image data:
   *       UINT32 Width, Height;
   *       const DWORD *Data;
   */
  void SaveAsPng( const std::filesystem::path &Path,
                  UINT32 Width, UINT32 Height, const DWORD *Data )
  {
    /* Creating bitmap with image data and checking it */
    ComPtr<IWICBitmap> Bitmap {};
    wic_factory::Get()->CreateBitmapFromMemory(Width, Height, GUID_WICPixelFormat32bppBGRA,
                                               Width << 2, (Width * Height) << 2, (BYTE *)Data,
                                               Bitmap.GetAddressOf());
    if (Bitmap.Get() == nullptr)
      throw std::runtime_error {"Failed create bitmap"};

    /* Creating encoder and checking it */
    ComPtr<IWICBitmapEncoder> Encoder {};
    wic_factory::Get()->CreateEncoder(GUID_ContainerFormatPng, nullptr, Encoder.GetAddressOf());
    if (Encoder.Get() == nullptr)
      throw std::runtime_error {"Failed create encoder"};

    /* Creating, initializing and checking output file stream */
    ComPtr<IStream> Stream {};

    /* Correct wchar string */
    HRESULT InitRes = SHCreateStreamOnFileEx(Path.c_str(), STGM_WRITE | STGM_TRANSACTED |
                                                           STGM_CREATE | STGM_SHARE_EXCLUSIVE,
                                             0, TRUE, nullptr, Stream.GetAddressOf());
    if (InitRes != S_OK)
      throw std::runtime_error {"Failed open file stream"};

    /* Initializing and checking encoder */
    InitRes = Encoder->Initialize(Stream.Get(), WICBitmapEncoderNoCache);
    if (InitRes != S_OK)
      throw std::runtime_error {"Failed initialize encoder"};

    /* Creating and checking image frame */
    ComPtr<IWICBitmapFrameEncode> EncodeFrame {};
    ComPtr<IPropertyBag2> Properties {};
    Encoder->CreateNewFrame(EncodeFrame.GetAddressOf(), Properties.GetAddressOf());
    if (EncodeFrame.Get() == nullptr)
      throw std::runtime_error {"Failed add image frame"};

    /* Initializing and checking image frame */
    InitRes = EncodeFrame->Initialize(Properties.Get());
    if (InitRes != S_OK)
      throw std::runtime_error {"Failed initialize image frame"};

    /* Loading pixels to frame */
    EncodeFrame->SetSize(Width, Height);
    WICRect Rect {0, 0, static_cast<INT>(Width), static_cast<INT>(Height)};
    EncodeFrame->WriteSource(Bitmap.Get(), &Rect);

    /* Saving frame and file */
    EncodeFrame->Commit();
    Encoder->Commit();
    Stream->Commit(STGC_OVERWRITE);
  } /* End of 'SaveAsPng' function */
} /* end of 'prj::img' namespace */

#endif /* __image_save_hpp__ */

/* END OF 'image_save.hpp' FILE */
