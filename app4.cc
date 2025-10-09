#include "thread.h"
#include <iostream>
#include <cassert>

using namespace std;

// LOCK TWICE 

void child(void *arg) {
    // First lock should succeed
    assert(thread_lock(1) == 0);
    cout << "Locked once\n";

    // Second lock on same lock should fail (reentrant lock not allowed)
    if (thread_lock(1) != -1) {
        cout << "ERROR: Was able to lock same lock twice!\n";
    } else {
        cout << "Correctly failed to lock same lock twice\n";
    }

    thread_unlock(1);
}

int main() {
    thread_libinit((thread_startfunc_t) child, nullptr);
}
