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
    ucontext_t* current_holder;
};

bool libinitCalledBefore = 0;


void thread_start(thread_startfunc_t func, void *arg) {
    interrupt.enable();

    //run user func
    func(arg); 

    interrupt.disable();

    if(!readyQ.empty()) {
        //cleanup -- idk how that looks with when creating a delete thread and deleting that 
    }
    else {
        cout << "Thread library exiting.\n";
        exit(0);
    }

    //TO DO: Ask Jeannie about the delete thread (extra thread for cleanup, because you can't delete a current thread)
}

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
//internal function, so we don't have to deal with interrupts
int thread_create_internal(thread_startfunc_t fun, void *arg) {
    ucontext_t* new_thread = new u_context_t;
    getcontext(new_thread);
    char* stack_new_thread = new char[STACK_SIZE];

    //error checking -- if ran out of memory
    if(stack_new_thread == nullptr) return -1;

    new_thread->uc_stack.ss_sp = stack_new_thread;
    new_thread->uc_stack.ss_size = STACK_SIZE;
    new_thread->uc_stack.ss_flags = 0;
    new_thread->uc_link = NULL;

    makecontext(new_thread, (void(*)()) thread_start, 2, func, arg);
    
    //push thread to readyQ
    readyQ.push(new_thread);

    return 0;
}

//TO DO: check with Jeanie and thread_create_internal -- but looks good
int thread_create(thread_startfunc_t func, void *arg) {
    // ensuring libinit is already called, otherwise exit
    if(!libinitCalledBefore) {
        return -1;
    }  
    interrupt.disable();
    //call internal function
    int result = thread_create_internal(func, arg);
    interrupt.enable();
    return result;
}

void run_next_ready() {
    //Save current thread context
    ucontext_t* previous_thread = current_thread;

    //get the next thread from readyQ
    ucontext_t* next_thread = readyQ.front();
    readyQ.pop();

    //update current thread pointer
    current_thread = next_thread;

    //swap context, with error checking
    if(previous != nullptr) {
        swapcontext(previous_thread, next_thread);
    }
}

int thread_yield(void);
int thread_lock(unsigned int lock);
int thread_unlock(unsigned int lock);

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