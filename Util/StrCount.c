#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Util.h>

EFIAPI STATIC UINTN StrCount(const CHAR16 *Str, CHAR16 Key) {
    UINTN Count = 0;
    if (Str) {
        for (UINTN Cur = 0; Str[Cur] != L'\0'; Cur++) {
            if (Str[Cur] == Key) {
                Count++;
            }
        }
    }
    
    return Count;
}
