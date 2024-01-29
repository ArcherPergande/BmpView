#include <Uefi.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

EFIAPI STATIC EFI_STATUS ReadHeader(SHELL_FILE_HANDLE FileHandle, BMP_HEADER *Header) {
    if (!Header) {
        return EFI_INVALID_PARAMETER;
    }
    
    UINTN ReadSize = sizeof(BMP_HEADER);
    ShellSetFilePosition(FileHandle,0);
    EFI_STATUS Status = ShellReadFile(FileHandle,&ReadSize,Header);
    if (Status == EFI_SUCCESS && (Header->Signature[0] != 'B' || Header->Signature[1] != 'M')) {
        Status = EFI_UNSUPPORTED;
    }
    return Status;
}

EFIAPI STATIC EFI_STATUS ReadColorTable(SHELL_FILE_HANDLE FileHandle, UINT16 BitsPerPixel, BMP_COLOR_TABLE *Color) {
    EFI_STATUS Status = EFI_SUCCESS;

    if (!Color) {
        return EFI_INVALID_PARAMETER;
    }

    switch (BitsPerPixel) {
        case BMP_FORMAT_MONO:
            Color->Entries = 2;
            break;
        case BMP_FORMAT_PALLET4:
            Color->Entries = 16;
            break;
        case BMP_FORMAT_PALLET8:
            Color->Entries = 256;
            break;
        default:
            Color->Entries = 0;
    }

    if (Color->Entries) {
        UINTN ReadSize = sizeof(BMP_COLOR_ENTRY) * Color->Entries;
        Status = gBS->AllocatePool(EfiLoaderData,ReadSize,(VOID*)&Color->Table);
        if (Status == EFI_SUCCESS) {
            ShellSetFilePosition(FileHandle,sizeof(BMP_HEADER));
            Status = ShellReadFile(FileHandle,&ReadSize,Color->Table);
        }
    }
    else {
        Color->Table = NULL;
    }

    return Status;
}

EFIAPI EFI_STATUS BmpOpen(CHAR16 *FilePath, BMP_FILE *Bmp) {
    if (!FilePath || !Bmp) {
        return EFI_INVALID_PARAMETER;
    }

    EFI_STATUS Status = ShellOpenFileByName(FilePath,&Bmp->FileHandle,EFI_FILE_MODE_READ,0);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    Status = ReadHeader(Bmp->FileHandle,&Bmp->Header);
    if (Status == EFI_SUCCESS) {
        Bmp->Scanline = BMP_SCANLINE(Bmp->Header.Info.Width,Bmp->Header.Info.BitsPerPixel); // Align to 4 byte boundary
        Status = ReadColorTable(Bmp->FileHandle,Bmp->Header.Info.BitsPerPixel,&Bmp->Color);
    }

    return Status;
}