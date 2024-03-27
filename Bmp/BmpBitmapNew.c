#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

// add some like hybrid between pool and page allocation
// would also need some reserved field in BMP_BITMAP tho to indicate if it was a pool or page allocation
// if BufferSize > some arbitrary value allocate pages
// lets say the temp memory should always be pool
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

// EFIAPI EFI_STATUS BmpBitmapNew(BMP_FILE *Bmp, BMP_BITMAP **Bitmap, UINT32 X, UINT32 Y, UINT32 Width, UINT32 Height)