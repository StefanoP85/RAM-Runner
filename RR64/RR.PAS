(*
 * ============================================================================
 * Project: RAM-Runner (64-bit Dual-Core DOS Engine)
 * File:    RR.PAS
 * Author:  Stefano Pollazzon (www.github.com/StefanoP85)
 * Description: Checks if START.ASM is running properly.
 * ============================================================================
*)
program RR;
uses Dos;
const
	RequestHeyYou      = $B3;
	RequestHandshake   = $00;
	RequestPhysAddress = $01;
	ResponseHereIam    = $A1C9;
var
	Regs: Registers;
	RegAX, RegDX: Word;
begin
	Regs.AX := RequestHeyYou shl 8 or RequestHandshake;
	Intr($2F, Regs);
	if Regs.AX = ResponseHereIam then
	begin
		{ SHR EAX, 16 = $66 $C1 $E8 $10 }
		asm
			MOV    AX, $B301
			INT    $2F
			MOV    [RegDX], DX
			DB $66
			DB $C1
			DB $E8
			DB $10
			MOV    [RegAX], AX
		end;
		Writeln('Physical address = ', RegDX, ':', RegAX);
	end
	else
		Writeln('You should run START.EXE!');
end.
