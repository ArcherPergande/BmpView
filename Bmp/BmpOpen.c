#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>
#include <Util.h>

/* Parses a path and returns its volume index and relative counterpart (returns -1 for invalid/relative paths) */
EFIAPI STATIC INTN PathToVolumeIndex(CHAR16 *Path, CHAR16 **Relative) {
    INTN Count = -1, Index = 2;
    if (CharToUpper(Path[0]) == L'F' && CharToUpper(Path[1]) == L'S' && Path[2] != L'\0') {
        Count = 0;
        while (Path[Index] != L':') {
            if (Path[Index] == L'\0') {
                Count = -1;
                break;
            }
            else {
                Count = Count * 10 + (Path[Index] - L'0');
            }
            Index++;
        }
        if (Relative) {
            *Relative = &Path[Index+1];
        }
    }
    
    return Count;
}

/* return must be gBS->Free()'d*/
EFIAPI STATIC CHAR16 *CpyShellVar(const CHAR16 *EnvKey) {
    const CHAR16 *Source = ShellGetEnvironmentVariable(EnvKey);
    if (!Source) {
        return NULL;
    }

    CHAR16 *Dest = NULL;
    UINTN Length = StrLen(Source)+1; // Null term
    if (gBS->AllocatePool(EfiLoaderData,Length*sizeof(CHAR16),(VOID*)&Dest) == EFI_SUCCESS) {
        StrCpyS(Dest,Length,Source);
    }

    return Dest;
}

EFIAPI STATIC UINTN ShellPathGet(CHAR16 ***Path) {
    CHAR16 *Raw = CpyShellVar(L"path");
    UINTN Count = StrCount(Raw,L';')+1; // +1 cus there is always one less ';' than paths ("FSO:\folder;FS1:\folder" - 2 paths and 1 ';')
    EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,Count*sizeof(CHAR16*),(VOID**)Path);
    if (Status != EFI_SUCCESS) {
        *Path = NULL;
        return 0;
    }

    (*Path)[0] = Raw; // [] comes before * in order of operations
    UINTN Cur = 0, Index = 1;
    while (Index <= Count) {
        if (Raw[Cur] == L';') {
            Raw[Cur] = L'\0';
            (*Path)[Index] = &Raw[Cur+1];
            Index++;
        }
        Cur++;
    }

    return Count;
}

EFIAPI STATIC VOID ShellPathFree(CHAR16 **Path) {
    if (Path) {
        gBS->FreePool(Path[0]);
        gBS->FreePool(Path);
    }
    return;
}

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
