
; vim: set ft=nasm noet ts=4:

%define STDIN_FILENO	0
%define STDOUT_FILENO	1
%define STDERR_FILENO	2

%define AT_EMPTY_PATH	0x1000
%define P_ALL			4

%define SYS_memfd_create	356
%define SYS_fork			2
%define SYS_waitpid			7
%define SYS_execve			11
%define SYS_open			5
%define SYS_lseek			19
%define SYS_dup2			63
%define SYS_execveat		358
%define SYS_vfork			190

%define EBP_bias			((_start-ehdr)-(__strempty-__self))

bits 32

org 0xEBDB0000

ehdr:	 ;~e_ident
		 ; jg short 0x47 (inc ebp) ; dec esp ; inc esi
	  db 0x7F,"ELF"						;!EI_MAGIC
%ifndef NO_FILE_MANAGER_COMPAT
	  db 0x01							;!EI_CLASS
	  db 0x01							;!EI_DATA
%endif
_parent.0:
%ifdef NO_FILE_MANAGER_COMPAT
	 xor ebx, ebx						; EI_CLASS, EI_DATA
%endif
	xchg eax, esi ; esi is zero now		; EI_VERSION
	 mov al, SYS_waitpid				; EI_OSABI, EI_ABIVERSION
	 int 0x80							; EI_PAD1, EI_PAD2

%ifdef NO_CHEATING
	 pop eax							; EI_PAD3
	 sub esp, 0x18 ; FIXME: HACK (see below) ; EI_PAD4..6
%else
	 lea esi, [esp + 0x08]				; EI_PAD3..6
%endif

	  db 0x3D ; cmp eax, ...			; EI_PAD7
	  dw 2								;!e_type
	  dw 3								;!e_machine
_parent.1:
%ifdef NO_CHEATING
	push edi ; subs 4 from esp, so we need to take this into account in the
             ; HACK lines as well		; e_version0
	 lea esi, [esp+4*eax+0x20] ; FIXME: HACK ; e_version1..3, !e_entry0
		 ; the last byte here^^^^ needs to be 0x20 to have a correct entrypoint
		 ; if you want to optimize this, you probably want to move the entrypt
		 ; somewhere else, but that means you have to change everything else
		 ; with it as well, so good luck.
%else
	 mov edx, esp 						; e_version0..1
	 lea ecx, [ebp+__strempty-__self+EBP_bias] ; e_version2..3, !e_entry0
%endif
	 add bl, bl							;!e_entry1..2
	  db 0xEB 	; jmp short _parent.2	;!e_entry3
	  dd phdr-ehdr	; 0x33				;!e_phoff
_start:
	 mov ax, SYS_memfd_create			; e_shoff
	 mov ebx, esp						; e_flags0..1
	 jmp short _start.1					; e_flags2..3
	  dw ehdr.end-ehdr	; 0x34 0x00		;!e_ehdrsize
	  dw phdr.end-phdr	; 0x20 0x00		;!e_phentsize
	  dw 1								;!e_phnum
_start.1:
	 int 0x80							; e_shentsize
%ifdef NO_CHEATING
	xchg edi, eax						; e_shnum0
%else
	 pop eax							; e_shnum0
%endif
	 jmp short _start.2					; e_shnum1, e_shstrndx0
phdr:
	  db 1					;! p_type0	; e_shstrndx1
ehdr.end:
   times 3 db 0				;! p_type1
	  dd 0					;!p_offset
	  dd ehdr				;!p_vaddr
_start.2:
	 mov ebp, __self-EBP_bias;!p_paddr, p_filesz0
	 jmp short _start.3		;~p_filesz1..2
	  db 0					;~p_filesz3
	  db 0xEB				;~p_memsz0
	 jmp short _start.3+4	;~p_memsz1..2
	  db 0					;~p_memsz3
	  db 5					;~p_flags0
_start.3:
%ifdef USE_VFORK
	 mov al, SYS_vfork		;~p_flags1..2
%else
	 mov al, SYS_fork		;~p_flags1..2
%endif
	 jmp short _start.4		;~p_flags3, p_align0

_parent.2:
%ifdef NO_CHEATING
	 pop ebx				; p_align1
phdr.endm2:
phdr.end equ phdr.endm2 + 2
	 lea ecx, [ebp+__strempty-__self+EBP_bias] ; p_align2..3, ...
%ifdef WANT_ARGV
	 lea edx, [esp+0x18]
%else
	xchg edx, esp
%endif
%else
	 mov bl, 3				; p_align1..2
phdr.endm1:
phdr.end equ phdr.endm1 + 1
%endif
	 mov ax, SYS_execveat	; p_align3, ...
	 mov di, AT_EMPTY_PATH
	;int 0x80 ;; fallthru to _start.4

_start.4:
	 int 0x80

%ifndef NO_FILE_MANAGER_COMPAT
	 xor ebx, ebx
%endif
	test eax, eax
	 jnz short _parent.0

_child:
		 ; dup stdout->demo
	 mov al, SYS_dup2
	xchg ebx, edi
	 mov cl, STDOUT_FILENO
	 int 0x80

		 ; open self for payload reading
	 lea ebx, [ebp+EBP_bias]
	 mov al, SYS_open
	 dec cl
	 int 0x80

		 ; seek
	 mov ebx, eax
	 mov al, SYS_lseek
	 mov cl, payload - ehdr
	 int 0x80

		 ; dup2 self->stdin
	 mov al, SYS_dup2
	 mov cl, STDIN_FILENO
	 int 0x80

		 ; execve
	 mov al, SYS_execve

	 lea ebx, [ebp+__zip-__self+EBP_bias]
%ifndef USE_GZIP
	push ecx
	push ebx
%endif
	xchg ecx, esp
	 int 0x80

__self:
	db '/proc/self/exe'
__strempty:
	db 0
__zip:
%ifdef USE_GZIP
	db '/bin/zcat',0
%else
%ifndef NO_UBUNTU_COMPAT
	db '/usr'
%endif
	db '/bin/xzcat',0
%endif

; if you insist
%ifdef TAG
	db TAG
%endif

END:

filesize equ END - ehdr

payload:

%if (_parent.2 - ehdr) != 0x50
%error "_parent.2: bad offset"
%endif

