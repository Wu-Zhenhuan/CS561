//
// Created by Zhenhuan Wu on 4/18/23.
//

#ifndef TEMPLATEDB_BUFFER_H
#define TEMPLATEDB_BUFFER_H

#include "Value.h"

//template<typename K, typename V>
typedef templatedb::Value Value;
typedef templatedb::Pair Pair;
typedef templatedb::run run;

class Buffer {
private:
    int capacity;
    //int length;
    int min;
    int max;

    std::vector<Pair> pairs;
public:
    explicit Buffer(int capacity) : capacity(capacity) {
        pairs.reserve(capacity);
    };
    /**
     * Gets value via key
     * @param key
     * @return value
     */
    Value get(int key);
    /**
     * Puts a pair (including tombstone)
     * @param key
     * @param val
     */
    void put(int key, templatedb::Value val);
    /**
     * Exports all (range query)
     * @return
     */
    std::vector<Pair> scan();
    /**
     * Range query
     * @param min_key
     * @param max_key
     * @return
     */
    std::vector<Pair> scan(int min_key, int max_key);
    /**
     * Insert a tombstone for given key
     * @param key
     */
    void del(int key);
    /**
     * Insert a range tombstone for given key range
     * @param key
     */
    void del(int min_key, int max_key);
    /**
     * For test only
     * @return
     */
    size_t size();
    /**
     * Determines if the buffer is full
     * @return
     */
    bool isFull();
    /**
     * Flushes buffer out
     * @return
     */
    std::tuple<run, int, int> flushOut();
};


#endif //TEMPLATEDB_BUFFER_H
