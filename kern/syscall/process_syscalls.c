#include <types.h>
#include <syscall.h>    // trapframe definition
#include <copyinout.h>  //has methods for copying data, does error checks unlike memcpy
#include <mips/trapframe.h>
#include <kern/errno.h> //has macros for errors, definitely needed for waitpid()
#include <proc.h> 
#include <current.h> // to get curthread
#include <kern/limits.h> //to get the __PID_MIN and __PID_MAX macros

//defined in machine-dependent types.h, evals to signed 32-bit int for MIPS

//In Linux getpid is always successful, so errno should be set to 0 in syscall.c
//Returns the pid of the current thread's parent process
int sys_getpid(int32_t *pid) {
	spinlock_acquire(&curthread->t_proc->p_lock);
	*pid = curthread->t_proc->pid;
	spinlock_release(&curthread->t_proc->p_lock);

 	return(0);
}

int sys_fork(struct trapframe *tf, int32_t *retpid) {
	int result;

	/* copy trapframe for child */
	struct trapframe *child_tf = kmalloc(sizeof(struct trapframe));

	if(child_tf == NULL) {
		return ENOMEM;
	}
	memmove(child_tf, tf, sizeof(struct trapframe));

	struct proc *child_proc;
	/* create new child process */
	if((child_proc = proc_create_fork("[userproc]")) == NULL) {
		return ENPROC;
	}
	
	*retpid = child_proc->pid;
	
	/* Fork the child process into a new thread */	
	if((result = thread_fork(curthread->t_name, child_proc,
				enter_forked_process, (void *) child_tf, 0))) {
		return result;
	}	
  	return 0;
}

