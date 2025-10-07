#include <iostream>
#include "thread.h"
#include <assert.h>

using namespace std;

void child(void *arg) {
    cout << "Child thread started with message: " << (char*) arg << endl;

    //yield to parent
    if(thread_yield()) {
        cout << "thread_yield failed\n";
        exit (1);
    }

    thread_yield();
}

void parent(void*arg) {
    cout << "Parent thread started.\n";

    if(thread_create((thread_startfunc_t) child, (void*) "hi from child")) {
        cout << "thread_create failed\n";
        exit(1);
    }

    cout << "Parent yielding to child.\n";

    //Yield to let child run
    if(thread_yield()) {
        cout << "thread_yield failed\n";
        exit(1);
    }

    cout << "Parent finishing.\n";
}

int main() {
    cout << "Starting test for thread _create...\n";

    if(thread_libinit((thread_startfunc_t) parent, nullptr)) {
        cout << "thread_libinit failed\n";
        exit(1);
    }
}

