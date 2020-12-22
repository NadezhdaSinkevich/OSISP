#include "ThreadPool.h"
#include <functional>
#include <fstream>
#include <string>
#include <wtypes.h>

#define MAX_THREADS 255
#define BUF_SIZE 255

using namespace std;

list<function<void()>>ThreadPool::tasks;
mutex ThreadPool::mutex;
bool ThreadPool::exitFlag = false;
int maxThreads;
int ThreadPool::currentNumOfThreads = 0;
HANDLE hThreadArray[MAX_THREADS];

void ThreadPool::WriteLog(string recording)
{
    ofstream out("Log.txt", ios::app);
    if (out.is_open())
    {
        out << recording << endl;
    }
    out.close();
}

DWORD WINAPI ThreadPool::Task(void* args)
{
    function<void()> task;
    
    while (!exitFlag)
    {
        if (!tasks.empty())
        {
            mutex.lock();
            if (!tasks.empty())
            {
                task = tasks.front();
                tasks.pop_front();
            }
            else
            {
                mutex.unlock();
                continue;
            }
            mutex.unlock();
            
            try
            {
                task();
            }
            catch (const exception & e)
            {
                WriteLog(e.what());
            }
            currentNumOfThreads--;
        }
    }
    return 0;
}

ThreadPool::ThreadPool(int n)
{
    DWORD dwThreadIdArray[MAX_THREADS];
    maxThreads = n;

    for (int i = 0; i < maxThreads; i++)
    {

        hThreadArray[i] = CreateThread(NULL, 0, ThreadPool::Task, 0, 0, &dwThreadIdArray[i]);
              
        ofstream out("Log.txt", ios::app);

        if (out.is_open())
        {
            out << "thread " << dwThreadIdArray[i] << " was create" << endl;
        }
        out.close();

    }

}
void ThreadPool::AddTask(function<void()> task)
{
    if (currentNumOfThreads < maxThreads)
    {
        tasks.push_back(task);
        currentNumOfThreads++;       
        WriteLog("new task");
    }
    else
    {        
        WriteLog("error with number of threads");
    }
    
}

void ThreadPool::StopThreads()
{
    exitFlag = true;
    WaitForMultipleObjects(maxThreads, hThreadArray, TRUE, INFINITE);

    for (int i = 0; i < maxThreads; i++)
    {
        CloseHandle(hThreadArray[i]);
    }
}


