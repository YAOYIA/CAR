#include "include/CachePolicy.h"
#include <memory>   

template<typename Key, typename Value> class LruCache;

template<typename Key, typename Value>
class FreqList
{
private:
    struct Node
    {
        int freq;
        Key key_;
        Value value_;
        std::weak_ptr<Node> pre;//上一节点改为weak_ptr打破循环
        std::shared_ptr<Node> next;

        Node():freq(1),next(nullptr){}
        Node(Key key,Value value):key_(key),value_(value),freq(1),next(nullptr){}
        LruCache *cache;
    };

    using NodePtr = std::shared_ptr<Node>;
    int freq_;
    NodePtr head_;
    NodePtr tail_;

public:
    explicit FreqList(int n):freq_(n)
    {
        head_ = std::make_shared<Node>();
        tail_ = std::make_shared<Node>();
        head_->next = tail_;
        tail_->pre = head_;
    }

    bool isEmpty() const
    {
        return head_->next = tail_;
    }

    void addNode(Key key,Value value)
    {
        NodePtr newNode = std::make_shared<Node>(key,value);
        newNode->next = tail_;
        newNode->pre = head_;
        head_ = newNode;
    }

    FreqList(/* args */);
    ~FreqList();



};





class LruCache : public LruCache
{
private:
    Key key;
public:
    LruCache : public LruCache(/* args */);
    ~LruCache : public LruCache();
};



