#include "thread.h"
#include <iostream>
#include <cstdint>

using namespace std;

constexpr int N_CHILDREN = 100000;

void child(void *arg) {
    // Do something trivial, then return
    volatile intptr_t x = reinterpret_cast<intptr_t>(arg);
    x += 1;
    (void)x; // silence unused warning
    // returning exits the thread in most thread libs using this API
}

void parent(void *arg) {
    for (int i = 0; i < N_CHILDREN; ++i) {
        // pass a small unique integer to each child
        void *payload = reinterpret_cast<void*>(static_cast<intptr_t>(i));
        if (thread_create(child, payload) != 0) {
            cout << "Created " << i << " threads before failure.\n";
            return;  // stop on failure
        }
    }
    // No printing on success; parent just returns
}

int main() {
    // Start the threading library with parent as the initial thread
    thread_libinit(parent, nullptr);
}
