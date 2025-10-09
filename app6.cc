#include "thread.h"
#include <iostream>

using namespace std;

// WAIT WITHOUT LOCK

void bad_waiter(void *arg) {
    // Try to wait on cond var without holding lock
    if (thread_wait(1, 1) != -1) {
        cout << "ERROR: Wait succeeded without lock\n";
    } else {
        cout << "Correctly failed wait without lock\n";
    }
}

int main() {
    thread_libinit((thread_startfunc_t) bad_waiter, nullptr);
}
