#ifndef __BMPVIEW_STATUS_BAR_H__
#define __BMPVIEW_STATUS_BAR_H__

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>

typedef struct {
    UINTN Ticks;
    UINTN Length, Width, X, Y; // Dimensions
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Foreground, Background; // Color scheme
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL DoubleBuffer[];
} STATUS_BAR;

EFIAPI STATUS_BAR *StatusBarCreate(UINTN RelativeX, UINTN RelativeY, UINTN RelativeLength, UINTN RelativeWidth, EFI_GRAPHICS_OUTPUT_BLT_PIXEL Scheme[2]);
EFIAPI EFI_STATUS StatusBarUpdate(STATUS_BAR *Bar, UINTN Percentage);

#endif //__BMPVIEW_STATUS_BAR_H__