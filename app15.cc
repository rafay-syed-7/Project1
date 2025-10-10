#include "thread.h"
#include <iostream>
#include <cassert>
using namespace std;

//TEST: libinit called twice 

void dummy(void *arg) {
    cout << "First initialization ran successfully.\n";
}

int main() {
    cout << "Testing double thread_libinit...\n";

    // First initialization — should succeed
    assert(thread_libinit((thread_startfunc_t) dummy, nullptr) == 0);

    // This line should never execute (first libinit takes over the thread system)
    // But if we hypothetically reached it, we could test double init
    int result = thread_libinit((thread_startfunc_t) dummy, nullptr);
    if (result == -1) {
        cout << "✅ Correctly returned -1 on second thread_libinit call.\n";
    } else {
        cout << "❌ ERROR: thread_libinit allowed reinitialization!\n";
    }
}