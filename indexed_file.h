/*
 * indexed_file.h
 *
 *  Created on: Oct 13, 2013
 *      Author: stefan
 */

#ifndef INDEXED_FILE_H_
#define INDEXED_FILE_H_

#include "user.h"
#include "stdio.h"

typedef struct
{
	int id;
	int index;
} indexed_rec_t;

typedef struct
{
	char index_fname[50];
	char master_fname[50];
	int index_fid;
	int master_fid;
	int size;
} indexed_file_t;

void index_init(indexed_file_t *file, char *master, char *index);
void index_open_transaction(indexed_file_t *file);
void index_close_transaction(indexed_file_t *file);

int index_get_user(indexed_file_t *file, int userid, user_t *buffer);

int index_get_midpoint(int min, int max);
int index_get_index(indexed_file_t *file, int id);
int index_add_index(indexed_file_t *file, int id, int index);

int index_add(indexed_file_t *file, user_t *user);
int index_update(indexed_file_t *file, user_t *user);


#endif /* INDEXED_FILE_H_ */
