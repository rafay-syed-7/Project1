#include "thread.h"
#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <ucontext.h>

/*
    thread_libinit initializes the thread library.  A user program should call
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

    getcontext(thread0_ptr);

    char* stack = new char[STACK_SIZE];
    thread0_ptr->uc_stack.ss_sp = stack;


}
    
