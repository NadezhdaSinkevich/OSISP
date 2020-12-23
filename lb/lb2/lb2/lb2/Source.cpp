#include <iostream>
#include <thread>
#include <chrono>
#include "ThreadPool.h"
using namespace std;

void FirstTask()
{
    this_thread::sleep_for(chrono::milliseconds(2000));
    cout << "ID" << this_thread::get_id() << "\t\tfirst task\n" << endl;
}
void SecondTask()
{
    this_thread::sleep_for(chrono::milliseconds(2000));
    cout << "ID" << this_thread::get_id() << "\t\tsecond task\n" << endl;
}
void ThirdTask()
{
    this_thread::sleep_for(chrono::milliseconds(2000));
    cout << "ID" << this_thread::get_id() << "\t\tthird task\n" << endl;
}

int main()
{
    int N;
    cout << "N= ";
    cin >> N;
    ThreadPool threadPool(N);

    threadPool.AddTask(FirstTask);
    threadPool.AddTask(SecondTask);
    threadPool.AddTask(ThirdTask);

    threadPool.StopThreads();

    exit(0);
}
