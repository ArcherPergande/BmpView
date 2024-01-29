#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Util.h>

/*
there should be an implicate 0th index being SCAN_ESC that can be used for a default exit
*/
EFIAPI EFI_STATUS KeyboardGetCallback(KEYBOARD_CALLBACK *KeyBinds, VOID *Param, UINTN *Index, UINTN Count) {
    EFI_INPUT_KEY Key = {0,0};
    EFI_STATUS Status;
    UINTN Cur;

    // WaitForEvent requires TPL_APPLICATION
    gBS->RestoreTPL(TPL_APPLICATION);

    while(TRUE) {
        gBS->WaitForEvent(1,&gST->ConIn->WaitForKey,&Cur);
        Status = gST->ConIn->ReadKeyStroke(gST->ConIn,&Key);
        if (Status != EFI_SUCCESS) {
            return Status;
        }

        if (Key.ScanCode == SCAN_ESC && Key.UnicodeChar == 0) {
            Cur = 0;
            break;
        }
        for (Cur = 0; Cur < Count; Cur++) {
            if (Key.ScanCode == KeyBinds[Cur].Key.ScanCode && Key.UnicodeChar == KeyBinds[Cur].Key.UnicodeChar) {
                Status = KeyBinds[Cur].Callback(Param);
                Cur++;
                break;
            }
        }
    }

    if (Index) {
        *Index = Cur;
    }
    return Status;
}