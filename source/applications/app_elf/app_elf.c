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

typedef struct
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

void elf_dump(ScreenClass * screen, const char* filename){
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

void app_elf(const char* filename){
	Canvas * cv = (Canvas*)sdk_prc_getCanvas();
	ScreenClass * screen = malloc(sizeof(ScreenClass));
	screen = ScreenClass_ctor(screen, cv);

	elf_dump(screen, "ELF.T");
	
	sdk_prc_die();
}
