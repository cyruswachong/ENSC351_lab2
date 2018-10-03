/*
    A library of functions to produce a trace JSON file describing the events that
    happened, to be used with another program.

    Current Functions:

    trace_start
    trace_flush
    trace_end
    trace_event_start
    trace_event_end
    trace_object_new
    trace_object_gone
    trace_instant_global
    trace_counter
*/
#ifndef TRACELIB_H_INCLUDED
#define TRACELIB_H_INCLUDED

#include <vector>
#include <cstring>
#include <fstream>
#include <iostream>
#include <chrono>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace trace
{
using Clock=std::chrono::high_resolution_clock;

//Constants and Statics
const int TRACE_MAX = 10000;
static bool traceActive = false;
static std::vector<std::string> dataVector;
static int PID_VALUE = 1; //For now, always 1
static int TID_VALUE = 1; //For now, always 1
static std::ofstream traceFile;
static char stringBuffer[1000];
static bool clockInit = false;
static std::chrono::high_resolution_clock::time_point startTime;

/*
    bool trace_start(filename)

    Starts the trace procedure. This includes opening the file (only written to on closing
    or exceeding the memory however), and allocating a vector to contain the data.

    Output is true if successful, false otherwise.
*/
inline bool trace_start(const char* filename)
{
    traceFile.open(filename);
    if(traceFile.is_open())
    {
        dataVector.reserve(TRACE_MAX);
        traceFile << "[\n"; //Opening brace of JSON
    }
    else
    {
        std::cerr << "Error: Unable to open file \"" << filename << "\" for trace output.\n";
        return 0;
    }
    //Clock should only be initialized once for all threads
    if(!clockInit)
    {
        startTime = Clock::now();
        clockInit = true;
    }
    traceActive = true;
    return 1;
}

/*
    void trace_flush()

    Flush the dataVector; that is, dump it to the file, and empty the array.
*/
inline void trace_flush()
{
    for(auto const& value : dataVector)
    {
        traceFile << value;
    }
    dataVector.clear(); //Memory is not reallocated on clear
}

/*
    void trace_end()

    Flush the output and close the traceFile. Also do closing details (closing bracket, remove extra comma).
*/
inline void trace_end()
{
    dataVector.back().pop_back(); dataVector.back().pop_back(); //Remove last two elements (",\n")
    trace_flush();
    traceFile << "\n]"; //Closing Brace of JSON
    traceFile.close();
    traceActive = false;
}

/*
    void trace_event_start(name, categories)

    Pushes a line to the dataVector to start an event (i.e. "ph" = "B"). Inputs are self explanatory
*/
inline void trace_event_start(const char* name, const char* categories, const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    sprintf(stringBuffer,
    "{\"name\": \"%s\", \"cat\": \"%s\", \"ph\": \"B\", \"pid\": %i, \"tid\": %i, \"ts\": %i},\n",
    name,   categories, PID_VALUE,  tid,  int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );

    dataVector.push_back(stringBuffer);
}

/*
    void trace_event_start(name, categories, argumentNames, argumentValues)

    Same as above, but takes arguments
*/
inline void trace_event_start(const char* name, const char* categories, std::initializer_list<const char*> argumentNames, std::initializer_list<const char*> argumentValues, const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    if(argumentNames.size() != argumentValues.size()) //Lists have different sizes
    {
        std::cerr << "Error: Argument lists for " << name << " in trace_event_start are not the same size; ignoring them.\n";
        trace_event_start(name, categories);
    }
    else
    {
        sprintf(stringBuffer,
        "{\"name\": \"%s\", \"cat\": \"%s\", \"ph\": \"B\", \"pid\": %i, \"tid\": %i, \"ts\": %i, \"args\": { ",
        name,   categories, PID_VALUE,  tid,  int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );
        auto itNames  = argumentNames.begin();
        auto itValues = argumentValues.begin();
        for(size_t i=0; i<argumentNames.size(); i++)
        {
            sprintf(stringBuffer + strlen(stringBuffer),
            "\"%s\": %s",
            *itNames++,*itValues++);

            if(i!=argumentNames.size()-1) sprintf(stringBuffer + strlen(stringBuffer),", "); //add comma if not last variable
        }
        sprintf(stringBuffer + strlen(stringBuffer),"} },\n");
        dataVector.push_back(stringBuffer);
    }
}

/*
    void trace_event_end()

    Pushes a line to the dataVector to end an event (i.e. "ph" = "E").
*/
inline void trace_event_end(const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    sprintf(stringBuffer,
    "{\"ph\": \"E\", \"pid\": %i, \"tid\": %i, \"ts\": %i},\n",
    PID_VALUE,  tid,  int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );

    dataVector.push_back(stringBuffer);
}

/*
    void trace_event_end(argumentNames, argumentValues)

    Same as above, but takes arguments
*/
inline void trace_event_end(std::initializer_list<const char*> argumentNames, std::initializer_list<const char*> argumentValues, const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    if(argumentNames.size() != argumentValues.size()) //Lists have different sizes
    {
        std::cerr << "Error: Argument lists in trace_event_end are not the same size; ignoring them.\n";
        trace_event_end();
    }
    else
    {
        sprintf(stringBuffer,
        "{\"ph\": \"E\", \"pid\": %i, \"tid\": %i, \"ts\": %i, \"args\": { ",
        PID_VALUE,  tid,  int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );
        auto itNames  = argumentNames.begin();
        auto itValues = argumentValues.begin();
        for(size_t i=0; i<argumentNames.size(); i++)
        {
            sprintf(stringBuffer + strlen(stringBuffer),
            "\"%s\": %s",
            *itNames++,*itValues++);

            if(i!=argumentNames.size()-1) sprintf(stringBuffer + strlen(stringBuffer),", "); //add comma if not last variable
        }
        sprintf(stringBuffer + strlen(stringBuffer),"} },\n");
        dataVector.push_back(stringBuffer);
    }
}

/*
    void trace_object_new(name, obj_pointer)

    Pushes a line to the dataVector to create an object (i.e. "ph" = "N")
*/
inline void trace_object_new(const char* name, const void* obj_pointer, const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    sprintf(stringBuffer,
    "{\"name\": \"%s\", \"ph\": \"N\", \"pid\": %i, \"tid\": %i, \"id\": %" PRIuPTR ", \"ts\": %i},\n",
    name, PID_VALUE,  tid, (uintptr_t)obj_pointer, int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );

    dataVector.push_back(stringBuffer);
}

/*
    void trace_object_gone(name, obj_pointer)

    Pushes a line to the dataVector to destroy an object (i.e. "ph" = "D")
*/
inline void trace_object_gone(const char* name, const void* obj_pointer, const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    sprintf(stringBuffer,
    "{\"name\": \"%s\", \"ph\": \"D\", \"pid\": %i, \"tid\": %i, \"id\": %" PRIuPTR ", \"ts\": %i},\n",
    name, PID_VALUE,  tid, (uintptr_t)obj_pointer, int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );

    dataVector.push_back(stringBuffer);
}

/*
    void trace_instant_global(name, scope='t')

    Pushes a line to the dataVector to create a global instant
*/
inline void trace_instant_global(const char* name, const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    sprintf(stringBuffer,
    "{\"name\": \"%s\", \"ph\": \"i\", \"pid\": %i, \"tid\": %i, \"s\": \"g\", \"ts\": %i},\n",
    name, PID_VALUE,  tid, int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );

    dataVector.push_back(stringBuffer);
}

/*
    void trace_counter(name, key, value)

    Pushes a line to the dataVector to create a counter event
    key contains the names, value contains the respective values
*/
inline void trace_counter(const char* name, std::initializer_list<const char*> key, std::initializer_list<const char*> value, const unsigned int tid=TID_VALUE)
{
    if(!traceActive) return; //Do nothing if trace_start not called

    if( dataVector.size() == dataVector.capacity() ) trace_flush(); //Flush if full

    if(key.size() != value.size()) //Lists have different sizes
    {
        std::cerr << "Error: Argument lists for " << name << " in trace_counter are not the same size; ignoring this event.\n";
        //return
    }
    else
    {
        sprintf(stringBuffer,
        "{\"name\": \"%s\", \"ph\": \"C\", \"pid\": %i, \"tid\": %i, \"ts\": %i, \"args\": { ",
        name, PID_VALUE,  tid, int(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-startTime).count()) );
        auto itNames  = key.begin();
        auto itValues = value.begin();
        for(size_t i=0; i<key.size(); i++)
        {
            sprintf(stringBuffer + strlen(stringBuffer),
            "\"%s\": %s",
            *itNames++,*itValues++);

            if(i!=key.size()-1) sprintf(stringBuffer + strlen(stringBuffer),", "); //add comma if not last variable
        }
        sprintf(stringBuffer + strlen(stringBuffer),"} },\n");
        dataVector.push_back(stringBuffer);
    }
}

}

#endif // TRACELIB_H_INCLUDED
