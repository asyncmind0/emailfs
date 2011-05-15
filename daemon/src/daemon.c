/* this is part of the libspopc library sources
 * copyright  2002 Benoit Rouits <brouits@free.fr>
 * released under the terms of GNU LGPL
 * (GNU Lesser General Public Licence).
 * libspopc offers simple API for a pop3 client (MTA).
 * See RFC 1725 for pop3 specifications.
 * more information on http://brouits.free.fr/libspopc/
 */

/* Using objects-level methods of libspopc is very
 * EASY but doesn't allow you to do everything you
 * want. It just makes the useful tasks
 */
#define DEBG(str) printf("%s\n",str);
#include <string.h> /* use of strcpy() */
#include <ctype.h> /* use of toupper() */
#include <stdio.h> /* use of printf() */
#include <stdlib.h> /* use of exit() */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "../libspopc.h"
#include "kern_sock.h"
#include "pop_ops.h"
/*
 * Butcher's hook for a look
 */
#define STRING 2048




int main(int argc,char** argv)
{

	char myservername[64];
	char username[64];
	char password[64];
	char tmp[2048];
	char *option;


	strcpy(myservername,"nmma.sancharnet.in");
	strcpy(username,"josephjoy");
	strcpy(password,"ann1990");
	//printf("atleast this\n");
	//printf("Connect: %s\n",
	pop_setup(myservername,username,password);
	//while(get_next_entry(tmp)!=-1)printf("file: %s\n",tmp);
	//get_next_entry(tmp);
	//get_list(tmp);
	//printf("entryyyy: %s",tmp);	
	//fill_cache();
	//while(get_next_entry(tmp)==0);
	//while(get_next_entry(tmp)==0);
	while(1)kern_msg_handler();
	//printf("atleast this\n");
	
	//popend(mysession);
	exit(0);
}



