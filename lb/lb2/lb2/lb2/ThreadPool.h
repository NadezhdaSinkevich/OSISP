#pragma once
#include <ostream>
#include <map>
#include <vector>
#include <list>
#include <mutex>
#include <functional>
#include <wtypes.h>

using namespace std;

typedef void (*Procedure)(void*);
class ThreadPool {
public:


    static void WriteLog(string recording);
    vector<thread> ths;
    static list<function<void()>> tasks;
    static mutex mutex;
    static bool exitFlag;
    int maxThreads;
    static int currentNumOfThreads;
    HANDLE  hThreadArray[];
    static DWORD WINAPI _stdcall Task(void* args);
    ThreadPool(int n);
    void AddTask(function<void()> task);
    void StopThreads();
};
