#include <Uefi.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

EFIAPI VOID BmpClose(BMP_FILE *Bmp) {
    if (Bmp) {
        ShellCloseFile(&Bmp->FileHandle);
        if (Bmp->Color.Table) {
            gBS->FreePool(Bmp->Color.Table);
        }
    }
    return;
}