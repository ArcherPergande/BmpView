#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/MmMp.h>
#include <Protocol/MpService.h>

//
// just some test code for utilizing multiple cores (deprecated)
//

// EFI_MP_SERVICES_PROTOCOL *Mp;

// /* Context for worker cores parsing bitmap data */
// typedef struct {
//     BMP_FILE *Bmp;                      /* The bitmap file */
//     BMP_BITMAP *Bitmap;                 /* Parsed bitmap */
//     volatile WORKER_JOB *Job;           /* Linked list of waiting jobs */
//     volatile WORKER_JOB *Finished;      /* Tail linked list of finished jobs */
//     volatile UINT32 Lock;               /* Spin-lock */
// } WORKER_CONTEXT;

// /* A row of bitmap data that requires parsing */
// typedef struct _WORKER_JOB {
//     struct _WORKER_JOB *Next; /* The next row to parse */
//     UINT8 Data[]; /* The current row to parse */
// } WORKER_JOB;

// EFIAPI VOID Worker(WORKER_CONTEXT *Context) {
//     UINTN Temp;
//     Mp->WhoAmI(Mp,&Temp);

//     while (Context->RowsNext < Context->Bitmap->Height) {
//         if (Context->RowsAvailable && __sync_lock_test_and_set(&Context->Lock, 1) == 0) {
//             Temp = Context->RowsNext;
//             Context->RowsAvailable--;
//             Context->RowsNext++;
//             __sync_synchronize();
//             __sync_lock_release(&Context->Lock);

//             // parse shit here
//         }
//     }
//     return;
// }

// // idk if this even fucking works
// EFIAPI EFI_STATUS WorkerContextFill(BMP_FILE *Bmp, WORKER_CONTEXT *Context, UINT32 BitmapWidth, UINT32 BitmapHeight) {
//     if (!Bmp || !Context) {
//         return EFI_INVALID_PARAMETER;
//     }

//     // Determine total number of workers (need all, not enabled bc volatile indices from Mp->WhoAmI())
//     UINTN ApCount, Temp;
//     EFI_STATUS Status = Mp->GetNumberOfProcessors(Mp,&ApCount,&Temp);
//     if (Status != EFI_SUCCESS) {
//         return Status;
//     }

//     // Allocate thread specific memory
//     Temp = Bmp->Scanline * ApCount;
//     Status = gBS->AllocatePool(EfiLoaderData,Temp,(VOID**)&Context->TssRaw);
//     if (Status != EFI_SUCCESS) {
//         return Status;
//     }

//     // Allocate bitmap memory
//     Context->Bitmap = BmpBitmapNew(BitmapWidth,BitmapHeight);
//     if (!Context->Bitmap) {
//         gBS->FreePool(Context->TssRaw);
//         return EFI_OUT_OF_RESOURCES;
//     }
    
//     // Null-initialize everything else
//     Context->Bmp = Bmp;
//     Context->RowsAvailable = 0;
//     Context->RowsNext = 0;
//     Context->Lock = 0;
//     return EFI_SUCCESS;
// }

// EFIAPI VOID WorkerContextFree(WORKER_CONTEXT *Context) {
//     if (Context) {
//         gBS->FreePool(Context->TssRaw);
//         BmpBitmapFree(Context->Bitmap);
//     }
//     return;
// }
