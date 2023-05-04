//
// Created by Zhenhuan Wu on 4/25/23.
//

#ifndef TEMPLATEDB_LEVELLING_H
#define TEMPLATEDB_LEVELLING_H

#include "Value.h"
#include "BloomFilter/BloomFilter.h"
//typedef templatedb::Value Value;
typedef templatedb::Pair Pair;
typedef templatedb::run run;

class Levelling {
private:
    int currentLevel;
    std::vector<run> levels;
    std::vector<BF::BloomFilter> bloomFilters;
    std::vector<int> mins, maxs;
public:
    //explicit Levelling(int currentLevel);
    explicit Levelling();
    templatedb::Value get(int key);
    void put(int key, templatedb::Value val);
    std::vector<Pair> scan();
    std::vector<Pair> scan(int min_key, int max_key);
    void del(int key);
    void del(int min_key, int max_key);
    void flushIn(std::tuple<run, int, int> buffer);
    int levelCapacity(int l);
    run merge(run higher, run lower);
    void newLevel();
};


#endif //TEMPLATEDB_LEVELLING_H
