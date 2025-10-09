#include "thread.h"
#include <iostream>

using namespace std;

// TOO MANY THREADS (checking memory)

void noop(void *arg) {
    while (true) thread_yield(); // never exits
}

void creator(void *arg) {
    int created = 0;
    while (true) {
        if (thread_create(noop, nullptr) != 0) {
            cout << "Created " << created << " threads before failure.\n";
            break;
        }
        created++;
    }
}

int main() {
    thread_libinit((thread_startfunc_t) creator, nullptr);
}
