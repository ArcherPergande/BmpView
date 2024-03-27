#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadedImage.h>

#include <../Include/Bmp.h>
#include <LoadOptions.h>
#include <Screen.h>
#include <StatusBar.h>

#define SCROLL 30

BMP_BITMAP *LoadBitmap(BMP_FILE *Bmp) {
    if (!Bmp) {
        return NULL;
    }
    
    // Allocate everything needed
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Scheme[2] = {{255,255,255,0},{145,145,145,0}};
    BMP_BITMAP *Bitmap = BmpBitmapNew(Bmp->Header.Info.Width,Bmp->Header.Info.Height);
    BMP_BITMAP *Temp = BmpBitmapNew(Bmp->Header.Info.Width,1);
    STATUS_BAR *Bar = StatusBarCreate(25,50,50,3,Scheme);
    if ( !Bitmap || !Temp || !Bar) {
        goto error;
    }

    // Read bitmap by rows and increment the status bar
    EFI_STATUS Status;
    UINTN Percentage;
    for (UINT32 Row = 0; Row < Bitmap->Height; Row++) {
        Percentage = (Row * 100) / Bitmap->Height;
        StatusBarUpdate(Bar,Percentage);
        Status = BmpBitmapLoad(Bmp,Temp,0,Row);
        if (Status == EFI_SUCCESS) {
            gBS->CopyMem(&Bitmap->Buffer[Row * Bitmap->Width],Temp->Buffer,Bitmap->Width*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
        }
        else {
            break;
        }
    }
    if (Status == EFI_SUCCESS) {
        StatusBarUpdate(Bar,100);
        gBS->FreePool(Bar);
        gBS->FreePool(Temp);
        return Bitmap;
    }

    error:
    gBS->FreePool(Bar);
    gBS->FreePool(Temp);
    gBS->FreePool(Bitmap);
    return NULL;
}

EFI_STATUS PrintBitmap(BMP_BITMAP *Bitmap, UINT32 SrcX, UINT32 SrcY) {
    UINT32 TruncatedWidth = gGop->Mode->Info->HorizontalResolution;
    UINT32 TruncatedHeight = gGop->Mode->Info->VerticalResolution;
    UINT32 DstX = 0, DstY = 0;

    // Truncate the width and center on x axis
    if (TruncatedWidth > Bitmap->Width) {
        SrcX = 0;
        TruncatedWidth = Bitmap->Width;
        DstX = (gGop->Mode->Info->HorizontalResolution / 2) - (Bitmap->Width / 2);
    }

    // Truncate the height and center on y axis
    if (TruncatedHeight > Bitmap->Height) {
        SrcY = 0;
        TruncatedHeight = Bitmap->Height;
        DstY = (gGop->Mode->Info->VerticalResolution / 2) - (Bitmap->Height / 2);
    }

    return gGop->Blt(gGop,Bitmap->Buffer,EfiLoaderData,SrcX,SrcY,DstX,DstY,TruncatedWidth,TruncatedHeight,Bitmap->Width*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
}

EFIAPI EFI_STATUS UpdatePosition(BMP_BITMAP *Bitmap, INTN *X, INTN *Y, EFI_INPUT_KEY *Key) {
    // Update coords
    INTN Stop = 0;
    if (Key->ScanCode == SCAN_RIGHT && Bitmap->Width > gGop->Mode->Info->HorizontalResolution) {
        Stop = Bitmap->Width - gGop->Mode->Info->HorizontalResolution;
        *X += (*X + SCROLL < Stop) ? SCROLL : Stop - *X;
    }
    else if (Key->ScanCode == SCAN_DOWN && Bitmap->Height > gGop->Mode->Info->VerticalResolution) {
        Stop = Bitmap->Height - gGop->Mode->Info->VerticalResolution;
        *Y += (*Y + SCROLL < Stop) ? SCROLL : Stop - *Y;
    }
    else if (Key->ScanCode == SCAN_LEFT) {
        Stop = 1;
        *X -= (*X - SCROLL > 0) ? SCROLL : *X;
    }
    else if (Key->ScanCode == SCAN_UP) {
        Stop = 1;
        *Y -= (*Y - SCROLL > 0) ? SCROLL : *Y;
    }

    // Print if changed
    if (Stop) {
        return PrintBitmap(Bitmap,*X,*Y);
    }
    return EFI_SUCCESS;
}

/*
wtf is wrong with the large bitmap buffer not rendering correctly??????
the issue is with printing with Blt()
the only thing that is different is the fucking delta

3 options:
figure out wtf is wrong with delta and try to fix it
create a double buffer and print without delta
print row by row without delta (worse than double buffer)
*/
EFIAPI EFI_STATUS UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Get user parameters (needs manual input option)
    CHAR16 *Path = LoadOptionsNext(LoadOptionsGet(NULL),NULL);
    if (!Path) {
        gST->ConOut->OutputString(gST->ConOut,L"ERROR: No file path specified\r\n");
        return EFI_INVALID_PARAMETER;
    }

    // Open the file
    BMP_FILE Bmp;
    EFI_STATUS Status = BmpOpen(Path,&Bmp);
    if (Status == EFI_UNSUPPORTED) {
        gST->ConOut->OutputString(gST->ConOut,L"ERROR: Incompatible file format. Only BMP files are supported.\r\n");
        return Status;
    }
    else if (Status != EFI_SUCCESS) {
        gST->ConOut->OutputString(gST->ConOut,L"ERROR: Failed to open/read file specified\r\n");
        return Status;
    }

    // Initialize the screen
    Status = ScreenInit();
    if (Status != EFI_SUCCESS) {
        gST->ConOut->OutputString(gST->ConOut,L"ERROR: Failed to initialize screen. Graphics Output Protocol may be missing.\r\n");
        return Status;
    }

    // Load and print the bitmap
    INTN X = 0, Y = 0;
    BMP_BITMAP *Bitmap = LoadBitmap(&Bmp);
    if (!Bitmap) {
        gST->ConOut->OutputString(gST->ConOut,L"ERROR: Out of resources.\r\n");
        return EFI_OUT_OF_RESOURCES;
    }

    ScreenBlackout();
    Status = PrintBitmap(Bitmap,X,Y);
    if (Status != EFI_SUCCESS) {
        gST->ConOut->OutputString(gST->ConOut,L"ERROR: Failed to fetch bitmap.\r\n");
        return Status;
    }
    
    // Runtime loop
    UINTN Index;
    EFI_INPUT_KEY Key;
    while (TRUE) {
        gBS->WaitForEvent(1,&gST->ConIn->WaitForKey,&Index);
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn,&Key);
        if (Status != EFI_SUCCESS || Key.ScanCode == SCAN_ESC) {
            break;
        }

        Status = UpdatePosition(Bitmap,&X,&Y,&Key);
        if (Status != EFI_SUCCESS) {
            break;
        }
    }
    if (Status != EFI_SUCCESS) {
        gST->ConOut->OutputString(gST->ConOut,L"ERROR: ???\r\n");
    }
 
    gBS->FreePool(Bitmap);
    BmpClose(&Bmp);
    ScreenFini();
    return Status;
}

/*
qemu-system-x86_64 -smp 4 -s -bios OVMF.fd -drive format=raw,file=fat:rw:Build/DEBUG_GCC5/X64/BmpView/BmpView/DEBUG -drive format=raw,file=fat:rw:Build/DEBUG_GCC5/X64/BmpView/BmpView/OUTPUT -display gtk,show-cursor=on,show-tabs=on -net none

*/