#include "thread.h"
#include <iostream>
#include <cstdint>

using namespace std;

int main() {
    // Start the threading library with parent as the initial thread
    thread_libinit(nullptr, nullptr);
    cout << "this line should not be printed" <<endl;
}