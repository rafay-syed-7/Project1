#include "thread.h"
#include <iostream>
#include <cassert>

using namespace std;

unsigned int LOCK_ID = 1;
unsigned int COND_ID = 99;

//signaling and broadcasting before waiting so empty cv queue

void worker(void *arg) {
    cout << "Worker started, acquiring lock...\n";
    assert(thread_lock(LOCK_ID) == 0);
    cout << "Worker acquired lock\n";

    // No one is waiting on COND_ID yet
    cout << "Worker signaling empty condition variable\n";
    assert(thread_signal(LOCK_ID, COND_ID) == 0);

    cout << "Worker broadcasting empty condition variable\n";
    assert(thread_broadcast(LOCK_ID, COND_ID) == 0);

    cout << "Worker done with empty signal/broadcast test\n";

    assert(thread_unlock(LOCK_ID) == 0);
}

int main() {
    thread_libinit((thread_startfunc_t) worker, nullptr);
}
