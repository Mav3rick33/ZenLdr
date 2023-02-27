[BITS 64]

Extern ZenLdr

GLOBAL Start

[SECTION .text$A]

Start:
	
	push rsi
	mov rsi, rsp
	and rsp, 0FFFFFFFFFFFFFFF0h
	sub rsp, 020h
	call ZenLdr
	mov rsp, rsi
	pop rsi
	ret
