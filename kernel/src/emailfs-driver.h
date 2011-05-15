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

#define EMFS_MAGIC 0xabcd
#define FILE_INODE_NUMBER 2
#include <linux/fs.h>
#define NETLINK_TEST  17

static struct inode *emailfs_root_inode;

/*function prototypes*/
//function to read the superblock during mount called by do_mount()
static struct super_block * emailfs_get_sb(struct file_system_type *fs_type, int flags, const char *devname, void *data);

//function to fill superblock
int emailfs_fill_sb(struct super_block *sb, void *data, int silent);
//cleanup function called during umount
static void emailfs_kill_sb(struct super_block *);

//super_operations begin
void emailfs_super_read_inode (struct inode *);
static int emailfs_super_write_inode(struct inode *inode, int wait);
//void emailfs_super_delete_inode (struct inode *);
//void emailfs_super_put_super (struct super_block *);
//int emailfs_super_statfs (struct super_block *, struct statfs *, int);
//super_operations end

//inode operations
static struct dentry * emailfs_inode_lookup(struct inode *parent_inode, struct dentry *dentry, struct nameidata *nameidata);
//static void emailfs_read_inode(struct inode *inode);
//static int emailfs_write_inode(struct inode *inode, int wait);

//end inode operations

//file operations
static int emailfs_file_open(struct inode *inode, struct file *file);
static int emailfs_file_readdir(struct file *file, void *dirent, filldir_t filldir);
static int emailfs_file_release (struct inode *ino, struct file *file);
//end file operations

//address space operations
static int emailfs_writepage(struct page *page, struct writeback_control *wbc);
static int emailfs_readpage(struct file *file, struct page *page);
static int emailfs_commit_write(struct file *file, struct page *page,unsigned from, unsigned to);
static int emailfs_prepare_write(struct file *file, struct page *page,unsigned from, unsigned to);
int emailfs_drevalidate(struct dentry *dent, int n);


