//
// Created by Zhenhuan Wu on 4/25/23.
//

#include "Levelling.h"
#include <iostream>

Levelling::Levelling() : currentLevel(0) {}

templatedb::Value Levelling::get(int key) {
    for (int i = 0; i < currentLevel - 1; i++) {
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
    return templatedb::Value(false);
}


int Levelling::levelCapacity(int l) {
    return BUFFER_SIZE << l;
}

void Levelling::flushIn(std::tuple<run, int, int> buffer) {
    run bufferRun = std::get<0>(buffer);
    if (this->levels.size() == 0) {
        this->newLevel();
        this->levels.push_back(std::move(bufferRun));
        this->mins.push_back(std::get<1>(buffer));
        this->maxs.push_back(std::get<2>(buffer));
    } else {
        run mergedRun = std::move(this->merge(bufferRun, this->levels.at(0)));
        this->levels.at(0) = std::move(mergedRun);
        this->mins.at(0) = this->levels.at(0).front().first;
        this->maxs.at(0) = this->levels.at(0).back().first;

        for (int i = 0; i < this->currentLevel - 1; i++) {
            if (levels.at(i).size() > levelCapacity(levels.size())) {
                mergedRun = std::move(this->merge(this->levels.at(i), this->levels.at(i+1)));
                this->levels.at(i+1) = std::move(mergedRun);
                this->levels.at(i) = run(this->levelCapacity(i));


                this->mins.at(i+1) = this->levels.at(0).front().first;
                this->maxs.at(i+1) = this->levels.at(0).back().first;
            }
        }
        if (levels.at(currentLevel - 1).size() > levelCapacity(currentLevel - 1)) {
            this->newLevel();
            mergedRun = std::move(this->merge(
                    this->levels.at(this->currentLevel - 2),
                    this->levels.at(this->currentLevel - 1)));
            this->levels.at(this->currentLevel - 1) = std::move(mergedRun);
            this->levels.at(this->currentLevel - 2) = run(this->levelCapacity(currentLevel - 2));


            this->mins.at(this->currentLevel - 1) = this->levels.at(0).front().first;
            this->maxs.at(this->currentLevel - 1) = this->levels.at(0).back().first;
        }
    }

    // create a bloom filter for the new run
//    BF::BloomFilter bf(bufferRun.size(), 10);
//
//    // insert keys from the new run into the bloom filter
//    for (auto& pair: bufferRun) {
//        bf.program(std::to_string(pair.first));
//    }
//
//    // add the bloom filter to the vector of bloom filters
//    this->bloomFilters.push_back(bf);
//
//    // add the new run to the current level
//    this->levels.at(this->currentLevel - 1) = this->merge(bufferRun, this->levels.at(this->currentLevel - 1));
}
//void Levelling::flushIn(std::tuple<run, int, int> buffer) {
//    run bufferRun = std::get<0>(buffer);
//    if (this->levels.size()==0) {
//        this->newLevel();
//        this->levels.push_back(bufferRun);
//        this->mins.push_back(std::get<1>(buffer));
//        this->maxs.push_back(std::get<2>(buffer));
//    }
//    else if (levels.at(levels.size()-1).size() > tierCapacity(levels.size())) {
//        this->currentLevel++;
//    }
//    // create a bloom filter for the new run
//    BF::BloomFilter bf(100, 10);
////
////    // insert keys from the new run into the bloom filter
////    for (auto& pair: bufferRun) {
////        bf.program(std::to_string(pair.first));
////    }
////
////    // add the bloom filter to the vector of bloom filters
////    this->bloomFilters.push_back(bf);
//
//    // add the new run to the current level
//    this->levels.at(this->currentLevel - 1) = this->merge(bufferRun, this->levels.at(this->currentLevel - 1));
//}


void Levelling::newLevel() {
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
run Levelling::merge(run higher, run lower) {

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

std::vector<Pair> Levelling::scan(int min_key, int max_key) {
    //std::cout<<"************************************"<<std::endl;
    std::vector<Pair> resultSet;
    // std::cout<<min_key<<", "<<max_key<<std::endl;
    for (int i = 0; i < currentLevel - 1; i++) {
        if ((this->mins.at(i) > max_key) || this->maxs.at(i) < min_key) {
            continue;
        }
        std::vector<Pair> levelResult;
        for (auto &pair: this->levels.at(i)) {
            if ((pair.first >= min_key) && (pair.first <= max_key))
                levelResult.push_back(pair);

        }
        this->merge(resultSet, levelResult);
    }
    return std::move(resultSet);
}
std::vector<Pair> Levelling::scan() {
    std::vector<Pair> resultSet;
    for (int i = 0; i < currentLevel - 1; i++) {
        std::vector<Pair> levelResult;
        for (auto &pair: this->levels.at(i)) {
            levelResult.push_back(pair);
        }
        this->merge(resultSet, levelResult);
    }
    return std::move(resultSet);

}