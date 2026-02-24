#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <iomanip>
#include <random>
#include <algorithm>

#include "include/CachePolicy.h"
#include "include/LruCache.h"

class Timer
{

public:
    Timer():start_(std::chrono::high_resolution_clock::now()){}

    double elapsed()
    {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now - start_).count();
    }

    ~Timer();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};


//辅助函数，打印结果
void printResults()
{
    
}

int main()
{

}