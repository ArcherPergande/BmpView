#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Screen.h>

EFIAPI EFI_STATUS ScreenSave(SCREEN_BUFFER *Screen) {
    if (!Screen) {
        return EFI_INVALID_PARAMETER;
    }

    UINTN Size = gGop->Mode->Info->HorizontalResolution * gGop->Mode->Info->VerticalResolution * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,Size,(VOID*)&Screen->Buffer);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    Status = gGop->Blt(gGop,Screen->Buffer,EfiBltVideoToBltBuffer,0,0,0,0,gGop->Mode->Info->HorizontalResolution,gGop->Mode->Info->VerticalResolution,0);
    if (Status != EFI_SUCCESS) {
        gBS->FreePool(Screen->Buffer);
        Screen->Buffer = NULL;
        Screen->CursorColumn = 0;
        Screen->CursorRow = 0;
    }
    else {
        Screen->CursorColumn = gST->ConOut->Mode->CursorColumn;
        Screen->CursorRow = gST->ConOut->Mode->CursorRow;
    }
    
    return Status;
}