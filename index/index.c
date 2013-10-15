#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "index.h"
#include "tokenizer.h"

int addTerm( char* new_term, char* file_path );
TermPtr createTermPtr();
FilePtr createFilePtr();
void deleteTerm( char* target_term );
int fileCompare( void*, void* );
int filePathCompare( char*, char* );
TermPtr findTerm( char* target_term );
int keyCompare( void*, void* );
int updateFileList( SortedListPtr files, char* target);


SortedListPtr keys;
TermPtr values;

int addTerm(char* new_term, char* file_path )
{
	TermPtr t = findTerm( new_term );

	/* If term already exists, update its list of associated files. */
	if( t != NULL )
	{
		return updateFileList( t->files, file_path );
	}

	/* Else... */

	/* Initialize TermPtr containing the new term and its list of associated files. */
	t = createTermPtr();
	t->term_length = strlen(new_term);
	t->term = malloc( t->term_length );
	strcpy( t->term, new_term );
	t->files = SLCreate(fileCompare);

	/* Insert the FilePtr into the term's list of associated files. */
	int successful = insertFileIntoList( t->files, file_path );

	/* If insert is unsuccessful, perform cleanup. */
	if( !successful )
	{
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

void deleteTerm( char* target )
{
	TermPtr t = findTerm( target );

	if( t != NULL )
	{
		HASH_DEL( values, t );
		SLRemove( keys, target );
	}
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

/*
* Description: retrieves the entire file contents as a string and places it in a fileString variable
* Parameters: fileName - the filename to retrieve the data from
* Returns: fileString if the string is successfully copied, NULL otherwise
*/
char *getFileContents(char* file_path )
{
	FILE *fp = fopen(file_path, "r");

	if (fp == NULL)
	{
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	size_t fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *fileString = (char *) malloc(fileSize);
	size_t result = fread(fileString, 1, fileSize, fp);
	fclose(fp);

	if (result == 0) {
		return NULL;
	}

	return fileString;
}

/**
 * Create a new FilePtr object wrapping the given file_path, and then
 * insert the FilePtr into the given list of files.
 *
 * Return 1 if successful; 0 otherwise.
 */
int insertFileIntoList( SortedListPtr files, char* file_path)
{
	FilePtr f = createFilePtr();
	f->file_path_length = strlen(file_path);
	f->file_path = malloc(f->file_path_length);
	strcpy( f->file_path, file_path );
	f->appearances++;

	int successful = SLInsert( files, f );

	if( !successful )
	{
		free( f );
		return 0;
	}

	return 1;
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
 * Parse through the given file_contents text, tokenizing and storing elements
 * as necessary.
 */
void parseFileContents( char* file_path, char* file_contents )
{
	TermPtr t;
	TokenizerT* tk = TKCreate( file_contents );
	char* token = TKGetNextToken( tk );

	while( token != NULL )
	{
		t = findTerm( token );
		if( t != NULL )
		{
			updateFileList( t->files, file_path );
		}
		else
		{
			addTerm( token, file_path );
		}
	}

	TKDestroy( tk );
}

/**
 * Parse the file_path to discover whether or not it is a directory.
 * If the given path is a directory, recursively loop through its contents.
 * If the given path is a file, forward it to functions to read its contents.
 */
void parseFilePath( char* file_path )
{
	DIR* dir = opendir( file_path );

	/* If the path is a directory... */
	if( dir != NULL )
	{
		/* ... iterate through its contents. */
		struct dirent* entry = readdir( dir );
		while( entry != NULL )
		{
			DIR* dir_test = opendir( entry->d_name );

			/* If the directory contains subdirectories... */
			if( dir_test != NULL )
			{
				/* ...recurse on them. */
				parseFilePath( entry->d_name );
				closedir( entry->d_name );
			}

			/* Else parse and store the contents of the file. */
			else
			{
				char* file_contents = getFileContents( entry->d_name );
				parseFileContents( file_contents, entry->d_name );
				free( file_contents );
			}

			entry = readdir( dir );
		}

		closedir( dir );
	}

	/* Else parse and store the contents of the file. */
	else
	{
		char* file_contents = getFileContents( file_path );
		parseFileContents( file_contents, file_path );
		free( file_contents );
	}
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
		printf("fp or files params in reinsertFile is NULL.\n");
		return 0;
	}

	int successful = SLRemove( files, fp );

	if( !successful )
	{
		printf("Unable to remove fp in reinsertFile.\n");
		return 0;
	}

	successful = SLInsert( files, fp );

	if( !successful )
	{
		printf("Unable to insert fp in reinsertFile.\n");
		return 0;
	}

	return 1;
}

/**
 * Iterate through the list of files.  If the given file is already in the list,
 * increase its number of appearances.  If the given file is not in the list,
 * add it.
 *
 * Return 1 if successfully updated, 0 otherwise.
 */
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

	int successful = insertFileIntoList( files, file_path );

	return successful;
}

int main( int argc, char** argv )
{
	/* Check input values. */
	if (argc != 3) {
		printf("ERROR: Invalid number of arguments\n");
		printf("USAGE: index <inverted-index file name> <directory or file name>\n");
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(argv[1], "r");

	if( fp != NULL )
	{
		fclose( fp );
		printf("A file with your inverted-index file name already exists.\n");
		printf("Please restart program and enter a new name.\n");
		exit( EXIT_FAILURE );
	}
	else{
		fclose( fp );
	}


	/* Initialize key list and values hash table. */
	keys = SLCreate(keyCompare);
	values = NULL;


	/* Iterate and sort contents of files as necessary. */
	parseFilePath( argv[ 2 ] );

	//Generate file with sorted items as its content.
	//TODO


	return 0;
}
