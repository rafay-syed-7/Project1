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

ucontext_t* current_thread;
queue<ucontext_t*> readyQ;
// blockedQ implementations

struct lock {
    // blockedQ
    // map CV -> blockedQ
    // bool free
    ucontext_t* cuurent_holder;
};

bool libinitCalledBefore = 0;

// this looks good
int thread_libinit(thread_startfunc_t func, void *arg) {
    
    if (!libinitCalledBefore) {

        libinitCalledBefore = 1;

        ucontext_t* thread0_ptr = new ucontext_t;
        getcontext(thread0_ptr);
        char* stack_thread0 = new char[STACK_SIZE];
        thread0_ptr->uc_stack.ss_sp = stack_thread0;
        thread0_ptr->uc_stack.ss_size = STACK_SIZE;
        thread0_ptr->uc_stack.ss_flags = 0;
        thread0_ptr-> uc_link = NULL;

        makecontext(thread0_ptr, (void(*) ()) thread_start, 2, func, arg);

        setcontext(thread0_ptr);

    } else {
        cout << "threadlibinit_failed\n" << endl;
        exit(1);
    }


}

int thread_create(thread_startfunc_t func, void *arg) {

    
}
int thread_yield(void);
int thread_lock(unsigned int lock);
int thread_unlock(unsigned int lock);
void run_next_ready();
void thread_start(thread_startfunc_t func, void *arg);
/*

global variable current_thread
global variable readyQ
global variable blockedQ 
global variable locks(ID) // maybe an array or something? or prob map lock ->  <ID, Queue> and then <lock, CV>, queue

//lock struct to hold blocked queuees 

struct lock {

    blockedQ
    map(CV to BlockQ) //unique matches will have there own queue

}


// do the internal enable interrupts disable interrupts trick for thread_create(), thread_lock(), thread_unlock()

function thread_libinit {
    creates first thread
    runs it (setcontext)
    Makes sure it can only get called once
}


function thread_create(func, arg) { // the way she does it is 1. disable interrupts 2. call thread_create_internal 
    make ucontext
        getcontext
        makecontext(thread_start) // *********
        stack space (SP) // USE NEW (which is the c++ equivalent of malloc)

    Push thread onto ready queue
}

function thread_start(func, arg) {
    enable interrupts here
    void* (func)(arg);
    disable interrupts here
    clean up threads and stuff
}

function run_next_ready {
    next thread = pop readyQ
    current = next
    swapcontext

}

function yield() {
    push current_thread to readyQ
    run_next_ready()
}

function lock(int ID) {
    check if lock is held
        if it is free
            mark it as not free
            return
        else
            add current_thread to BlockedQ
            run_next_ready
}

function unlock(int ID) {
    make sure current_thread has lock
    mark lock as free
    if threads are on blockedQ
        pop front
        give lock to popped thread
        add to readyQ
}


*/