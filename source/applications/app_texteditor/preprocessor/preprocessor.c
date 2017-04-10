#include "preprocessor.h"

#include <utils/filesystem/filesystem.h>
#include <details/memdetails.h>
#include <string_shared.h>

#define DEF_BUF_SIZE 256

void preprocess_oneliner(list_t * text_lines, char * buf, int screenWidth);
void preprocess_multiliner(list_t * text_lines, char * buf, int screenWidth, int pos);

/*
		Preprocessing split input file for \n symbols.
		Result stores in list.
*/
list_t * preprocess_file(const char* path, int screenWidth){
	list_t* text_lines = list_new();
	
	char buf[DEF_BUF_SIZE];
	memset(buf, 0, sizeof(char) * DEF_BUF_SIZE);
	unsigned int rb;
	
	FILE * file = fs_open_file(path, 'r');
	
	do{	
		int res = fs_read_file(file, buf, DEF_BUF_SIZE, &rb);
		if (res != FS_OK){
			list_destroy(text_lines);
			return 0;
		}
		
		int pos = find(buf, '\n', 0);
		
		if (pos == -1)
			preprocess_oneliner(text_lines, buf, screenWidth);
		else 
			preprocess_multiliner(text_lines, buf, screenWidth, pos);
			
	} while(rb == DEF_BUF_SIZE);

	return text_lines;
}

void preprocess_oneliner(list_t * text_lines, char * buf, int screenWidth){
	char* oneline = (char*)calloc(screenWidth);
	strcpy(oneline, buf);
	list_node_t* onl_node = list_node_new(oneline);
	list_rpush(text_lines, onl_node);
}

void preprocess_multiliner(list_t * text_lines, char * buf, int screenWidth, int pos){
	int old_pos = -1;
	while (pos != -1){				
		char* line = (char*) calloc(screenWidth);
		strncpy(line, &buf[old_pos + 1], pos - old_pos - 1);
		
		list_rpush(text_lines, list_node_new(line));
		
		old_pos = pos;
		pos = find(buf, '\n', pos + 1);
	}
}

void close_preprocessed_file(list_t * text_lines){
	list_destroy(text_lines);
}