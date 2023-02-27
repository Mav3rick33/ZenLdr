#pragma once

#include <Windows.h>
#include "Macro.h"
#include "Ntdll.h"

static ULONG_PTR Start(VOID);
static ULONG_PTR GetIp(VOID);

typedef BOOL(WINAPI* ZenDllMain)(HINSTANCE Dll, DWORD Reason, LPVOID Reserved);

typedef struct _IMAGE_RELOC {
    WORD Offset : 12;
    WORD Type : 4;
} IMAGE_RELOC, * PIMAGE_RELOC;


typedef struct {

    struct {

        WIN32(RtlAnsiStringToUnicodeString); 
        WIN32(NtAllocateVirtualMemory); 
        WIN32(NtProtectVirtualMemory); 
        WIN32(LdrGetProcedureAddress);
        WIN32(RtlFreeUnicodeString);
        WIN32(RtlInitAnsiString);
        WIN32(LdrLoadDll); 
    }Function;

    struct {

        PVOID Ntdll; 
    }Dll;

}RESOLVE, *PRESOLVE;





