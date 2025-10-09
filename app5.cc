#include "thread.h"
#include <iostream>
#include <cassert>

// UNLOCK WITHOUT OWNING LOCK

using namespace std;

void other(void *arg) {
    if (thread_unlock(1) != -1) {
        cout << "ERROR: Unlocked lock that I never held\n";
    } else {
        cout << "Correctly failed to unlock unheld lock\n";
    }
}

void main_thread(void *arg) {
    thread_lock(1); // Main thread acquires lock
    thread_create((thread_startfunc_t) other, nullptr);
    thread_yield(); // Let other thread try to unlock
    thread_unlock(1);
}

int main() {
    thread_libinit((thread_startfunc_t) main_thread, nullptr);
}
