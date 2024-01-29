#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>

extern BOOLEAN __ScreenCursorVisible;

EFIAPI EFI_STATUS ScreenFini() {
    EFI_STATUS Status = ScreenRestore(&gSavedScreen);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    if (__ScreenCursorVisible) {
        gST->ConOut->EnableCursor(gST->ConOut,TRUE);
    }

    ScreenFree(&gSavedScreen);
    return Status;
}