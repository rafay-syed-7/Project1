#include "thread.h"
#include <iostream>
#include <cassert>
using namespace std;

//TEST: libinit called twice 

void dummy(void *arg) {
    cout << "First initialization ran successfully.\n";
}

int main() {
    thread_libinit(dummy, nullptr);
    thread_libinit(nullptr, nullptr);

}