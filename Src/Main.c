#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/ShellCEntryLib.h>

#include <Bmp.h>
#include <Screen.h>
#include <Util.h>

#define SCROLL_AMOUNT 15

BMP_FILE Bmp;
BMP_BITMAP *Bitmap = NULL;
UINT32 CurX = 0, CurY = 0;

EFIAPI EFI_STATUS ShiftRight(VOID *) {
    UINT32 Stop = Bmp.Header.Info.Width - Bitmap->Width;
    if (CurX + SCROLL_AMOUNT >= Stop) {
        return EFI_SUCCESS;
    }

    CurX += (CurX + SCROLL_AMOUNT < Stop) ? SCROLL_AMOUNT : Stop - CurX;
    EFI_STATUS Status = BmpBitmapLoad(&Bmp,&Bitmap,CurX,CurY,0,0,Bitmap->Width,Bitmap->Height);
    if (Status != EFI_SUCCESS) {
        return Status;
    }
    else {
        return ScreenPrint(Bitmap->Buffer,0,0,0,0,Bitmap->Width,Bitmap->Height);
    }
}

EFIAPI EFI_STATUS ShiftLeft(VOID *) {
    if (CurX == 0) {
        return EFI_SUCCESS;
    }

    CurX -= (CurX - SCROLL_AMOUNT > 0) ? SCROLL_AMOUNT : CurX;
    EFI_STATUS Status = BmpBitmapLoad(&Bmp,&Bitmap,CurX,CurY,0,0,Bitmap->Width,Bitmap->Height);
    if (Status != EFI_SUCCESS) {
        return Status;
    }
    else {
        return ScreenPrint(Bitmap->Buffer,0,0,0,0,Bitmap->Width,Bitmap->Height);
    }
}

EFIAPI EFI_STATUS ShiftDown(VOID *) {
    UINT32 Stop = Bmp.Header.Info.Height - Bitmap->Height;
    if (CurY + SCROLL_AMOUNT >= Stop) {
        return EFI_SUCCESS;
    }

    CurY += (CurY + SCROLL_AMOUNT < Stop) ? SCROLL_AMOUNT : Stop - CurY;
    EFI_STATUS Status = BmpBitmapLoad(&Bmp,&Bitmap,CurX,CurY,0,0,Bitmap->Width,Bitmap->Height);
    if (Status != EFI_SUCCESS) {
        return Status;
    }
    else {
        return ScreenPrint(Bitmap->Buffer,0,0,0,0,Bitmap->Width,Bitmap->Height);
    }
}

EFIAPI EFI_STATUS ShiftUp(VOID *) {
    if (CurY == 0) {
        return EFI_SUCCESS;
    }
    
    CurY -= (CurY - SCROLL_AMOUNT > 0) ? SCROLL_AMOUNT : CurY;
    EFI_STATUS Status = BmpBitmapLoad(&Bmp,&Bitmap,CurX,CurY,0,0,Bitmap->Width,Bitmap->Height);
    if (Status != EFI_SUCCESS) {
        return Status;
    }
    else {
        return ScreenPrint(Bitmap->Buffer,0,0,0,0,Bitmap->Width,Bitmap->Height);
    }
}

EFIAPI EFI_STATUS Init() {
    UINT32 Width = (Bmp.Header.Info.Width > gGop->Mode->Info->HorizontalResolution) ? gGop->Mode->Info->HorizontalResolution : 0;
    UINT32 Height = (Bmp.Header.Info.Height > gGop->Mode->Info->VerticalResolution) ? gGop->Mode->Info->VerticalResolution : 0;
    EFI_STATUS Status = BmpBitmapLoad(&Bmp,&Bitmap,CurX,CurY,0,0,Width,Height);
    if (Status != EFI_SUCCESS) {
        return Status;
    }
    else {
        return ScreenPrint(Bitmap->Buffer,0,0,0,0,Bitmap->Width,Bitmap->Height);
    }
}

EFIAPI INTN Runtime() {
    INTN Retval = 0;
    KEYBOARD_CALLBACK KeyBinds[] = {
        {{SCAN_RIGHT,0},ShiftRight},
        {{SCAN_LEFT,0},ShiftLeft},
        {{SCAN_DOWN,0},ShiftDown},
        {{SCAN_UP,0},ShiftUp}
    };

    if (ScreenInit() != EFI_SUCCESS || Init() != EFI_SUCCESS) {
        Retval = 1;
        goto end;
    }

    UINTN Index;
    EFI_STATUS Status = KeyboardGetCallback(KeyBinds,NULL,&Index,4);
    if (Status != EFI_SUCCESS && Index != 0) {
        Retval = 2;
        goto end;
    }
    
    end:
    BmpBitmapFree(Bitmap);
    ScreenFini();
    return Retval;
}

INTN EFIAPI ShellAppMain(IN UINTN Argc, IN CHAR16 **Argv) {
    // Determine if a valid argument was passed
    if (Argc < 2) {
        Print(L"ERROR: No file path was specified.\r\nUSAGE: Img Folder/Image.bmp\r\n");
        return EFI_INVALID_PARAMETER;
    }

    // Attempt to open the file specified
    EFI_STATUS Status = BmpOpen(Argv[Argc-1],&Bmp);
    if (Status == EFI_UNSUPPORTED) {
        Print(L"ERROR: Incompatible file format. Only BMP files are supported.\r\n");
        return Status;
    }
    else if (Status != EFI_SUCCESS) {
        Print(L"ERROR: Failed to open the file specified.\r\n");
        return Status;
    }
    
    Status = Runtime();
    if (Status != EFI_SUCCESS) {
        Print(L"fml\r\n");
    }
    BmpClose(&Bmp);
    return Status;
}
