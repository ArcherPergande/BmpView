#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/LoadedImage.h>

#include <LoadOptions.h>

// Returns the application's CHAR16 load options. Optionally returns the load options length.
EFIAPI CHAR16 *LoadOptionsGet(OPTIONAL UINT32 *LoadOptionsLen) {
    EFI_LOADED_IMAGE_PROTOCOL *Img;
    EFI_GUID Guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_STATUS Status = gBS->HandleProtocol(gImageHandle,&Guid,(VOID*)&Img);
    if (Status != EFI_SUCCESS) {
        return NULL;
    }
    else if (LoadOptionsLen) {
        *LoadOptionsLen = (Img->LoadOptionsSize / sizeof(CHAR16)) - 1;
    }
    return Img->LoadOptions;
}