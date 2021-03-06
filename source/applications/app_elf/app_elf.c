#include "app_elf.h"

#include "utils/filesystem/filesystem.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/dkc/disk_drive.h"
#include "kernel/hardware/dkc/disk_driver.h"

#include "stdlib/details/memdetails.h"

#include "sdk/scr/ScreenClass.h"
#include <sdk/kyb/keyboard.h>

#include "app_disasm/decoder/asm_decoder.h"
#include "containers/list.h"

#include <string_shared.h>
#include <stdio_shared.h>
#include <stdlib_shared.h>

void hex_dump(ScreenClass * screen, const char* filename){
	FILE * file = fs_open_file(filename, 'r');
	if (file){
		unsigned char buff[1024];
		int rb;
		
		memset(buff, 0, 1024);
		fs_read_file(file, buff, 1024, &rb);
				
		while(rb > 0){
			screen->setBackColor(screen, CANVAS_COLOR_BLUE);
			screen->printf(screen, "\n%-12s%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c", "", 
									'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
									'a', 'b', 'c', 'd', 'e', 'f');
			screen->setBackColor(screen, CANVAS_COLOR_BLACK);
			for ( int i = 0 ; i < rb; i++){
				if (i % 0x10 == 0){
					screen->setBackColor(screen, CANVAS_COLOR_BLUE);
					screen->printf(screen, "\n0x%-10x", i);
					screen->setBackColor(screen, CANVAS_COLOR_BLACK);
					sdk_prc_sleep(100);
				}
				screen->printf(screen, "%-2x ", buff[i]);
				
			}
			
			memset(buff, 0, 1024);
			if (rb < 1024)
				break;
				
			fs_read_file(file, buff, 1024, &rb);
		}
		screen->printf(screen, "\n");
		fs_close_file(file);
	} else {
		screen->printf(screen, "Failed to open file.\n");
	}
}

typedef struct Elf32_Ehdr {
    unsigned char e_ident[16];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint32_t      e_entry;
    uint32_t      e_phoff;
    uint32_t      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} Elf32_Ehdr;

typedef struct Elf32_Shdr {
    uint32_t   sh_name;
    uint32_t   sh_type;
    uint32_t   sh_flags;
    uint32_t   sh_addr;
    uint32_t   sh_offset;
    uint32_t   sh_size;
    uint32_t   sh_link;
    uint32_t   sh_info;
    uint32_t   sh_addralign;
    uint32_t   sh_entsize;
} Elf32_Shdr;

typedef struct Elf32_Sym {
       uint32_t      st_name;
       uint32_t      st_value;
       uint32_t      st_size;
       unsigned char   st_info;
       unsigned char   st_other;
       unsigned short  st_shndx;
} Elf32_Sym;

typedef struct Elf32_Rela{
       uint32_t      r_offset;
       uint32_t		 r_info;
	   int 			 addend;
} Elf32_Rela;

char * bytes_order[] = {"Unknown", "BIG", "LITTLE"};
char * section_type[] = {"SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB", 
						"SHT_RELA", "SHT_HASH", "SHT_DYNAMIC", "SHT_NOTE", 
						"SHT_NOBITS", "SHT_REL"};

unsigned int elf_get_sys_symbol(list_t * sym_table, char * sym_name){
	if (!sym_name)
		return 0;
	if (sym_name[0] == '_')
		sym_name = &sym_name[1];
	

	list_node_t * node = sym_table->head;
	while (node){
		STR_SYMBOL *s = (STR_SYMBOL *)node->val;
		if (strcmp(s->name, sym_name) == 0){
			return s->address;
		}
		node = node->next;
	}
	return 0;
}

void print_section(ScreenClass * screen, Elf32_Shdr * section, FILE * file, char * title){
	screen->setBackColor(screen, CANVAS_COLOR_BLUE);
	screen->printf(screen, "\n%s:%-80s", title, "");
	screen->printf(screen, "\n%-12s%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c%-3c", "", 
							'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
							'a', 'b', 'c', 'd', 'e', 'f');
	screen->setBackColor(screen, CANVAS_COLOR_BLACK);
	
	
	unsigned char * data = malloc(section->sh_size);
	int rb;
	fs_seek(file, section->sh_offset);
	fs_read_file(file, data, section->sh_size, &rb);
	
	int textdatalen = rb;
	
	for (int i = 0 ; i < textdatalen; i++){
		if (i % 0x10 == 0){
			screen->setBackColor(screen, CANVAS_COLOR_BLUE);
			screen->printf(screen, "\n0x%-10x", i);
			screen->setBackColor(screen, CANVAS_COLOR_BLACK);
		}
		screen->printf(screen, "%-2x ", data[i]);
		
	}
	free(data);
}

int elf_dump(ScreenClass * screen, const char* filename, int draw){
	FILE * file = fs_open_file(filename, 'r');
	int error_flag = 0;
	if (file){
		int rb;
		Elf32_Ehdr elf_header;
		fs_seek(file, 0);
		fs_read_file(file, (char*)&elf_header, sizeof(Elf32_Ehdr), &rb);
		if (draw){
			screen->setBackColor(screen, CANVAS_COLOR_BLUE);
			screen->printf(screen, "ELF header:%-80s\n", "");
			screen->setBackColor(screen, CANVAS_COLOR_BLACK);
			
			screen->printf(screen, "e_ident: magic: %c%c%c%c\n", 
										elf_header.e_ident[0],
										elf_header.e_ident[1],
										elf_header.e_ident[2],
										elf_header.e_ident[3]);
			screen->printf(screen, "         word size: %d\n", 32 * elf_header.e_ident[4]);
			screen->printf(screen, "         bytes order: %s\n", bytes_order[elf_header.e_ident[5]]);
			
			screen->printf(screen, "e_type: %d\n", elf_header.e_type); 
			screen->printf(screen, "e_machine: %d\n", elf_header.e_machine);
			screen->printf(screen, "e_version: %d\n", elf_header.e_version);
			screen->printf(screen, "e_entry: 0x%x\n", elf_header.e_entry);
			screen->printf(screen, "e_phoff: 0x%x\n", elf_header.e_phoff);
			screen->printf(screen, "e_shoff: 0x%x\n", elf_header.e_shoff);
			
			screen->printf(screen, "e_flags: %d\n", elf_header.e_flags);
			screen->printf(screen, "e_ehsize: %d\n", elf_header.e_ehsize);
			screen->printf(screen, "e_phentsize: %d\n", elf_header.e_phentsize);
			screen->printf(screen, "e_phnum: %d\n", elf_header.e_phnum );
			screen->printf(screen, "e_shentsize: %d\n", elf_header.e_shentsize);
			screen->printf(screen, "e_shnum: %d\n", elf_header.e_shnum);
			screen->printf(screen, "e_shstrndx: %d\n", elf_header.e_shstrndx);
		}

		// section with sections names
		Elf32_Shdr elf_secheader_names;
		fs_seek(file, elf_header.e_shoff + elf_header.e_shstrndx * elf_header.e_shentsize);
		fs_read_file(file, (char*)&elf_secheader_names, sizeof(Elf32_Shdr), &rb);
		
		char * names = malloc(elf_secheader_names.sh_size);
		fs_seek(file, elf_secheader_names.sh_offset);
		fs_read_file(file, names, elf_secheader_names.sh_size, &rb);
		
		// table of sections headers
		if (draw){
			screen->setBackColor(screen, CANVAS_COLOR_BLUE);
			screen->printf(screen, "ELF sections headers:%-80s\n", "");
			screen->printf(screen, "%-3s%-16s%-13s%-10s%-5s%-7s%-5s%-5s%-5s%-8s%-3s\n", 
										"#",
										"name", 
										"type",
										"flags",
										"addr",
										"offs",
										"size",
										"link",
										"info",
										"align",
										"es");
			screen->setBackColor(screen, CANVAS_COLOR_BLACK);
		}
		
		Elf32_Shdr * sections = malloc(sizeof(Elf32_Shdr) * elf_header.e_shnum);
		for (int i = 1; i < elf_header.e_shnum; i++){
			Elf32_Shdr elf_secheader;
			fs_seek(file, elf_header.e_shoff + i * elf_header.e_shentsize);
			fs_read_file(file, (char*)&elf_secheader, sizeof(Elf32_Shdr), &rb);
			
			sections[i] = elf_secheader;
			
			if (draw){
				int fl = elf_secheader.sh_flags;			
				int b = 0;
				char flags[4];
				memset(flags, 0, 4);
				if (fl & 0x1)
					flags[b++] = 'W';
				if (fl & 0x2)
					flags[b++] = 'A';
				if (fl & 0x4)
					flags[b++] = 'E';
				
				screen->printf(screen, "%-3d%-16s%-13s%-10s0x%-3x0x%-5x%-5d%-5d%-5d%-8d%-3d\n", 
										elf_secheader.sh_name, 
										&names[elf_secheader.sh_name], 
										section_type[elf_secheader.sh_type],
										flags,
										elf_secheader.sh_addr,
										elf_secheader.sh_offset,
										elf_secheader.sh_size,
										elf_secheader.sh_link,
										elf_secheader.sh_info,
										elf_secheader.sh_addralign,
										elf_secheader.sh_entsize);
			}
		}
		
		// symboltable	
		Elf32_Shdr symsec;
		Elf32_Shdr symnamessec;
		Elf32_Shdr textsec;
		Elf32_Shdr reltextsec;
		Elf32_Shdr rodatasec;
		
		for (int i = 0; i < elf_header.e_shnum; i++){
			if (strcmp(&names[sections[i].sh_name], ".symtab") == 0)
				symsec = sections[i];
			if (strcmp(&names[sections[i].sh_name], ".strtab") == 0)
				symnamessec = sections[i];
			if (strcmp(&names[sections[i].sh_name], ".text") == 0)
				textsec = sections[i];
			if (strcmp(&names[sections[i].sh_name], ".rel.text") == 0)
				reltextsec = sections[i];
			if (strcmp(&names[sections[i].sh_name], ".rodata") == 0)
				rodatasec = sections[i];
		}
		
		char * symnames = malloc(symnamessec.sh_size);
		fs_seek(file, symnamessec.sh_offset);
		fs_read_file(file, symnames, symnamessec.sh_size, &rb);
		
		if (draw){
			screen->setBackColor(screen, CANVAS_COLOR_BLUE);
			screen->printf(screen, "ELF symbol table:%-80s\n", "");
			screen->printf(screen, "%-20s%-12s%-10s%-10s%-10s%-20s\n",
										"name",
										"value",
										"size",
										"info",
										"shndx",
										"CROS addr");
			screen->setBackColor(screen, CANVAS_COLOR_BLACK);
		}
		
		unsigned char * textdata = malloc(textsec.sh_size);
				
		list_t * sym_table = init_symbol_table();
		Elf32_Sym * elf_symbols = malloc(sizeof(Elf32_Sym)*(symsec.sh_size / symsec.sh_entsize));
		for (int j = 1; j < symsec.sh_size / symsec.sh_entsize; j++){
			Elf32_Sym elf_sym;
			fs_seek(file, symsec.sh_offset + j * symsec.sh_entsize);
			fs_read_file(file, (char*)&elf_sym, sizeof(Elf32_Sym), &rb);
			
			// is relocation for SDK function?
			unsigned int offset = elf_get_sys_symbol(sym_table, &symnames[elf_sym.st_name]);
			
			if (draw){
				screen->printf(screen, "%-20s0x%-10x%-10d%-10d%-12s0x%-20x\n",
										&symnames[elf_sym.st_name],
										elf_sym.st_value,
										elf_sym.st_size,
										elf_sym.st_info,
										&names[sections[elf_sym.st_shndx].sh_name],
										offset);
			}
			
			// Fatal: Can't find symbol in ELF and System SDK. 
			/*
				GENERIC ERROR, SHOULD BE DISPLAYED IN ANY CASE
			*/
			if (offset == 0 && elf_sym.st_shndx == 0){
				screen->setBackColor(screen, CANVAS_COLOR_RED);
				screen->printf(screen, "FATAL! Symbol '%s' not found.%-80s\n", &symnames[elf_sym.st_name], "");
				screen->setBackColor(screen, CANVAS_COLOR_BLACK);
				error_flag = 1;
			}
			
			if (offset != 0)
				elf_sym.st_value = offset;
			else {
				/*
					IMPORTANT: Patch for callbacks and other pointers.
					
					If symbol already in text section, it should take an absolute offset, not local
				*/
				if (sections[elf_sym.st_shndx].sh_name == textsec.sh_name && elf_sym.st_info == 16){
					elf_sym.st_value += (unsigned int)&textdata[0];
				}
			}
			
			elf_symbols[j] = elf_sym;
		}
		if (error_flag)
			return error_flag;		
		
		// dump .text section BEFORE patching
		
		fs_seek(file, textsec.sh_offset);
		fs_read_file(file, textdata, textsec.sh_size, &rb);
		
		if (draw){
			print_section(screen, &textsec, file, ".text section BEFORE patching");
		}
		
		// .rodata section (constants like strings)
		
		unsigned char * rodata = malloc(rodatasec.sh_size);
		fs_seek(file, rodatasec.sh_offset);
		fs_read_file(file, rodata, rodatasec.sh_size, &rb);
		
		if (draw){
			//print_section(screen, &reltextsec, file, ".rel.text section");
		}
		
		// .text section PATCHING
		for (int i = 0; i < reltextsec.sh_size / reltextsec.sh_entsize; i++){
			Elf32_Rela rel_entry;
			fs_seek(file, reltextsec.sh_offset + i * reltextsec.sh_entsize);
			fs_read_file(file, (char*)&rel_entry, reltextsec.sh_entsize, &rb);

			// apply patch
			#define ELF32_R_SYM(i)  ((i)>>8)
		    #define ELF32_R_TYPE(i)   ((unsigned char)(i))
		    #define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))
			
			unsigned int obj_id = ELF32_R_SYM(rel_entry.r_info);
			unsigned int obj_type = ELF32_R_TYPE(rel_entry.r_info);
			
			unsigned int additional_offset = (unsigned int)textdata[rel_entry.r_offset];
			
			// PATCH FOR system FUNCIONS
			unsigned int NAD = elf_symbols[obj_id].st_value - (unsigned int)&textdata[0] - rel_entry.r_offset + additional_offset;
			
			// PATCH FOR elf RODATA
			//sdk_debug_logf("shndx: %d, rod: %d", sections[elf_symbols[obj_id].st_shndx].sh_name, rodatasec.sh_name);
			
			if (sections[elf_symbols[obj_id].st_shndx].sh_name == rodatasec.sh_name){
				NAD = ((unsigned int)&rodata[elf_symbols[obj_id].st_value] - (unsigned int)&textdata[rel_entry.r_offset]) + additional_offset;
			}
			
			if (draw){
				screen->printf(screen, "\nrelocate: 0x%-10x %-5d %-5d %-5d %s", rel_entry.r_offset, obj_id, obj_type, rel_entry.r_info, &symnames[elf_symbols[obj_id].st_name]);
			}
			textdata[rel_entry.r_offset] = NAD;
			textdata[rel_entry.r_offset + 1] = NAD >> 8;
			textdata[rel_entry.r_offset + 2] = NAD >> 16;
			textdata[rel_entry.r_offset + 3] = NAD >> 24;
		}
		
		// dump .text section AFTER patching
		if (draw){
			//print_section(screen, &textsec, file, ".text section AFTER patching");
			int textdatalen = textsec.sh_size;
	
			for (int i = 0 ; i < textdatalen; i++){
				if (i % 0x10 == 0){
					screen->setBackColor(screen, CANVAS_COLOR_BLUE);
					screen->printf(screen, "\n0x%-10x", i);
					screen->setBackColor(screen, CANVAS_COLOR_BLACK);
				}
				screen->printf(screen, "%-2x ", textdata[i]);
				
			}
		}
		
		// run app
		if (!draw){
			int pid = sdk_prc_create_process((unsigned int)&textdata[0], filename, 0, (Canvas*)sdk_prc_getCanvas());
			
			/*
				TODO: better way to sync process finish
			*/
			sdk_prc_wait_till_process_die(pid);
		}				
		free(rodata);
		free(textdata);
		free(symnames);
		free(sections);
		free(names);
		fs_close_file(file);
	} else {
		screen->printf(screen, "Failed to open file.\n");
	}
	
	return 0;
}

void msgHndlrElf(int type, int reason, int value, void * userdata){
	if (type == SDK_PRC_MESSAGE_KYB){
		if (reason == KEY_STATE_KEYPRESSED){
			if (value == KEY_RETURN){
				*(int*)userdata = 0;
			}
		}
	}
}

void app_elf(const char* filename){
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	
	ScreenClass * screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);
	
	elf_dump(screen, filename, 1);
	
	screen->printf(screen, "\n\n\t\tPRESS Enter TO EXIT\n\n");
	
	int noexit = 1;
	while(noexit){		
		while (sdk_prc_haveNewMessage()){
			sdk_prc_handleMessage(msgHndlrElf, &noexit);
		}
		sdk_prc_sleep_until_new_messages();
	}
	
	sdk_prc_die();
}

void app_elfrun(const char* filename){
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	
	ScreenClass * screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);	

	if (elf_dump(screen, filename, 0)){
		// error processing
		int noexit = 1;
		while(noexit){		
			while (sdk_prc_haveNewMessage()){
				sdk_prc_handleMessage(msgHndlrElf, &noexit);
			}	
			sdk_prc_sleep_until_new_messages();
		}
	}
	sdk_prc_die();
}
