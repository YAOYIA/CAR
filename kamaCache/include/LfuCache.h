#pragma once
#include <memory>
#include <cmath>
#include <vector>
#include <mutex>
#include <thread>
#include <unordered_map>


#include "CachePolicy.h"


namespace KaiYiCache
{
template<typename Key,typename Value> class LfuCache;
template<typename Key,typename Value>
class FreqList
{

public:
    FreqList(/* args */);
    ~FreqList();
private:
    struct Node
    {
        int freq;
        Key key_;
        Value value_;
        std::weak_ptr<Node> pre;
        std::shared_ptr<Node> next;

        Node():freq(1),next(nullptr){}
        Node(Key key,Value value):freq(1),key(key),value(value),next(nullptr){}
    };
    
    using NodePtr = std::shared_ptr<Node>;
    int freq_;//访问频率
    NodePtr head_;//假的头节点
    NodePtr tail_;//假的尾节点
};






    
}//namespace 