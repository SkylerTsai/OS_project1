#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>

#define PMAX       16  /* process number maximun    */
#define RRTQ      500  /* RR time quantum           */
#define PCPU        0  /* Parent CPU                */
#define CCPU        1  /* Child CPU                 */
#define get_time  333  /* system call for get_time  */
#define printk    334  /* system call for p1_printk */
#define FIFO        1
#define RR          2
#define SJF         3
#define PSJF        4

void unit_of_time(){
	volatile unsigned long i;
	for(i = 0; i < 1000000UL; i++);
	return;
}

typedef struct process{
	char name[32];
	int ready;
	int exec;
	pid_t pid;
} Process;

void CPU_assigning(int pid, int CPU){
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(CPU, &mask);

	sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
}

void block_process(int pid){
	struct sched_param param;
	param.sched_priority = 0;
	sched_setscheduler(pid, SCHED_IDLE, &param);

	return;
}

void wake_process(int pid){
	struct sched_param param;
	param.sched_priority = 0;
	sched_setscheduler(pid, SCHED_OTHER, &param);

	return;
}

void exec_process(Process *p){
	p->pid = fork();

	if(p->pid == 0){
		p->pid = getpid();
		fprintf(stdout, "%s %d\n", p->name, p->pid);
		
		struct timespec s, e;
//		syscall(get_time, &s);
		for(int i = 0; i < p->exec; i++){
			unit_of_time();
			fprintf(stdout, "Run %s\n", p->name);
		}
//		syscall(get_time, &e);
	
//		syscall(printk, s.tv_sec, s.tv_nsec, e.tv_sec, e.tv_nsec);

		exit(0);
	}
	else CPU_assigning(p->pid, CCPU);
	
	return;
}

int find_next(int s_type, Process P[], int p_num, int p_now){
	if(p_now != -1 && s_type != PSJF) return p_now;
	
	int prio = -1, fin = 0;
	for(int i = 0; i < p_num; i++){
		if(P[i].pid == -1) continue;
		else if(P[i].exec == 0) fin++;
		else if(prio == -1) prio = i;
		else if(s_type == FIFO && P[i].ready < P[prio].ready) prio = i;
		else if(s_type != FIFO && P[i].exec  < P[prio].exec ) prio = i;
	}

	if(fin == p_num) return fin;
	else return prio;
}

void queue_push(int queue[], int q_now, int p_num, int p){
	for(int i = q_now; ; i = (i+1)%p_num){
		if(queue[i] == -1){
			queue[i] = p;
			break;
		}
	}
	return;
}

int queue_pop(Process P[], int p_num, int queue[], int q_now, int period){
	if(period % RRTQ == 0){
		int p = queue[q_now];
		queue[q_now] = -1, q_now = (q_now+1)%p_num;
		queue_push(queue, q_now, p_num, p);
	}

	if(queue[q_now] == -1){
		int q_prev = q_now, fin = 0;
		q_now = (q_now+1)%p_num;
		while(q_now != q_prev){
			if(queue[q_now] != -1) break;
			q_now = (q_now+1)%p_num;
		}

		if(q_now == q_prev){
			for(int i = 0; i < p_num; i++){
				if(P[i].exec == 0) fin++;
			}
			if(fin == p_num) q_now == p_num;
		}
	}
	
	return q_now;
}

void schedule(int s_type, Process P[], int p_num){
	int time_now = 0, p_now = -1;
	int queue[PMAX], q_now = 0, last_switch = 0;

	for(int i = 0; i < p_num; i++) queue[i] = -1;
	queue[p_num] = p_num;

	wake_process(getpid());
	CPU_assigning(getpid(), PCPU);

	while(1){
		if(p_now != -1 && P[p_now].exec == 0){
			waitpid(P[p_now].pid, NULL, 0);
			p_now = -1;
			if(s_type == RR){
				queue[q_now] = -1;
				q_now = (q_now+1)%p_num;
			}
		}

		for(int i = 0; i < p_num; i++){
			if(P[i].ready == time_now){
				exec_process(&P[i]);
				block_process(P[i].pid);
				if(s_type == RR) queue_push(queue, q_now, p_num, i);
			}
		}

		int next = -1;
		if(s_type == RR){
			q_now = queue_pop(P, p_num, queue, q_now, time_now - last_switch);
			next = queue[q_now];
		}
		else next = find_next(s_type, P, p_num, p_now);

		fprintf(stdout, "%d ", time_now);
		
		if(next == p_num) break;
		else if(next == -1) fprintf(stdout, "No process\n");
		else if(p_now != next){
			block_process(P[p_now].pid);
			p_now = next, last_switch = time_now;
			wake_process(P[p_now].pid);
		}

		unit_of_time();

		if(p_now != -1){
			P[p_now].exec--;
			fprintf(stdout, "%s left %d\n", P[p_now].name, P[p_now].exec);
		}
		
		time_now++;
	}

	return;
}

int main(){
	char policy[8] = {};
	int p_num = 0, s_type = 0;

	setpriority(PRIO_PROCESS, getpid(), 1);
	
	scanf("%s %d", policy, &p_num);

	Process *P = (Process *)malloc(p_num * sizeof(Process));

	for(int i = 0; i < p_num; i++){
		scanf("%s %d %d", P[i].name, &P[i].ready, &P[i].exec);
		P[i].pid = -1;
	}

	if(strcmp(policy, "FIFO") == 0)      s_type = FIFO;
	else if(strcmp(policy, "RR") == 0)   s_type = RR;
	else if(strcmp(policy, "SJF") == 0)  s_type = SJF;
	else if(strcmp(policy, "PSJF") == 0) s_type = PSJF;

	schedule(s_type, P, p_num);

	return 0;
}