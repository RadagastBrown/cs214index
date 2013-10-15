#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "index.h"
#include "tokenizer.h"

int addFile(SortedListPtr files, char* file_path );
int addTerm( char* new_term, char* file_path );
void cleanup();
TermPtr createTermPtr();
FilePtr createFilePtr();
void deleteTerm( char* target_term );
int fileCompare( void*, void* );
int filePathCompare( char*, char* );
TermPtr findTerm( char* target_term );
char *getFileContents(char* file_path );
int insertFileIntoList( SortedListPtr files, char* file_path );
int keyCompare( void*, void* );
void parseFileContents( char* file_path, char* file_contents );
void processInput( char* file_path );
void writeFile( char* file_path );

SortedListPtr keys;
TermPtr values;

/**
 * Iterate through the list of files.  If the given file is already in the list,
 * increase its number of appearances.  If the given file is not in the list,
 * add it.
 *
 * Return 1 if successfully updated, 0 otherwise.
 */
int addFile(SortedListPtr files, char* file_path )
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
			shiftNodeUp( files, curr_file );//This may have errors
			return 1;
		}

		file_node = file_node->next;
	}

	int successful = insertFileIntoList( files, file_path );

	return successful;
}

/**
 * Create a TermPtr and add it to the values hashtable while also
 * adding the associated file_path to the keys list.
 *
 * Return 1 if successfully added, 0 otherwise.
 */
int addTerm(char* new_term, char* file_path )
{
	/* Initialize TermPtr containing the new term and its list of associated files. */
	TermPtr t = createTermPtr();
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

	SLInsert(keys, new_term);
	HASH_ADD_KEYPTR( hh, values, new_term, strlen(new_term), t );

	return 1;
}

void cleanup()
{
	SortedListIteratorPtr key_iter = SLCreateIterator( keys );

	while( SLHasNext( key_iter ) )
	{
		char* key = SLNextItem( key_iter );
		TermPtr t = findTerm( key );

		SortedListIteratorPtr file_iter = SLCreateIterator( t->files );

		while( SLHasNext( file_iter ) )
		{
			FilePtr f = SLNextItem( file_iter );
			free( f->file_path );
			free( f );
		}

		SLDestroyIterator( file_iter );
		SLDestroy( t->files );

		free( t->term );
		deleteTerm( key );

		free( key );
	}

	SLDestroyIterator( key_iter );
	SLDestroy( keys );
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
			int successful = addFile( t->files, file_path );
			if( successful )
			{
				t->number_of_files++;
			}
			else
			{
				//TODO handle failure
			}
		}
		else
		{
			int successful = addTerm( token, file_path );
			if( !successful )
			{
				//TODO handle failure
			}
		}
	}

	TKDestroy( tk );
}

/**
 * Parse the file_path to discover whether or not it is a directory.
 * If the given path is a directory, recursively loop through its contents.
 * If the given path is a file, forward it to functions to read its contents.
 */
void processInput( char* file_path )
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
				processInput( entry->d_name );
				closedir( dir_test );
			}

			/* Else parse and store the contents of the file. */
			else
			{
				char* file_contents = getFileContents( entry->d_name );
				parseFileContents( entry->d_name, file_contents );
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

void writeFile( char* file_path )
{
	FILE* new_file = fopen( file_path, "w" );
	SortedListIteratorPtr key_iter = SLCreateIterator( keys );

	while( SLHasNext( key_iter ) )
	{
		char* key = SLNextItem( key_iter );
		TermPtr t = findTerm( key );

		fputs("<list> ", new_file);
		fputs(key, new_file);
		fputs("\n", new_file);

		SortedListIteratorPtr file_iter = SLCreateIterator( t->files );

		int i = 0;
		while( SLHasNext( file_iter ) )
		{
			if( i == 5 )
			{
				i = 0;
				fputs("\n", new_file);
			}

			FilePtr f = SLNextItem( file_iter );
			fputs( f->file_path, new_file );
			fputs( " ", new_file );

			char appearances[20];
			snprintf(appearances, 20, "%zd", f->appearances );
			fputs(appearances, new_file);
			fputs(" ", new_file );

			i++;
		}

		fputs("\n", new_file);
		fputs("</list>", new_file);
		fputs("\n", new_file);

		SLDestroyIterator( file_iter );
	}

	SLDestroyIterator( key_iter );
	fclose(new_file);
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
	processInput( argv[ 2 ] );

	/* Generate file with sorted items as its content. */
	writeFile( argv[ 1 ] );

	/* Destroy contents of Index. (i.e. perform cleanup) */
	cleanup();

	return 0;
}
