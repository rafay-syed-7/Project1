#include "thread.h"
#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>


using namespace std;

struct Requester {
    int id;
    ifstream file; //ensures each disk is only reading from its own track 
};

//Global shared state
unordered_map<int, int> diskQueue;   //requester id and track in a hashtable
int num_alive_requesters = 0; //threads alive
int currentDiskPos = 0; // disk head position
int max_disk_queue = 0; // will be passed in as first argument in command line 
vector<Requester> requesters; //not used after createThread, requester threads are used in DiskQueue 


//locks and CVs
const unsigned int queue_lock = 1;
const unsigned int queue_not_full = 2; //requesters 
const unsigned int queue_full_enough = 3; //servicers
const unsigned int request_serviced = 100; //each requester gets an offset (r id + 100), so know what specific requester to wake


void Request(void* arg) {
    Requester* r = (Requester*) arg; 
    int track; 

    while(r -> file >> track) {
        thread_lock(queue_lock);

        //if queue is full wait to request
        while(diskQueue.size() >= min(max_disk_queue, num_alive_requesters)) { // might have to change conditional logic here
            thread_wait(queue_lock, queue_not_full);
        }

        //now woken up so out of the loop and acquires lock automatically

        //find the track to service
        diskQueue[r-> id] = track; // this is a dictionary

        //print requester statement
        cout << "requester " << r -> id << " track " << track << endl;

        //signal Servicer
        thread_signal(queue_lock, queue_full_enough);

        //once call servicer need to wait for it to be serviced
        thread_wait(queue_lock, r->id + request_serviced);

        //now can unlock thread
        thread_unlock(queue_lock);
    }

    thread_lock(queue_lock);
    //requester thread is no longer alive
    num_alive_requesters--;
    //signal servicer it can look at other requesters 
    thread_signal(queue_lock, queue_full_enough);
    thread_unlock(queue_lock);

}
void Service(void* arg) {
     while(!diskQueue.empty() || num_alive_requesters > 0) { // added this in case servicer gets called first

        thread_lock(queue_lock);

        //if queue is not full wait to service
        while(diskQueue.size() < min(max_disk_queue, num_alive_requesters) && num_alive_requesters > 0) { // might have to change conditional logic here
            thread_wait(queue_lock, queue_full_enough);
        }

        //need an if statement because disk_queue could be empty this iteration of while loop
        if(!diskQueue.empty()) {
            int closestYet = 100000; //arbitrary large number
            int closestYetID = 0;
            for (auto &entry: diskQueue) {
                //Potential issue: SSTF logic 
                if (abs(entry.second - currentDiskPos) < abs(closestYet - currentDiskPos)) {
                    closestYet = entry.second;
                    closestYetID = entry.first;
                }
            }
            //potential issue: order of the updates and whether or not inside our outside of loops
            cout << "service requester " << closestYetID << " track " << closestYet << endl;
            currentDiskPos = closestYet;
            diskQueue.erase(closestYetID);
            thread_signal(queue_lock, closestYetID + request_serviced);
            thread_signal(queue_lock, queue_not_full); 
        }
        thread_unlock(queue_lock);
    }  
}

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

// void Request(void* arg) {
//     //Will need to cast the pointer back so the thread knows its own requester id and file
//     //This is because ThreadCreate requires void*
//     /*
//     acquire lock
//     for (line in r->file) {
//         add line to requestsQueue;
//         wait until line serviced // release lock here
//     }  
//     release lock
//     */
// }



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
        Requester r;
        r.id = i - 2; //first disc will start at 0
        r.file.open(argv[i]);
        if(!r.file.is_open()) {
            exit(1);
        }

        requesters.push_back(move(r)); //pushes the file into the vector 
    }

    //cout << num_alive_requesters << endl;

    //create thread system
    if (thread_libinit((thread_startfunc_t) createThreads, nullptr)) {
        return 1;
    }
}