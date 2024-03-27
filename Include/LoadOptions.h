#include <Uefi.h>

#define LOAD_MANUAL_CMD_SIZE (256)

EFIAPI CHAR16 *LoadOptionsGet(OPTIONAL UINT32 *LoadOptionsLen);
EFIAPI CHAR16 *LoadOptionsNext(CHAR16 *LoadOptions, OPTIONAL UINT32 *LoadOptionsLen);
EFIAPI CHAR16 *LoadOptionsGetManual(OPTIONAL UINT32 *LoadOptionsLen);
EFIAPI CHAR16 *LoadOptionsExtract(CHAR16 *LoadOptions, OPTIONAL UINT32 *ExtractLen);