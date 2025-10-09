#include "thread.h"
#include <iostream>

using namespace std;

// SIGNAL BEFORE WAIT

void signaler(void *arg) {
    if (thread_lock(1)) exit(1);
    cout << "Signaling before anyone waits...\n";
    thread_signal(1, 2); // should be safe
    thread_unlock(1);
}

int main() {
    thread_libinit((thread_startfunc_t) signaler, nullptr);
}
