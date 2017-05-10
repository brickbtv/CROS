#include "sdk/os/debug.h"
#include "sdk/os/process.h"
#include "sdk/clk/clock.h"
#include "sdk/scr/screen.h"
#include "sdk/dkc/disk_drive.h"
#include "sdk/kyb/keyboard.h"
#include "sdk/nic/network.h"

#include "stdlib/stdlib_shared.h"
#include "stdlib/stdio_shared.h"
#include "stdlib/string_shared.h"

#define NEXT_BYTE caddr++; instr = *caddr;

#define IMM32 NEXT_BYTE unsigned long imm32 = instr; NEXT_BYTE imm32 = imm32 | instr << 8; NEXT_BYTE imm32 = imm32 | instr << 16; NEXT_BYTE imm32 = imm32 | instr << 24;
#define SIMM32 NEXT_BYTE long imm32 = instr; NEXT_BYTE imm32 = imm32 | instr << 8; NEXT_BYTE imm32 = imm32 | instr << 16; NEXT_BYTE imm32 = imm32 | instr << 24;
#define IMM16 NEXT_BYTE unsigned long imm16 = instr; NEXT_BYTE imm16 = imm16 | instr << 8; 
#define OFFSET(a)unsigned int offset = 0; if (a == 1){NEXT_BYTE offset = instr; } else if (a == 2){ IMM16 offset = imm16;} else if (a == 3){ IMM32 offset = imm32; }

#define ASM_NO_ARGS(a, b) if (instr == a){ sdk_scr_printf(canvas, b); return caddr;}

char * alu_opcode[] = {"AND", "EOR", "SUB", "RSB", "ADD", "OP ", "SLL", "SRL", "SRA"};
char * mul_div[] = {"SMUL", "UMUL", "SDIV", "UDIV"};
char * push_pop[] = {"PUSH", "POP"};
char * mrs_msr[] = {"MRS", "MSR"};
char * branch_suffix[] = {"EQ", "NE", "CS/HS", "CC/LO", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "AL"};

typedef struct STR_SYMBOL{
	unsigned int address;
	char name[64];
} STR_SYMBOL;

list_t * sym_table;

int get_symbol(int address, char * name){
	list_node_t * node = sym_table->head;
	while (node){
		STR_SYMBOL *s = (STR_SYMBOL *)node->val;
		if (s->address == address){
			strcpy(name, s->name);
			return 1;
		}
		node = node->next;
	}
	return 0;
}

char* reverse_string(char *str)
{
    char temp;
    size_t len = strlen(str) - 1;
    size_t stop = len/2;
    size_t i,k;

    for(i = 0, k = len; i < stop; i++, k--)
    {
        temp = str[k];
        str[k] = str[i];
        str[i] = temp;
    }
    return str;
}

#define ADD_SYMBOL(a){STR_SYMBOL s1;	strcpy(s1.name, #a);	s1.address = (int)a;list_rpush(sym_table, list_node_new(&s1));} 

void init_symbol_table(){
	sym_table = list_new();

	ADD_SYMBOL(sdk_debug_log);
	ADD_SYMBOL(sdk_debug_logf);
	
	ADD_SYMBOL(sdk_prc_create_process);
	ADD_SYMBOL(sdk_prc_die);
	ADD_SYMBOL(sdk_prc_getCanvas);
	ADD_SYMBOL(sdk_prc_getHeapPointer);
	ADD_SYMBOL(sdk_prc_get_scheduler_list);
	ADD_SYMBOL(sdk_prc_handleMessage);
	ADD_SYMBOL(sdk_prc_haveNewMessage);
	ADD_SYMBOL(sdk_prc_is_focused);
	ADD_SYMBOL(sdk_prc_lock);
	ADD_SYMBOL(sdk_prc_sleep);
	ADD_SYMBOL(sdk_prc_sleep_until_new_messages);
	ADD_SYMBOL(sdk_prc_unlock);
	
	ADD_SYMBOL(sdk_scr_clearArea);
	ADD_SYMBOL(sdk_scr_clearScreen);
	ADD_SYMBOL(sdk_scr_printf);
	ADD_SYMBOL(sdk_scr_printfXY);
	ADD_SYMBOL(sdk_scr_printfXY_no_variadic);
	ADD_SYMBOL(sdk_scr_printf_no_variadic);
	ADD_SYMBOL(sdk_scr_putchar);
	ADD_SYMBOL(sdk_scr_setBackColor);
	ADD_SYMBOL(sdk_scr_setTextColor);
	
	ADD_SYMBOL(sdk_clk_readCountdownTimer);
	ADD_SYMBOL(sdk_clk_setCountdownTimer);
	ADD_SYMBOL(sdk_clk_timeSinceBoot);
	
	ADD_SYMBOL(sdk_dkc_get_disk_info);
	ADD_SYMBOL(sdk_dkc_isReady);
	ADD_SYMBOL(sdk_dkc_read);
	ADD_SYMBOL(sdk_dkc_write);
	
	ADD_SYMBOL(sdk_nic_recv);
	ADD_SYMBOL(sdk_nic_send);
	ADD_SYMBOL(sdk_nic_sendf);
}

unsigned char * decode_instruction(Canvas * canvas, unsigned char * caddr){
	sdk_scr_printf(canvas, "\n");
	sdk_scr_setTextColor(canvas, SCR_COLOR_GREEN);
	//unsigned char * caddr = start_addr + i;
	if (*caddr >= 16)
		sdk_scr_printf(canvas, " 0x%x    %x", caddr, *caddr);
	else 
		sdk_scr_printf(canvas, " 0x%x    %x ", caddr, *caddr);
	
	sdk_scr_setTextColor(canvas, SCR_COLOR_WHITE);
	
	unsigned char instr = *caddr;
	
	// ALU
	if (instr >= 0 && instr <= 0x2F){
		int code = instr;
		int opcode = instr % 16;			
		NEXT_BYTE
		int reg = instr / 16;
		int rop1 = instr % 16;
		if (code >= 0 && code <=0x0F){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    %s R%d, R%d, R%d", alu_opcode[opcode], reg, rop1, instr / 16);
		} else if (code > 0xF && code <=0x1F){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    %s R%d, R%d, %d", alu_opcode[opcode], reg, rop1, instr);
		} else if (code > 0x1F && code <=0x2F){
			IMM32
			sdk_scr_printf(canvas, "    %s R%d, R%d, %lu", alu_opcode[opcode], reg, rop1, imm32);
		}
		return caddr;
	}
	
	// Multiplication/division
	if (instr >= 0x3 && instr <= 0x37){
		int opcode = instr % 16;
		NEXT_BYTE
		int r0 = instr / 16;
		int r1 = instr % 16;
		NEXT_BYTE
		int rop1 = instr / 16;
		int rop2 = instr % 16;
		if (opcode % 2 == 1){
			IMM32
			sdk_scr_printf(canvas, "    %s R%d, R%d, R%d, %lu", mul_div[opcode / 2], r0, r1, rop1, imm32);
		} else {
			sdk_scr_printf(canvas, "    %s R%d, R%d, R%d, R%d", mul_div[opcode / 2], r0, r1, rop1, rop2);
		}
			
		return caddr;
	}
	
	ASM_NO_ARGS(0x38, "    NOP");
	ASM_NO_ARGS(0x39, "    DBGBRK");
	ASM_NO_ARGS(0x69, "    HWI");
	ASM_NO_ARGS(0x6A, "    SWI");
	ASM_NO_ARGS(0x6B, "    HLT");	
	
	if (instr == 0x3A){
		NEXT_BYTE
		int rdest = instr / 16;
		int rval = instr % 16;
		NEXT_BYTE
		sdk_scr_printf(canvas, "    SWP R%d, R%d, R%d, %c", rdest, rval, instr / 16, (instr%16 == 0)?'B':'W');
		return caddr;
	}
	
	if (instr == 0x3B){
		NEXT_BYTE
		sdk_scr_printf(canvas, "    NOT R%d, R%d", instr / 16, instr % 16);
		return caddr;
	}
	
	if (instr == 0x3C || instr == 0x3D){
		int code = instr;
		NEXT_BYTE
		int rbase = instr / 16;
		int flags = instr % 16;
		NEXT_BYTE
		int byte1 = instr;
		NEXT_BYTE
		char flags_s[256];
		itoa(flags, flags_s, 2);		
		char registers_s[256];
		itoa(byte1, registers_s, 2);
		char * p = reverse_string(registers_s);			
		
		char registers_s2[256];
		itoa(instr, registers_s2, 2);
		char * p2 = reverse_string(registers_s2);	
		
		char reglist[] = "0000000000000000";
		
		strncpy(reglist, p, strlen(p));
		strncpy(&reglist[8], p2, strlen(p2));
				
		sdk_scr_printf(canvas, "    %s R%d, b%s, b%s", push_pop[code % 2], rbase, flags_s, reglist);
		return caddr;
	}
	
	if (instr == 0x3E || instr == 0x3F){
		int code = instr;
		NEXT_BYTE
		sdk_scr_printf(canvas, "    %s R%d", mrs_msr[code % 2], instr / 16);
		return caddr;
	}
	
	// store/load single
	if (instr >= 0x40 && instr <= 0x53){
		int code = instr;
		int args = instr % 16;
		
		int c = args >> 2;
		int a = args % 4;
		
		NEXT_BYTE
		int rbase = instr / 16;
		int flags = instr % 16;
		
		int size =  flags % 4;
		
		OFFSET(a)
		char cmd[4] = "STR";
		if (code >= 0x50)
			strcpy(cmd, "LDR");
		
		if (c == 0){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    %s [R%d+%x], R%d", cmd, rbase, offset, instr / 16);
		} else {
			OFFSET(c)
			sdk_scr_printf(canvas, "    %s [R%d+%x], %d", cmd, rbase, offset, offset);
		}
		return caddr;
	}
				
	// mov
	if (instr >= 0x60 && instr <= 0x63){
		int code = instr;
		NEXT_BYTE
		if (code == 0x60){
			sdk_scr_printf(canvas, "    MOV R%d, R%d", instr / 16, instr % 16);
		} else if (code == 0x61){
			sdk_scr_printf(canvas, "    MOV R%d, %d", instr / 16, instr % 16);
		} else if (code == 0x62){
			int reg = instr / 16;
			NEXT_BYTE
			sdk_scr_printf(canvas, "    MOV R%d, %d", reg, instr);
		} else if (code == 0x63){
			int reg = instr / 16;
			IMM32
			sdk_scr_printf(canvas, "    MOV R%d, %lu", reg, imm32);
		}
		
		return caddr; 
	}
	
	// CMP
	if (instr >= 0x64 && instr <= 0x67){
		int code = instr;
		NEXT_BYTE
		if (code == 0x64){
			sdk_scr_printf(canvas, "    CMP R%d, R%d", instr / 16, instr % 16);
		} else if (code == 0x65){
			sdk_scr_printf(canvas, "    CMP R%d, %d", instr / 16, instr % 16);
		} else if (code == 0x66){
			int reg = instr / 16;
			NEXT_BYTE
			sdk_scr_printf(canvas, "    CMP R%d, %d", reg, instr);
		} else if (code == 0x67){
			int reg = instr / 16;
			IMM32
			sdk_scr_printf(canvas, "    CMP R%d, %lu", reg, imm32);
		}
		
		return caddr; 
	}
	
	if (instr == 0x68){
		NEXT_BYTE
		sdk_scr_printf(canvas, "    RDTSC R%d, R%d", instr / 16, instr % 16);
		return caddr;
	}
			
	// branches 
	if (instr >= 0x80 && instr <= 0xBF){
		int base = (int)caddr;
		int code = instr;
		int cond = instr % 16;
		if (instr % 2 == 0){
			SIMM32				
			char name[64] = "";
			int fres = get_symbol(base+imm32, name);
			sdk_scr_printf(canvas, "    B%s %ld (0x%x)", branch_suffix[cond], imm32, base+imm32);
			if (fres > 0){
				sdk_scr_setTextColor(canvas, SCR_COLOR_RED);
				sdk_scr_printf(canvas, " %s", name);
				sdk_scr_setTextColor(canvas, SCR_COLOR_WHITE);
			}
		} else {
			NEXT_BYTE
			sdk_scr_printf(canvas, "    B%s R%x", branch_suffix[cond], instr / 16);
		}
		return caddr;
	}
	
	// Floating point extension
	
	if (instr == 0xff){
		//NEXT_BYTE
		
		return caddr;
	}
	
	// UNDOCUMENTED
	
	//memcpy
	if (instr == 0x6D){
		NEXT_BYTE
		int r0 = instr / 16;
		int r1 = instr % 16;
		NEXT_BYTE
		sdk_scr_printf(canvas, "    MEMCPY R%d, R%d, R%d", r0, r1, instr / 16);
		return caddr;
	}
	
	return ++caddr;
}

