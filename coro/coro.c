#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

#include "list.h"

typedef struct 
{
    int cnt;
    char name[16];
    jmp_buf env;
    int max;
    void (*func)(void *);
} task_metadata_t;

typedef struct 
{
    task_metadata_t metadata;
    struct list_head node;
} task_t;

// Define a task list
LIST_HEAD(head);

jmp_buf sched;
int l = 0;

void task_func(void *args)
{
    task_t *task = (task_t*)args;

    if (setjmp(task->metadata.env) == 0)
    {
        INIT_LIST_HEAD(&task->node);
        list_add_tail(&task->node, &head);
        longjmp(sched, 1);
    }

    while (task->metadata.cnt < task->metadata.max)
    {
        task->metadata.cnt++;
        printf("%s: %d\n", task->metadata.name, task->metadata.cnt);

        if (setjmp(task->metadata.env) == 0)
        {
            // finished its work, yield
            INIT_LIST_HEAD(&task->node);
            list_add_tail(&task->node, &head);
            longjmp(sched, 1);
        }

        // schedule() jumps to here
        printf("%s resumed\n", task->metadata.name);
    }
    printf("%s completed, %d\n", task->metadata.name, task->metadata.cnt);
    longjmp(sched, 1); // back to schedule(), is no longer called by schedule()
}

void task_switch(void)
{
    if (!list_empty(&head))
    {
        task_t *t =  list_first_entry(&head, task_t, node);
        list_del(&t->node);
        longjmp(t->metadata.env, 1);
    }
}

void schedule(task_t *tasks)
{
    setjmp(sched);

    while (l)
    {
        l--;
        tasks[l].metadata.func(&tasks[l]);
    }

    task_switch();
}

int main(void)
{
    task_t tasks[4] = {
        {.metadata.func = task_func, .metadata.cnt = 0, .metadata.max = 10, .metadata.name = "task0"},
        {.metadata.func = task_func, .metadata.cnt = 0, .metadata.max = 20, .metadata.name = "task1"},
        {.metadata.func = task_func, .metadata.cnt = 0, .metadata.max = 30, .metadata.name = "task2"},
        {.metadata.func = task_func, .metadata.cnt = 0, .metadata.max = 40, .metadata.name = "task3"}
    };

    l = sizeof(tasks) / sizeof(tasks[0]);

    schedule(tasks);

    return 0;
}
