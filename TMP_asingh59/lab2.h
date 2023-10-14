#ifndef _LAB2_H_
#define _LAB2_H_

#define EXPDISTSCHED 1
#define LINUXSCHED 2

void setschedclass(int sched_class);
int getschedclass();
int getNextProcess(int random);
void next_linux();

#endif