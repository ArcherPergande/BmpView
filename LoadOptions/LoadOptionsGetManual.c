#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/LoadedImage.h>

#include <LoadOptions.h>

EFIAPI STATIC UINT16 AppendKey(CHAR16 *Cmd) {
    UINTN Index;
    EFI_INPUT_KEY Key = {SCAN_ESC,0};
    if (Cmd) {
        gBS->WaitForEvent(1,&gST->ConIn->WaitForKey,&Index);
        gST->ConIn->ReadKeyStroke(gST->ConIn,&Key);
        (*Cmd) = Key.UnicodeChar;
    }
    return Key.ScanCode;
}

EFIAPI EFI_STATUS OptionsGrow(CHAR16 **Prompt, UINTN *Len) {
    CHAR16 *Temp = NULL;
    UINTN TempLen = (*Len) + LOAD_MANUAL_CMD_SIZE;
    EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,TempLen*sizeof(CHAR16),(VOID*)&Temp);
    if (Status == EFI_SUCCESS) {
        if (Prompt) {
            gBS->CopyMem(Temp,*Prompt,(*Len)*sizeof(CHAR16));
            gBS->FreePool(*Prompt);
        }
        *Prompt = Temp;
        *Len = TempLen;
    }
    return Status;
}

// add left and right key navigation (i would have to zero initialize the buffer and add logic to appendkey)
EFIAPI CHAR16 *LoadOptionsGetManual(OPTIONAL UINT32 *LoadOptionsLen) {
    gST->ConOut->OutputString(gST->ConOut,L": ");

    EFI_STATUS Status;
    UINTN Cur = 0, Len = 0;
    CHAR16 *LoadOptions = NULL;
    UINT16 ScanCode;
    while (TRUE) {
        // Allocate new buffer if old is full
        if (Cur == Len) {
            Status = OptionsGrow(&LoadOptions,&Len);
            if (Status != EFI_SUCCESS) {
                gBS->FreePool(LoadOptions);
                LoadOptions = NULL;
                break;
            }
        }

        // Key callbacks
        ScanCode = AppendKey(&LoadOptions[Cur]);
        if (Status != EFI_SUCCESS || ScanCode == SCAN_ESC) {
            Cur = 0;
            break;
        }
        else if (LoadOptions[Cur] == L'\r') {
            LoadOptions[Cur] = L'\0';
            break;
        }
        else if (LoadOptions[Cur] == L'\b') {
            if (Cur > 0) {
                gST->ConOut->SetCursorPosition(gST->ConOut,gST->ConOut->Mode->CursorColumn-1,gST->ConOut->Mode->CursorRow);
                Print(L"%c",L'\0');
                gST->ConOut->SetCursorPosition(gST->ConOut,gST->ConOut->Mode->CursorColumn-1,gST->ConOut->Mode->CursorRow);
                Cur--;
            }
        }
        else if (ScanCode == SCAN_LEFT) {
            gST->ConOut->SetCursorPosition(gST->ConOut,gST->ConOut->Mode->CursorColumn-1,gST->ConOut->Mode->CursorRow);
        }
        else if (ScanCode == SCAN_RIGHT) {
            gST->ConOut->SetCursorPosition(gST->ConOut,gST->ConOut->Mode->CursorColumn+1,gST->ConOut->Mode->CursorRow);
        }
        else if (LoadOptions[Cur]) {
            Print(L"%c",LoadOptions[Cur]);
            Cur++;
        }
    }

    if (Cur == 0) {
        gBS->FreePool(LoadOptions);
        LoadOptions = NULL;
    }
    if (LoadOptionsLen) {
        *LoadOptionsLen = Cur;
    }
    gST->ConOut->OutputString(gST->ConOut,L"\r\n");
    return LoadOptions;
}