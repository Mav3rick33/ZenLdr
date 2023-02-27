#include "General.h"

GRP_SEC(E) UINT32 HashFunction(PVOID String, DWORD Length) {

	UCHAR		CurrChar		= 0; 
	ULONG		Hash			= HASH_KEY; 
	PUCHAR		CharPtr			= NULL; 

	CharPtr = (PVOID)String; 

	while (TRUE) {

		CurrChar = *CharPtr; 

		if (!Length) {

			if (!*CharPtr) {

				break; 
			}
		}
		else {

			if ((ULONG)(CharPtr - (PUCHAR)String) >= Length) {

				break; 
			}

			if (!*CharPtr) {

				CharPtr++;

				continue; 

			}
		}

		if (CurrChar >= 'a') {

			CurrChar -= 0x20; 
		}


		Hash = ((Hash << 5) + Hash) + CurrChar; 

		CharPtr++;
	}

	return Hash; 
}

GRP_SEC(E) PVOID LoadPebModule(DWORD Hash) {

	PLDR_DATA_TABLE_ENTRY	ModuleLdr = NULL; 
	PLIST_ENTRY				PebModule = NULL; 
	PLIST_ENTRY				NextEntry = NULL;

	PebModule = &((PPEB)PebLdr)->Ldr->InLoadOrderModuleList; 

	NextEntry = PebModule->Flink; 

	do {
		
		ModuleLdr = (PLDR_DATA_TABLE_ENTRY)NextEntry;
		 
		
		if (HashFunction(ModuleLdr->BaseDllName.Buffer, ModuleLdr->BaseDllName.Length) == Hash) {
			
			return ModuleLdr->DllBase;
		}
		
		NextEntry = NextEntry->Flink;

	} while (PebModule != NextEntry);

	return NULL; 
}

GRP_SEC(E) PVOID LoadExports(PVOID ZenImage, DWORD Hash) {

	PIMAGE_EXPORT_DIRECTORY		ExportDir	= NULL; 
	PIMAGE_DATA_DIRECTORY		DataDir		= NULL;
	PIMAGE_NT_HEADERS			NtHeaders	= NULL; 
	PIMAGE_DOS_HEADER			DosHeader	= NULL; 

	PDWORD	NameAddress		= NULL;
	PDWORD	FuncAddress		= NULL;
	PWORD	OrdAddress		= NULL;

	DosHeader	= (PIMAGE_DOS_HEADER)ZenImage; 
	

	NtHeaders	= CONVERT(PIMAGE_NT_HEADERS, DosHeader, DosHeader->e_lfanew); 
	

	DataDir		= &NtHeaders->OptionalHeader.DataDirectory[0]; 

	if (DataDir->VirtualAddress) {

		ExportDir	= CONVERT(PIMAGE_EXPORT_DIRECTORY, DosHeader, DataDir->VirtualAddress); 
		

		NameAddress 	= CONVERT(PDWORD, DosHeader, ExportDir->AddressOfNames);
	

		FuncAddress 	= CONVERT(PDWORD, DosHeader, ExportDir->AddressOfFunctions); 
		

		OrdAddress	= CONVERT(PWORD, DosHeader, ExportDir->AddressOfNameOrdinals);  
	

		for (DWORD Index = 0; ExportDir->NumberOfNames != 0; Index++) {

			if (HashFunction(CONVERT(PVOID, DosHeader, NameAddress[Index]), 0) == Hash) {

				return CONVERT(PVOID, DosHeader, FuncAddress[OrdAddress[Index]]); 
			}
		}
	}

	return NULL; 
}

GRP_SEC(E) PVOID LoadImports(PVOID ZenImage, PVOID ImportDir) {

	PIMAGE_IMPORT_DESCRIPTOR	ImportDesc	= NULL; 
	PIMAGE_IMPORT_BY_NAME		ImportName	= NULL; 
	PIMAGE_THUNK_DATA			OrgThunk	= NULL;
	PIMAGE_THUNK_DATA			FirstThunk	= NULL;

	UNICODE_STRING	UniDllName	= { 0 }; 
	ANSI_STRING		AniDllName	= { 0 };
	RESOLVE			Resolve		= { 0 }; 
	
	NTSTATUS		Status		= NULL;
	HMODULE			DllHandle	= NULL; 
	PVOID			FuncAddr	= NULL;
	PCHAR			Name		= NULL;

	Resolve.Dll.Ntdll = LoadPebModule(NTDLL_HASH);

	Resolve.Function.RtlAnsiStringToUnicodeString	= LoadExports(Resolve.Dll.Ntdll, RTLANSISTRINGTOUNICODESTRING_HASH);
	Resolve.Function.LdrGetProcedureAddress			= LoadExports(Resolve.Dll.Ntdll, LDRGETPROCEDUREADDRESS_HASH);
	Resolve.Function.RtlFreeUnicodeString			= LoadExports(Resolve.Dll.Ntdll, RTLFREEUNICODESTRING_HASH);
	Resolve.Function.RtlInitAnsiString				= LoadExports(Resolve.Dll.Ntdll, RTLINITANSISTRING_HASH);
	Resolve.Function.LdrLoadDll						= LoadExports(Resolve.Dll.Ntdll, LDRLOADDLL_HASH);

	

	for (ImportDesc = (PVOID)ImportDir; ImportDesc->Name != 0; ImportDesc++) {

		Name = CONVERT(PVOID, ZenImage, ImportDesc->Name); 

		Resolve.Function.RtlInitAnsiString(&AniDllName, Name); 

		Status = Resolve.Function.RtlAnsiStringToUnicodeString(&UniDllName, &AniDllName, TRUE);

		if (NT_SUCCESS(Status)) {

			Status = Resolve.Function.LdrLoadDll(NULL, 0, &UniDllName, &DllHandle); 

			if (NT_SUCCESS(Status)) {

				OrgThunk	= CONVERT(PIMAGE_THUNK_DATA, ZenImage, ImportDesc->OriginalFirstThunk); 
				

				FirstThunk	= CONVERT(PIMAGE_THUNK_DATA, ZenImage, ImportDesc->FirstThunk); 
				


				while (OrgThunk->u1.AddressOfData != 0) {

					if (IMAGE_SNAP_BY_ORDINAL(OrgThunk->u1.Ordinal)) {

						Status = Resolve.Function.LdrGetProcedureAddress(DllHandle, 0, IMAGE_ORDINAL(OrgThunk->u1.Ordinal), &FuncAddr); 

						if (NT_SUCCESS(Status)) {

							FirstThunk->u1.Function = FuncAddr; 
						}
					}
					else {

						ImportName = CONVERT(PIMAGE_IMPORT_BY_NAME, ZenImage, OrgThunk->u1.AddressOfData); 
					

						Resolve.Function.RtlInitAnsiString(&AniDllName, (PVOID)ImportName->Name);

						Status = Resolve.Function.LdrGetProcedureAddress(DllHandle, &AniDllName, 0, &FuncAddr); 

						if (NT_SUCCESS(Status)) {

							FirstThunk->u1.Function = FuncAddr;
						}

					}

					OrgThunk++; 

					FirstThunk++; 
				}
			}

			Resolve.Function.RtlFreeUnicodeString(&UniDllName); 
		}


	}
}

GRP_SEC(E) PVOID LoadRelocations(PVOID ZenImage, PVOID BaseRelocDir, PVOID ImageBase) {

	PIMAGE_BASE_RELOCATION	ImgBaseReloc	= NULL; 
	PIMAGE_RELOC			Relocation		= NULL;
	PBYTE					Delta			= 0; 


	ImgBaseReloc	= (PVOID)BaseRelocDir; 

	Delta			= (PVOID)((ULONG_PTR)ZenImage - (ULONG_PTR)ImageBase); 

	for (; ImgBaseReloc->VirtualAddress != 0; ImgBaseReloc = (PVOID)Relocation) {

		Relocation = (PIMAGE_RELOC)(ImgBaseReloc + 1); 

		for (; (PBYTE)Relocation != CONVERT(PBYTE, ImgBaseReloc, ImgBaseReloc->SizeOfBlock); Relocation++){ 

			switch (Relocation->Type) {

				case IMAGE_REL_BASED_ABSOLUTE:

					break; 

				case IMAGE_REL_TYPE:

					*(ULONG_PTR*)((PBYTE)ZenImage + ImgBaseReloc->VirtualAddress + Relocation->Offset) += (ULONG_PTR)Delta; 

				
			}
		}

	}


}
