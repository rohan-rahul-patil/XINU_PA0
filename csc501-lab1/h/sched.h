#ifndef _SCHED_H_
#define _SCHED_H_


#define EXPDISTSCHED 1
#define LINUXSCHED 2

extern int sched_class;

void setschedclass (int sched_class);
int getschedclass();

#endif
