#ifndef __BMPVIEW_UTIL_H__
#define __BMPVIEW_UTIL_H__

#include <Uefi.h>

typedef EFI_STATUS (EFIAPI *KEYBOARD_CALLBACK_FUNC)(VOID *);

typedef struct {
    EFI_INPUT_KEY Key;
    KEYBOARD_CALLBACK_FUNC Callback;
} KEYBOARD_CALLBACK;

/*
Waits for a valid key-bind to be pressed which is represented in KeyBinds.
The associated callback function is executed and Index is updated with the 
KeyBind index that was executed starting from 1. Index 0 is interpreted as the default exit key-bind (ESC key)
and is used to exit the callback environment.
*/
EFIAPI EFI_STATUS KeyboardGetCallback(KEYBOARD_CALLBACK *KeyBinds, VOID *Param, UINTN *Index, UINTN Count);
#endif // __BMPVIEW_UTIL_H__