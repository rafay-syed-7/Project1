#include "thread.h"
#include <iostream>
#include <cassert>

using namespace std;

//check if functions are called before thread_libnit

unsigned int LOCK_ID = 1;
unsigned int COND_ID = 42;

int main() {
    cout << "Testing invalid calls before thread_libinit...\n";

    // None of these should work — the library is not initialized yet.
    assert(thread_lock(LOCK_ID) == -1);
    cout << "thread_lock correctly returned -1\n";

    assert(thread_unlock(LOCK_ID) == -1);
    cout << "thread_unlock correctly returned -1\n";

    assert(thread_signal(LOCK_ID, COND_ID) == -1);
    cout << "thread_signal correctly returned -1\n";

    assert(thread_broadcast(LOCK_ID, COND_ID) == -1);
    cout << "thread_broadcast correctly returned -1\n";

    assert(thread_wait(LOCK_ID, COND_ID) == -1);
    cout << "thread_wait correctly returned -1\n";

    assert(thread_yield() == -1);
    cout << "thread_yield correctly returned -1\n";

    assert(thread_create((thread_startfunc_t) nullptr, nullptr) == -1);
    cout << "thread_create correctly returned -1\n";

    cout << "All pre-libinit calls correctly failed.\n";
}
