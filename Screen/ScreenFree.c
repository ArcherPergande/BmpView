#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>

EFIAPI VOID ScreenFree(SCREEN_BUFFER *Screen) {
    if (Screen && Screen->Buffer) {
        gBS->FreePool(Screen->Buffer);
    }
    return;
}