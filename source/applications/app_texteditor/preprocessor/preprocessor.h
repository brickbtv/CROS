#ifndef _TE_PREPROCESSOR_H_
#define _TE_PREPROCESSOR_H_

#include <containers/list.h>

list_t * preprocess_file(const char* path, int screenWidth);
void close_preprocessed_file(list_t * text_lines);

#endif