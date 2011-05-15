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

#define DEBGOK(str) printk("EMFS: %s: OK\n",str);
#define DEBG(str) printk("EMFS: %s\n",str);

struct sk_buff *skb = NULL;
struct nlmsghdr *nlh = NULL;
struct sock *nl_sk = NULL;
int err;
u32 pid;
void nl_data_ready (struct sock *sk, int len)
{
	wake_up_interruptible(sk->sk_sleep);
}


void daemon_connect(){
	//DEBG("connecting to daemon ..\n");
	nl_sk = netlink_kernel_create(NETLINK_TEST,nl_data_ready);
	//DEBGOK("Connected .\n");
	skb = skb_recv_datagram(nl_sk, 0, 0, &err);
	nlh = (struct nlmsghdr *)skb->data;
	printk("%s: received netlink message payload:%s\n",__FUNCTION__, NLMSG_DATA(nlh));
	pid = nlh->nlmsg_pid; /*pid of sending process */
	daemon_send("CONNECT",0);
	DEBGOK("Connection Setup");
}
int daemon_send(char *buff,int flag)
{	
	strcpy(NLMSG_DATA(nlh),buff);
	NETLINK_CB(skb).groups = 0; /* not in mcast group */
	NETLINK_CB(skb).pid = 0;      /* from kernel */
	NETLINK_CB(skb).dst_pid = pid;
	NETLINK_CB(skb).dst_groups = 0;  /* unicast */
	//DEBG("Unicasting to user\n")
	DEBGOK("About to send");
	netlink_unicast(nl_sk, skb, pid, flag);
	DEBGOK("Unicast send success..\n")
	return 1;
}

int daemon_recv(char *buf,int n)
{	
	DEBG("Recev from daemon\n");
	skb = skb_recv_datagram(nl_sk, 0, 0, &err);
	DEBG("Recev done ..\n");
	nlh = (struct nlmsghdr *)skb->data;
	printk("%s: received netlink message payload:%s\n",__FUNCTION__, NLMSG_DATA(nlh));
	pid = nlh->nlmsg_pid; /*pid of sending process */
	return 1;
}

void daemon_diconnect(){
	sock_release(nl_sk->sk_socket);	
	//DEBGOK("Disconnect");
}

// void netlink_test() {
// 	struct sk_buff *skb = NULL;
// 	struct nlmsghdr *nlh = NULL;
// 	int err;
// 	u32 pid;     
// 	nl_sk = netlink_kernel_create(NETLINK_TEST, 
// 				      nl_data_ready);
// 	/* wait for message coming down from user-space */
// 	
// 	
// 	printk("%s: received netlink message payload:%s\n", 
// 	       __FUNCTION__, NLMSG_DATA(nlh));
// 	pid = nlh->nlmsg_pid; /*pid of sending process */
// 	NETLINK_CB(skb).groups = 0; /* not in mcast group */
// 	NETLINK_CB(skb).pid = 0;      /* from kernel */
// 	NETLINK_CB(skb).dst_pid = pid;
// 	NETLINK_CB(skb).dst_groups = 0;  /* unicast */
// 	netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
// 	sock_release(nl_sk->socket);
// }    

