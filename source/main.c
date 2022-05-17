#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define WITH_SIGNALS
#define NUM_CHILD 60

#ifdef WITH_SIGNALS//2 versions(task4.a)
int interrupted = 0;
void interrupt_handler();
void terminate_handler();
#endif

// print an appropriate message and send to all already created child processes
void finish_children(int amount, int* pids)
{
    for(int j=0; j<amount; j++)
    {
        kill(pids[j], SIGTERM);
        printf("parent[%d]: Sending SIGTERM to PID %d\n", (int)getpid(), (int)pids[j]);
    }
}

int main(int argc, char** argv)
{
    #ifdef WITH_SIGNALS
    for(int i=1; i<NSIG; i++){ // ignoring of all signals with the signal() (task3.a)
        signal(i, SIG_IGN);
    }
    signal(SIGCHLD, SIG_DFL); // restore SIGCHLD(task3.a)
    signal(SIGINT, interrupt_handler);//  set my own keyboard interrupt signal handler (task3.b)
    #endif
    pid_t child_pids[NUM_CHILD]; //the number of child pids
    int children_count = 0; // set the count to 0
    pid_t pid; //initialize

    for(int i=0; i<NUM_CHILD; i++)
    {
        #ifdef WITH_SIGNALS
        if(interrupted != 0) // check interrupt(task3.c)
        {
            // print message and exit loop
            finish_children(i, child_pids);
            printf("parent[%d]: Child creation interrupted.\n", (int)getpid());
            break;
        }
        #endif
        
        pid = fork();// create one of NUM_CHILD child process
        if(pid < 0)// if fork() not succeeded(task2.2)
        {
            // Send SIGTERM to all already forked children
            finish_children(i, child_pids);
            // And exit with error code 1
            return 1;
        }
        else if(pid > 0)// if fork() succeeded(task2.3)
        {
            child_pids[i] = pid; // Add to PID array
            printf("Parent PID [%d]: Child process is created. CHILD PID: %d\n", (int)getpid(), (int)child_pids[i]);
            ++children_count;
            sleep(1);// insert one second delay(task2.1)
            continue;
        }
        else// pid==0: child process
        {
            #ifdef WITH_SIGNALS
            signal(SIGINT, SIG_IGN);
            signal(SIGTERM, terminate_handler);
            #endif
            //child process algorithm (task2)
            printf("child[%d]: PID of parent process: %d\n", (int)getpid(), (int)getppid());
            sleep(10);
            printf("child[%d]: Execution completed: exiting with code 0\n", (int)getpid());
            return 0;
        }
    }
    // print ending message and the number of exit codes(task2.4)
    int exit_codes[children_count];
    int finished = 0;
    int code;
    while(finished < children_count)
    {
        pid = wait(&code);// wait for child to exit (task2.4)
        for(int i=0; i<children_count; i++)
        {
            if(child_pids[i] == pid)
            {
                exit_codes[i] = code;// store exit code
                break;
            }
        }
        ++finished;
    }
    printf("parent[%d]: No more children remaining. List of all exit codes:\n", (int)getpid());
    for(int i=0; i<children_count; i++)
    {
        printf("PID: %d Exit Code: %d\n", child_pids[i], exit_codes[i]);
    }

    #ifdef WITH_SIGNALS
    // restore signal handlers
    for(int i=1; i<NSIG; i++)
        signal(i, SIG_DFL);
    #endif
    return 0;
}

#ifdef WITH_SIGNALS
void interrupt_handler()
{
    printf("parent[%d]: Received keyboard interrupt\n", (int)getpid());
    interrupted = 1;
}

void terminate_handler()
{
    printf("child[%d]: Received SIGTERM signal.\n", (int)getpid());
}
#endif
