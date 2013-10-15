#include "indexed_file.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include <string.h>

#include <stdlib.h>

void index_init(indexed_file_t *file, char *master, char *index)
{
	strcpy(file->master_fname, master);
	strcpy(file->index_fname, index);

	//Get the size of the file and then figure out how many users are in it
	/*struct stat sb;
	stat(file->master_fname, &sb);
	int size = (sb.st_size) / sizeof(user_t);*/

	//Make some space for the records
	file->records = vector_alloc_with_size(100);
}

void index_open_transaction(indexed_file_t *file)
{
	file->index_file = fopen(file->index_fname, "a+");
	file->master_fid = open(file->master_fname, O_RDWR | O_CREAT, S_IRWXU);

	//Fill the records in our datastructure
	int i;
	for (i = 0; i < file->records->length; i++)
	{
		indexed_rec_t *record = vector_get(file->records, i);
		fscanf(file->index_file, "%i %i", &record->id, &record->index);
	}

	fclose(file->index_file);
}

void index_close_transaction(indexed_file_t *file)
{
	//Overwrite and open the index file
	file->index_file = fopen(file->index_fname, "w");

	//Make sure we are at the beginning
	fseek(file->index_file, 0, SEEK_SET);

	//Flush the contents out to the file
	int i;
	indexed_rec_t *record;

	for (i = 0; i < file->records->length; i++)
	{
		record = (indexed_rec_t *) vector_get(file->records, i);
		fprintf(file->index_file, "%i %i\n", record->id, record->index);
	}

	//Close both files
	fclose(file->index_file);
	close(file->master_fid);

	file->master_fid = -1;
}

int index_get_midpoint(int min, int max)
{
	return (max - min) / 2;
}

int index_add_index_rec(indexed_file_t *file, int id, int index, int min, int max)
{
	int midpoint = index_get_midpoint(min, max);

	indexed_rec_t *mid = (indexed_rec_t *) vector_get(file->records, midpoint);

	if (id == mid->id)
	{
		indexed_rec_t record;
		record.id = id;
		record.index = index;

		vector_insert(file->records, midpoint, &record, sizeof(indexed_rec_t));

		return midpoint;
	}
	else if (id > mid->id)
	{
		return index_add_index_rec(file, id, index, midpoint + 1, max);
	}
	else
	{
		return index_add_index_rec(file, id, index, min, midpoint - 1);
	}
}

int index_add_index(indexed_file_t *file, int id, int index)
{
	if (file->records->length != 0)
	{
		return index_add_index_rec(file, id, index, 0, file->records->length - 1);
	}
	else
	{
		indexed_rec_t record;
		record.id = id;
		record.index = index;
		vector_add(file->records, &record, sizeof(indexed_rec_t));
		return 0;
	}
}

int index_get_index_rec(indexed_file_t *file, int id, int min, int max)
{
	int midpoint = index_get_midpoint(min, max);

	indexed_rec_t *mid = (indexed_rec_t *) vector_get(file->records, midpoint);

	if (id == mid->id)
	{
		return mid->index;
	}
	else if (id > mid->id)
	{
		return index_get_index_rec(file, id, midpoint + 1, max);
	}
	else
	{
		return index_get_index_rec(file, id, min, midpoint - 1);
	}
}

int index_get_index(indexed_file_t *file, int id)
{
	return index_get_index_rec(file, id, 0, file->records->length - 1);
}

int index_add(indexed_file_t *file, user_t *user)
{
	//Move the cursor to the end of the file
	lseek(file->master_fid, 0, 2);

	//Write user to the end of the file
	write(file->master_fid, user, sizeof(user_t));

	//Move the cursor to the end of the file
	fseek(file->index_file, 0, SEEK_END);

	//Update the index in our data structure
	index_add_index(file, user->userid, file->records->length);

	//Update in the file



	return 0;
}

int index_update(indexed_file_t *file, user_t *user)
{
	//Find out the index of the user
	int index = index_get_index(file, user->userid);

	//Move the cursor to the index
	lseek(file->master_fid, index * sizeof(user_t), 0);

	//Write the user out
	write(file->master_fid, user, sizeof(user_t));
}

int index_delete(indexed_file_t *file, int id)
{
	return 0;
}
