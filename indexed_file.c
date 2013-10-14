#include "indexed_file.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include <string.h>


void index_init(indexed_file_t *file, char *master, char *index)
{
	strcpy(file->master_fname, master);
	strcpy(file->index_fname, index);

	//Get the size of the file and then figure out how many users are in it
	struct stat sb;
	stat(file->master_fname, &sb);
	file->size = (sb.st_size) / sizeof(user_t);
}

void index_open_transaction(indexed_file_t *file)
{
	file->index_file = fopen(file->index_fname, "ab+");
	file->master_fid = open(file->master_fname, O_RDWR | O_CREAT, S_IRWXU);
}

void index_close_transaction(indexed_file_t *file)
{
	//fclose(file->index_file);
	close(file->master_fid);

	file->master_fid = -1;
}

int index_add(indexed_file_t *file, user_t *user)
{
	int index = file->size * sizeof(user_t);

	//Move the cursor to the end of the file
	lseek(file->master_fid, 0, 2);

	//Write user to the end of the file
	write(file->master_fid, user, sizeof(user_t));

	//Move the cursor to the end of the file
	fseek(file->index_file, 0, SEEK_END);

	//Write the data to the index file
	fprintf(file->index_file, "%d %d\n", user->userid, index);

	return 0;
}

int index_update(indexed_file_t *file, user_t *user)
{
	return 0;
}
int index_delete(indexed_file_t *file, int id)
{
	return 0;
}
