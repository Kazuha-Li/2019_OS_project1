#define _GNU_SOURCE
#include "process.h"
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define GET_TIME 335
#define PRINTK 336

int proc_assign_cpu(int pid, int core)
{
	if (core > sizeof(cpu_set_t))
	{
		fprintf(stderr, "Core index error.");
		return -1;
	}

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);

	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0)
	{
		perror("sched_setaffinity");
		exit(1);
	}

	return 0;
}

int proc_exec(struct process proc)
{
	int pid = fork();

	if (pid < 0)
	{
		perror("fork");
		return -1;
	}

	if (pid == 0)
	{
		struct timespec start_sec, end_sec;
		syscall(GET_TIME, &start_sec);
		for (int i = 0; i < proc.t_exec; i++)
		{
			UNIT_T();
		}
		syscall(GET_TIME, &end_sec);
		syscall(PRINTK, getpid(), &start_sec, &end_sec);
		exit(0);
	}

	/* Assign child to another core prevent from interupted by parant */
	proc_assign_cpu(pid, CHILD_CPU);

	return pid;
}

int proc_block(int pid)
{
	struct sched_param param;

	/* SCHED_IDLE should set priority to 0 */
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_IDLE, &param);

	if (ret < 0)
	{
		perror("sched_setscheduler");
		return -1;
	}

	return ret;
}

int proc_wakeup(int pid)
{
	struct sched_param param;

	/* SCHED_OTHER should set priority to 0 */
	param.sched_priority = 0;

	int ret = sched_setscheduler(pid, SCHED_OTHER, &param);

	if (ret < 0)
	{
		perror("sched_setscheduler");
		return -1;
	}

	return ret;
}
