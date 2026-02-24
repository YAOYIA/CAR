# fiber
该类提供了协程的基本功能，包括创建、管理、切换和销毁协程，使用ucontext结构保存和恢复协程的上下文，并通过std::function来存储协程的执行逻辑。

协程的状态。
这里只设置为3种状态：就绪态、运行态、结束态，一个协程要么正在运行，要么准备、要么运行结束

Fiber类大量使用static函数，核心时为了管理 协程-线程 的全局关联关系，提供无实例依赖的工具能力，同时契合协程调度的底层设计原理。




核心原理：局部线程存储（Thread-Local Storage）
这些static方法真正的威力在于与 线程局部存储 结合。
1.为什么需要线程局部存储？
协程是线程相关的：
* 每个线程有自己独立的协程集合
* 协程切换只能在同一个线程内进行
* 每个线程在任何时刻只运行一个协程
就像是一个线程的**私有全局变量**，每个线程都需要知道自己当前运行的是哪个协程。
2.具体实现的机制
实现的过程中，通过有这样的局部变量：
```
// 每个线程独立的当前协程指针
static thread_local Fiber* t_current_fiber = nullptr;

// 每个线程独立的主协程（调度协程）
static thread_local std::shared_ptr<Fiber> t_main_fiber = nullptr;
```
3.静态方法如何工作？
```
// 获取当前协程
static std::shared_ptr<Fiber> GetThis() {
    // 如果没有设置当前协程（第一次调用）
    if (!t_current_fiber) {
        // 创建主协程
        auto main_fiber = std::make_shared<Fiber>();
        t_current_fiber = main_fiber.get();
        t_main_fiber = main_fiber;  // 保存主协程
    }
    return t_current_fiber->shared_from_this();
}
```
4.协程切换流程
```
void Fiber::resume() {
    // 1. 保存当前协程（当前正在运行的）
    Fiber* cur = GetThis();
    
    // 2. 设置自己为当前运行协程
    SetThis(this);
    
    // 3. 上下文切换：从cur->m_ctx切换到this->m_ctx
    swapcontext(&cur->m_ctx, &m_ctx);
}

void Fiber::yeild() {
    // 1. 获取主协程（调度协程）
    auto main_fiber = t_main_fiber;
    
    // 2. 设置主协程为当前运行协程
    SetThis(main_fiber.get());
    
    // 3. 切换回主协程
    swapcontext(&m_ctx, &main_fiber->m_ctx);
}
```
5.为什么static能解决这个问题？
场景1：不同线程中的协程切换
线程A: 协程1 → 协程2 → 协程3
线程B: 协程4 → 协程5

线程A的t_current_fiber: 协程1 → 协程2 → 协程3
线程B的t_current_fiber: 协程4 → 协程5

场景2：静态方法作为ucontext回调
static void MainFunc() {
    // 问题：这个静态函数如何知道要执行哪个协程的任务？
    // 答案：通过t_current_fiber获取！
    
    auto cur = GetThis();  // 获取当前协程
    cur->m_cb();           // 执行协程函数
}

关键优势：
1.线程安全：每个线程有自己的副本，无需加锁
2.全局访问：任何地方都可以获取当前的协程
3.无需传递参数：不需要再函数间传递当前协程指针
4.与C api兼容：ucontext回调需要普通函数，静态函数完美匹配

总结：
这种设计模式本质上是线程局部单例模式
* 每个线程有且只有一个“当前协程”
* 静态方法提供了访问这个线程局部单例的统一接口
* 协程切换就是修改这个线程局部变量的过程
这就是为什么协程库大量使用static方法——它们管理的是线程级别的全局状态，而不是对象级别的状态。