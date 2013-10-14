#ifndef index_index_h
#define index_index_h

#include "sorted-list.h"
#include "uthash.h"

struct File
{
	char* file_path;
	size_t file_path_length;
	size_t appearances;
};
typedef struct File* FilePtr;

struct Term
{
	char* term;
	size_t term_length;
	SortedListPtr files;
	size_t number_of_files;
	UT_hash_handle hh;
};
typedef struct Term* TermPtr;

#endif
