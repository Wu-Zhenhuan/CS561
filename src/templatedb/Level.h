//
// Created by Zhenhuan Wu on 4/25/23.
//

#ifndef TEMPLATEDB_LEVEL_H
#define TEMPLATEDB_LEVEL_H

#include "Value.h"
#include "Run.h"
#include "BloomFilter/BloomFilter.h"
typedef templatedb::Value Value;
typedef templatedb::Pair Pair;
class Level {
private:
    int currentLevel;
    std::vector<std::vector<templatedb::Pair>> levels;
    std::vector<BF::BloomFilter> bloomFilters;
public:
    //explicit Level(int currentLevel);
    explicit Level();
    Value get(int key);
    void put(int key, Value val);
    std::vector<Pair> scan();
    std::vector<Pair> scan(int min_key, int max_key);
    void del(int key);
    void del(int min_key, int max_key);
    void flushIn(std::vector<Pair> buffer);
    int levelCapacity(int l);
    Run merge(Run a, Run b);
    void newLevel();
};


#endif //TEMPLATEDB_LEVEL_H
