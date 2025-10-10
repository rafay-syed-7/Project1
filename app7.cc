#include "thread.h"
#include <iostream>
#include <cstdint>

using namespace std;

// THIS IS NEW. testing handoff lock functionalit

static const unsigned int L = 1;

volatile int phase = 0;      // parent signals "about to unlock"
volatile int b_was_here = 0; // what we measure success on for the handoff by

void B_waiter(void *arg) {
    // Should block because parent holds L
    if (thread_lock(L) != 0) return;

    // If we have a handoff lock, we should get here immediately after parent unlocks
    b_was_here = 1;

    thread_unlock(L);
}

void C_competitor(void *arg) {
    // Let B run first and block on the lock
    thread_yield();

    // Wait until parent announces it's about to unlock
    while (phase == 0) {
        thread_yield();
    }

    // Try to acquire the lock right after parent unlocks
    if (thread_lock(L) == 0) {
        // If we beat B into the CS, the lock didn't hand off to the waiter
        if (!b_was_here) {
            cout << "BUG: unlock did not hand off to waiting thread (C acquired before B)\n";
        }
        thread_unlock(L);
    }
}

void parent(void *arg) {
    if (thread_lock(L) != 0) return;

    // Create waiter (B) and competitor (C)
    thread_create(B_waiter, nullptr);
    thread_create(C_competitor, nullptr);

    // Give B a chance to run and block on L; give C a chance to get ready
    thread_yield();
    thread_yield();

    // Announce we're about to unlock; C will start racing after this flips
    phase = 1;

    // Critical moment: with a correct handoff lock, B should immediately get the lock.
    // With a buggy "free-on-unlock", C may acquire before B.
    thread_unlock(L);

    // Parent does nothing else; no prints on success
}

int main() {
    thread_libinit(parent, nullptr);
}

