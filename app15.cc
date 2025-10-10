#include "thread.h"
#include <iostream>
#include <cassert>
using namespace std;

//TEST: libinit called twice 

void dummy(void *arg) {
    cout << "First initialization ran successfully.\n";
}

int main() {
    int a = thread_libinit(dummy, nullptr);
    int b = thread_libinit(nullptr, nullptr);
    cout << a << endl;
    cout << b << endl; 
}