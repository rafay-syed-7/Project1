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


bool libinitCalledBefore = 0;


// serving as a wrapper for threadCreate just to check if called before
int thread_libinit(thread_startfunc_t func, void *arg) {
    if (!libinitCalledBefore) {
        libinitCalledBefore = 1;
        // just call thread_create to the rest
        int thread_create(thread_startfunc_t func, void *arg);
    } else {
        cout << "threadlibinit_failed\n" << endl;
        exit(1);
    }

}

int thread_create(thread_startfunc_t func, void *arg) {

    ucontext_t* thread_ptr = new ucontext_t;
   
    getcontext(thread_ptr);
    char* stack_thread0 = new char[STACK_SIZE];
    thread_ptr->uc_stack.ss_sp = stack_thread0;
    thread_ptr->uc_stack.ss_size = STACK_SIZE;
    thread_ptr->uc_stack.ss_flags = 0;
    thread_ptr-> uc_link = NULL; //This thread just dies at the end of its life

    makecontext(thread_ptr, (void(*) ()) func, 1, arg); 

    setcontext(thread_ptr);
    
    // PROBLEM: doesnt come back to this function to call schedule()
    // SOLUTION: Check lecture slides for the thread_start() function (internal function 
    //           that gets called that allows us to do clean up)
}


