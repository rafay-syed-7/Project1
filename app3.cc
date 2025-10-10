//testing wait, signal, broadcast
#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

#include <iostream>
#include "thread.h"
using namespace std;

unsigned int LOCK_ID = 1;
unsigned int COND_ID = 1;

int shared_state = 0;

// CHECKS MONITORS 

void waiter(void *arg) {
    long id = (long) arg;

    if (thread_lock(LOCK_ID)) {
        cout << "Waiter " << id << " failed to acquire lock\n";
        exit(1);
    }

    cout << "Waiter " << id << " acquired lock, going to wait\n";

    // Wait releases the lock and blocks until signaled
    if (thread_wait(LOCK_ID, COND_ID)) {
        cout << "Waiter " << id << " failed in wait\n";
        exit(1);
    }

    // Once woken up, waiter must reacquire lock before proceeding
    cout << "Waiter " << id << " woke up and reacquired lock, shared_state = " << shared_state << "\n";

    if (thread_unlock(LOCK_ID)) {
        cout << "Waiter " << id << " failed to unlock\n";
        exit(1);
    }
}

void signaler(void *arg) {
    if (thread_lock(LOCK_ID)) {
        cout << "Signaler failed to acquire lock\n";
        exit(1);
    }

    cout << "Signaler acquired lock, signaling one waiter...\n";
    shared_state = 1;

    if (thread_signal(LOCK_ID, COND_ID)) {
        cout << "thread_signal failed\n";
        exit(1);
    }

    if (thread_unlock(LOCK_ID)) {
        cout << "Signaler failed to unlock\n";
        exit(1);
    }
}

void broadcaster(void *arg) {
    if (thread_lock(LOCK_ID)) {
        cout << "Broadcaster failed to acquire lock\n";
        exit(1);
    }

    cout << "Broadcaster acquired lock, broadcasting to all waiters...\n";
    shared_state = 2;

    if (thread_broadcast(LOCK_ID, COND_ID)) {
        cout << "thread_broadcast failed\n";
        exit(1);
    }

    if (thread_unlock(LOCK_ID)) {
        cout << "Broadcaster failed to unlock\n";
        exit(1);
    }
}

void parent(void *arg) {
    cout << "Parent starting test...\n";

    // Create 3 waiting threads
    for (long i = 1; i <= 3; i++) {
        if (thread_create((thread_startfunc_t) waiter, (void*) i)) {
            cout << "thread_create failed for waiter " << i << endl;
            exit(1);
        }
    }

    // Give waiters a chance to start and block
    thread_yield();

    // Create signaler thread (wakes one waiter)
    if (thread_create((thread_startfunc_t) signaler, (void*) 0)) {
        cout << "thread_create failed for signaler" << endl;
        exit(1);
    }

    thread_yield();

    // Create broadcaster thread (wakes all waiters)
    if (thread_create((thread_startfunc_t) broadcaster, (void*) 0)) {
        cout << "thread_create failed for broadcaster" << endl;
        exit(1);
    }

    thread_yield();

    cout << "Parent done.\n";
}

int main() {
    if (thread_libinit((thread_startfunc_t) parent, (void*) 0)) {
        cout << "thread_libinit failed\n";
        exit(1);
    }
}
