using namespace std;

#include "tracelib.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdio>
#include <ctime>

mutex m;
bool on_your_marks = false;
int DOOR = 0;

void through_door(int id){
    m.lock();
    trace::trace_event_start("Method1Incr","fuckshit", id);
        //cout << "Fan #" << id << " has entered." << endl;
        DOOR++;
        trace::trace_event_end(id);
    m.unlock();
}

void wait_you_dumb_fans(int id){ //acting as a main for threads
    while(!(on_your_marks)){
        this_thread::sleep_for(chrono::nanoseconds(1));
    }
    through_door(id);
}

int main(){
  trace::trace_start("Lab2Pt1.json");
  trace::trace_event_start("Method1init", "shit");
    thread threads[1000];

    for(int i = 0; i < 1000; i++){
        threads[i] = thread(wait_you_dumb_fans, i+2); //what i call here is the main func. for the thread
    }
    trace::trace_event_end();
    trace::trace_event_start("Method1","extrashit");
    on_your_marks = true;

    for (auto& th : threads) th.join();
    trace::trace_event_end();
    cout << endl << "All fans have entered the building" << endl;
    trace::trace_end();

    return 0;

}
