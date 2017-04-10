#include "preprocessor.h"

#include <utils/filesystem/filesystem.h>
#include <details/memdetails.h>
#include <string_shared.h>

list_t * preprocess_file(const char* path, int screenWidth){
	/*
		Preprocessing split input file for \n symbols.
		Result stores in list.
	*/
	
	list_t* text_lines = list_new();
	
	// step 1
	#define DEF_BUF_SIZE 256
	char buf[DEF_BUF_SIZE];
	memset(buf, 0, sizeof(char) * DEF_BUF_SIZE);
	unsigned int rb;
	
	FILE * file = fs_open_file(path, 'r');
	
	if (file){
		do{	
			int res = fs_read_file(file, buf, DEF_BUF_SIZE, &rb);
			if (res != FS_OK){
				list_destroy(text_lines);
				return FALSE;
			}
			
			// splitting
			int old_pos = -1;
			int pos = find(buf, '\n', 0);
			
			if (pos == -1){
				char* oneline = (char*)calloc(screenWidth);
				strcpy(oneline, buf);
				list_node_t* onl_node = list_node_new(oneline);
				list_rpush(text_lines, onl_node);
			}
			
			while (pos != -1){				
				char* line = (char*) calloc(screenWidth);
				strncpy(line, &buf[old_pos + 1], pos - old_pos - 1);
				
				list_rpush(text_lines, list_node_new(line));
				
				old_pos = pos;
				pos = find(buf, '\n', pos + 1);
			}
		} while(rb == DEF_BUF_SIZE);
	} else {
		return 0;
	}
	
	return text_lines;
}

void close_preprocessed_file(list_t * text_lines){
	list_destroy(text_lines);
}