#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>
#include <StatusBar.h>

EFIAPI EFI_STATUS StatusBarUpdate(STATUS_BAR *Bar, UINTN Percentage) {
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    if (Bar && Percentage <= 100) {
        UINTN Index = (Percentage * Bar->Length) / 100;
        if (Bar->Ticks && Index <= Bar->Ticks) {
            return EFI_SUCCESS;
        }

        UINTN TickChange = Index - Bar->Ticks;
        for (UINTN Row = 0; Row < Bar->Width; Row++) {
            for (UINTN Cur = 0; Cur < TickChange; Cur++) {
                Bar->DoubleBuffer[Row * Bar->Length + (Cur + Bar->Ticks)] = Bar->Foreground;
            }
        }
        Status = gGop->Blt(gGop,Bar->DoubleBuffer,EfiBltBufferToVideo,0,0,Bar->X,Bar->Y,Bar->Length,Bar->Width,0);
    }
    return Status;
}