//
// Created by Zhenhuan Wu on 4/25/23.
//

#include "Level.h"
Level::Level() : currentLevel(0) {}

templatedb::Value Level::get(int key) {
    for (int i = this->currentLevel - 1; i >= 0; i--) {
        if ((this->mins.at(i) > key) || this->maxs.at(i) < key) {
            continue;
        }
        for (auto &pair: this->levels.at(i)) {
            if (pair.first == key) {
                return pair.second;
            } else if ((!pair.second.visible) && (pair.first <= key) && (key <= pair.first + pair.second.range) ) {
                return templatedb::Value(false);
            }

        }
    }
}


int Level::levelCapacity(int l) {
    return BUFFER_SIZE << l;
}
void Level::flushIn(std::tuple<run, int, int> buffer) {
    run bufferRun = std::get<0>(buffer);
    if (this->levels.size()==0) {
        this->newLevel();
        this->levels.push_back(bufferRun);
        this->mins.push_back(std::get<1>(buffer));
        this->maxs.push_back(std::get<2>(buffer));
    }
    else if (levels.at(levels.size()-1).size() > levelCapacity(levels.size())) {
        this->currentLevel++;
    }
    std::vector<Pair> level;

}
void Level::newLevel() {
    std::vector<templatedb::Pair> level(this->levelCapacity(this->currentLevel));
    this->levels.push_back(level);
    this->currentLevel++;
}

run Level::merge(run a, run b) {
    run resultSet(a.size() + b.size());
    std::merge(a.begin(),a.end(),b.begin(),b.end(), std::back_inserter(resultSet), [ ](const Pair& lhs, const Pair& rhs )
    {
        return lhs.first < rhs.first;
    });
    return resultSet;
    /*
    size_t i = 0; size_t j = 0;
    run resultSet;
    resultSet.reserve(a.size() + b.size())
    while (i < a.size() && j < b.size()) {
        if (a.at(i).first < b.at(j).first) {
            resultSet.push_back(a.at(i++));
        } else {
            resultSet.push_back(b.at(j++));
        }
    }

    if (i < a.size()) {
        resultSet.insert(resultSet.end(), a.begin() + i, a.end());
    } else if (j < b.size()) {
        resultSet.insert(resultSet.end(), a.begin() + j, b.end());
    }*/
}