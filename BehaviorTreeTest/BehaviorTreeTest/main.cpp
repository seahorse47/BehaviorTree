//
//  main.cpp
//  BehaviorTreeTest
//
//  Created by hailong on 7/8/13.
//  Copyright (c) 2013年 hailong. All rights reserved.
//

#include <iostream>
#include "Behavior.h"
#include "../third_party/dlmalloc/dlmalloc-2.8.6.h"

#include <time.h>

using namespace behavior;

typedef void (*TestFunc)();
class TimerProfiler
{
public:
    TimerProfiler() {
        restart();
    }
    void restart() {
        time(&start_);
    }
    double elapsedTime() {
        time_t now;
        time(&now);
        return difftime(now, start_);
    }
private:
    time_t start_;
};

void runTest(TestFunc test, const char* name)
{
    printf(">>>> %s >>>>\n", name);
    test();
    printf("<<<<<<<<\n");
}

void test_memory_pool()
{
//    Behavior* p = new Behavior();
//    Behavior* pArr = new Behavior[1];
//    
//    delete p;
//    delete[] pArr;
    const int loops1 = 10000;
    const int loops2 = 5000;
    TimerProfiler profiler;
    for (int i=0; i<loops1; i++) {
        for (int j=0; j<loops2; j++) {
            int * p = new int;
            *p = 14;
            int * pArr = new int[10];
            pArr[5] = *p;
            
            delete p;
            delete[] pArr;
        }
    }
    printf("use global operator new/delete and new[]/delete[]: %f\n", profiler.elapsedTime());
    
    profiler.restart();
    for (int i=0; i<loops1; i++) {
        for (int j=0; j<loops2; j++) {
            int * p = (int*)dlmalloc(sizeof(int));
            *p = 14;
            int * pArr = (int*)dlmalloc(sizeof(int)*10);
            pArr[5] = *p;
            dlfree(p);
            dlfree(pArr);
        }
    }
    printf("use dlmalloc: %f\n", profiler.elapsedTime());
}

int main(int argc, const char * argv[])
{
    runTest(test_memory_pool, "test_memory_pool");
    
//    behavior::Behavior b;
    
    return 0;
}

