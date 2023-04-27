//
// Created by Zhenhuan Wu on 4/25/23.
//

#ifndef TEMPLATEDB_LEVEL_H
#define TEMPLATEDB_LEVEL_H
#include "db.hpp"
#include "BloomFilter/BloomFilter.h"

class Level {
private:
    int currentLevel;
    std::vector<std::vector<pair>> levels;
    std::vector<BF::BloomFilter> bloomFilters;
public:
    explicit Level(int currentLevel);

    Value get(int key);
    void put(int key, Value val);
    std::vector<pair> scan();
    std::vector<pair> scan(int min_key, int max_key);
    void del(int key);
    void del(int min_key, int max_key);
    void flushIn(run buffer);
    int levelCapacity(int l);
    run merge(run a, run b);
    void newLevel();
};


#endif //TEMPLATEDB_LEVEL_H
