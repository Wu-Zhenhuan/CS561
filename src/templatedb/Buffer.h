//
// Created by Zhenhuan Wu on 4/18/23.
//

#ifndef TEMPLATEDB_BUFFER_H
#define TEMPLATEDB_BUFFER_H

#include "Value.h"

//template<typename K, typename V>
typedef templatedb::Value Value;
typedef templatedb::Pair Pair;
typedef std::vector<Pair> run;
class Buffer {
private:
    int capacity;
    int length;
    int min;
    int max;

    std::vector<Pair> pairs;
public:
    explicit Buffer(int capacity) : capacity(capacity), length(0) {
        pairs.reserve(capacity);
    };
    Value get(int key);
    void put(int key, templatedb::Value val);
    std::vector<Pair> scan();
    std::vector<Pair> scan(int min_key, int max_key);
    void del(int key);
    void del(int min_key, int max_key);
    size_t size();
    bool isFull();
    run flushOut();
};


#endif //TEMPLATEDB_BUFFER_H
