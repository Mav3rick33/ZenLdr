#pragma once

/*Macros used within ZenLdr functions*/

#ifdef _WIN64
#define PebLdr __readgsqword(0x60)
#else
#define PebLdr __readfsdword(0x30)
#endif 

#ifdef _WIN64
#define IMAGE_REL_TYPE IMAGE_REL_BASED_DIR64
#else
#define IMAGE_REL_TYPE IMAGE_REL_BASED_HIGHLOW
#endif

#define HASH_KEY	5381

#define RTLANSISTRINGTOUNICODESTRING_HASH		0x6c606cba
#define NTALLOCATEVIRTUALMEMORY_HASH			0xf783b8ec
#define NTPROTECTVIRTUALMEMORY_HASH				0x50e92888
#define LDRGETPROCEDUREADDRESS_HASH				0xfce76bb6
#define RTLFREEUNICODESTRING_HASH				0x61b88f97
#define RTLINITANSISTRING_HASH					0xa0c8436d
#define LDRLOADDLL_HASH							0x9e456a43
#define NTDLL_HASH								0x1edab0ed


#define SYMBOL( x )						( ULONG_PTR )( GetIp( ) - ( ( ULONG_PTR ) & GetIp - ( ULONG_PTR ) x ) )

#define GRP_SEC( x )					__attribute__(( section( ".text$" #x ) ))

#define	WIN32( x )						__typeof__( x ) * x

#define MemCpy							__builtin_memcpy

#define CONVERT(type, base, rva)		(type)((ULONG_PTR)base + rva)

#ifdef _WIN64 
#define CODE_END( x )	(ULONG_PTR)( GetIp( ) + 11 )
#else
#define CODE_END( x )	(ULONG_PTR)( GetIp( ) + 10 )
#endif
