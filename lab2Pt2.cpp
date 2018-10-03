using namespace std;

#include "tracelib.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

mutex m;
bool on_your_marks[1001];
int DOOR = 0;

void through_door(int id){
        //cout << "Fan #" << id << " has entered." << endl;
        trace::trace_event_start("Method2Incr","fuckshit", id+1);
        DOOR++;
        trace::trace_event_end(id+1);
        on_your_marks[id] = true;
}

void wait_you_dumb_fans(int i){
    while(!(on_your_marks[i])){
        this_thread::sleep_for(chrono::nanoseconds(1));
    }
    through_door(i+1);
}

int main(){
  trace::trace_start("Lab2Pt2.json");
  trace::trace_event_start("Method2init", "shit");
    thread threads[1000];
    for(int i = 0; i < 1000; i++){
        on_your_marks[i] = false;
    }
    for(int i = 0; i < 1000; i++){
        threads[i] = thread(wait_you_dumb_fans, i);
    }
    trace::trace_event_end();
    trace::trace_event_start("Method2","extrashit");
    on_your_marks[0] = true; //Initial case to get it all going

    for (auto& th : threads) th.join();
    trace::trace_event_end();
    cout << endl << "All fans have entered the building" << endl;
    trace::trace_end();

    return 0;

}
