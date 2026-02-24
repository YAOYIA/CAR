#include <iostream>
#include <thread>
#include <chrono>

// 1.定义全局TLS变量，每个线程独立
thread_local int global_var = 0;

// 2.函数内局部TLS变量
void set_local_tls(int thread_id)
{
    thread_local int local_tls_var = 0;//每个线程首次调用时初始化一次
    local_tls_var = thread_id;
    std::cout<<"线程:"<<thread_id<<":局部tls变量 = "<< local_tls_var <<std::endl;
}

//线程执行函数
void worker(int thread_id)
{
    //修改全局变量
    global_var = thread_id;
    std::cout<<"线程:"<<thread_id<<":设置全局tls变量 = "<< global_var <<std::endl;


    //等待一下
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::thread::id id = std::this_thread::get_id();
    std::cout<<"线程id:"<<id<<std::endl;
    //读取全局tls变量
    std::cout<<"线程:"<<thread_id<<":读取全局tls变量 = "<< global_var <<std::endl;
    
    //调用局部TLS变量函数
    set_local_tls(thread_id);
}


int main()
{
    //创建两个线程
    std::thread t1(worker,1);
    std::thread t2(worker,2);

    t1.join();
    t2.join();

    return 0;
}
