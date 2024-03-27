#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

EFIAPI VOID BmpClose(BMP_FILE *Bmp) {
    if (Bmp) {
        Bmp->FileHandle->Close(Bmp->FileHandle);
        if (Bmp->Color.Table) {
            gBS->FreePool(Bmp->Color.Table);
        }
    }
    return;
}