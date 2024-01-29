#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

EFIAPI EFI_STATUS BmpBitmapCopy(BMP_BITMAP *Dst, BMP_BITMAP *Src, UINT32 SrcX, UINT32 SrcY, UINT32 DstX, UINT32 DstY, UINT32 Width, UINT32 Height) {
    if (!Dst || !Src) {
        return EFI_INVALID_PARAMETER;
    }
    else if (SrcX + Width > Src->Width || SrcY + Height > Src->Height) {
        return EFI_BAD_BUFFER_SIZE;
    }
    else if (DstX + Width > Dst->Width || DstY + Height > Dst->Height) {
        return EFI_BUFFER_TOO_SMALL;
    }

    UINTN Size = Width * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    UINTN SrcIndex, DstIndex;
    for (UINT32 Y = 0; Y < Height; Y++) {
        SrcIndex = (SrcY + Y) * Src->Width + SrcX;
        DstIndex = (DstY + Y) * Dst->Width + DstX;
        gBS->CopyMem((VOID*)&Dst->Buffer[DstIndex],(VOID*)&Src->Buffer[SrcIndex],Size);
    }

    return EFI_SUCCESS;
}