CC_X64	:= x86_64-w64-mingw32-gcc

CFLAGS	:= $(CFLAGS) -Os -fno-asynchronous-unwind-tables -nostdlib 
CFLAGS 	:= $(CFLAGS) -fno-ident -fpack-struct=8 -falign-functions=1
CFLAGS  := $(CFLAGS) -s -ffunction-sections -falign-jumps=1 -w
CFLAGS	:= $(CFLAGS) -falign-labels=1 -fPIC -Wl,-TSectionLink.ld
LFLAGS	:= $(LFLAGS) -Wl,-s,--no-seh,--enable-stdcall-fixup

OUTX64	:= ZenLdr.x64.exe
BINX64	:= ZenLdr.x64.bin

all:
	@ echo [+] Compiling ZenLdr
	@ nasm -f win64 asm/Start.asm -o Start.x64.o
	@ nasm -f win64 asm/GetIp.asm -o GetIp.x64.o
	@ $(CC_X64) *.c Start.x64.o GetIp.x64.o -o $(OUTX64) $(CFLAGS) $(LFLAGS) -I.
	@ echo [+] Extracting .text section into $(BINX64)
	@ python3 python3/extract.py -f $(OUTX64) -o $(BINX64)

clean:
	@ rm -rf *.o
	@ rm -rf *.bin
	@ rm -rf *.exe
