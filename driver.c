/*
 * driver.c
 *
 *  Created on: Oct 13, 2013
 *      Author: stefan
 */

#include "indexed_file.h"
#include "user.h"

#include "stdio.h"

int main()
{
	indexed_file_t file;

	index_init(&file, "user.master", "user.index");

	index_open_transaction(&file);

	user_t user;
	user_init(&user, 1, "Stefan", "Bossbaly", 23);

	index_add(&file, &user);

	index_close_transaction(&file);

	printf("Hello\n");

	return 0;
}
