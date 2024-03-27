#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <LoadOptions.h>

// Extracts the first load option from the load options into a separate string. Return must call gBS->Free().
// Optionally ExtractLen is set to the length of the returned string.
EFIAPI CHAR16 *LoadOptionsExtract(CHAR16 *LoadOptions, OPTIONAL UINT32 *ExtractLen) {
    CHAR16 *Str = NULL;
    UINT32 Len = 0;
    if (LoadOptions) {
        while (LoadOptions[Len] != L' ' && LoadOptions[Len] != L'\0') {
            Len++;
        }
    }

    if (Len) {
        EFI_STATUS Status = gBS->AllocatePool(EfiLoaderData,(Len+1)*sizeof(CHAR16),(VOID*)&Str);
        if (Status == EFI_SUCCESS) {
            StrnCpyS(Str,Len+1,LoadOptions,Len);
        }
    }
    if (ExtractLen) {
        *ExtractLen = Len;
    }
    return Str;
}