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
int max_disk_queue = 0;

//locks and CVs
const unsigned int queue_lock = 1;
const unsigned int queue_not_full = 2; //requesters 
const unsigned int queue_full_enough = 3; //servicers
const unsigned int request_serviced = 100; //each requester gets an offset (r id + 100)

struct Requester {
    int id;
    std::ifstream file; //ensures each disk is only reading from its own track 
};

std::vector<Requester> requesters;

// void Request(void* arg) {

// }

// void Service(void* arg) {

// }

// void createThread(void* arg) {

// }

int main(int argc, char* argv[]) {
    //ensures proper amount of command line arguments
    if(argc < 3) {
        return 0;
    }

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

        requesters.push_back(std::move(r)); //pushes the file into the vector 
    }

    num_alive_requesters = requesters.size();
    cout << num_alive_requesters;
}