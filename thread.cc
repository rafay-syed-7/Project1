#include "thread.h"
#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>
#include <ucontext.h>
#include <queue>
#include <string>
#include "interrupt.h"

using namespace std;

ucontext_t* delete_thread;
ucontext_t* current_thread;
queue<ucontext_t*> readyQ;

struct Lock {
    unordered_map< int, queue<ucontext_t*> > CVtoBlockedQ;
    queue<ucontext_t*> blockedThreads;
    bool free;
    int id;
    ucontext_t* current_holder;
};

unordered_map<unsigned int, Lock*> lockDictionary;


//locks(id) --> map(ID, lock)
//  lock struct
//  int id
//. queue of blocked ucontext _ts
//  boolean free (for a given lock is it free or not)
//. map(int cv, queue of blocked threads)


bool libinitCalledBefore = 0;

/*
function lock(int ID) {
    Lock* l //create instance 
    if (ID exists in lockDicitionary) {
        check if lock is held
        if it is free
            mark it as not free
            assign current_thread as Lock->current_holder
            return
        else
            add current_thread to BlockedQ
            run_next_ready
            return
    } else { // lock does not exist
        Lock newLock;
        newLock.CVtoBlockedQ = new unordered_map< int, queue<ucontext_t*> > 
        newLock.blockedThreads = new queue<ucontext_t*>;
        newLock.free = false;
        newLock.id = lock;
        newLock.current_holder = current_thread;
        lockDictionary[newLock.id] = &newLock;
        return;
    }

}
*/

void run_next_ready() {
    //Save current thread context
    ucontext_t* previous_thread = current_thread;

    //get the next thread from readyQ
    ucontext_t* next_thread = readyQ.front();
    readyQ.pop();

    //update current thread pointer
    current_thread = next_thread;
 
    //swap context, with error checking
    if(previous_thread != nullptr) {
        swapcontext(previous_thread, next_thread);
    }
}

int thread_lock_internal(unsigned int lock) {
    //if lock exists
    if (lockDictionary.find(lock) != lockDictionary.end()) {
        Lock* currentLock = lockDictionary[lock];
        //if lock free
        if(currentLock->free) {
            currentLock->free = false;
            currentLock->current_holder = current_thread;
            return 0;
        //if lock not free
        } else { 
            //add to blocked queue 
            currentLock->blockedThreads.push(current_thread);
            run_next_ready();
            return 0;
        }
    } else { // lock does not exist, first time lock has been called 
        Lock* newLock = new Lock();
        newLock->free = false;
        newLock->id = lock;
        newLock->current_holder = current_thread;
        lockDictionary[newLock->id] = newLock;
        return 0;
    }
}

int thread_lock(unsigned int lock) {
    // ensuring libinit is already called, otherwise exit
    if(!libinitCalledBefore) {
        return -1;
    }  
    interrupt_disable();
    int result = thread_lock_internal(lock);
    interrupt_enable();
    return result;
}

int thread_unlock_internal(unsigned int lock) {
    //error catching -- make sure lock actually exists 
    if (lockDictionary.find(lock) == lockDictionary.end()) {
        exit(1);
    } 
    
    Lock* currentLock = lockDictionary[lock];
    
    if (currentLock->current_holder != current_thread) {
        exit(1);
    }

    //if not free, unlock 
    if(!currentLock->free) {
        currentLock->free = true;
        
        //if blocked threads
        if(!currentLock->blockedThreads.empty()) {
            currentLock->current_holder = currentLock->blockedThreads.front();
            currentLock->blockedThreads.pop();
            currentLock->free = false;
            readyQ.push(currentLock->current_holder);
            return 0;
        } else {
            currentLock->current_holder = nullptr;
            return 0;
        }
    }
    exit(1);
}

int thread_unlock(unsigned int lock) {
    // ensuring libinit is already called, otherwise exit
    if(!libinitCalledBefore) {
        return -1;
    }  
    interrupt_disable();
    int result = thread_unlock_internal(lock);
    interrupt_enable();
    return result;
}

// function unlock(int ID) {
//     make sure current_thread has lock
//     mark lock as free
//     if threads are on blockedQ
//         pop front
//         give lock to popped thread
//         add to readyQ
// }


void thread_start(thread_startfunc_t func, void *arg) {
    //run user func
    func(arg); // might have a type cast issue might need void *

    interrupt_disable();

    
    if(!readyQ.empty()) {
        //cleanup -- idk how that looks with when creating a delete thread and deleting that 
        //check if its not null

        if(delete_thread != nullptr) {
            delete[](char*) delete_thread->uc_stack.ss_sp; //free stack
            delete delete_thread; //free ucontext
        }

        delete_thread = current_thread; //mark this thread to be deleted by the next one
        run_next_ready(); //swap to the next thread
    }
    else {
        cout << "Thread library exiting.\n"; // might need to deallocate the space of the current thread if readyQ empty
        exit(0);
    }
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
        current_thread = thread0_ptr; //new add
        setcontext(thread0_ptr);

    } else {
        cout << "threadlibinit_failed\n" << endl;
        exit(1);
    }

    return 0;
}

//internal function, so we don't have to deal with interrupts
int thread_create_internal(thread_startfunc_t func, void *arg) {
    ucontext_t* new_thread = new ucontext_t;

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
    interrupt_disable();
    int result = thread_create_internal(func, arg);
    interrupt_enable();
    return result;
}



//are we using interrupts for thread_yield?
int thread_yield(void) {
    interrupt_disable();
    readyQ.push(current_thread);
    run_next_ready();
    interrupt_enable();
    return 0;
}


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