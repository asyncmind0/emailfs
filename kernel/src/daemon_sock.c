/***************************************************************************
 *   Copyright (C) 2005 by Steven Joseph  				   *
 *   stevenjose@gmail.com   						   *
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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/statfs.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <asm/errno.h>
#include <linux/buffer_head.h>
#include <linux/pagemap.h>
#include <net/sock.h>
#include <linux/netlink.h> 
//#include "/lib/modules/2.6.9-1.667/build/include/linux/netlink.h"
#define __NO_VERSION__
//#include "sock.h"
#define MAX_PAYLOAD 1024
//#define NULL 0


u32 pid;

static char file_buf[PAGE_SIZE] = "Hello World\n";

int daemon_command(struct sock* nl_sk,char *cmd,void *ret);
void nl_data_ready (struct sock *sk, int len);
spinlock_t my_lock = SPIN_LOCK_UNLOCKED;
void nl_data_ready (struct sock *sk, int len)
{
//DEBG("CALLBACK CALLED");
wake_up_interruptible(sk->sk_sleep);
//DEBG("UNLOCK_EXIT");
}
int daemon_get_next_dentry(struct sock* nl_sk, char *list){
	daemon_command(nl_sk,"LISTNEXT",(void*)list);
	if(strcmp(list,"\n")==0)
		return -1;
	if(strcmp(list,"\t")==0)
		return -2;
	//if(strcmp(list,"LISTNEXT")==0)
	//	return -2;
	return 0;
}


int daemon_readpage(struct file *file, struct page *page,struct sock* nl_sk){
	char cmd[100];
	strcpy(cmd,"RDPG:0:");
	strcat(cmd,file->f_dentry->d_name.name);
	DEBUG("PAGE_SIZEis  :%d\n", PAGE_SIZE );
	daemon_command(nl_sk,cmd,file_buf);
	memcpy(page_address(page),(void *) file_buf, PAGE_SIZE);
	DEBUG("PAGE CONTENT: %s, FILE_BUF :%s \n",page_address(page),file_buf);
	return 0;
}

int check_dcache_valid(struct sock* nl_sk){
	char ret[2];
	daemon_command(nl_sk,"CHECKDCACHE",(void *)ret);
	if(strcmp(ret,"invalid")==0)return 0;
	if(strcmp(ret,"valid")==0)return 1;
	return -1;
	
}
int daemon_ilookup(char * dname,struct sock* nl_sk){
	char name[100]="LKP";
	int ret=5;
	strcat(name,dname);
	DEBUG("ILOOKUP");
	daemon_command(nl_sk,name,(void *)&ret);

	DEBUG("ILOOKUP ok %d",ret);
//f(strcmp(ret,"ENOI")==0)return -1;
	return ret;
	
}

int daemon_command(struct sock* nl_sk,char *cmd,void *ret)
{
        spin_lock(&my_lock);
	int err;	
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh = NULL;
	nl_sk = netlink_kernel_create(17,nl_data_ready);    
	//DEBG("CREATED SOCET\n");        
	/* wait for message coming down from user-space */
      	skb =	skb_recv_datagram(nl_sk,0, 0, &err);	    
      	//DEBG("RECIEVED 1\n");
	nlh = (struct nlmsghdr *)skb->data;
	pid = nlh->nlmsg_pid;
       // printk("EMFS: First recieve  %s: received netlink 
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = 0;  /* from kernel */
	nlh->nlmsg_flags = 0;
	/* sender is in group 1<<0 */
	NETLINK_CB(skb).groups = 0; /* not in mcast group */
        NETLINK_CB(skb).pid = 0;      /* from kernel */
        NETLINK_CB(skb).dst_pid = pid;
        NETLINK_CB(skb).dst_groups = 0;  /* unicast */
	strcpy(NLMSG_DATA(nlh), cmd);
	//printk("EMFS: daemon_send sending : %s %d\n",NLMSG
	netlink_unicast(nl_sk, skb, pid,0);		    
	//DEBG("SEND OK");
	//strcpy(NLMSG_DATA(nlh),"noop");
        /* wait for message coming down from user-space */
	skb = skb_recv_datagram(nl_sk,0, 0, &err);       
	//DEBG("RECEIVED 2\n");
        //printk("EMFS: %s: received netlink message payload
	//DEBG("DISCONNECTING\n");
	//strcpy(ret,NLMSG_DATA(nlh));
	memcpy(ret,NLMSG_DATA(nlh),NLMSG_SPACE(MAX_PAYLOAD));
	sock_release(nl_sk->sk_socket);
	spin_unlock(&my_lock);
	//DEBG("DISCONNECTED\n");
	return 0;
}



