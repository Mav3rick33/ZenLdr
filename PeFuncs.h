#pragma once

#include <Windows.h>
#include "Macro.h"

GRP_SEC(E) UINT32 HashFunction(PVOID String, DWORD Length); 

GRP_SEC(E) PVOID LoadPebModule(DWORD Hash);

GRP_SEC(E) PVOID LoadExports(PVOID ZenImage, DWORD Hash);

GRP_SEC(E) PVOID LoadImports(PVOID ZenImage, PVOID ImportDir);

GRP_SEC(E) PVOID LoadRelocations(PVOID ZenImage, PVOID BaseRelocDir, PVOID ImageBase); 
