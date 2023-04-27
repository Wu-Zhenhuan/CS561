//
// Created by Zhenhuan Wu on 4/18/23.
//

#ifndef TEMPLATEDB_BUFFER_H
#define TEMPLATEDB_BUFFER_H
#include "db.hpp"

//template<typename K, typename V>
typedef templatedb::Value Value;
typedef std::pair<int,Value> pair;
typedef std::vector<pair> run;
class Buffer {
private:
    int capacity;
    int length;
    int min;
    int max;

    std::vector<pair> pairs;
public:
    explicit Buffer(int capacity) : capacity(capacity), length(0) {
        pairs.reserve(capacity);
    };
    Value get(int key);
    void put(int key, Value val);
    std::vector<pair> scan();
    std::vector<pair> scan(int min_key, int max_key);
    void del(int key);
    void del(int min_key, int max_key);
    size_t size();
    bool isFull();
    run flushOut();
};


#endif //TEMPLATEDB_BUFFER_H
