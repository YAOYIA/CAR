#pragma once
#include <iostream>
#include <memory>
#include <atomic>
#include <functional>
#include <cassert>
#include <ucontext.h>
#include <unistd.h>
#include <mutex>


namespace sylar
{
class Fiber : public std::enable_shared_from_this<Fiber>
{
private:
    // 协程状态
    enum State
    {
        READY,
        RUNING,
        TERM
    };
private:
    //仅由GetThis()调用--》私有---》创建主协程
    //私有的构造函数，禁止外部直接调用，仅由静态方法GetThis()调用，用于创建 线程主协程
    // 每个线程默认有一个主协程（作为协程调度的【根上下文】），线程启动的时候运行的是主协程，其他的协程切换的时候均基于主协程的上下文。
    Fiber();

public:
    // 用于构造普通协程
    // run_in_scheduler 表示是否交给调度器管理
    Fiber(std::function<void()> cb,size_t stacksize = 0,bool run_in_scheduler = true);
    //其主要作用是释放协程栈内存、校验协程状态
    ~Fiber();

    //重用一个协程
    //复用一个已经终止（term）的协程对象，重新设置执行回调，避免频繁创建、销毁协程的开销
    void reset(std::function<void()> cb);

    //任务线程恢复执行
    //将协程的状态从READY转换为RUNIGN状态，并执行其回调函数
    void resume();
    //任务线程让出执行权
    void yeild();

    uint64_t getId() const {return m_id;}
    State getState() const {return m_state;}
public:
    //这些静态方法用于管理当前线程正在运行的协程。由于每个线程在同一时间只能运行一个协程，这个信息需要在线程基本共享，但不应该组为类的实例成员
    //管理 线程局部存储 中的当前协程，调度协程，因为每个线程独立维护协程上下文。
    //作用：将传入的 协程指针 f 存入线程局部存储，标记为【当前线程正在运行的协程】
    static void SetThis(Fiber *f);

    //获取当前线程正在运行的协程的智能指针，是协程调度的核心入口
    static std::shared_ptr<Fiber> GetThis();

    //设置调度协程
    //设置当前线程的 调度协程（调度器专用协程，负责管理所有业务写成的切换）
    // 逻辑：将调度协程指针存入TLS，供yeild时切换上下文使用
    static void SetSchedulerFiber(Fiber* f);

    //得到当前的运行的协程id
    static uint64_t GetFiberId();

    //协程函数。这是一个静态函数入口，当切换到新的协程的时候，ucontext会调用这个函数。他必须是静态的，因为ucontext不支持成员函数作为入口
    static void MainFunc();

private:
    //id
    uint64_t m_id = 0;
    //栈大小
    uint32_t m_stacksize = 0;
    //协程状态
    State m_state = READY;
    //协程上下文
    ucontext_t m_ctx;
    //协程栈指针
    void* m_stack = nullptr;
    //协程函数
    std::function<void()> m_cb;
    //是否让出执行权交给调度协程
    bool m_runInScheduler; 
public:
    std::mutex m_mutex;


};//class

}//namespace 
