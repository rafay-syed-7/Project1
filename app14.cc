#include "thread.h"
#include <iostream>
#include <cassert>

using namespace std;

//TEST signal without a lock (not an error)

unsigned int LOCK_ID = 1;
unsigned int COND_ID = 42;

void worker(void *arg) {
    cout << "Worker thread starting...\n";

    // This thread does NOT hold the lock
    int signal_result = thread_signal(LOCK_ID, COND_ID);
    if (signal_result == 0) {
        cout << "Correctly allowed signal without holding lock.\n";
    } else {
        cout << "ERROR: Signal without lock incorrectly returned -1.\n";
    }

    int broadcast_result = thread_broadcast(LOCK_ID, COND_ID);
    if (broadcast_result == 0) {
        cout << "Correctly allowed broadcast without holding lock.\n";
    } else {
        cout << "ERROR: Broadcast without lock incorrectly returned -1.\n";
    }
}

int main() {
    cout << "Testing signal and broadcast without owning lock...\n";
    assert(thread_libinit((thread_startfunc_t) worker, nullptr) == 0);
}
