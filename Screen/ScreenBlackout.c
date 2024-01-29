#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>

EFIAPI EFI_STATUS ScreenBlackout() {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL BlackPixel = {0,0,0,0};
    EFI_STATUS Status = gST->ConOut->SetCursorPosition(gST->ConOut,0,0);
    if (Status == EFI_SUCCESS) {
        Status = gGop->Blt(gGop,&BlackPixel,EfiBltVideoFill,0,0,0,0,gGop->Mode->Info->HorizontalResolution,gGop->Mode->Info->VerticalResolution,0);
    }
    return Status;
}