#pragma once

namespace KaiYiCache
{

template<typename Key,typename Value>
class CachePolicy
{
private:
    Key key_;
    Value value_;
public:
    //添加缓存接口
    virtual void put(Key key,Value Value) = 0;

    //key是传入参数 访问到的值以传出参数的形式返回 
    virtual bool get(Key key,Value &Value) = 0;

    //如果缓存缓存中可以直接找到key，直接返回
    virtual Value get(Key key) = 0;
    
    virtual ~CachePolicy(){}
};



}//namespace kaiyicache