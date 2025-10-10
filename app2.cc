#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

// CHECKS LOCKS WORK

unsigned int LOCK_ID = 1;  // simple lock identifier

// Shared counter
int shared_counter = 0;

// Worker thread function
void worker(void *arg) {
    long id = (long) arg;

    for (int i = 0; i < 3; i++) {
        // Try to acquire lock
        if (thread_lock(LOCK_ID)) {
            cout << "Thread " << id << " failed to acquire lock\n";
            exit(1);
        }

        // Critical section
        cout << "Thread " << id << " entering critical section (iteration " << i << ")\n";
        int temp = shared_counter;
        thread_yield();  // yield inside lock 
        shared_counter = temp + 1;
        cout << "Thread " << id << " leaving critical section, counter = " << shared_counter << "\n";

        // Unlock
        if (thread_unlock(LOCK_ID)) {
            cout << "Thread " << id << " failed to unlock\n";
            exit(1);
        }

        // Yield outside of lock to let others run
        thread_yield();
    }

    cout << "Thread " << id << " finished.\n";
}

// Parent thread function
void parent(void *arg) {
    cout << "Parent thread starting.\n";

    // Create a few worker threads
    for (long i = 1; i <= 3; i++) {
        if (thread_create((thread_startfunc_t) worker, (void*) i)) {
            cout << "thread_create failed for thread " << i << endl;
            exit(1);
        }
    }

    // Parent also participates in work
    worker((void*) 0);

    cout << "Parent thread done.\n";
}

int main() {
    cout << "Starting lock/unlock test...\n";
    if (thread_libinit((thread_startfunc_t) parent, (void*) 0)) {
        cout << "thread_libinit failed\n";
        exit(1);
    }
}
