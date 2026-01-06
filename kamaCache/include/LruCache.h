#pragma once
#include <iostream>
#include <cstring>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "include/CachePolicy.h"
 
namespace KaiYiCache
{
// 前向声明
template<typename Key, typename Value> class LruCache;

template<typename Key, typename Value>
class LruNode
{
private:
    Key key_;
    Value value_;
    size_t accessCount_;
    std::weak_ptr<LruNode<Key,Value>> prev_;// 改为weak_ptr 打破循环引用
    std::shared_ptr<LruNode<Key,Value>> next_;

public:
    LruNode(Key key,Value value):key_(key),value_(value),accessCount_(1){}

    // 提供必要的访问器
    Key getKey() const {return key_;}
    Value getValue() const {return value_;}
    void setValue(const Value& value) {value_ = valuel;}
    size_t getAccessCount() {return accessCount_;}
    void incrementAccessCount() {++accessCount_;}
    
    friend class LruCache<Key,Value>;
};


template<typename Key, typename Value> 
class LruCache : public CachePolicy<Key,Value>
{
public:
    using LruNodeType = LruNode<Key,Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unordered_map<Key,NodePtr>;

    LruCache(int capacity):capacity_(capacity)
    {
        
    }

    ~LruCache() override = default;
private:
    /**
     * @brief 这段代码是双向链表的初始化，核心作用是创建 dummyHead（虚拟头节点）和 dummyTail（虚拟尾节点），并构建一个 “空链表” 的基础结构（虚拟头→虚拟尾，虚拟尾→虚拟头）。
     * 
    */
    void initializeList()
    {
        //1.创建虚拟头节点：用key()和Value()的默认构造函数初始化键值
        dummyHead_ = std::make_shared<LruNodeType>(Key(),Value());
        dummyTail_ = std::make_shared<LruNodeType>(Key(),Value());
        dummyHead_->next_ = dummyTail_;
        dummyTail_->prev_ = dummyHead_;
    }

    void updateExistingNode(NodePtr node,const Value& value)
    {
        node->setValue(value);
        moveToMostRecent(node);
    }

    void addNewNode(const Key& key,const Value& value)
    {
        if (nodeMap_.size()>=capacity_)
        {
            evictLeastRecent();
        }

        NodePtr newNode = std::make_shared<LruNodeType>(key,value);
        insertNode(newNode);
        nodeMap_[key] = newNode;
    }

    void moveToMostRecent(NodePtr node)
    {
        node->next_ = dummyHead_->next_;
        dummyHead_->next_ = node;
        node->prev_ = dummyHead_;
    }

    void removeNode(NodePtr node)
    {
        if(!node->prev_.expired() && node->next_)//判断prev_指向的前驱节点是否已经被销毁（弱引用过期）
        {
            auto prev = node->prev_.lock();//使用lock解锁获取shared_ptr
            prev->next_ = node->next_;
            node->next_->prev_ = prev;
            node->next_ = nullptr;//清空指针，彻底断开节点与链表的连接
        }
    }

    void insertNode(NodePtr node)
    {
        node->next = dummyTail_;
        node->prev_ = dummyTail_->prev_;
        dummyTail_->prev_.lock()->next_ = node;
        dummyTail_->prev_ = node;
    }

    //驱逐最近访问
    void evictLeastRecent()
    {
        NodePtr least = dummyHead_;
        removeNode(least);
        nodeMap_.erase(least->getKey());
    }

private:
    int capacity_;//缓存容量
    NodeMap nodeMap_;
    std::mutex mutex_;
    NodePtr dummyHead_;//虚拟头节点
    NodePtr dummyTail_;
};










}//namespace kaiyicache

