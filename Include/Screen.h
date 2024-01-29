#ifndef __BMPVIEW_SCREEN_H__
#define __BMPVIEW_SCREEN_H__

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>

typedef struct {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Buffer;
    INT32 CursorColumn;
    INT32 CursorRow;
} SCREEN_BUFFER;

extern EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;
extern SCREEN_BUFFER gSavedScreen;

#define ScreenPrint(Buffer,SrcX,SrcY,DstX,DstY,Width,Height) gGop->Blt(gGop,Buffer,EfiBltBufferToVideo,SrcX,SrcY,DstX,DstY,Width,Height,0)

EFIAPI EFI_STATUS ScreenSave(SCREEN_BUFFER *Screen);
EFIAPI EFI_STATUS ScreenRestore(SCREEN_BUFFER *Screen);
EFIAPI VOID ScreenFree(SCREEN_BUFFER *Screen);
EFIAPI EFI_STATUS ScreenBlackout();
EFIAPI EFI_STATUS ScreenInit();
EFIAPI EFI_STATUS ScreenFini();

#endif // __BMPVIEW_SCREEN_H__