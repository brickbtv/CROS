#include "asm_decoder.h"

#include "sdk/os/debug.h"
#include "sdk/os/process.h"
#include "sdk/clk/clock.h"
#include "sdk/scr/screen.h"
#include "sdk/scr/ScreenClass.h"
#include "sdk/dkc/disk_drive.h"
#include "sdk/kyb/keyboard.h"
#include "sdk/nic/network.h"

#include "stdlib/stdlib_shared.h"
#include "stdlib/stdio_shared.h"
#include "stdlib/string_shared.h"
#include "stdlib/extern/tlsf/tlsf.h"

#include "utils/filesystem/filesystem.h"
#include "utils/gui/gui.h"
#include "utils/gui/GuiClass.h"
#include "utils/gui/charmap/charmap.h"
#include "utils/timers_and_clocks/timers.h"

#define NEXT_BYTE caddr++; instr = *caddr;

#define IMM32 NEXT_BYTE unsigned long imm32 = instr; NEXT_BYTE imm32 = imm32 | instr << 8; NEXT_BYTE imm32 = imm32 | instr << 16; NEXT_BYTE imm32 = imm32 | instr << 24;
#define SIMM32 NEXT_BYTE long imm32 = instr; NEXT_BYTE imm32 = imm32 | instr << 8; NEXT_BYTE imm32 = imm32 | instr << 16; NEXT_BYTE imm32 = imm32 | instr << 24;
#define IMM16 NEXT_BYTE unsigned long imm16 = instr; NEXT_BYTE imm16 = imm16 | instr << 8; 
#define OFFSET(a)unsigned int offset = 0; if (a == 1){NEXT_BYTE offset = instr; } else if (a == 2){ IMM16 offset = imm16;} else if (a == 3){ IMM32 offset = imm32; }

#define ASM_NO_ARGS(a, b) if (instr == a){ sdk_scr_printf(canvas, b); return caddr;}

char * alu_opcode[] = {"AND", "EOR", "SUB", "RSB", "ADD", "OP ", "SLL", "SRL", "SRA"};
char * mul_div[] = {"SMUL", "UMUL", "SDIV", "UDIV"};
char * push_pop[] = {"PUSH", "POP"};
char * push_pop_f[] = {"FPUSH", "FPOP"};
char * mrs_msr[] = {"MRS", "MSR"};
char * branch_suffix[] = {"EQ", "NE", "CS/HS", "CC/LO", "MI", "PL", "VS", "VC", "HI", "LS", "GE", "LT", "GT", "LE", "AL"};

char * alu_f_opcode[] = {"FADD", "FSUB", "FMUL", "FDIV", "FPOW", "FMOD"};
char * instr_f[] = {"FFIX", "FFLT", "FMOV", "FMVN", "FABS", "FRND", "FSQT", "FLOG", "FLGN", "FEXP", "FSIN", "FCOS", "FTAN", "FASN", "FACS"};

int get_symbol(int address, char * name, list_t * sym_table){
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

#define ADD_SYMBOL(a){STR_SYMBOL * s1 = malloc(sizeof(STR_SYMBOL));	strcpy(s1->name, #a);	s1->address = (int)a;list_rpush(sym_table, list_node_new(s1));} 
list_t * sym_table = 0;

list_t * init_symbol_table(){
	if (sym_table)
		return sym_table;
		
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
	ADD_SYMBOL(sdk_prc_get_total_memory);
	
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
	
	ADD_SYMBOL(sdk_nic_getAddress);
	ADD_SYMBOL(sdk_nic_recv);
	ADD_SYMBOL(sdk_nic_send);
	ADD_SYMBOL(sdk_nic_sendf);
	
	ADD_SYMBOL(fs_mount_drive);
	ADD_SYMBOL(fs_make_filesystem);
	ADD_SYMBOL(fs_open_file);
	ADD_SYMBOL(fs_close_file);
	ADD_SYMBOL(fs_write_file);
	ADD_SYMBOL(fs_write_file_binary);
	ADD_SYMBOL(fs_read_file);

	ADD_SYMBOL(fs_mkdir);
	ADD_SYMBOL(fs_remove);

	ADD_SYMBOL(fs_opendir);
	ADD_SYMBOL(fs_closedir);
	ADD_SYMBOL(fs_readdir);

	ADD_SYMBOL(fs_exists);

	ADD_SYMBOL(fs_unlink);
	ADD_SYMBOL(fs_chdir);
	ADD_SYMBOL(fs_chdrive);
	ADD_SYMBOL(fs_getcwd);

	ADD_SYMBOL(fs_seek);

	ADD_SYMBOL(fs_getsize);
	
	ADD_SYMBOL(strlen);
	ADD_SYMBOL(find);
	ADD_SYMBOL(strncpy);
	ADD_SYMBOL(strcpy);
	ADD_SYMBOL(strcmp);
	ADD_SYMBOL(strncmp);
	ADD_SYMBOL(atoi);
	ADD_SYMBOL(sprintf);
	
	ADD_SYMBOL(malloc);
	ADD_SYMBOL(calloc);
	ADD_SYMBOL(free);
	
	ADD_SYMBOL(gui_charmap_draw_blink);
	ADD_SYMBOL(gui_charmap_get_symbol);
	ADD_SYMBOL(gui_charmap_handleMessage);
	ADD_SYMBOL(gui_charmap_new);
	ADD_SYMBOL(gui_charmap_redraw);
	ADD_SYMBOL(gui_charmap_set_symbol);
	ADD_SYMBOL(gui_draw_area);
	ADD_SYMBOL(gui_draw_bottom);
	ADD_SYMBOL(gui_draw_header);
	
	ADD_SYMBOL(timers_add_timer);
	ADD_SYMBOL(timers_del_timer);
	ADD_SYMBOL(timers_handleMessage);
	
	ADD_SYMBOL(tlsf_get_pool);
	ADD_SYMBOL(tlsf_walk_pool);
	
	ADD_SYMBOL(ScreenClass_ctor);
	ADD_SYMBOL(GuiClass_ctor);	
	
	return sym_table;
}

char * opcodes_min[] = {"and (logical AND)",  "eor (exclusive OR)",  "sub (subtraction)",  "rsb (reverse subtraction)",  "add (addition)",  "or  (logical OR)",  "sll (shift left logical/Shift left arithmetic)",  "srl (shift right logical)",  "sra (shift right arithmetic)",  "nop (no operation)",  "movrim0 (set rim0 upper 24 bits)",  "movrim1 (set rim1 upper 24 bits)",  "smul (signed multiplication)",  "umul (unsigned multiplication)",  "sdiv (signed division)",  "udiv (unsigned division)",  "strb (store byte)",  "strh (store half-word)",  "str  (store word)",  "ldrsb (load signed byte)",  "ldrub (load unsigned byte)",  "ldrsh (load signed half-word)",  "ldruh (load unsigned half-word)",  "ldrw  (load word)",  "not (bitwise NOT)",  "mov (move immediate/register to register)",  "cmp (compare and set flags)",  "msr/mrs (Store/Load flags register)",  "stm (store multiple)",  "ldm (load multiple)",  "dbgbrk (debug break)",  "nextirq (get next irq information)",  "rdtsc (read time stamp counter)",  "hwf (hardware function)",  "swi (software interrupt. aka: System call)",  "hlt (halt cpu)",  "ctxswitch (context switch)",  "memcpy (memory copy)",  "memset (memory set)",  "cmpxchg (compare and exchange)",  "RESERVED",  "RESERVED",  "RESERVED",  "RESERVED",  "RESERVED",  "RESERVED",  "RESERVED",  "RESERVED",  "beq (branch if equal) Z==1",  "bne (branch if not equal) Z==0",  "bcs/bhs (Carry set/ if unsigned higher or same) C==1",  "bcc/blo (Carry clear / if unsigned lower) C==0",  "bmi (if Negative set) N==1",  "bpl (if Negative clear) N==0",  "bvs (if Overflow set) V==1",  "bvc (if Overflow clear) V==0",  "bhi (if unsigned higher) C==1 && Z==0",  "bls (if unsigned lower or same) C==0 || Z==1",  "bge (if signed greater than or equal) N==V",  "blt (if signed less than) N!=V",  "bgt (if signed greater than) Z==0 && N==V",  "ble (if signed less than or equal) Z==1 || N!=V",  "b (branch always)",  "RESERVED",  "bleq (branch with link if equal)",  "blne (branch with link if not equal)",  "blcs/blhs",  "blmi",  "blpl",  "blvs",  "blvc",  "blhi",  "blhi",  "blls",  "blge",  "bllt",  "blgt",  "blle",  "bl",  "RESERVED",  "fadd (floating point addition)",  "fsub (floating point subtraction)",  "fmul (floating point multiplication)",  "fdiv (floating point division)",  "fpow (raise to power)",  "fmod (floating point remainder)",  "RESERVED",  "RESERVED",  "fstrs (floating point store single)",  "fstrd (floating point store double)",  "fldrs (floating point load single)",  "fldrd (floating point load double)",  "fstm (floating point store multiple)",  "fldm (floating point load multiple)",  "fcmp (floating point compare)",  "RESERVED",  "ffix (truncate floating point to integer value)",  "fflt (converts an integer value to a floating point)",  "fmov (copy floating point register)",  "fmvn (copy negated floating point register)",  "fabs (absolute value)",  "frnd (rounds to the neareast integral value)",  "fsqt (square root)",  "flog (base-10 logarithm)",  "flgn (base e logarithm)",  "fexp (base e exponential.  fst = e^fsrc)",  "fsin (sine of an angle)",  "fcos (cosine of an angle)",  "ftan (tangent of an angle)",  "fasn (arc sine)",  "facs (arc cosine)",  "fatn (arc tangent)",  "fmsr (copy bits as-is from an integer to float)",  "fmrs (copy bits as-is from a float to an integer) ",  "fmdr (copy bits as-is from an integer to a double)",  "fmrd (copy bits as-is from a double to an integer)", };

void decode_instruction(Canvas * canvas, unsigned char * caddr, list_t * sym_table){
	sdk_scr_printf(canvas, "\n");
	sdk_scr_setTextColor(canvas, CANVAS_COLOR_GREEN);
	
	
	sdk_scr_printf(canvas, " 0x%x    %-3x %-3x %-3x %-3x", caddr,*(caddr + 3), *(caddr + 2), *(caddr + 1), *caddr );
	
	
	sdk_scr_setTextColor(canvas, CANVAS_COLOR_WHITE);
	
	unsigned char instr = *(caddr + 3);
	if (instr < 116)
		if (instr != 0x4E)
			sdk_scr_printf(canvas, "    %s", opcodes_min[instr]);
		else {
			signed short offset = ((unsigned int)(*(caddr + 2)) << 16) + ((unsigned int)(*(caddr + 1)) << 8) + *(caddr);			
			
					
			char name[64] = "";
			int fres = get_symbol((int)((offset + 1) * 4 + caddr), name, sym_table);
			sdk_scr_printf(canvas, "    bl, %x", (offset + 1) * 4 + caddr);
			if (fres > 0){
				sdk_scr_setTextColor(canvas, CANVAS_COLOR_RED);
				sdk_scr_printf(canvas, " %s", name);
				sdk_scr_setTextColor(canvas, CANVAS_COLOR_WHITE);
			} else {
				sdk_scr_printf(canvas, "    bl, %x", (offset + 1) * 4 + caddr);
			}
		}
	else 
		sdk_scr_printf(canvas, "    RESERVED");
}

unsigned char * decode_instruction_deprecated(Canvas * canvas, unsigned char * caddr, list_t * sym_table){
	sdk_scr_printf(canvas, "\n");
	sdk_scr_setTextColor(canvas, CANVAS_COLOR_GREEN);
	//unsigned char * caddr = start_addr + i;
	if (*caddr >= 16)
		sdk_scr_printf(canvas, " 0x%x    %x", caddr, *caddr);
	else 
		sdk_scr_printf(canvas, " 0x%x    %x ", caddr, *caddr);
	
	sdk_scr_setTextColor(canvas, CANVAS_COLOR_WHITE);
	
	unsigned char instr = *caddr;
	
	// ALU
	if (instr <= 0x2F){
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

		if (rval = 0x00)
			sdk_scr_printf(canvas, "    NEXTIRQ  R%d", rdest);
		else if (rval = 0xFF)
			sdk_scr_printf(canvas, "    NEXTIRQ");
		else 
			sdk_scr_printf(canvas, "    NEXTIRQ %d", rdest);
			
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
	
	if (instr == 0x6C){
		NEXT_BYTE
		sdk_scr_printf(canvas, "    CTXSWITCH R%d", instr / 16);
	}
			
	//memcpy
	if (instr == 0x6D){
		NEXT_BYTE
		int r0 = instr / 16;
		int r1 = instr % 16;
		NEXT_BYTE
		sdk_scr_printf(canvas, "    MEMCPY R%d, R%d, R%d", r0, r1, instr / 16);
		return caddr;
	}
	
	//memset
	if (instr == 0x6E){
		NEXT_BYTE
		int r0 = instr / 16;
		int r1 = instr % 16;
		NEXT_BYTE
		sdk_scr_printf(canvas, "    MEMSET R%d, R%d, R%d", r0, r1, instr / 16);
		return caddr;
	}
	
	//cmpxchg
	if (instr == 0x6E){
		NEXT_BYTE
		int r0 = instr / 16;
		int r1 = instr % 16;
		NEXT_BYTE
		sdk_scr_printf(canvas, "    CMPXCHG R%d, R%d, R%d", r0, r1, instr / 16);
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
			int fres = get_symbol(base+imm32, name, sym_table);
			sdk_scr_printf(canvas, "    B%s %ld (0x%x)", branch_suffix[cond], imm32, base+imm32);
			if (fres > 0){
				sdk_scr_setTextColor(canvas, CANVAS_COLOR_RED);
				sdk_scr_printf(canvas, " %s", name);
				sdk_scr_setTextColor(canvas, CANVAS_COLOR_WHITE);
			}
		} else {
			NEXT_BYTE
			sdk_scr_printf(canvas, "    B%s R%x", branch_suffix[cond], instr / 16);
		}
		return caddr;
	}
	
	// Floating point extension
	
	if (instr == 0xff){
		while (instr == 0xff){
			NEXT_BYTE
		}
		
		// ALU
		if (instr <= 0x0F){
			int code = instr;
			NEXT_BYTE
			int fdst = instr / 16;
			int fop1 = instr % 16;
			NEXT_BYTE
			sdk_scr_printf(canvas, "    %s F%d, F%d, F%d", alu_f_opcode[code], fdst, fop1, instr / 16);
			return caddr;
		}
		
		// FSTR/ FLDR
		if (instr >= 0x10 && instr <= 0x1F){
			int code = instr;
			int args = instr % 16;
			
			int c = (args >> 2) % 2;
			int a = args % 4;
			
			NEXT_BYTE
			int freg = instr / 16;
			int rbase = instr % 16;
			
			char cmd[6] = "FSTR ";
			if (code >= 0x18)
				strcpy(cmd, "FLDR ");
			
			if (c == 0)
				cmd[4] = 's';
			else 
				cmd[4] = 'd';
			
			OFFSET(a)
			sdk_scr_printf(canvas, "    %s [R%d+%x], F%d", cmd, rbase, offset, freg);
			
			return caddr;
		}
		
		// float math
		if (instr >= 0x20 && instr <= 0x2F){
			int code = instr % 16;
			NEXT_BYTE
			sdk_scr_printf(canvas, "    %s F%d, F%d", instr_f[code], instr/16, instr % 16);
			
			return caddr;
		}
		
		// FSTM/FLDM
		if (instr == 0x30 || instr == 0x31){
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
		
		// FCMP
		if (instr == 0x32){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    FCMP F%d F%d", instr / 16, instr % 16);
			return caddr;
		}
		
		// 0x33 reserved
		
		// FMSR
		if (instr == 0x34){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    FMSR F%d F%d", instr / 16, instr % 16);
			return caddr;
		}
		
		// FMRS
		if (instr == 0x35){
			NEXT_BYTE
			sdk_scr_printf(canvas, "    FMRS F%d F%d", instr / 16, instr % 16);
			return caddr;
		}
		
		// FMDR
		if (instr == 0x36){
			NEXT_BYTE
			int rsrcHi = instr / 16;
			int rsrcLo = instr % 16;
			NEXT_BYTE
			int freg = instr / 16;
			sdk_scr_printf(canvas, "    FMDR Rsrc%d%d, F%d", rsrcHi, rsrcLo, freg);
			return caddr;
		}
		
		// FMRD
		if (instr == 0x37){
			NEXT_BYTE
			int rsrcHi = instr / 16;
			int rsrcLo = instr % 16;
			NEXT_BYTE
			int freg = instr / 16;
			sdk_scr_printf(canvas, "    FMRD Rsrc%d%d, F%d", rsrcHi, rsrcLo, freg);
			return caddr;
		}
		
		sdk_scr_printf(canvas, "    == ext ==");
		
		return caddr;
	}
	
	// UNDOCUMENTED
	
	return ++caddr;
}


