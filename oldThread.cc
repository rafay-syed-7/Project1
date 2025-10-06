#include "thread.h"
#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <ucontext.h>

using namespace std;

struct TCB {
    int thread_id;
    ucontext_t* context;
    char* stack;
    enum State {READY, RUNNING, BLOCKED, FINISHED} state;
};

queue<TCB*> readyQ;
queue<TCB*> waitingForMonitorLock;
queue<TCB*> waitingForSignal;

ucontext_t scheduler_thread;



void schedule() {
    /*
    

    deallocate the thread that just came in
    if readyQ == empty --> exit(0) and print(thread library finished)
    else swapcontext to next thread on readyQ

    */
}


/*
    thread_libinit initializs the thread library.  A user program should call
    thread_libinit exactly once (before calling any other thread functions).
    thread_libinit creates and runs the first thread.  This first thread is
    initialized to call the function pointed to by func with the single
    argument arg.  Note that a successful call to thread_libinit will not
    return to the calling function.  Instead, control transfers to func, and
    the function that calls thread_libinit will never execute again.
*/

int thread_libinit(thread_startfunc_t func, void *arg) {
    // 1. needs test to see if this is first thread function called
    // 2. it also cant be called more than once

    ucontext_t* thread0_ptr = new ucontext_t;
    //error catching needs to be added

    
    //seperate thread -- scheduler context
    getcontext(scheduler_thread);
    char* stack_scheduler = new char[STACK_SIZE];
    scheduler_thread->uc_stack.ss_sp = stack_scheduler;
    scheduler_thread->uc_stack.ss_size = STACK_SIZE;
    scheduler_thread->uc_stack.ss_flags = 0;
    scheduler_thread->uc_link = NULL;
    makecontext(scheduler_thread, (void (*)()) schedule, 0);

    // setting up our first thread
    getcontext(thread0_ptr);
    char* stack_thread0 = new char[STACK_SIZE];
    thread0_ptr->uc_stack.ss_sp = stack_thread0;
    thread0_ptr->uc_stack.ss_size = STACK_SIZE;
    thread0_ptr->uc_stack.ss_flags = 0;
    thread0_ptr-> uc_link = scheduler_thread; //control goes back to the scheduler
    makecontext(thread0_ptr, (void(*) ()) func, 1, arg);
    //need to switch into the first thread: swap_context 

    setcontext(thread0_ptr);
    //missing interrupts


    // For future use when we forget all of this:
    // ucontext_t is the TCB?
    // We keep track of the threads through the 3 queues mentioned above
    // threadlibinit is unique in that it creates a thread and runs it. thread create creates a thread and adds it to readyQ
    // threadYield swaps the current thread with the next thread on readyQ
}
    
