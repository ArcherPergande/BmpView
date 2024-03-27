#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

// Aligns bitmaps whose startx is in the middle of a byte
EFIAPI VOID AlignRawBuffer(UINT8 *RawBuffer, UINTN Size, UINT32 StartX, UINT16 BitsPerPixel) {
    UINT8 BitsToShift = (BitsPerPixel == BMP_FORMAT_PALLET4) ? 4 : StartX % 8;
    if (BitsPerPixel <= 4 && BitsPerPixel) {
        UINT8 Temp;
        UINT8 Bitmask = (1 << BitsToShift) - 1;
        for (UINTN Cur = 1; Cur < Size; Cur++) {
            Temp = (RawBuffer[Cur] >> (8 - BitsToShift)) & Bitmask;
            RawBuffer[Cur-1] = (RawBuffer[Cur-1] << BitsToShift) & ~Bitmask;
            RawBuffer[Cur-1] |= Temp;
        }

        RawBuffer[Size-1] = RawBuffer[Size-1] << BitsToShift & ~Bitmask;
    }
    
    return;
}

// Parses a bitmap row and converts it to a blt row. Assumes sizeof(Raw) = Bmp->Scanline.
EFIAPI VOID PalletToBlt(BMP_FILE *Bmp, UINT8 *Raw, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt, UINT32 Width) {
    UINT16 BitsPerPixel = Bmp->Header.Info.BitsPerPixel;
    UINT8 Bitmask = (1 << BitsPerPixel) - 1;

    UINT32 BitOffset, ColorIndex;
    for (UINT32 Cur = 0; Cur < Width; Cur++) {
        BitOffset = 8 - BitsPerPixel * (Cur % (8 / BitsPerPixel)) - BitsPerPixel;
        ColorIndex = (Raw[Cur / (8 / BitsPerPixel)] >> BitOffset) & Bitmask;

        Blt[Cur].Red   = Bmp->Color.Table[ColorIndex].Red;
        Blt[Cur].Green = Bmp->Color.Table[ColorIndex].Green;
        Blt[Cur].Blue  = Bmp->Color.Table[ColorIndex].Blue;
        Blt[Cur].Reserved = 0;
    }

    return;
}

// Parses a bitmap row and converts it to a blt row. Assumes sizeof(Raw) = Bmp->Scanline.
EFIAPI VOID Rgb16ToBlt(BMP_FILE *Bmp, UINT8 *Raw, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt, UINT32 Width) {
    BMP_RGB16 *Row = (BMP_RGB16*)Raw;
    for (UINT32 X = 0; X < Width; X++) {
        Blt[X].Red   = Row[X].Red;
        Blt[X].Green = Row[X].Green;
        Blt[X].Blue  = Row[X].Blue;
        Blt[X].Reserved = 0;
    }
    return;
}

// Parses a bitmap row and converts it to a blt row. Assumes sizeof(Raw) = Bmp->Scanline.
EFIAPI VOID Rgb24ToBlt(BMP_FILE *Bmp, UINT8 *Raw, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt, UINT32 Width) {
    BMP_RGB24 *Row = (BMP_RGB24*)Raw;
    for (UINT32 X = 0; X < Width; X++) {
        Blt[X].Red   = Row[X].Red;
        Blt[X].Green = Row[X].Green;
        Blt[X].Blue  = Row[X].Blue;
        Blt[X].Reserved = 0;
    }
    return;
}

// all the size checks should be done when allocating the buffer not after
// move this shit to BmpBitmapLoad bozo
EFIAPI EFI_STATUS ReadRow(BMP_FILE *Bmp, UINT8 *Raw, UINTN ReadSize, UINT32 X, UINT32 Y) {
    Y = Bmp->Header.Info.Height-1-Y;

    UINTN Temp;
    UINTN Position = Bmp->Header.DataOffset + Bmp->Scanline * Y + ((X * Bmp->Header.Info.BitsPerPixel) / 8);
    EFI_STATUS Status = Bmp->FileHandle->SetPosition(Bmp->FileHandle,Position);
    if (Status == EFI_SUCCESS) {
        Temp = ReadSize;
        Status = Bmp->FileHandle->Read(Bmp->FileHandle,&Temp,(VOID*)Raw);
        if (Temp != ReadSize) {
            Status = EFI_END_OF_FILE;
        }
    }

    AlignRawBuffer(Raw,ReadSize,X,Bmp->Header.Info.BitsPerPixel);
    return Status;
}

// EFIAPI VOID BmpBitmapPrint(BMP_BITMAP *Bitmap, UINT32 X, UINT32 Y) {
//     if (Bitmap) {
//         UINTN Index;
//         for (; Y < Bitmap->Height; Y++) {
//             for (; X < Bitmap->Width; X++) {
//                 Index = Y * Bitmap->Width + X;
//                 Print(L"Red %d | Green %d | Blue %d\r\n",Bitmap->Buffer[Index].Red,Bitmap->Buffer[Index].Green,Bitmap->Buffer[Index].Blue);
//             }
//             Print(L"\r\n");
//         }
//     }
//     return;
// }

EFIAPI EFI_STATUS BmpBitmapLoad(BMP_FILE *Bmp, BMP_BITMAP *Bitmap, UINT32 X, UINT32 Y) {
    // Parsing logic
    VOID (EFIAPI *Parse[4])(BMP_FILE*, UINT8*, EFI_GRAPHICS_OUTPUT_BLT_PIXEL*, UINT32) = {
        PalletToBlt,
        PalletToBlt,
        Rgb16ToBlt,
        Rgb24ToBlt
    };
    if (!Bmp || !Bitmap || X >= Bmp->Header.Info.Width || Y >= Bmp->Header.Info.Height) {
        return EFI_INVALID_PARAMETER;
    }

    // Align row width to coordinates and allocate read buffer
    UINT8 *Raw;
    UINT32 RawWidth = (Bmp->Header.Info.Width-X < Bitmap->Width) ? Bmp->Header.Info.Width-X : Bitmap->Width;
    UINTN RawSize = (RawWidth * Bmp->Header.Info.BitsPerPixel + 7) / 8;
    EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,RawSize,(VOID*)&Raw);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    // Iterate through bitmap rows
    for (UINT32 Cur = 0; Cur < Bitmap->Height; Cur++) {
        Status = ReadRow(Bmp,Raw,RawSize,X,Y);
        if (Status != EFI_SUCCESS) {
            break;
        }
        Parse[Bmp->Header.Info.BitsPerPixel/8](Bmp,Raw,&Bitmap->Buffer[Cur*Bitmap->Width],RawWidth);
        Y++;
    }

    gBS->FreePool(Raw);
    return Status;
}