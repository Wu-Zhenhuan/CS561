//
// Created by Zhenhuan Wu on 4/18/23.
//

#include "Buffer.h"

Value Buffer::get(int key) {
    if ((key < this->min) || (key > this->max)) return Value(false);
    for (auto &pair: this->pairs) {
        if ((pair.first == key) && (pair.second.visible)) {
            return pair.second;
        }
    }
    return Value(false);
}

void Buffer::put(int key, Value val) {
    if (this->size() == 0) {
        this->max = key;
        this->min = key;
    } else if (this->max < key) { this->max = key; }
    else if (this->min > key) { this->min = key; }

    /* Updates if the key already exists */
    for (auto &pair: this->pairs) {
        if ((pair.first == key) /* && (Pair.second.visible) */) {
            pair.second = val;
            return;
        } else if (!pair.second.visible) {
            pair.first = key;
            pair.second = val;
        }
    }
    /* Inserts if the key is new */
    this->pairs.push_back(Pair(key, val));
    this->length++;

    // Vector will have to reallocate the rest of elements if new element is not inserted to the end of the vector
}

std::vector<Pair> Buffer::scan() {
    std::vector<Pair> resultSet;
    for (auto &pair : this->pairs) {
        if (pair.second.visible) resultSet.push_back(pair);
    }

    std::sort(resultSet.begin(), resultSet.end(), [ ](const Pair& lhs, const Pair& rhs )
    {
        return lhs.first < rhs.first;
    });
    return resultSet;
}

std::vector<Pair> Buffer::scan(int min_key, int max_key) {
    std::vector<Pair> resultSet;
    for (auto &pair : this->pairs) {
        if ((pair.second.visible) && (pair.first >=min_key) && (pair.first <= max_key))
            resultSet.push_back(pair);
    }

    std::sort(resultSet.begin(), resultSet.end(), [ ](const Pair& lhs, const Pair& rhs )
    {
        return lhs.first < rhs.first;
    });
    return resultSet;
}

void Buffer::del(int key) {
    if ((key < this->min) || (key > this->max)) return;
    for (auto &pair: this->pairs) {
        if ((pair.first == key) && (pair.second.visible)) {
            pair.second.visible = false;
            this->length --;
            return;
        }
    }

    // If it's no longer in the buffer:
    this->put(key, Value(false));
}

void Buffer::del(int min_key, int max_key) {
    this->put(min_key, Value(false, max_key - min_key));
    /*
    for (auto &pair : this->pairs) {
        if ((pair.second.visible) && (pair.first >=min_key) && (pair.first <= max_key))
            pair.second.visible = false;
            this->length --;
    }
     */
}

size_t Buffer::size() {
    return this->length;
}

bool Buffer::isFull() {
    return this->pairs.size() >= this->capacity;
}

std::vector<Pair> Buffer::flushOut() {
    std::vector<Pair> resultSet = this->pairs;
    /*
    for (auto &pair : this->pairs) {
        if (pair.second.visible) resultSet.push_back(pair);
    }*/

    std::sort(resultSet.begin(), resultSet.end(), [ ](const Pair& lhs, const Pair& rhs )
    {
        return lhs.first < rhs.first;
    });
    pairs.clear();
    return resultSet;
}