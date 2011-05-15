#ifndef _EMFS_DEBUG_H_
#define _EMFS_DEBUG_H_

#define EMFS_DEBUG 1

#ifdef EMFS_DEBUG
#define DEBUG(x...) printk("EMFS>" KERN_DEBUG __FUNCTION__ ": " x)
#else
#define DEBUG(x...) ;
#endif

#ifdef EMFS_VERBOSE
#define VERBOSE(x...) printk("EMFS>" KERN_NOTICE __FUNCTION__ ": " x)
#else
#define VERBOSE(x...) ;
#endif




#endif
