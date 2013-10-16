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

	//Make some space for the records
	file->records = vector_alloc_with_size(100);
}

void index_open_transaction(indexed_file_t *file)
{
	file->index_file = fopen(file->index_fname, "a+");
	file->master_fid = open(file->master_fname, O_RDWR | O_CREAT, S_IRWXU);

	//Fill the records in our datastructure
	indexed_rec_t record;
	while(fscanf(file->index_file, "%i %i", &record.id, &record.index) != EOF)
	{
		vector_add(file->records, &record, sizeof(indexed_rec_t));
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
	return min + ((max - min) / 2);
}

int index_get_user(indexed_file_t *file, int userid, user_t *buffer)
{
	//Get the index of the user
	int index = index_get_index(file, userid);

	//Make sure the userid exists
	if (index == -1)
	{
		return -1;
	}

	//Go to the user at the index in the master file
	lseek(file->master_fid, sizeof(user_t) * index, 0);

	//Read the data from from the master file and store it into our buffer
	read(file->master_fid, buffer, sizeof(user_t));

	return 0;
}

int index_add_index(indexed_file_t *file, int id, int index)
{
	indexed_rec_t record;
	record.id = id;
	record.index = index;

	//Insert using insertion sort
	int i;
	for (i = 0; i < file->records->length; i++)
	{
		indexed_rec_t *point = (indexed_rec_t *) vector_get(file->records, i);

		if (point->id > id)
		{
			vector_insert(file->records, i, &record, sizeof(indexed_rec_t));
			return i;
		}
	}

	//Needs to be added at the end
	vector_add(file->records, &record, sizeof(indexed_rec_t));
	return file->records->length - 1;
}

int index_get_index_rec(indexed_file_t *file, int id, int min, int max)
{
	//There is no index
	if (min > max)
	{
		return -1;
	}

	//Get the midpoint
	int midpoint = index_get_midpoint(min, max);
	indexed_rec_t *mid = (indexed_rec_t *) vector_get(file->records, midpoint);

	//See if the we have the same id
	if (id == mid->id)
	{
		return mid->index;
	}
	//If the id is bigger search the upper half
	else if (id > mid->id)
	{
		return index_get_index_rec(file, id, midpoint + 1, max);
	}
	//If the id is lower search the lower half
	else
	{
		return index_get_index_rec(file, id, min, midpoint - 1);
	}
}

int index_get_index(indexed_file_t *file, int id)
{
	//Can't search a length zero array
	if (file->records->length == 0)
	{
		return -1;
	}

	return index_get_index_rec(file, id, 0, file->records->length - 1);
}

int index_add(indexed_file_t *file, user_t *user)
{
	if (index_get_index(file, user->userid) != -1)
	{
		return -1;
	}

	//Move the cursor to the end of the file
	lseek(file->master_fid, 0, 2);

	//Write user to the end of the file
	write(file->master_fid, user, sizeof(user_t));

	//Update the index in our data structure
	index_add_index(file, user->userid, file->records->length);

	return 0;
}

int index_update(indexed_file_t *file, user_t *user)
{
	//Find out the index of the user
	int index = index_get_index(file, user->userid);

	//Make sure the user exists in our index
	if (index == -1)
	{
		return -1;
	}

	//Move the cursor to the index
	lseek(file->master_fid, index * sizeof(user_t), 0);

	//Write the user out
	write(file->master_fid, user, sizeof(user_t));

	return 0;
}
