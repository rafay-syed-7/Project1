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

ucontext_t* delete_thread = nullptr;
ucontext_t* current_thread = nullptr;
queue<ucontext_t*> readyQ;

struct Lock {
    unordered_map<int, queue<ucontext_t*> > CVtoBlockedQ;
    queue<ucontext_t*> blockedThreads;
    bool free;
    int id;
    ucontext_t* current_holder;
};

unordered_map<unsigned int, Lock*> lockDictionary;


bool libinitCalledBefore = 0;

void run_next_ready() {
    assert_interrupts_disabled();
    //check if ready queue is empty
    if(readyQ.empty()) {
        //no threads runnable remain
        cout << "Thread library exiting.\n";
        exit(0);
    }

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
    assert_interrupts_disabled();
    //if lock exists
    if (lockDictionary.find(lock) != lockDictionary.end()) {
        Lock* currentLock = lockDictionary[lock];
        //if lock free
        if(currentLock->free) {
            currentLock->free = false;
            currentLock->current_holder = current_thread;
            return 0;
        //if lock not free
        } else if (currentLock->current_holder == current_thread) { // if this thread alr has lock and tries to lock again
            return -1;
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
    assert_interrupts_disabled();
    if (lockDictionary.find(lock) == lockDictionary.end()) {
        return -1;
    } 
    
    Lock* currentLock = lockDictionary[lock];
    
    if (currentLock->current_holder != current_thread) {
        return -1;
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
    return -1;
}

int thread_unlock(unsigned int lock) {
    // ensuring libinit is already called, otherwise exit
    if(!libinitCalledBefore) {
        return -1;
    }  
    interrupt_disable();
    //debug_interrupts_disable(__FUNCTION__);

    int result = thread_unlock_internal(lock);
    interrupt_enable();
    //debug_interrupts_enable(__FUNCTION__);
    return result;
}


void thread_start(thread_startfunc_t func, void *arg) {

    interrupt_enable();

    func(arg); // might have a type cast issue might need void *

    interrupt_disable();

    if(delete_thread != nullptr) {
        delete[](char*) delete_thread->uc_stack.ss_sp; //free stack
        delete delete_thread; //free ucontext
    }

    delete_thread = current_thread; //mark this thread to be deleted by the next one
    run_next_ready(); //swap to the next thread
}

int thread_libinit(thread_startfunc_t func, void *arg) {
    interrupt_disable();
    if(func == nullptr) return -1;

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
        return 0;
    }

    cout << "threadlibinit_failed\n" << endl;
    return -1;    
}

int thread_create_internal(thread_startfunc_t func, void *arg) {
    assert_interrupts_disabled();

    try {
        //allocate ucontext
        ucontext_t* new_thread = new(nothrow) ucontext_t;

        //if fail to allocate context
        if(new_thread == nullptr) {
            return -1;
        }

        //failed to initialize
        if(getcontext(new_thread) == -1) {
            delete new_thread;
            return -1;
        }

        //Allocate
        char* stack_new_thread = new char[STACK_SIZE];

        //if stack space not allocated
        if(stack_new_thread == nullptr) {
            delete new_thread; //cleanup the partial allocation 
            return -1;
        }

        new_thread->uc_stack.ss_sp = stack_new_thread;
        new_thread->uc_stack.ss_size = STACK_SIZE;
        new_thread->uc_stack.ss_flags = 0;
        new_thread->uc_link = NULL;

        makecontext(new_thread, (void(*)()) thread_start, 2, func, arg);
    
        //push thread to readyQ
        readyQ.push(new_thread);
        return 0;
    }   
    catch (bad_alloc&){
        return -1;
    }
    //all other input
    catch (...) {
        //generic catch all
        return -1;
    }

    // ucontext_t* new_thread = new(nothrow) ucontext_t;
    // if(new_thread == nullptr) return -1;

    // getcontext(new_thread);

    // char* stack_new_thread = new(nothrow) char[STACK_SIZE]; // trying to catch if we're creating too many threads
    // if (stack_new_thread == nullptr) {
    //     delete new_thread;
    //     return -1;
    // }

    // //error checking -- if ran out of memory
    // if(stack_new_thread == nullptr) return -1;

    // new_thread->uc_stack.ss_sp = stack_new_thread;
    // new_thread->uc_stack.ss_size = STACK_SIZE;
    // new_thread->uc_stack.ss_flags = 0;
    // new_thread->uc_link = NULL;

    // makecontext(new_thread, (void(*)()) thread_start, 2, func, arg);
    
    // //push thread to readyQ
    // readyQ.push(new_thread);
    // return 0;
}

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

int thread_yield(void) {
    if(!libinitCalledBefore) {
        return -1;
    }  
    interrupt_disable();
   
    readyQ.push(current_thread);
    run_next_ready();
    interrupt_enable();

    return 0;
}

int thread_wait_internal(unsigned int lock, unsigned int cond) {
    assert_interrupts_disabled();

    //checks to see if lock exists and that calling owns this lock
    if (lockDictionary.find(lock) != lockDictionary.end() && 
        lockDictionary.find(lock)->second->current_holder == current_thread) {
        Lock* currentLock = lockDictionary[lock];

        currentLock->CVtoBlockedQ[cond].push(current_thread);
        thread_unlock_internal(lock);
        run_next_ready();                   // these 3 lines are risquè
        thread_lock_internal(lock);
        return 0;
    }
    return -1;
}

int thread_wait(unsigned int lock, unsigned int cond) {
    if(!libinitCalledBefore) {
        return -1;
    }  
    interrupt_disable();
    int result = thread_wait_internal(lock, cond);
    interrupt_enable();
    return result;
}

int thread_signal_internal(unsigned int lock, unsigned int cond) {
    assert_interrupts_disabled();

    //checks to see if lock exists and that calling owns this lock
    if (lockDictionary.find(lock) != lockDictionary.end()) {
        // check to see if this CVtoBlockedQ Q actually exists (wait needs to have been called before signal)
        Lock* currentLock = lockDictionary[lock];
        auto checkCV = currentLock->CVtoBlockedQ.find(cond);
        
        //check if unordered map acc has CV as a key and that the queue is not empty
        if(checkCV != currentLock->CVtoBlockedQ.end() && !checkCV->second.empty()) {
            readyQ.push(checkCV->second.front());
            checkCV->second.pop();
            return 0;
        }
    }
    return 0;
}

int thread_signal(unsigned int lock, unsigned int cond) {
    if(!libinitCalledBefore) {
        return -1;
    } 
    interrupt_disable();
    int result = thread_signal_internal(lock, cond);
    interrupt_enable();
    return result;
}

int thread_broadcast_internal(unsigned int lock, unsigned int cond) {

    assert_interrupts_disabled();


    auto lockExist = lockDictionary.find(lock);
    //if lock doesnt exist
    if(lockExist == lockDictionary.end()) {
        return 0;
    }

    Lock* currentLock = lockExist->second;
    auto cvExist = currentLock->CVtoBlockedQ.find(cond);

    //if cv doesnt exist
    if(cvExist == currentLock->CVtoBlockedQ.end()) {
        return 0;
    }

    //keep signaling while the queue is not empty
    while(!cvExist->second.empty()) {
        thread_signal_internal(lock, cond);
    }

    return 0;
}

int thread_broadcast(unsigned int lock, unsigned int cond) {
    if(!libinitCalledBefore) {
        return -1;
    }
    interrupt_disable();
    int result = thread_broadcast_internal(lock, cond);
    interrupt_enable();
    return result;
}