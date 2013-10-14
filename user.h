/*
 * user.h
 *
 *  Created on: Oct 13, 2013
 *      Author: stefan
 */

#ifndef USER_H_
#define USER_H_

#include <string.h>

typedef struct
{
	int userid;
	char firstname[20];
	char lastname[20];
	int age;
} user_t;

void user_init(user_t *user, int userid, char *firstname, char *lastname, int age);
#endif /* USER_H_ */
