#include "thread.h"
#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <fstream>

using namespace std;

//Global shared state
std::unordered_map<int, int> diskQueue;   //requester id and track
int num_alive_requesters = 0; //threads alive
int currentDiskPos = 0; //disk head position
int max_disk_queue = 0; // will be passed in as first argument in command line 


//locks and CVs
const unsigned int queue_lock = 1;
const unsigned int queue_not_full = 2; //requesters 
const unsigned int queue_full_enough = 3; //servicers
const unsigned int request_serviced = 100; //each requester gets an offset (r id + 100)

struct Requester {
    int id;
    std::ifstream file; //ensures each disk is only reading from its own track 
};

// Global variables
std::vector<Requester> requesters;
std::queue<int> requestsQueue;

void createThreads(void* arg) {
    num_alive_requesters = requesters.size(); // seems problematic

    //create a thread per requester
    for(auto &r: requesters) {
        //checks to see if the thread is actually created
        thread_create((thread_startfunc_t) Request, &r);
    }
    
    thread_create((thread_startfunc_t) Service, nullptr);

}
    // Old implementation (commeneted out bc i don't think that is how thread_create works)
        //     if(thread_create(thread_startfunc_t) Request, &r) {
    //         exit(1);
    //     }
    // }

    // if(thread_create(thread_startfunc_t), Service, nullptr) {
    //     exit(1);
//}

void Request(void* arg) {
    //Will need to cast the pointer back so the thread knows its own requester id and file
    //This is because ThreadCreate requires void*
    Requester* r = (Requester*) arg;

    /*
    lock
    for (line in r->file) {
        add line to requestsQueue;
        ...
    }  
    unlock
    */
}

void Service(void* arg) {

}

int main(int argc, char* argv[]) {
    //ensures proper amount of command line arguments
    if(argc < 3) {
        return 0;
    }

    //TO DO: ADD MORE COMMAND LINE EDGE CASES TO CHECK

    max_disk_queue = atoi(argv[1]);


    //build the requesters
    //output is a vector of each requester id and the open file
    for(int i = 2; i < argc; i++) {
        Requester r; // made this ri so that the name of the Requester would change
        r.id = i - 2; //first disc will start at 0
        r.file.open(argv[i]);
        if(!r.file.is_open()) {
            exit(1);
        }

        requesters.push_back(std::move(r)); //pushes the file into the vector 
    }

    //cout << num_alive_requesters << endl;

    //Start creating thread system
    if (thread_libinit((thread_startfunc_t) createThreads, nullptr)) {
        return 1;
    }
}