#include <stdio.h>
#include <string.h>
#include "index.h"
#include "tokenizer.h"

int addTerm( char* new_term, char* file_path );
TermPtr createTermPtr();
FilePtr createFilePtr();
TermPtr deleteTerm( char* target_term );
int fileCompare( void*, void* );
int filePathCompare( char*, char* );
TermPtr findTerm( char* target_term );
int keyCompare( void*, void* );
int updateFileList( SortedListPtr files, char* target);


SortedListPtr keys = NULL;
TermPtr values = NULL;

int addTerm(char* new_term, char* file_path )
{
	TermPtr t = findTerm( new_term );

	if( t != NULL )
	{
		return updateFileList( t->files, file_path );
	}

	FilePtr f = createFilePtr();
	f->file_path = file_path;
	f->file_path_length = strlen(file_path);
	f->appearances++;

	t = createTermPtr();
	t->term = new_term;
	t->term_length = strlen(new_term);
	t->files = SLCreate(fileCompare);

	int successful = SLInsert( t->files, f );

	if( !successful )
	{
		free(f);
		free(t->files);
		free(t);

		return 0;
	}

	t->number_of_files++;

	SLInsert(keys, t);
	HASH_ADD_KEYPTR( hh, values, new_term, strlen(new_term), t );

	return 1;
}

FilePtr createFilePtr()
{
	FilePtr f = malloc(sizeof(*f));
	f->file_path = NULL;
	f->file_path_length = 0;
	f->appearances = 0;

	return f;
}

TermPtr createTermPtr()
{
	TermPtr t = malloc(sizeof(*t));
	t->term = NULL;
	t->term_length = 0;
	t->files = NULL;
	t->number_of_files = 0;

	return t;
}

TermPtr deleteTerm( char* target )
{
	//TODO
	return NULL;
}

/**
 * Compares values a and b, and return -1 if a < b, 0 if a == b, 1 if a > b.
 * This function assumes a and b are of type FilePtr, and the comparison is made
 * on the struct's "appearances" size_t counter.
 */
int fileCompare( void* a, void* b )
{
	size_t a_int = *(size_t*)((FilePtr)a)->appearances;
	size_t b_int = *(size_t*)((FilePtr)b)->appearances;

	return a_int - b_int;
}

int filePathCompare( char* a, char* b )
{
	return strcmp( a, b );
}

TermPtr findTerm( char* target )
{
	TermPtr t = NULL;
	HASH_FIND_STR( values, target, t );
	return t;
}

/**
 * Compare keys a and b, and return -1 if a < b, 0 if a == b, 1 if a > b.
 * This function assumes a and b are of type char*, and so comparison is
 * based on lexicographical ordering.
 */
int keyCompare( void* a, void* b )
{
	char* a_string = a;
	char* b_string = b;

	return strcmp( a_string, b_string );
}

/**
 * Reinsert a file already in the given list of files.  This should occur after
 * an a file has been changed in such a way as to render its current position
 * in the ordering incorrect (such as a FilePtr->appearances count change).
 *
 * Return 1 if successful; 0 otherwise.
 */
int reinsertFile( SortedListPtr files, FilePtr fp )
{
	if( fp == NULL || files == NULL )
	{
		printf("fp or files params in fileCountIncrement is NULL.\n");
		return 0;
	}

	int successful = SLRemove( files, fp );

	if( !successful )
	{
		printf("Unable to remove fp in fileCountIncrement.\n");
		return 0;
	}

	successful = SLInsert( files, fp );

	if( !successful )
	{
		printf("Unable to insert fp in fileCountIncrement.\n");
		return 0;
	}

	return 1;
}

int updateFileList(SortedListPtr files, char* file_path )
{

	Node file_node = files->head;
	int comparison = 0;
	FilePtr curr_file = NULL;

	while( file_node != NULL )
	{
		curr_file = (FilePtr)(file_node->object);

		comparison = strcmp( file_path, curr_file->file_path );

		if( comparison == 0 )
		{
			curr_file->appearances++;
			return 1;
		}

		file_node = file_node->next;
	}

	FilePtr f = createFilePtr();
	f->file_path = file_path;
	f->file_path_length = strlen(file_path);
	f->appearances++;

	int successful = SLInsert( files, f );

	if( !successful )
	{
		free(f);
		return 0;
	}

	return 1;
}

int main( int argc, char** argv )
{
	//First check input values
	//Then initialize key list
	keys = SLCreate(keyCompare);
	//Then iterate sort contents of files as necessary

	return 0;
}
