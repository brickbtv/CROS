#include "app_elf.h"

#include "utils/filesystem/filesystem.h"
#include "sdk/os/process.h"
#include "sdk/os/debug.h"
#include "sdk/dkc/disk_drive.h"
#include "kernel/hardware/dkc/disk_driver.h"

#include "stdlib/details/memdetails.h"

#include "sdk/scr/ScreenClass.h"


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

typedef struct Elf32_Ehdr
{
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

typedef struct Elf32_Shdr
{
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

char * bytes_order[] = {"Unknown", "BIG", "LITTLE"};

void elf_dump(ScreenClass * screen, const char* filename){
	FILE * file = fs_open_file(filename, 'r');
	if (file){
		unsigned char buff[4096];
		int rb;
		
		memset(buff, 0, 4096);
		Elf32_Ehdr elf_header;
		fs_read_file(file, buff, 4096, &rb);
		memcpy(&elf_header, buff, sizeof(Elf32_Ehdr));
		screen->setBackColor(screen, CANVAS_COLOR_BLUE);
		screen->printf(screen, "ELF header:\n");
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
				
		//sdk_prc_sleep(5000);	
		
		// section with sections names
		Elf32_Shdr elf_secheader_names;
		memcpy(&elf_secheader_names, &buff[elf_header.e_shoff + elf_header.e_shstrndx * elf_header.e_shentsize], sizeof(Elf32_Shdr));
		
		char * names = &buff[elf_secheader_names.sh_offset];
		
		// table of sections headers
		for (int i = 0; i < elf_header.e_shnum; i++){
			Elf32_Shdr elf_secheader;
			memcpy(&elf_secheader, &buff[elf_header.e_shoff + i * elf_header.e_shentsize], sizeof(Elf32_Shdr));
			if (i == 0)	// always empty 
				continue;
				
			screen->setBackColor(screen, CANVAS_COLOR_BLUE);
			screen->printf(screen, "ELF section header for %d, offset 0x%x:\n", i, elf_header.e_shoff + i * elf_header.e_shentsize);
			screen->setBackColor(screen, CANVAS_COLOR_BLACK);
			
			screen->printf(screen, "sh_name: %d (%s)\n", elf_secheader.sh_name, &names[elf_secheader.sh_name]);
			screen->printf(screen, "sh_type: %d\n", elf_secheader.sh_type);
			screen->printf(screen, "sh_flags: %d\n", elf_secheader.sh_flags);
			screen->printf(screen, "sh_addr: %d\n", elf_secheader.sh_addr);
			screen->printf(screen, "sh_offset: %d\n", elf_secheader.sh_offset);
			screen->printf(screen, "sh_size: %d\n", elf_secheader.sh_size);
			screen->printf(screen, "sh_link: %d\n", elf_secheader.sh_link);
			screen->printf(screen, "sh_info: %d\n", elf_secheader.sh_info);
			screen->printf(screen, "sh_addralign: %d\n", elf_secheader.sh_addralign);
			screen->printf(screen, "sh_entsize: %d\n", elf_secheader.sh_entsize);
			
			sdk_prc_sleep(2000);
		}
		
		fs_close_file(file);
	} else {
		screen->printf(screen, "Failed to open file.\n");
	}
}

void app_elf(const char* filename){
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	ScreenClass * screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);

	elf_dump(screen, "ELF.T");
	sdk_prc_sleep(10000);
	
	sdk_prc_die();
}
