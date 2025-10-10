#include "thread.h"
#include <iostream>
#include <cassert>

using namespace std;

//TEST wait without holding the lock

unsigned int LOCK_ID = 1;
unsigned int COND_ID = 42;

void child(void *arg) {
    cout << "Child thread starting...\n";

    // Attempt to wait on a condition variable without holding the lock
    int result = thread_wait(LOCK_ID, COND_ID);
    if (result == -1) {
        cout << "Correctly failed to wait without holding the lock.\n";
    } else {
        cout << "ERROR: Wait succeeded without holding the lock!\n";
    }
}

int main() {
    cout << "Testing wait without owning the lock...\n";
    assert(thread_libinit((thread_startfunc_t) child, nullptr) == 0);
}
