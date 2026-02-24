#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

static int a = 0;
std::mutex mtx;
void add()
{
    std::lock_guard<std::mutex> lock(mtx);
    for (size_t i = 0; i < 100000; i++)
    {
        a++;
    }
}

int main()
{
    std::vector<std::thread> threads;
    for (size_t i = 0; i < 5; i++)
    {
        threads.emplace_back(add);
    }

    for (auto& t:threads)
    {
        t.join();
    }
    std::cout<<"a="<<a<<std::endl;
    
}

//编译指令 g++ ./test_lock.cc -o test -pthread