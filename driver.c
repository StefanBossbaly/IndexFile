/**
 * Author: Stefan Bossbaly
 * Driver to the indexed file
 * Compile: gcc -o index.o indexed_file.c vector.c user.c user.h driver.c
 * Run command: ./index.o user.master user.index user.update
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

	//Master file is empty
	if (file->size == 0)
	{
		printf("Master file is empty\n");
	}

	//Go to the beginning of the file
	lseek(file->master_fid, 0, SEEK_SET);

	//Go through all the users in the master file
	for (i = 0; i < file->size; i++)
	{
		//Read the next user in the master file
		read(file->master_fid, &user, sizeof(user_t));

		//Print out the user
		printf("%i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
	}
}

int main(int argc, char *argv[])
{

	//Make sure that there are at least 3 + 1 arguments
	if (argc != 4) {
		printf("Please enter the the path of the files in this order [master] [index] [update]\n");
		return -1;
	}

	//Open the update file with read permissions
	FILE *update = fopen(argv[3], "r");

	//File didn't open for some reason
	if (update == NULL)
	{
		printf("Error: Can not open update file\n");
		return -1;
	}

	//Create the index file structure
	indexed_file_t file;
	index_init(&file, argv[1], argv[2], sizeof(user_t));

	//Open a transaction
	index_open_transaction(&file);

	//Print the output of the master file
	printf("=== Opened transaction. Master file output === \n");
	print_users(&file);
	printf("=== Master file output complete === \n");

	//Create our buffers for reading the update file contents
	char buffer[1024];
	int parameter1;
	char parameter2[20];
	char parameter3[20];
	int parameter4;
	user_t user;

	//Get each line from the update file delimited by \n
	while (fgets(buffer, 1024, update))
	{
		//Check to see if it is the add command
		if (sscanf(buffer, "+ %i %s %s %i", &parameter1, parameter2, parameter3, &parameter4))
		{
			//Get the index of the user
			int index = index_get_index(&file, parameter1);

			//Make sure the index doesn't exists, else add the user
			if (index  != -1)
			{
				printf("ERROR - userid exists.\n");
				index_get_data(&file, parameter1, &user);
				printf("%i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
			}
			else
			{
				printf("ADD: %i, %s, %s, %i\n", parameter1, parameter2, parameter3, parameter4);
				user_init(&user, parameter1, parameter2, parameter3, parameter4);
				index_add(&file, user.userid, &user);
			}
		}
		//Check to see if the command if the update age command
		else if (sscanf(buffer, "* %i %i", &parameter1, &parameter4))
		{
			//Get the result code from getting the user
			int resultCode = index_get_data(&file, parameter1, &user);

			//Something bad happened
			if (resultCode == -1)
			{
				printf("Error - userid = %i does not exist\n", parameter1);
			}
			else
			{
				printf("BEFORE: %i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
				user.age = parameter4;
				index_update(&file, user.userid, &user);
				printf("AFTER: %i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
			}
		}
		//Check to see if it was the query command
		else if (sscanf(buffer, "? %i", &parameter1))
		{
			//Get the result code from getting the user
			int resultCode = index_get_data(&file, parameter1, &user);

			if (resultCode == -1)
			{
				printf("Error - userid = %i does not exist\n", parameter1);
			}
			else
			{
				printf("QUERY: %i, %s, %s, %i\n", user.userid, user.lastname, user.firstname, user.age);
			}

		}
		//Command is invalid
		else
		{
			printf("Invalid command\n");
		}
	}

	//Print the output of the master file
	printf("=== Closing transaction. Master file output === \n");
	print_users(&file);
	printf("=== Master file output complete === \n");

	//Close the transaction
	index_close_transaction(&file);

	return 0;
}
