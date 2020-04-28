#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage void sys_p1_printk(int pid, unsigned long ss, unsigned long sns, unsigned long es, unsigned long ens)
{
	printk("[project1] %d %lu.%lu %lu.%lu\n", pid, ss, sns, es, ens);
 	return;
}
