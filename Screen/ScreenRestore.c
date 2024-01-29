#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>

EFIAPI EFI_STATUS ScreenRestore(SCREEN_BUFFER *Screen) {
    if (!Screen) {
        return EFI_INVALID_PARAMETER;
    }

    EFI_STATUS Status = gGop->Blt(gGop,Screen->Buffer,EfiBltBufferToVideo,0,0,0,0,gGop->Mode->Info->HorizontalResolution,gGop->Mode->Info->VerticalResolution,0);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    Status = gST->ConOut->SetCursorPosition(gST->ConOut,Screen->CursorColumn,Screen->CursorRow);
    return Status;
}