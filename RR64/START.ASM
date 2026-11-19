COMMENT $
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    START.ASM
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Allocates 12 MB and installs an handler for interrupt 2Fh
 * It gives two services to the main program: the first one is a check of
 * installation, the second one is gives the physical address of the XMS Bloc.
 * ============================================================================
$
IDEAL
P586
JUMPS
LOCALS
MODEL SMALL, PASCAL

RequestHeyYou      = 0B3h   ; Byte di riconoscimento per la chiamata all'int 2Fh
RequestHandshake   = 00h    ; Richiesta se presente in RAM
RequestPhysAddress = 01h    ; Richiesta indirizzo fisico del blocco XMS
ResponseHereIam    = 0A1C9h ; Risposta di riconoscimento alla chiamata
CProgramFileSpec   EQU 'RR.EXE'

; Struttura richiesta per il servizio DOS INT 21H AH=4Bh
STRUC TExecParametersBlock
	; Segmento del blocco dell'environment
	Environment  DW ?
	; Puntatore agli argomenti della linea di comando
	CommandLine  DD ?
	; Puntatori alle strutture FCB
	FCB0         DD ?
	FCB1         DD ?
	; Contiene il valore iniziale di SS e SP al ritorno
	StackPointer DD ?
	; Contie il valore iniziale di CS e IP al ritorno
	CodePointer  DD ?
ENDS  TExecParametersBlock

MACRO MakeCString Id, Msg
	IFB <Id>
		DISPLAY "Bisogna specificare il nome della stringa in MakeCString"
		ERR
	ELSE
		IFB <Msg>
			DISPLAY "Bisogna passare la stringa a MakeCString"
			ERR
		ELSE
			Id DB &Msg&
			DB 0
		ENDIF
	ENDIF
ENDM  MakeCString
MACRO MakePascalString Id, Msg
	LOCAL EndStr
	IFB <Id>
		DISPLAY "Bisogna specificare il nome della stringa in MakePascalString"
		ERR
	ELSE
		IFB <Msg>
			DISPLAY "Bisogna passare la stringa a MakePascalString"
			ERR
		ELSE
			Id DB EndStr - Id - 1, &Msg&
			LABEL EndStr BYTE
		ENDIF
	ENDIF
ENDM  MakePascalString

STACK 100h

DATASEG
MakeCString PacManFileSpec, CProgramFileSpec
MakeCString PacManParameters, <"">
MakeCString MsgNoXMS, <"Error: XMS not found.$">
MakeCString MsgAllocErr, <"Error XMS: unable to allocate 12 MB.$">
MakeCString MsgLockErr, <"Error XMS: unable to lock the allocated 12 MB block.$">
MakeCString MsgExecErr, <"Error: unable to run the game RR.EXE.$">

UDATASEG
XMMEntryPoint DD ?
XMSHandle     DW ?
ProgramExec   TExecParametersBlock <?>

CODESEG

; Memorizza l'indirizzo fisico del blocco XMS
PhysicalAddress DD ?

; Memorizza il vettore precedente
OldInt2FHandler DD 0

PROC   NewInt2FHandler FAR
	CMP    AH, RequestHeyYou
	JNE    @@Chain
	CMP    AL, RequestHandshake
	JNE    @@TestPhysAddress
	; Servizio RequestHandshake: restituisce
	; la parola d'ordine
	MOV    AX, ResponseHereIam
	IRET
@@TestPhysAddress:
	CMP    AL, RequestPhysAddress
	JNE    @@Chain
	MOV    EAX, [CS:PhysicalAddress]
	IRET
@@Chain:
	JMP  [DWORD CS:OldInt2FHandler]
ENDP   NewInt2FHandler

PROC   DosDisplayString
ARG    @@String:PTR BYTE
	MOV    DX, [@@String]
	MOV    AH, 09h
	INT    21h
	RET
ENDP   DosDisplayString

PROC   DosGetIntVec
ARG    @@IntNo:BYTE, @@Vector:FAR PTR DWORD
USES   DS, ES
	MOV    AH, 35h
	MOV    AL, [@@IntNo]
	INT    21h
	MOV    AX, [WORD HIGH @@Vector]
	MOV    DS, AX
	MOV    SI, [WORD LOW  @@Vector]
	MOV    [SI], BX
	MOV    BX, ES
	MOV    [SI + 2], BX
	RET
ENDP   DosGetIntVec

PROC   DosSetIntVec
ARG    @@IntNo:BYTE, @@Vector:DWORD
USES   DS, ES
	MOV    AH, 25h
	MOV    AL, [@@IntNo]
	MOV    DX, [WORD HIGH @@Vector]
	MOV    DS, DX
	MOV    DX, [WORD LOW  @@Vector]
	INT    21h
	RET
ENDP   DosSetIntVec

PROC   DosExec
ARG    @@FileName:DATAPTR, @@CmdLine:DATAPTR
LOCAL  @@Params:TExecParametersBlock
USES   DS, ES, AX, BX, CX, DX, SI, DI

	MOV    [@@Params.Environment], 0
	MOV    DX, DS
	MOV    [WORD HIGH @@Params.CommandLine], DX
	MOV    DX, [@@CmdLine]
	MOV    [WORD LOW  @@Params.CommandLine], DX
	MOV    DX, CS
	MOV    AX, 0005Ch
	MOV    [WORD HIGH @@Params.FCB0], DX
	MOV    [WORD LOW  @@Params.FCB0], AX
	MOV    AX, 0006Ch
	MOV    [WORD HIGH @@Params.FCB1], DX
	MOV    [WORD LOW  @@Params.FCB1], AX

	PUSH   SS
	PUSH   BP
	PUSH   SP

	MOV    AX, 04B00h
	MOV    DX, [@@FileName]
	MOV    BX, SS
	MOV    ES, BX
	LEA    BX, [@@Params]
	INT    21h
	SETNC  AL

	POP    SP
	POP    BP
	POP    SS
	RET
ENDP   DosExec

PROC   Shrink
ARG    @@Paragraph:WORD
USES   DS, ES
	MOV    AH, 62h
	INT    21h
	MOV    ES, BX
	MOV    AH, 4Ah
	MOV    BX, [@@Paragraph]
	INT    21h
	RET
ENDP   Shrink

Begin:
	MOV    AX, @data
	MOV    DS, AX
	MOV    ES, AX

	; Modifica il blocco di memoria
	CALL   Shrink, 200h                   ; 8 KB sono sufficienti per questo stub

	; Verifica presenza XMS (INT 2Fh, AX=4300h)
	MOV    AX, 4300h
	INT    2Fh
	CMP    AL, 80h
	JE     @@XmsPresent
	CALL   DosDisplayString, OFFSET MsgNoXMS
	JMP    @@FatalError
@@XmsPresent:
	; Ottiene l'Entry Point del driver XMS (INT 2Fh, AX=4310h)
	MOV    AX, 4310h
	INT    2Fh
	MOV    [WORD LOW XMMEntryPoint], BX
	MOV    [WORD HIGH XMMEntryPoint], ES

	; Alloca 12 MB (12288 KB) usando il servizio XMS 3.0 (AH=89h)
	MOV    AH, 89h
	MOV    EDX, 12288                     ; Dimensione in KB
	CALL   [XMMEntryPoint]
	CMP    AX, 0001h
	JE     @@AllocOk
	CALL   DosDisplayString, OFFSET MsgAllocErr
	JMP    @@FatalError
@@AllocOk:
	MOV    [XMSHandle], DX                ; Salva l'handle restituito in DX

	; Blocca il blocco per ottenere l'indirizzo fisico usando il servizio XMS 3.0 (AH=8Ch)
	MOV    AH, 8Ch
	MOV    DX, [XMSHandle]
	CALL   [XMMEntryPoint]
	CMP    AX, 0001h
	JE     @@Lock30Ok
	; Fallback a XMS 2.0 (AH=0Ch -> restituisce DX:BX)
	MOV    AH, 0Ch
	MOV    DX, [XMSHandle]
	CALL   [XMMEntryPoint]
	CMP    AX, 0001h
	JE     @@Lock20Ok
	; Se entrambi i lock falliscono, dealloca e chiudi
	CALL   DosDisplayString, OFFSET MsgLockErr
	JMP    @@FreeAndExit
@@Lock30Ok:
	MOV    [CS:PhysicalAddress], EDX
	JMP    @@LockDone
@@Lock20Ok:
	MOV    [WORD LOW  CS:PhysicalAddress], BX
	MOV    [WORD HIGH CS:PhysicalAddress], DX
@@LockDone:

	; Installa il nuovo gestore dell'interrupt 2Fh
	CALL   DosGetIntVec, 2Fh, CS, OFFSET OldInt2FHandler
	CALL   DosSetIntVec, 2Fh, CS OFFSET NewInt2FHandler

	; Esegue il programma principale
	CALL   DosExec, OFFSET PacManFileSpec, OFFSET PacManParameters
	JNC    @@ProgramReturned
	
	; Se l'avvio del programma fallisce
	CALL   DosDisplayString, OFFSET MsgExecErr

@@ProgramReturned:
	; Ripristina il vecchio gestore dell'interrupt 2Fh
	CALL   DosSetIntVec, 2Fh, [CS:OldInt2FHandler]

@@FreeAndExit:
	; Sblocca il blocco XMS (AH=0Dh)
	MOV    AH, 0Dh
	MOV    DX, [XMSHandle]
	CALL   [XMMEntryPoint]

	; Dealloca il blocco XMS (AH=0Ah)
	MOV    AH, 0Ah
	MOV    DX, [XMSHandle]
	CALL   [XMMEntryPoint]

	; Esci al DOS normalmente
	MOV AX, 4C00h
	INT 21h

@@FatalError:
	MOV AX, 4C01h
	INT 21h
END Begin
