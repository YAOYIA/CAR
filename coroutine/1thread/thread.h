#pragma once
#include <mutex>
#include <condition_variable>
#include <functional>

namespace sylar
{
//用于线程方法间的同步
class Semaphore
{
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

public:
    //信号量初始为0
    explicit Semaphore(int count_ = 0):count(count_){}

    //p操作
    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);
        //循环判断，处理虚假的信息
        while(count == 0)
        {
            //做两件事
            //1.原子性地解锁 mtx，阻塞等待条件变量通知
            //2.被唤醒之后，重新加锁mtx，然后检查条件 count==0
            cv.wait(lock);
        }
        count --;
        //析构时自动解锁mtx
    }

    //v操作
    void signal()
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;//通知前确保共享资源已更新
        cv.notify_one();
    }

    //~Semaphore();
};

// 一共两种线程：1.由系统自动创建的主线程 2.由Thread类创建的线程
class Thread
{
public:
    Thread(std::function<void()> cd,const std::string& name);
    ~Thread();

    pid_t getId() const {return m_id;}
    const std::string& getName() const {return m_name;}

    void join();
public:
    //获取系统分配的线程id
    static pid_t GetThreadId();
    //获取当前所在的线程
    static Thread* GetThis();

    //获取当前线程的名字
    static const std::string& GetName();
    //设置当前线程的名字
    static void SetName(const std::string& name);
private:
    //线程函数
    static void* run(void* arg);

private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;

    //线程需要运行的函数
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;
};



}//namespace