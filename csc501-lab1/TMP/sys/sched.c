#include <sched.h>

int sched_class;

void setschedclass (int s) {
	sched_class = s;
}

int getschedclass() {
	return sched_class;
}
