[BITS 64]

GLOBAL GetIp

[SECTION .text$F]

GetIp:

	call ret_ptr
	
ret_ptr:

	pop rax
	sub rax, 5
	ret
	
Leave:
	db 'E', 'N', 'D', 'O', 'F', 'C', 'O', 'D', 'E'