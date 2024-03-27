#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <LoadOptions.h>

/*
future code if additional params are needed
*/

// Returns the pointer to the next option in the load options. Optionally updates the load options length.
EFIAPI CHAR16 *LoadOptionsNext(CHAR16 *LoadOptions, OPTIONAL UINT32 *LoadOptionsLen) {
    UINT32 Cur = 0;
    if (LoadOptions) {
        while (LoadOptions[Cur] != L'\0') {
            if (LoadOptions[Cur] == L' ' && LoadOptions[Cur+1] != L' ') {
                Cur++;
                break;
            }
            Cur++;
        }
        if (LoadOptionsLen) {
            (*LoadOptionsLen) -= Cur;
        }
    }
    if (LoadOptions[Cur] == L'\0') {
        return NULL;
    }
    else {
        return &LoadOptions[Cur];
    }
}