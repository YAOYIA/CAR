#include <iostream>
#include <memory>
#include <mutex>


class Node
{
private:
    /* data */
public:
    int value;
    std::shared_ptr<Node> prev;
    // 将 prev 改为 weak_ptr 的核心是打破了 “双向强引用” 的闭环：
    // next 保留为 shared_ptr，保证 “后继节点” 的生命周期能被当前节点正常持有；
    // prev 改为 weak_ptr，避免对 “前驱节点” 形成强引用，从而让前驱节点的引用计数能正常减至 0 并析构；
    // 简单来说：weak_ptr 既保留了 “指向另一个节点” 的能力，又不会因为这个指向而增加引用计数，从而打破了循环引用的死锁，让对象的引用计数能正常归零并析构。
    // std::weak_ptr<Node> prev;
    std::shared_ptr<Node> next;
    Node(int val):value(val)
    {
        std::cout<<"Node 构造"<<std::endl;
    }
    ~Node()
    {
        std::cout<<"Node 析构"<<std::endl;
    }
};

/*
为什么会这样？我们一步步拆解引用计数的变化过程：
创建节点时：
node1（shared_ptr）指向 Node(1)，Node(1) 的引用计数 = 1；
node2（shared_ptr）指向 Node(2)，Node(2) 的引用计数 = 1。
互相指向时：
node1->next = node2：Node(2) 新增一个 shared_ptr 指向它，引用计数 +1 → 2；
node2->prev = node1：Node(1) 新增一个 shared_ptr 指向它，引用计数 +1 → 2。
函数结束时：
node1 和 node2 是 testCycle 函数的局部变量，出作用域后会被销毁：
node1 销毁 → Node(1) 的引用计数 -1 → 1（此时仍有 node2->prev 指向它）；
node2 销毁 → Node(2) 的引用计数 -1 → 1（此时仍有 node1->next 指向它）。
最终状态：
Node(1) 和 Node(2) 的引用计数都停留在 1，永远无法减至 0；
系统无法判断这两个对象是否还在被使用，因此不会释放它们的内存 —— 这就是循环引用导致的内存泄漏。
*/
void testCycle()
{
    //1.创建两个shared_ptr,分别管理node1和node2
    std::shared_ptr<Node> node1 = std::make_shared<Node>(1);
    std::shared_ptr<Node> node2 = std::make_shared<Node>(2);

    //2.让两个节点互相指向，形成闭环
    node1->next = node2;
    node2->prev = node1;

}

int main()
{
    testCycle();
    std::cout<<"test end"<<std::endl;
    return 0;
}