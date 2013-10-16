/*
 * user.c
 *
 *  Created on: Oct 13, 2013
 *      Author: stefan
 */

#include "user.h"

void user_init(user_t *user, int userid, char *lastname, char *firstname, int age)
{
	user->userid = userid;
	strcpy(user->firstname, firstname);
	strcpy(user->lastname, lastname);
	user->age = age;
}
