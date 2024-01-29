#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

EFIAPI BMP_BITMAP *BmpBitmapNew(UINT32 Width, UINT32 Height) {
    BMP_BITMAP *Bitmap = NULL;
    if (Width && Height) {
        UINTN BufferSize = Width * Height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
        EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,BufferSize+sizeof(BMP_BITMAP),(VOID*)&Bitmap);
        if (Status != EFI_SUCCESS) {
            return NULL;
        }
        else {
            Bitmap->Width = Width;
            Bitmap->Height = Height;
            gBS->SetMem((VOID*)&Bitmap->Buffer,BufferSize,0);
        }
    }

    return Bitmap;
}