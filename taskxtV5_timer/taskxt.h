#ifndef __TASKXT_H_
#define __TASKXT_H_

#include <linux/ioctl.h>

#define TASKXT_GET_PROCESSNAME _IOR('q', 1, taskxt_arg_t *)
#define TASKXT_SMP_PROCESSNAME _IO('q', 2)
#define TASKXT_SET_PROCESSNAME _IOW('q', 3, taskxt_arg_t *)

//structues

typedef struct {
   char pid[5000000];
   int cnt;
//	unsigned int magic;
//	unsigned int version;
//	unsigned long long s_addr;
//	unsigned long long e_addr;
//	unsigned char reserved[8];
} __attribute__ ((__packed__)) task_features;

typedef struct {
   int map_count;
   unsigned long page_table_lock;
   unsigned long hiwater_rss; 
   unsigned long hiwater_vm;
   unsigned long total_vm;
   unsigned long shared_vm;
   unsigned long exec_vm;
   unsigned long nr_ptes;
   long int      utime;
   long int      stime;
   unsigned long nvcsw;
   unsigned long nivcsw;
   unsigned long min_flt;
   unsigned long slock;
   int           fscount;
} __attribute__ ((__packed__)) sample;


typedef struct
{
   int status;
   char processName[100];
} taskxt_arg_t;

#endif
