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
/**
 * A lovely merge that deal with duplicate in newer way that keeps newer one
 * @param newer
 * @param older
 * @return
 */
run Level::merge(run newer, run older) {

    /*
    run resultSet(newer.size() + older.size());
    std::merge(newer.begin(), newer.end(), older.begin(), older.end(), std::back_inserter(resultSet), [ ](const Pair& lhs, const Pair& rhs )
    {
        return lhs.first < rhs.first;
    });
    return resultSet;
    */
    size_t i = 0; size_t j = 0;
    run resultSet;
    resultSet.reserve(newer.size() + older.size());
    while (i < newer.size() && j < older.size()) {
        if (newer.at(i).first = older.at(j).first) {
            resultSet.push_back(newer.at(i++));
            j++;
        }
        else if (newer.at(i).first < older.at(j).first) {
            resultSet.push_back(newer.at(i++));
        } else {
            resultSet.push_back(older.at(j++));
        }
    }

    if (i < newer.size()) {
        resultSet.insert(resultSet.end(), newer.begin() + i, newer.end());
    } else if (j < older.size()) {
        resultSet.insert(resultSet.end(), newer.begin() + j, older.end());
    }
}
std::vector<Pair> Level::scan(int min_key, int max_key) {
    std::vector<Pair> resultSet;
    for (int i = this->currentLevel - 1; i >= 0; i--) {
        if ((this->mins.at(i) > max_key) || this->maxs.at(i) < min_key) {
            continue;
        }
        std::vector<Pair> levelResult;
        for (auto &pair: this->levels.at(i)) {
            levelResult.push_back(pair);

        }
        this->merge(resultSet, levelResult);
    }
    return resultSet;
}
