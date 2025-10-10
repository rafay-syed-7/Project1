#include "thread.h"
#include <iostream>
#include <cassert>

using namespace std;

//unlocks without holding the lock

unsigned int LOCK_ID = 1;

void child(void *arg) {
    cout << "Child thread starting...\n";

    // Try to unlock a lock that this thread never locked
    int result = thread_unlock(LOCK_ID);
    if (result == -1) {
        cout << "✅ Correctly failed to unlock a lock not held.\n";
    } else {
        cout << "❌ ERROR: Was able to unlock without holding lock!\n";
    }
}

int main() {
    cout << "Testing unlock without owning the lock...\n";
    assert(thread_libinit((thread_startfunc_t) child, nullptr) == 0);
}
