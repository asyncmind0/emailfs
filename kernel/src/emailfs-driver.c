
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

#define EMFS_DEBUG 1

#ifdef EMFS_DEBUG
#define DEBUG(x...) 	printk("EMFS>" /*KERN_DEBUG __FUNCTION__*/ ": " x)
#else
#define DEBUG(x...) ;
#endif

#ifdef EMFS_VERBOSE
#define VERBOSE(x...) 	printk("EMFS>"/* KERN_NOTICE __FUNCTION__*/ ": " x)
#else
#define VERBOSE(x...) ;
#endif

#include "emailfs-driver.h"
#include <net/sock.h>
#include <linux/netlink.h>
#include "daemon_sock.c"
#include "emailfs.h"
#define SERVER "/tmp/server"

char buff[1050];
MODULE_DESCRIPTION("emailfs kernel module");
MODULE_AUTHOR("Steven Joseph (stevenjose@gmail.com)");
MODULE_LICENSE("GPL");
static struct file_system_type emailfs_type=			//registers the file system with the system
{
	.name           = "emailfs",
	.get_sb         = emailfs_get_sb,	
	.kill_sb        = emailfs_kill_sb	
};

struct super_operations emailfs_sops=			//super block operations - 	
{
	.read_inode=emailfs_super_read_inode,	
	.write_inode=emailfs_super_write_inode,	
	// .delete_inode=emailfs_super_delete_inode,
	//.put_super=emailfs_super_put_super,		
	.statfs=simple_statfs//emailfs_super_statfs	
};

static struct inode_operations emailfs_iops =		// inode operations struct
{
	.lookup=emailfs_inode_lookup
};

static struct file_operations emailfs_fops =		//file operations struct
{
	.open		= emailfs_file_open,
	.read		= &generic_file_read,
	.readdir	= &emailfs_file_readdir,
	.write		= &generic_file_write,
	.release	= &emailfs_file_release,
	.fsync		= simple_sync_file
};

static struct address_space_operations emailfs_aops =	// adress space operations
{
	.readpage = emailfs_readpage,
	//.writepage = emailfs_writepage,
	//.prepare_write = emailfs_prepare_write,
	//.commit_write = emailfs_commit_write
};

//struct dentry_operations  emailfs_dops = {
//        .d_revalidate = emailfs_drevalidate
//};
///*struct dentry_operations {
//        int (*d_revalidate)(struct dentry *, int);
//        int (*d_hash) (struct dentry *, struct qstr *);
//        int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
//        void (*d_delete)(struct dentry *);
//        void (*d_release)(struct dentry *);
//        void (*d_iput)(struct dentry *, struct inode *);
//};*/
//
static int file_size = NLMSG_SPACE(MAX_PAYLOAD);
int inoc;
//int emailfs_drevalidate(struct dentry *dent, int n){
//return 0;
//}
static struct super_block * emailfs_get_sb(struct file_system_type *fs_type, int flags, const char *devname, void *data)
{
	return get_sb_single(
	           fs_type,
	           flags,
	           data,
	           &emailfs_fill_sb);
}


int emailfs_fill_sb(struct super_block *sb, void *data, int silent)
{
	//static int;
	sb->s_blocksize = 1024;
	sb->s_blocksize_bits = 10;
	sb->s_magic = EMFS_MAGIC;
	sb->s_op = &emailfs_sops; 			// super block operations
	sb->s_type = &emailfs_type; 			// file_system_type

	emailfs_root_inode = iget(sb, 1); 		// allocate an inode
	emailfs_root_inode->i_op = &emailfs_iops;	// set the inode ops
	emailfs_root_inode->i_mode = S_IFDIR|S_IRWXU;
	emailfs_root_inode->i_fop = &emailfs_fops;

	if(!(sb->s_root = d_alloc_root(emailfs_root_inode)))
	{
		iput(emailfs_root_inode);
		return -ENOMEM;
	}

	return 0;
}

static void emailfs_kill_sb(struct super_block *super)
{
	kill_anon_super(super);
}

void emailfs_super_read_inode(struct inode *inode)		//here inodes are read from deamon
{
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_mapping->a_ops = &emailfs_aops;
}

static int emailfs_super_write_inode(struct inode *inode, int wait)
{
	DEBUG( "emailfs_super_write_inode (i_ino = %d) = %d\n" , (int)inode->i_ino , (int)i_size_read(inode) );
	if(inode->i_ino == FILE_INODE_NUMBER||inode->i_ino == 3)
	{
		file_size = i_size_read(inode);
	}
	return 0;
}


/**
 * this function creates inodes , listing files look here
 * @param parent_inode 
 * @param dentry 
 * @param nameidata 
 * @return 
 */
static struct dentry * emailfs_inode_lookup(struct inode *parent_inode, struct dentry *dentry, struct nameidata *nameidata)
{
	struct inode *file_inode;
	int inum;
	struct sock *nl_sk = NULL;
	if(parent_inode->i_ino != emailfs_root_inode->i_ino )
	{
		//DEBUG("nulled = %s\n",dentry->d_name.name);
		d_add(dentry, NULL);
		goto out;
	}

	DEBUG("LOOKUP allocated =%s\n, inode num= %d",dentry->d_name.name,dentry->d_count); 			//allocating inode
	//if(!strncmp(dentry->d_name.name, "hello.txt", dentry->d_name.len))
	inum=(int)(dentry->d_name.name[0]-'\0');
	file_inode = iget(parent_inode->i_sb,/*daemon_ilookup(dentry->d_name.name,nl_sk));*/inum);//FILE_INODE_NUMBER);
	DEBUG("EMFS LOOKUP OK");
	/*if(!strncmp(dentry->d_name.name, "hello2.xt", dentry->d_name.len))
	file_inode = iget(parent_inode->i_sb, 3);*/			
	if(!file_inode)
		return ERR_PTR(-EACCES);
	file_inode->i_size = file_size;
	file_inode->i_mode = S_IFREG|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
	file_inode->i_fop = &emailfs_fops;
	//file_inode->u.generc_ip=
	//  file_inode->i_fop
	d_add(dentry, file_inode);
out:
	return NULL;
}

/*file operations */

static int emailfs_file_open(struct inode *inode, struct file *file)
{
	static char buff[1024];
	DEBUG("FILE OPEN");
	printk("max_readahead (file-open) = %d (size = %d)\nname= %s\n", (int)file->f_ra.ra_pages, file_size,file->f_dentry->d_name.name);
	file->f_ra.ra_pages = 0; /* No read-ahead */
	//daemon_connect( (struct sock *)inode->u.generic_ip);	
	//DEBG("CONNECTED");
	return generic_file_open(inode, file);
}

static int emailfs_file_release (struct inode *ino, struct file *file)
{
	struct dentry *dentry;
	//BUG("FILE RELEASE\n");
	dentry = file->f_dentry;
	return 0;
}

static int emailfs_file_readdir(struct file *file, void *dirent, filldir_t filldir)
{
	struct dentry *de = file->f_dentry;
	struct sock *nl_sk = NULL;
	int ret;
	int inum=0;
	if(file->f_pos > 2) //check_dcache_valid(nl_sk)==1)//file->f_pos > 2)
		return 1;
	DEBUG("EMFS: filepos: %d, INODE :%s \n",file->f_pos++,file->f_dentry->d_name.name);
	if(filldir(dirent, ".", 1, file->f_pos++, de->d_inode->i_ino, DT_DIR))
		return 0;
	if(filldir(dirent, "..", 2, file->f_pos++, de->d_parent->d_inode->i_ino, DT_DIR))
		return 0;
	inum=de->d_parent->d_inode->i_ino;

/*	if(filldir(dirent, "hello.txt", 9, file->f_pos++, inum++, DT_REG))
		return 0;
	if(filldir(dirent, "hello2.xt", 9, file->f_pos++, inum++, DT_REG))
		return 0;*/
	do{
		DEBUG("Going to get");
		ret=daemon_get_next_dentry(nl_sk,buff);				
		if(ret==-2)return 1;
		DEBUG("dir: %s\n" , buff);
		if(ret==0)if(filldir(dirent,buff, strlen(buff), file->f_pos++,inum++, DT_REG))
		return 0;
	}while(ret==0);
	return 1;
}

/* address_space_operations */
static int emailfs_writepage(struct page *page, struct writeback_control *wbc)
{
	/*DEBUG("[EMAILFS] offset = %d\n", (int)page->index);
	DEBUG("emailFS: WritePage: [%s] [%s] [%s] [%s]\n",
	       PageUptodate(page) ? "Uptodate" : "Not Uptodate",
	       PageDirty(page) ? "Dirty" : "Not Dirty",
	       PageWriteback(page) ? "PageWriteback Set" : "PageWriteback Cleared",
	       PageLocked(page) ? "Locked" : "Unlocked");*/
	memcpy(file_buf, page_address(page), PAGE_SIZE);
	ClearPageDirty(page);
	if(PageLocked(page))
		unlock_page(page);
	return 0;
}

static int emailfs_readpage(struct file *file, struct page *page)
{	
	char buff[100];
	struct sock *nl_sk = NULL;
	int err=0;
	DEBUG("READPAGE filename = %s\n",file->f_dentry->d_name.name);
	DEBUG("EMFS: readpage called for page index=[%d]\n", (int)page->index);
	if(page->index > 3)
	{
		return -ENOSPC;
	}

	DEBUG("EMFS: Page: [%s] [%s] [%s] [%s]\n",PageUptodate(page) ? "Uptodate" : "Not Uptodate",PageDirty(page) ? 
"Dirty" : "Not Dirty",PageWriteback(page) ? "PageWriteback Set" : "PageWriteback Cleared",PageLocked(page) ? "Locked" : "Unlocked");

	if(daemon_readpage(file,page,nl_sk)==-ENOSPC)return -ENOSPC;
	SetPageUptodate(page);
	//DEBUG("daemon readpage return: %s\n",buff);
	if(PageLocked(page))
		unlock_page(page);
	return err;
}

static int emailfs_prepare_write(struct file *file, struct page *page,unsigned from, unsigned to)
{
	return 0;
}

/* modified from generic_commit_write. generic_commit_write calls the
*block device layer to write set up buffer heads for I/O.
*/
static int emailfs_commit_write(struct file *file, struct page *page,unsigned from, unsigned to)
{
	struct inode *inode = page->mapping->host;
	loff_t pos = ((loff_t)page->index << PAGE_CACHE_SHIFT) + to;

	/*VERBOSE("EMFS: commit_write: [%s] [%s] [%s] \n",
	       PageUptodate(page) ? "Uptodate" : "Not Uptodate",
	       PageDirty(page) ? "Dirty" : "Not Dirty",
	       PageLocked(page) ? "Locked" : "Unlocked");
	*/
	if(page->index == 0)
	{
		memcpy(file_buf, page_address(page), PAGE_SIZE);
		ClearPageDirty(page);
	}

	/*
	* No need to use i_size_read() here, the i_size
	* cannot change under us because we hold i_sem.
	*/
	if (pos > inode->i_size)
	{
		i_size_write(inode, pos);
		mark_inode_dirty(inode);
	}


	SetPageUptodate(page);
	return 0;
}

/*module initialization and release**/
static int emailfs_init_module(void)
{
	DEBUG( "Module emailfs init\n" );
	register_filesystem(&emailfs_type);

	return 0;
}

static void emailfs_exit_module(void)
{	
	DEBUG( "Module emailfs exit\n" );
	unregister_filesystem(&emailfs_type);
}

module_init(emailfs_init_module);
module_exit(emailfs_exit_module);

