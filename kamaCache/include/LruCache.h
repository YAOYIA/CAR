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
        initializeList();
    }

    ~LruCache() override = default;

    //添加缓存
    void put(Key key,Value value) override
    {
        if (capacity_ < =0)
        {
            return;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if(it != nodeMap_.end())
        {
            //如果在当前的容器中，则更新value，并调用get方法，代表该数据刚被访问
            updateExistingNode(it->second, value);
            return;
        }

        addNewNode(key,value);
    }

    bool get(Key key,Value& value) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end())
        {
            moveToMostRecent(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }

    Value get(Key key) override
    {
        Value value();
        get(key,value);
        return value;
    }

    void remove(Key key)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end())
        {
            removeNode(it->second);
            nodeMap_.erase(it);
        }
    }
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

    // 将节点移动到最新的位置
    void moveToMostRecent(NodePtr node)
    {
        removeNode(node);
        insertNode(node);
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

    //驱逐最近最少访问
    void evictLeastRecent()
    {
        NodePtr least = dummyHead_->next_;
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


//LRU优化，LRU-k版本，通过继承的方式进行再次优化
template<typename Key,typename Value>
class LruKCache : public LruCache<Key,Value>
{

public:
    LruKCache(int capacity, int historyCapacity, int k)
                :LruCache<Key,Value>(capacity)//调用基类构造
                ,historyList_(std::make_unique<LruCache<Key,size_t>>(historyCapacity))
                ,k_(k){}
    ~LruKCache();

    Value get(Key key)
    {
        //首先尝试从主缓存获取数据
        Value value{};
        bool inMainCache = LruCache<Key,Value>::get(key,value);

        //获取并更新访问历史计数
        size_t historyCount = historyList_->get(key);
        historyCount++;
        historyList_->put(key,historyCount);

        //如果数据在主缓存中，直接返回
        if(inMainCache)
        {
            return value;
        }

        //如果数据不在主缓存，但访问次数达到了k次
        if (historyCount >- k_)
        {
            //检查是否有历史值记录
            auto it = historyValueMap_.find(key);
            if (it != historyValueMap_.end())
            {
                //有历史值，将其添加到主缓存中
                Value storeValue = it->second;

                //从历史记录中移除
                historyList_->remove(key);
                historyValueMap_.erase(it);

                //添加到主缓存中
                LruCache<key,Value>::put(key,storeValue);

                return storeValue;
            }
            // 没有历史值记录，无法添加道缓存，再返回默认值
        }
        //数据不在主缓存且不满足添加条件，返回默认值
        return value;
    }
private:
    int k_;//进入缓存队列的评判标准
    std::unique_ptr<LruCache<Key,size_t>> historyList_;//访问数据历史记录
    std::unordered_map<Key,Value> historyValueMap_;//存储未达到k次访问的数据值
};



//lru优化：对lru进行分片，提高高并发使用的性能
template<typename Key,typename Value>
class KHashLruCache
{

public:
    KHashLruCache(size_t capacity,int sliceNum):capacity_(capacity),sliceNum_(sliceNum>0 ? sliceNum : std::thread::hardware_concurrency())
    {
        size_t sliceSize = std::ceil(capacity / static_cast<double>(sliceNum_));//获取每个分片的大小
        for (int i = 0; i < sliceNum_; i++)
        {
            lruSliceCaches_.emplace_back(new LruCache<Key,Value>(sliceSize));
        }
        
    }

    void put(Key key,Value value)
    {
        // 获取key的hash值，并计算出对应的分片索引
        size_t sliceIndex = Hash(key) % sliceNum_;
        lruSliceCaches_[sliceIndex]->put(key, value);
    }

    bool get(Key key,Value value)
    {
        // 获取key的hash值，并计算出对应的分片索引
        size_t sliceIndex = Hash(key) % sliceNum_;
        return lruSliceCaches_[sliceIndex]->get(key, value);
    }
  
    Value get(Key key)
    {
        Value value;
        memset(&value,0,sizeof(Value));
        get(key,value);
        return value;
    }
  
    ~KHashLruCache();
private:
    // 将key转换为对应的hash值
    size_t Hash(Key key)
    {
        std::hash<Key> hashFunc;
        return hashFunc(key);
    }

private:
    size_t capacity_;//总容量
    int sliceNum_;//切片数量
    std::vector<std::unique_ptr<LruCache<Key,Value>>> lruSliceCaches_;//切片缓存的数量
};


}//namespace kaiyicache

