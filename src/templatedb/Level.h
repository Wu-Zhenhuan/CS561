//
// Created by Zhenhuan Wu on 4/25/23.
//

#ifndef TEMPLATEDB_LEVEL_H
#define TEMPLATEDB_LEVEL_H

#include "Value.h"
#include "BloomFilter/BloomFilter.h"
//typedef templatedb::Value Value;
typedef templatedb::Pair Pair;
typedef templatedb::run run;

class Level {
private:
    int currentLevel;
    std::vector<run> levels;
    std::vector<BF::BloomFilter> bloomFilters;
    std::vector<int> mins, maxs;
public:
    //explicit Level(int currentLevel);
    explicit Level();
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


#endif //TEMPLATEDB_LEVEL_H
