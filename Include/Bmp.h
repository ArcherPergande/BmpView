#ifndef __BMPVIEW_BMP_H__
#define __BMPVIEW_BMP_H__

#include <Uefi.h>

#include <Protocol/Shell.h>
#include <Protocol/GraphicsOutput.h>

#define BMP_COMPRESSION_NONE 0
#define BMP_COMPRESSION_RLE8 1
#define BMP_COMPRESSION_RLE4 2

#define BMP_FORMAT_MONO 1
#define BMP_FORMAT_PALLET4 4
#define BMP_FORMAT_PALLET8 8
#define BMP_FORMAT_RGB16 16
#define BMP_FORMAT_RGB24 24

#define BMP_SCANLINE(Width,BitsPerPixel) ((((Width * BitsPerPixel + 7) / 8) + 3) & ~3)

#pragma pack(1)

//
// FILE STRUCTURE
//

// Detailed information about bitmap.
typedef struct {
    UINT32 Size;            // Size of info header.
    UINT32 Width;           // Width of the bitmap in pixels.
    UINT32 Height;          // Height of the bitmap in pixels.
    UINT16 Planes;          // Number of planes.
    UINT16 BitsPerPixel;    // Color depth of bitmap in bits.
    UINT32 Compression;     // Type of compression (BI_RGB, BI_RLE8, BI_RLE4).
    UINT32 ImageSize;       // Size of compressed image.
    UINT32 XpixelsPerM;     // Horizontal resolution of bitmap in pixels per meter.
    UINT32 YpixelsPerM;     // Vertical resolution of bitmap in pixels per meter.
    UINT32 ColorsUsed;      // Number of actually used colors.
    UINT32 ImportantColors; // Number of important colors.
} BMP_INFO_HEADER;

// Standard file header.
typedef struct {
    CHAR8 Signature[2]; // Magic number "BM".
    UINT32 FileSize;    // Size of the file.
    UINT32 Reserved;    // Unused.
    UINT32 DataOffset;  // Offset from the beginning of the file to bitmap data
    BMP_INFO_HEADER Info;
} BMP_HEADER;

// Represents a pixel in a color lookup table. Only present in pallet and mono formats
typedef struct {
    UINT8 Red;
    UINT8 Green;
    UINT8 Blue;
    UINT8 Reserved;
} BMP_COLOR_ENTRY;

//
// BITMAP STRUCTURE
//

// Bitmap data that corresponds to BMP_FORMAT_RGB24
typedef struct {
    UINT8 Blue;
    UINT8 Green;
    UINT8 Red;
} BMP_RGB24;

// Bitmap data that corresponds to BMP_FORMAT_RGB16
typedef struct {
    UINT16 Blue : 5;
    UINT16 Green : 5;
    UINT16 Red : 5;
} BMP_RGB16;

// Bitmap data offset that corresponds to BMP_FORMAT_PALLET8
typedef UINT8 BMP_PALLET8;

// Bitmap data offset that corresponds to BMP_FORMAT_PALLET4
typedef struct {
    UINT8 Pixel1 : 4;
    UINT8 Pixel2 : 4;
} BMP_PALLET4;

// Bitmap data offset that corresponds to BMP_FORMAT_MONO
typedef struct {
    UINT8 Pixel1 : 1;
    UINT8 Pixel2 : 1;
    UINT8 Pixel3 : 1;
    UINT8 Pixel4 : 1;
    UINT8 Pixel5 : 1;
    UINT8 Pixel6 : 1;
    UINT8 Pixel7 : 1;
    UINT8 Pixel8 : 1;
} BMP_PALLET1;

#pragma pack()

//
// HIGH LEVEL REPRESENTATION
//

typedef struct {
    BMP_COLOR_ENTRY *Table;
    UINT32 Entries;
} BMP_COLOR_TABLE;

// Info must always follow directly after Header
typedef struct {
    SHELL_FILE_HANDLE FileHandle;
    BMP_HEADER Header;
    BMP_COLOR_TABLE Color;
    UINT32 Scanline;       
} BMP_FILE;

typedef struct {
    UINTN Width;
    UINTN Height;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Buffer[];
} BMP_BITMAP;

//
// PROTOTYPES
//

EFIAPI EFI_STATUS BmpOpen(CHAR16 *FilePath, BMP_FILE *Bmp);
EFIAPI VOID BmpClose(BMP_FILE *Bmp);
EFIAPI BMP_BITMAP *BmpBitmapNew(UINT32 Width, UINT32 Height);
EFIAPI VOID BmpBitmapFree(BMP_BITMAP *Bitmap);
EFIAPI EFI_STATUS BmpBitmapLoad(BMP_FILE *Bmp, BMP_BITMAP **Bitmap, UINT32 SrcX, UINT32 SrcY, UINT32 DstX, UINT32 DstY, UINT32 Width, UINT32 Height);
EFIAPI EFI_STATUS BmpBitmapCopy(BMP_BITMAP *Dst, BMP_BITMAP *Src, UINT32 SrcX, UINT32 SrcY, UINT32 DstX, UINT32 DstY, UINT32 Width, UINT32 Height);

#endif //__BMPVIEW_BMP_H__