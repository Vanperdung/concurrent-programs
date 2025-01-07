#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> /* For pid_t */
#include <sys/wait.h>  /* For wait */
#include <unistd.h>    /* For getpid */
#include <signal.h>
#include <sched.h>

#define NUMBER_OF_PROC 3
#define STACK_SIZE (1024 * 1024)

struct proc_meta
{
    pid_t pid;
    void *stack;
    int repeatition;
};

int vos_proc_func(void *args)
{
    int n = 0;
    struct proc_meta *meta = (struct proc_meta*)args;

    while (n < meta->repeatition)
    {
        printf("proc is working, n = %d\n", n);
        sleepms(1);
        n++;
        sched_yield();
    }
}

void vos_proc_create(struct proc_meta *meta)
{
    for (int i = 0; i < NUMBER_OF_PROC; i++)
    {
        meta[i].stack = malloc(STACK_SIZE);
        if (meta[i].pid = clone(vos_proc_func, meta[i].stack + STACK_SIZE,
                                    SIGCHLD, (void*)&meta[i]) < 0)
        {
            perror("clone");
            break;
        }
    }
}

void vos_proc_wait(struct proc_meta *meta)
{
    for (int i = 0; i < NUMBER_OF_PROC; i++)
    {
        if (meta[i].pid >= 0)
        {
            waitpid(meta[i].pid, NULL, 0);
            free(meta[i].stack);
            printf("proc%d finished\n", i); 
        }
    }
}

int main(int argc, char *argv[])
{
    struct proc_meta metas[NUMBER_OF_PROC] = {
        {0, NULL, 10},
        {0, NULL, 20},
        {0, NULL, 30}
    };

    vos_proc_create(metas);
    vos_proc_wait(metas);
    return 0;
}