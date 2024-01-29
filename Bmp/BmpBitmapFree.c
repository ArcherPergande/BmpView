#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

EFIAPI VOID BmpBitmapFree(BMP_BITMAP *Bmp) {
    if (Bmp) {
        gBS->FreePool(Bmp);
    }
    return;
}