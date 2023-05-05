//
// Created by Zhenhuan Wu on 5/4/23.
//

#include "Tiering.h"
Tiering::Tiering() : currentLevel(0) {}

templatedb::Value Tiering::get(int key) {
    for (int i = 0; i < this->currentLevel - 1; i++) {
        if ((this->mins.at(i) > key) || this->maxs.at(i) < key) {
            continue;
        }
        for (auto &pair: this->levels.at(i)) {
            if (pair.first == key) {
                return pair.second;
            } else if ((!pair.second.visible) && (pair.first <= key) && (key <= pair.first + pair.second.range)) {
                return templatedb::Value(false);
            }

        }
    }
    return NULL;
}


int Tiering::levelCapacity(int l) {
    return BUFFER_SIZE << l;
}

void Tiering::flushIn(std::tuple<run, int, int> buffer) {
    run bufferRun = std::get<0>(buffer);
    if (this->levels.size() == 0) {
        this->newLevel();
        this->levels.push_back(std::move(bufferRun));
        this->mins.push_back(std::get<1>(buffer));
        this->maxs.push_back(std::get<2>(buffer));
    } else {
        run mergedRun = std::move(this->merge(bufferRun, this->levels.at(0)));
        this->levels.at(0) = std::move(mergedRun);
        for (int i = 0; i < this->currentLevel - 1; i++) {
            if (levels.at(i).size() > levelCapacity(levels.size())) {
                mergedRun = std::move(this->merge(this->levels.at(i), this->levels.at(i+1)));
                this->levels.at(i+1) = std::move(mergedRun);
                this->levels.at(i) = run(this->levelCapacity(i));
            }
        }
        if (levels.at(currentLevel - 1).size() > levelCapacity(currentLevel - 1)) {
            this->newLevel();
            mergedRun = std::move(this->merge(
                    this->levels.at(this->currentLevel - 2),
                    this->levels.at(this->currentLevel - 1)));
            this->levels.at(this->currentLevel - 1) = std::move(mergedRun);
            this->levels.at(this->currentLevel - 2) = run(this->levelCapacity(currentLevel - 2));
        }
    }

}


void Tiering::newLevel() {
    std::vector<templatedb::Pair> level(this->levelCapacity(this->currentLevel));
    this->levels.push_back(level);
    this->currentLevel++;
}

/**
 * A lovely merge that deal with duplicate in higher way that keeps higher one
 * @param higher
 * @param lower
 * @return
 */
run Tiering::merge(run higher, run lower) {

    /*
    run resultSet(higher.size() + lower.size());
    std::merge(higher.begin(), higher.end(), lower.begin(), lower.end(), std::back_inserter(resultSet), [ ](const Pair& lhs, const Pair& rhs )
    {
        return lhs.first < rhs.first;
    });
    return resultSet;
    */
    size_t i = 0;
    size_t j = 0;
    run resultSet;
    resultSet.reserve(higher.size() + lower.size());
    while (i < higher.size() && j < lower.size()) {
        if (higher.at(i).first == lower.at(j).first) {
            resultSet.push_back(higher.at(i++));
            if (lower.at(j).second.visible || lower.at(j).second.range <= 0) {
                // Not a range tombstone
                j++;
            }
        } else if (higher.at(i).first < lower.at(j).first) {
            resultSet.push_back(higher.at(i++));
        } else {
            resultSet.push_back(lower.at(j++));
        }
    }

    if (i < higher.size()) {
        resultSet.insert(resultSet.end(), higher.begin() + i, higher.end());
    } else if (j < lower.size()) {
        resultSet.insert(resultSet.end(), higher.begin() + j, lower.end());
    }
    return std::move(resultSet);
}

std::vector<Pair> Tiering::scan(int min_key, int max_key) {
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
    return std::move(resultSet);
}