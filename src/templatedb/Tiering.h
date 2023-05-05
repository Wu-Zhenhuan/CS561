//
// Created by Zhenhuan Wu on 5/4/23.
//

#ifndef TEMPLATEDB_TIERING_H
#define TEMPLATEDB_TIERING_H

#include "Value.h"
#include "BloomFilter/BloomFilter.h"
typedef templatedb::Pair Pair;
typedef templatedb::run run;
class Tiering {
private:
    int currentLevel;
    std::vector<run> tiers;
    std::vector<BF::BloomFilter> bloomFilters;
    std::vector<int> mins, maxs;
public:
    explicit Tiering();
    templatedb::Value get(int key);
    void put(int key, templatedb::Value val);
    std::vector<Pair> scan();
    std::vector<Pair> scan(int min_key, int max_key);
    void del(int key);
    void del(int min_key, int max_key);
    void flushIn(std::tuple<run, int, int> buffer);
    int levelCapacity(int l);
    run merge(run higher, run lower);
    void newTier();

};


#endif //TEMPLATEDB_TIERING_H
