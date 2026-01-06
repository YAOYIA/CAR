#include "include/CachePolicy.h"

int main() {
    CachePolicy cachePolicy;
    cachePolicy.setCacheSize(1000);
    cachePolicy.setCacheType(CacheType::LRU);
    cachePolicy.setCachePolicy(CachePolicyType::LRU);
    cachePolicy.setCachePolicy(CachePolicyType::LFU);
    cachePolicy.setCachePolicy(CachePolicyType::FIFO);
    cachePolicy.setCachePolicy(CachePolicyType::LFU);
    cachePolicy.setCachePolicy(CachePolicyType::LRU);
}