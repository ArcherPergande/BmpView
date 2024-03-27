#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Bmp.h>

typedef struct {
    UINTN Count;
    EFI_FILE_PROTOCOL *Root[];
} VOLUME_ROOT;

typedef struct {
    const CHAR16 *Path;
    UINTN Start;
    UINTN End;
} PATH_ITERATOR;

EFIAPI EFI_STATUS VolumeRootOpen(VOLUME_ROOT **Vol) {
    UINTN NoProtocols;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **Fs = NULL;
    EFI_STATUS Status = EfiLocateProtocolBuffer(&gEfiSimpleFileSystemProtocolGuid,&NoProtocols,(VOID***)&Fs);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    UINTN Size = sizeof(VOLUME_ROOT) + (NoProtocols*sizeof(EFI_FILE_PROTOCOL*));
    Status = gBS->AllocatePool(EfiLoaderData,Size,(VOID**)Vol);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    for (UINTN Cur = 0; Cur < NoProtocols; Cur++) {
        Status = Fs[Cur]->OpenVolume(Fs[Cur],&(*Vol)->Root[Cur]);
        if (Status != EFI_SUCCESS) {
            (*Vol)->Root[Cur] = NULL;
        }
    }

    gBS->FreePool(Fs);
    (*Vol)->Count = NoProtocols;
    return Status;
}

EFIAPI VOID VolumeRootClose(VOLUME_ROOT *Vol) {
    for (UINTN Cur = 0; Cur < Vol->Count; Cur++) {
        Vol->Root[Cur]->Close(Vol->Root[Cur]);
    }
    gBS->FreePool(Vol);
    return;
}

/* Parses a path and returns its volume index and relative counterpart (returns -1 for invalid/relative paths) */
EFIAPI INTN VolumePathToIndex(CHAR16 *Path, CHAR16 **Relative) {
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

EFIAPI UINT8 PathIteratorGet(PATH_ITERATOR *Iter) {
    Iter->Start = 0;
    Iter->End = 0;
    Iter->Path = ShellGetEnvironmentVariable(L"path");
    if (Iter->Path) {
        while (TRUE) {
            if (Iter->Path[Iter->End] == L';' || Iter->Path[Iter->End] == L'\0') {
                return 0;
            }
            Iter->End++;
        }
    }
    return 1;
}

EFIAPI UINT8 PathIteratorNext(PATH_ITERATOR *Iter) {
    if (Iter && Iter->Path[Iter->End] != L'\0') {
        UINTN Cur = Iter->End+1;
        while (TRUE) {
            if (Iter->Path[Cur] == L';' || Iter->Path[Cur] == L'\0') {
                Iter->Start = Iter->End+1;
                Iter->End = Cur;
                return 0;
            }
            Cur++;
        }
    }
    return 1;
}

EFIAPI EFI_STATUS OpenAbsolute(CHAR16 *Absolute, VOLUME_ROOT *Vol, EFI_FILE_PROTOCOL **NewHandle) {
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    CHAR16 *Relative;
    INTN Index = VolumePathToIndex(Absolute,&Relative);
    if (Index >= 0 && Index < Vol->Count) {
        Status = Vol->Root[Index]->Open(Vol->Root[Index],NewHandle,Relative,EFI_FILE_READ_ONLY,0);
    }
    return Status;
}

/* 
Attempts to open the relative path from the current working directory.
Absolute is the allocated string of cwd + relative and must be freed by gBS->FreePool(). (Absolute is volatile)
*/
EFIAPI EFI_STATUS OpenCwd(CHAR16 *Relative, CHAR16 **Absolute, VOLUME_ROOT *Vol, EFI_FILE_PROTOCOL **File) {
    EFI_STATUS Status = EFI_NOT_FOUND;
    *Absolute = NULL;

    const CHAR16 *Cwd = ShellGetEnvironmentVariable(L"cwd");
    UINTN CwdLen = StrLen(Cwd), RelativeLen = StrLen(Relative);
    if (Cwd) {
        // Allocate space for string.
        UINTN AllocLen = CwdLen + RelativeLen + 2;
        Status = gBS->AllocatePool(EfiLoaderData,AllocLen*sizeof(CHAR16),(VOID**)Absolute);
        if (Status != EFI_SUCCESS) {
            return Status;
        }

        // Merge strings and format.
        StrnCpyS(*Absolute,AllocLen,Cwd,CwdLen);
        (*Absolute)[CwdLen] = L'\\';
        StrnCpyS((*Absolute)+CwdLen+1,AllocLen-CwdLen-1,Relative,RelativeLen);

        // Try to open.
        Status = OpenAbsolute(*Absolute,Vol,File);
    }
    return Status;
}

EFIAPI EFI_STATUS OpenRelative(CHAR16 *Relative, VOLUME_ROOT *Vol, EFI_FILE_PROTOCOL **File) {
    CHAR16 *Absolute = NULL;
    EFI_STATUS Status = OpenCwd(Relative,&Absolute,Vol,File);
    if (Status == EFI_SUCCESS || Status != EFI_NOT_FOUND) {
        gBS->FreePool(Absolute);
        return Status;
    }

    PATH_ITERATOR Iter;
    UINTN CurrentLen = StrLen(Absolute), RelativeLen = StrLen(Relative), Temp;
    PathIteratorGet(&Iter);
    do {
        // Allocate new string if old is too small.
        Temp = (Iter.End - Iter.Start) + RelativeLen + 1;
        if (Temp > CurrentLen) {
            gBS->FreePool(Absolute);
            Status = gBS->AllocatePool(EfiLoaderData,Temp*sizeof(CHAR16),(VOID*)&Absolute);
            if (Status != EFI_SUCCESS) {
                break;
            }
            else {
                CurrentLen = Temp;
            }
        }

        // Merge strings together.
        Temp = Iter.End-Iter.Start;
        StrnCpyS(Absolute,CurrentLen,&Iter.Path[Iter.Start],Temp);
        StrnCpyS(Absolute+Temp,CurrentLen-Temp,Relative,RelativeLen);

        // Try to open.
        Status = OpenAbsolute(Absolute,Vol,File);
        if (Status == EFI_SUCCESS) {
            break;
        }
    }
    while (!PathIteratorNext(&Iter));

    gBS->FreePool(Absolute);
    return Status;
}

EFIAPI STATIC EFI_STATUS ReadHeader(EFI_FILE_PROTOCOL *FileHandle, BMP_HEADER *Header) {
    FileHandle->SetPosition(FileHandle,0);
    UINTN Size = sizeof(BMP_HEADER);
    EFI_STATUS Status = FileHandle->Read(FileHandle,&Size,Header);
    if (Status == EFI_SUCCESS && (Header->Signature[0] != 'B' || Header->Signature[1] != 'M')) {
        Status = EFI_UNSUPPORTED;
    }
    return Status;
}

EFIAPI STATIC EFI_STATUS ReadColorTable(EFI_FILE_PROTOCOL *FileHandle, UINT16 BitsPerPixel, BMP_COLOR_TABLE *Color) {
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
            FileHandle->SetPosition(FileHandle,sizeof(BMP_HEADER));
            Status = FileHandle->Read(FileHandle,&ReadSize,Color->Table);
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

    VOLUME_ROOT *Vol;
    EFI_STATUS Status = VolumeRootOpen(&Vol);
    if (Status != EFI_SUCCESS) {
        return Status;
    }

    if ((Status = OpenAbsolute(FilePath,Vol,&Bmp->FileHandle)) == EFI_SUCCESS || (Status = OpenRelative(FilePath,Vol,&Bmp->FileHandle)) == EFI_SUCCESS) {
        Status = ReadHeader(Bmp->FileHandle,&Bmp->Header);
        if (Status == EFI_SUCCESS) {
            Bmp->Scanline = BMP_SCANLINE(Bmp->Header.Info.Width,Bmp->Header.Info.BitsPerPixel); // Align to 4 byte boundary
            Status = ReadColorTable(Bmp->FileHandle,Bmp->Header.Info.BitsPerPixel,&Bmp->Color);
        }
    }

    gBS->FreePool(Vol);
    return Status;
}