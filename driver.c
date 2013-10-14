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
	return 0;
}
