#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>
#include <StatusBar.h>

EFIAPI STATUS_BAR *StatusBarCreate(UINTN RelativeX, UINTN RelativeY, UINTN RelativeLength, UINTN RelativeWidth, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Scheme[2]) {
    STATUS_BAR *Bar = NULL;
    if (RelativeLength <= 100 && RelativeWidth <= 100 && RelativeX <= 100 && RelativeY <= 100) {
        // Determine width and height based off screen dimensions
        RelativeLength = (RelativeLength * gGop->Mode->Info->HorizontalResolution) / 100;
        RelativeWidth = (RelativeWidth * gGop->Mode->Info->VerticalResolution) / 100;

        // Allocate double buffer
        UINTN Size = sizeof(STATUS_BAR) + RelativeLength * RelativeWidth * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
        EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,Size,(VOID*)&Bar);
        if (Status == EFI_SUCCESS) {
            // Set defaults
            Bar->X = (RelativeX * gGop->Mode->Info->HorizontalResolution) / 100;
            Bar->Y = (RelativeY * gGop->Mode->Info->VerticalResolution) / 100;
            Bar->Ticks = 0;
            Bar->Width = RelativeWidth;
            Bar->Length = RelativeLength;
            Bar->Foreground = Scheme[0];
            Bar->Background = Scheme[1];

            // Set double buffer
            UINTN Total = RelativeLength * RelativeWidth;
            for (UINTN Cur = 0; Cur < Total; Cur++) {
                Bar->DoubleBuffer[Cur] = Bar->Background;
            }
        }
        
    }
    return Bar;
}