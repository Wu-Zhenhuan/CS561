//
// Created by Zhenhuan Wu on 5/1/23.
//

#ifndef TEMPLATEDB_VALUE_H
#define TEMPLATEDB_VALUE_H

#include <vector>
#define BUFFER_SIZE 1024

namespace templatedb {
    class Value
    {
    public:
        std::vector<int> items;
        bool visible = true;
        int range = 0;
        Value() {}
        Value(bool _visible) {visible = _visible;}
        Value(bool _visible, int _range) {visible = _visible; range = _range;}
        Value(const std::vector<int> & _items) { items = _items;}
        //Value(const Value&) = delete;
        //Value(const Value& value) = delete;

        bool operator ==(Value const & other) const
        {
            return (visible == other.visible) && (items == other.items);
        }
    };
    typedef std::pair<int,Value> Pair;
    typedef std::vector<Pair> run;
}

#endif //TEMPLATEDB_VALUE_H
