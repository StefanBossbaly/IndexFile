/*
 * driver.c
 *
 *  Created on: Oct 13, 2013
 *      Author: stefan
 */

#include "indexed_file.h"
#include "user.h"

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

void print_users(indexed_file_t *file)
{
	int i = 0;
	user_t user;

	if (file->size == 0)
	{
		printf("Master file is empty\n");
	}

	lseek(file->master_fid, 0, SEEK_SET);

	for (i = 0; i < file->size; i++)
	{
		read(file->master_fid, &user, sizeof(user_t));

		printf("%i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
	}
}

int main(int argc, char *argv[])
{

	if (argc != 4) {
		printf("Please enter the the path of the files in this order [master] [index] [update]\n");
		return -1;
	}

	FILE *update = fopen(argv[3], "r");

	if (update == NULL)
	{
		printf("Error: Can not open update file\n");
		return -1;
	}

	indexed_file_t file;
	index_init(&file, argv[1], argv[2]);

	index_open_transaction(&file);

	printf("Opened transaction. Master file output \n");
	print_users(&file);

	char buffer[1024];
	int parameter1;
	char parameter2[20];
	char parameter3[20];
	int parameter4;
	user_t user;

	while (fgets(buffer, 1024, update))
	{
		if (sscanf(buffer, "+ %i %s %s %i", &parameter1, parameter2, parameter3, &parameter4))
		{
			int index = index_get_index(&file, parameter1);

			if (index  != -1)
			{
				printf("ERROR - userid exists.\n");
				index_get_user(&file, parameter1, &user);
				printf("%i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
			}
			else
			{
				printf("ADD: %i, %s, %s, %i\n", parameter1, parameter2, parameter3, parameter4);
				user_init(&user, parameter1, parameter2, parameter3, parameter4);
				index_add(&file, &user);
			}
		}
		else if (sscanf(buffer, "* %i %i", &parameter1, &parameter4))
		{
			int resultCode = index_get_user(&file, parameter1, &user);

			if (resultCode == -1)
			{
				printf("Error - userid = %i does not exist\n", parameter1);
			}
			else
			{
				printf("BEFORE: %i, %s, %s, %i", user.userid, user.lastname, user.firstname, user.age);
				user.age = parameter4;
				index_update(&file, &user);
				printf("AFTER: %i, %s, %s, %i", user.userid, user.lastname, user.firstname, user.age);
			}
		}
		else if (sscanf(buffer, "? %i", &parameter1))
		{
			int resultCode = index_get_user(&file, parameter1, &user);

			if (resultCode == -1)
			{
				printf("Error - userid = %i does not exist\n", parameter1);
			}
			else
			{
				printf("QUERY: %i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
			}

		}
		else
		{
			printf("Invalid command\n");
		}
	}


	printf("Closing transaction. Master file output \n");
	print_users(&file);

	index_close_transaction(&file);

	return 0;
}
