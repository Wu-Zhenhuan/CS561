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
        for (auto &pair: this->tiers.at(i)) {
            if (pair.first == key) {
                return pair.second;
            } else if ((!pair.second.visible) && (pair.first <= key) && (key <= pair.first + pair.second.range)) {
                return templatedb::Value(false);
            }

        }
    }
    return templatedb::Value(false);
}


int Tiering::tierCapacity(int t) {
    return BUFFER_SIZE << t;
}

void Tiering::flushIn(std::tuple<run, int, int> buffer) {
    run bufferRun = std::get<0>(buffer);
    if (this->tiers.size() == 0) {
        this->newTier();
        this->tiers.push_back(std::move(bufferRun));
        this->mins.push_back(std::get<1>(buffer));
        this->maxs.push_back(std::get<2>(buffer));
    } else {
        run mergedRun = std::move(this->merge(bufferRun, this->tiers.at(0)));
        this->tiers.at(0) = std::move(mergedRun);
        for (int i = 0; i < this->currentLevel - 1; i++) {
            if (tiers.at(i).size() > tierCapacity(tiers.size())) {
                mergedRun = std::move(this->merge(this->tiers.at(i), this->tiers.at(i + 1)));
                this->tiers.at(i + 1) = std::move(mergedRun);

                this->mins.at(0) = this->tiers.at(0).front().first;
                this->maxs.at(0) = this->tiers.at(0).back().first;

                this->tiers.at(i) = run(this->tierCapacity(i));
            }
        }
        if (tiers.at(currentLevel - 1).size() > tierCapacity(currentLevel - 1)) {
            this->newTier();
            mergedRun = std::move(this->merge(
                    this->tiers.at(this->currentLevel - 2),
                    this->tiers.at(this->currentLevel - 1)));
            this->tiers.at(this->currentLevel - 1) = std::move(mergedRun);


            this->mins.at(this->currentLevel - 1) = this->tiers.at(this->currentLevel - 1).front().first;
            this->maxs.at(this->currentLevel - 1) = this->tiers.at(this->currentLevel - 1).back().first;

            this->tiers.at(this->currentLevel - 2) = run(this->tierCapacity(currentLevel - 2));
        }
    }

}


void Tiering::newTier() {
    std::vector<templatedb::Pair> level(this->tierCapacity(this->currentLevel));
    this->tiers.push_back(level);
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
        for (auto &pair: this->tiers.at(i)) {
            if ((pair.first >= min_key) && (pair.first <= max_key))
                levelResult.push_back(pair);
        }
        this->merge(resultSet, levelResult);
    }
    return std::move(resultSet);
}
std::vector<Pair> Tiering::scan() {
    std::vector<Pair> resultSet;
    for (int i = this->currentLevel - 1; i >= 0; i--) {
        std::vector<Pair> levelResult;
        for (auto &pair: this->tiers.at(i)) {
            levelResult.push_back(pair);
        }
        this->merge(resultSet, levelResult);
    }
    return std::move(resultSet);

}