#include <Uefi.h>
#include <Library/ShellLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

EFIAPI STATIC VOID AlignRawBuffer(UINT8 *RawBuffer, UINTN Size, UINT32 StartX, UINT16 BitsPerPixel) {
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

// add some parsing for when the bitmap is compressed??? very rare to encounter one
EFIAPI STATIC EFI_STATUS ReadRow(BMP_FILE *Bmp, UINT8 *Raw, UINT32 Row, UINT32 StartX, UINT32 NumPixels) {
    Row = Bmp->Header.Info.Height-1-Row; // Invert index because bmp files store bitmap in reverse order.

    UINTN Offset = Bmp->Header.DataOffset + Bmp->Scanline * Row + ((StartX * Bmp->Header.Info.BitsPerPixel) / 8);
    EFI_STATUS Status = ShellSetFilePosition(Bmp->FileHandle,Offset);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    UINTN Size = (NumPixels * Bmp->Header.Info.BitsPerPixel + 7) / 8;
    Status = ShellReadFile(Bmp->FileHandle,&Size,(VOID*)Raw);
    if (Status == EFI_SUCCESS) {
        AlignRawBuffer(Raw,Size,StartX,Bmp->Header.Info.BitsPerPixel);
    }

    return Status;
}

EFIAPI STATIC VOID Rgb24ToBlt(BMP_FILE *Bmp, UINT8 *Raw, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt, UINT32 Width, UINT32 Height) {
    UINT32 Index;
    BMP_RGB24 *Row;
    for (UINT32 Y = 0; Y < Height; Y++) {
        Row = (BMP_RGB24*)(Raw + Y * Bmp->Scanline);
        for (UINT32 X = 0; X < Width; X++) {
            Index = Y * Width + X;

            Blt[Index].Red   = Row[X].Red;
            Blt[Index].Green = Row[X].Green;
            Blt[Index].Blue  = Row[X].Blue;
            Blt[Index].Reserved = 0;
        }
    }
    return;
}

EFIAPI STATIC VOID Rgb16ToBlt(BMP_FILE *Bmp, UINT8 *Raw, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt, UINT32 Width, UINT32 Height) {
    UINT32 Index;
    BMP_RGB16 *Row;
    for (UINT32 Y = 0; Y < Height; Y++) {
        Row = (BMP_RGB16*)(Raw + Y * Bmp->Scanline);
        for (UINT32 X = 0; X < Width; X++) {
            Index = Y * Width + X;

            Blt[Index].Red   = Row[X].Red;
            Blt[Index].Green = Row[X].Green;
            Blt[Index].Blue  = Row[X].Blue;
            Blt[Index].Reserved = 0;
        }
    }
    return;
}

EFIAPI STATIC VOID PalletToBlt(BMP_FILE *Bmp, UINT8 *Raw, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt, UINT32 Width, UINT32 Height) {
    UINT16 BitsPerPixel = Bmp->Header.Info.BitsPerPixel;
    UINT8 Bitmask = (1 << BitsPerPixel) - 1;

    UINT32 BitOffset, BltIndex, ColorIndex;
    for (UINT32 Y = 0; Y < Height; Y++) {
        for (UINT32 X = 0; X < Width; X++) {
            BitOffset = 8 - BitsPerPixel * (X % (8 / BitsPerPixel)) - BitsPerPixel;
            ColorIndex = (Raw[(Y * Bmp->Scanline) + (X / (8 / BitsPerPixel))] >> BitOffset) & Bitmask;
            BltIndex = Y * Width + X;

            Blt[BltIndex].Red   = Bmp->Color.Table[ColorIndex].Red;
            Blt[BltIndex].Green = Bmp->Color.Table[ColorIndex].Green;
            Blt[BltIndex].Blue  = Bmp->Color.Table[ColorIndex].Blue;
            Blt[BltIndex].Reserved = 0;
        }
    }
    return;
}

// should work with overlapping Dst and Src bitmaps
EFIAPI STATIC inline EFI_STATUS ParseBitmap(BMP_FILE *Bmp, BMP_BITMAP *Bitmap, UINT32 X, UINT32 Y, UINT32 Width, UINT32 Height) {
    if (!Bmp || !Bitmap) {
        return EFI_INVALID_PARAMETER;
    }
    else if (Width > Bmp->Header.Info.Width || Height > Bmp->Header.Info.Height || X >= Bmp->Header.Info.Width || Y >= Bmp->Header.Info.Height) {
        return EFI_BAD_BUFFER_SIZE;
    }
    else if (Width > Bitmap->Width || Height > Bitmap->Height) {
        return EFI_BUFFER_TOO_SMALL;
    }

    // Allocate space for raw data
    VOID *Raw;
    UINTN Size = Bmp->Header.Info.Height * Bmp->Scanline;
    EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,Size,&Raw);
    if (Status != EFI_SUCCESS) {
        return Status;
    }
    
    // lookup table for parsing raw data (index = BitsPerPixel / 8)
    VOID (EFIAPI *Parse[4])(BMP_FILE *Bmp, UINT8 *Raw, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Blt, UINT32 Width, UINT32 Height) = {
        PalletToBlt,
        PalletToBlt,
        Rgb16ToBlt,
        Rgb24ToBlt
    };

    // Parse each row of the raw data
    for (UINT32 Cur = 0; Cur < Height; Cur++) {
        Status = ReadRow(Bmp,Raw,Y+Cur,X,Width);
        if (Status == EFI_SUCCESS) {
            Parse[Bmp->Header.Info.BitsPerPixel / 8](Bmp,(UINT8*)Raw,&Bitmap->Buffer[Cur * Bitmap->Width],Width,1);
        }
        else {
            break;
        }
    }

    gBS->FreePool(Raw);
    return Status;
}

EFIAPI EFI_STATUS BmpBitmapLoad(BMP_FILE *Bmp, BMP_BITMAP **Bitmap, UINT32 SrcX, UINT32 SrcY, UINT32 DstX, UINT32 DstY, UINT32 Width, UINT32 Height) {
    // Set default width and height if necessary
    Width = (Width == 0) ? (Bmp->Header.Info.Width - SrcX) : Width;
    Height = (Height == 0) ? (Bmp->Header.Info.Height - SrcY) : Height;
    
    // Create a temporary bitmap
    BMP_BITMAP *Temp = BmpBitmapNew(Width,Height);
    if (!Temp) {
        return EFI_OUT_OF_RESOURCES;
    }
    
    // Parse the bmp file into the temporary buffer
    EFI_STATUS Status = ParseBitmap(Bmp,Temp,SrcX,SrcY,Width,Height);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    // Set Temp to Bitmap if Bitmap is not allocated otherwise copy Temp into Bitmap
    if (*Bitmap == NULL) {
        *Bitmap = Temp;
    }
    else {
        Status = BmpBitmapCopy(*Bitmap,Temp,0,0,DstX,DstY,Width,Height);
        BmpBitmapFree(Temp);
    }

    return Status;
}
