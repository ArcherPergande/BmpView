#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>

SCREEN_BUFFER gSavedScreen;
EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;
BOOLEAN __ScreenCursorVisible;

EFIAPI EFI_STATUS ScreenInit() {
    EFI_STATUS Status;

    EFI_GUID Guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    Status = gBS->LocateProtocol(&Guid,NULL,(VOID*)&gGop);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    __ScreenCursorVisible = gST->ConOut->Mode->CursorVisible;
    gST->ConOut->EnableCursor(gST->ConOut,FALSE);

    Status = ScreenSave(&gSavedScreen);
    if (Status == EFI_SUCCESS) {
        Status = ScreenBlackout();
    }

    return Status;
}