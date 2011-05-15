/***************************************************************************
 *   Copyright (C) 2005 by Steven Joseph   *
 *   stevenjose@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#define DEBG(str) printf("%s\n",str);
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <ctype.h> /* use of toupper() */



#include "pop_ops.h"
#define STRING 2048

char* c_err=NULL;
popsession* mysession;
int nextent=0,i_popnum;

char* strstrup(const char* string, const char* word){
	char upstring[STRING];
	char upword[STRING];
	int i;
	char* match;
	
	strncpy(upstring, string, STRING);
	strncpy(upword, word, STRING);

	for (i=0; upstring[i]!='\0'; i++){
		upstring[i]=toupper(upstring[i]);
	}
	for (i=0; upword[i]!='\0'; i++){
		upword[i]=toupper(upword[i]);
	}

	match=strstr(upstring, upword);
	if (match!=NULL){
		i=match-upstring;
		return((char*)((int)string+i));
	}else{
		return(NULL);
	}
}

char* getlinewhere(const char* header, const char* word){
	char *begin, *end, *line;
	
	begin=strstrup(header, word);
	begin=strchr(begin,':');
	begin=begin+sizeof(char);
	if(begin==NULL){
		return(NULL);
	}
	end=strstr(begin, "\r");
	//printf("OK1 GL\n");
	line=(char*)malloc(((end-begin)*sizeof(char))+1);
	strncpy(line,begin,(end-begin));
	line[end-begin]='\0';
	//line=strchr(line,':');
	//line=line+sizeof(char);
	//printf("OK2 GL\n");
	return line;
}
void freeline(char* line){
	free(line);
}


int pop_setup(char *myservername,char *username,char *password)
{
	int i=0, answer=9999999;


	c_err=popbegin(myservername,username,password,&mysession);
	if(c_err) {
		printf("%s",c_err);
		//free(err);
		return -1;
	}
}


	
int fill_dcache(){
	//printf("okay!1\n");
	char tbuff[2048];

	char* erro=NULL;
	char* subject=NULL;
	char* from=NULL;
	static int i=1; 
	
	//printf("Got %d messahhges\n",popnum(mysession));
	
	if((dcache.nentries=popnum(mysession)) > 0){
		//printf("Holds %d bytes of data.\n",popbytes(mysession));
		/* listing */
		dcache.cache=(char**)malloc(sizeof(char*)*dcache.nentries);
		dcache.inums=(int *)malloc(sizeof(int)*dcache.nentries);
		for(i=1;i<=i_popnum;i++){
			erro=popgethead(mysession,i);
			//from=strchr(erro,":");
			from=getlinewhere(erro, "From:");
			subject=getlinewhere(erro, "Subject:");
			sprintf(tbuff,"%d ",i);
			//strcpy(tbuff,);//popmsguid(mysession,i));
			strcat(tbuff,from);
			strcat(tbuff,subject);
			dcache.cache[i-1]=(char *)malloc(strlen(tbuff)*sizeof(char)+1);
			dcache.inums[i]=i;	
			strcpy(dcache.cache[i-1],tbuff);
			//printf("name: %s\n",tbuff);
			freeline(from); from=NULL;
			freeline(subject);subject=NULL;
			free(erro);erro=NULL;
			//strcpy(buff,tbuff);
		
		}
		//else {
		//	i=0;
		//	//strcpy(buff,"\n");
		//	return -1;
		//}
		
		
	}	
	return 0;
}
		

int get_next_entry(char *buff)
{
//static int i=0;
int j;
//if(i<=3){
//strcpy(buff,"directory");
//i++;
//return 0;
//
//}else{
//i=0;
//strcpy(buff,"\n");
//return -1;
//}

//printf("OK1\n");
if(c_err){
	strcpy(buff,"CONNECTION ERROR");
	return -1;
}
if(dcache.nentries!=(i_popnum=popnum(mysession))||dcache.cache==NULL)fill_dcache();
//for(j=0;j<dcache.nentries;j++)printf("CACHE %s\n",dcache.cache[j]);
if(nextent<dcache.nentries){
//	strcpy(buff,"nextentry");
	strcpy(buff,dcache.cache[nextent]);
	nextent++;
	return 0;
}
else{
	strcpy(buff,"\n");
	nextent=0;
	return -1;
}
}

int check_mail_changes(){
	if(dcache.nentries!=popnum(mysession))return -1;
	else return 1;
}

int dcache_ilookup(char *name){
	int i;
	
	for(i=0;i<dcache.nentries;i++){
		if(strcmp(name,dcache.cache[i])==0){
			return dcache.inums[i];
		}
		
	}
		return 1;
}

int get_page(char * filename,int pagenum,char *buff,int size){
	char id[17],*emsg;
	int nid;
	strncpy(id,filename,2);
	printf("READPAGE for msg ID:%s\n",id);
	nid=atoi((const char*)id);
	printf("ID num = %d ",nid);
	emsg = popgetmsg(mysession,nid);
	strncpy(buff,emsg,size);
	printf("\nbuff = %s\n",buff);
	return 0;
}
