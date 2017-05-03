#include "app_test.h"

#include "sdk/os/debug.h"
#include "sdk/os/process.h"
#include <sdk/scr/screen.h>

#define NEXT_BYTE caddr++; i++; instr = *caddr;

#define IMM32 NEXT_BYTE unsigned long imm32 = instr; NEXT_BYTE imm32 = imm32 | instr << 8; NEXT_BYTE imm32 = imm32 | instr << 16; NEXT_BYTE imm32 = imm32 | instr << 32;

int run(){
	sdk_debug_log("IT's MY TRIUMPH");
	return 0;
}

char * alu_opcode[] = {"AND", "EOR", "SUB", "RSB", "ADD", "OP ", "SLL", "SRL", "SRA"};

void app_test(void){	
	Canvas * canvas = (Canvas *)sdk_prc_getCanvas();
	unsigned char * start_addr = asm_test;
	
	sdk_scr_printf(canvas, "DUMP: 0x%x\n", start_addr);
	
	for (int i = 0; i < 40; i++){
		sdk_scr_printf(canvas, "\n");
		unsigned char * caddr = start_addr + i;
		if (*caddr >= 10)
			sdk_scr_printf(canvas, " 0x%x    %x", caddr, *caddr);
		else 
			sdk_scr_printf(canvas, " 0x%x    %x ", caddr, *caddr);
		
		unsigned char instr = *caddr;
		
		// ALU
		if (instr >= 0 && instr <= 0x2F){
			if (instr >= 0 && instr <=0x0F){
				int opcode = instr % 16;			
				NEXT_BYTE
				int reg = instr / 16;
				int rop1 = instr % 16;
				NEXT_BYTE
				int rop2 = instr / 16;
				sdk_scr_printf(canvas, "    %s R%d, R%d, R%d", alu_opcode[opcode], reg, rop1, rop2);
			} else if (instr > 0x0F && instr <=0x1F){
				int opcode = instr % 16;			
				NEXT_BYTE
				int reg = instr / 16;
				int rop1 = instr % 16;
				NEXT_BYTE
				int imm8 = instr;
				sdk_scr_printf(canvas, "    %s R%d, R%d, %d", alu_opcode[opcode], reg, rop1, imm8);
			} else if (instr > 0x1F && instr <=0x2F){
				int opcode = instr % 16;			
				NEXT_BYTE
				int reg = instr / 16;
				int rop1 = instr % 16;
				IMM32
				sdk_scr_printf(canvas, "    %s R%d, R%d, %lu", alu_opcode[opcode], reg, rop1, imm32);
			}
			continue;
		}
				
		if (instr == 0x38){
			sdk_scr_printf(canvas, "    NOP");
		} else if (instr == 0x60){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    MOV R%d, R%d", instr / 16, instr % 16);
		} else if (instr == 0x61){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    MOV R%d, %d", instr / 16, instr % 16);
		} else if (instr == 0x62){
			NEXT_BYTE
			int reg = instr / 16;
			NEXT_BYTE
			sdk_scr_printf(canvas, "    MOV R%d, %d", reg, instr);
		} else if (instr == 0x63){
			NEXT_BYTE
			int reg = instr / 16;
			IMM32
			sdk_scr_printf(canvas, "    MOV R%d, %lu", reg, imm32);
		}
	}
	
	
	while(1){
		sdk_prc_sleep_until_new_messages();
	}
}