//
// Created by Zhenhuan Wu on 4/25/23.
//

#include "Levelling.h"
// Get the value of a given key
        templatedb::Value Levelling::get(int key) {
    // Iterate through all levels from top to bottom
    for (int i = this->currentLevel - 1; i >= 0; i--) {
        // Check if the key falls within the min/max range of the current level
        if ((this->mins.at(i) > key) || this->maxs.at(i) < key) {
            continue; // If not, skip to the next level
        }
        bool mayContainKey = false;
        // Check if any of the bloom filters at this level may contain the key
        for (auto& bf : this->bloomFilters) {
            if (bf.query(std::to_string(key))) {
                mayContainKey = true;
                break;
            }
        }
        if (!mayContainKey) {
            continue; // If none of the bloom filters may contain the key, skip to the next level
        }
        // Check each key-value pair in the current level, and return the value if the key matches
        for (auto &pair: this->levels.at(i)) {
            if (pair.first == key) {
                return pair.second;
            } else if ((!pair.second.visible) && (pair.first <= key) && (key <= pair.first + pair.second.range)) {
                return templatedb::Value(false); // If the key is in a range tombstone, return false
            }

        }
    }
    return NULL; // If the key is not found in any level, return NULL
}


// This function calculates the capacity of a level given its level number.
// It uses a left shift operation to multiply BUFFER_SIZE by 2^l.
int Levelling::levelCapacity(int l) {
    return BUFFER_SIZE << l;
}

// This function flushes a buffer into the levelling data structure.
// It merges the buffer with the top level, and rebalances the levels if necessary.
// It also creates a bloom filter for the new run, and adds it to the vector of bloom filters.
void Levelling::flushIn(std::tuple<run, int, int> buffer) {
    // Get the buffer run from the input tuple.
    run bufferRun = std::get<0>(buffer);

    // If the levels vector is empty, create a new level and add the buffer run to it.
    if (this->levels.size() == 0) {
        this->newLevel();
        this->levels.push_back(std::move(bufferRun));
        this->mins.push_back(std::get<1>(buffer));
        this->maxs.push_back(std::get<2>(buffer));
    }
        // If the levels vector is not empty, merge the buffer with the top level,
        // and rebalance the levels if necessary.
    else {
        // Merge the buffer with the top level.
        run mergedRun = std::move(this->merge(bufferRun, this->levels.at(0)));
        this->levels.at(0) = std::move(mergedRun);

        // Rebalance the levels.
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

    // Create a bloom filter for the new run.
    BF::BloomFilter bf(bufferRun.size(), 10);

    // Insert keys from the new run into the bloom filter.
    for (auto& pair: bufferRun) {
        bf.program(std::to_string(pair.first));
    }

    // Add the bloom filter to the vector of bloom filters.
    this->bloomFilters.push_back(bf);

    // Add the new run to the current level.
    this->levels.at(this->currentLevel - 1) = this->merge(bufferRun, this->levels.at(this->currentLevel - 1));
}

// This function creates a new level.
// It creates a vector of Pair objects with the capacity calculated by levelCapacity,
// and adds it to the levels vector.
void Levelling::newLevel() {
    std::vector<templatedb::Pair> level(this->levelCapacity(this->currentLevel));
    this->levels.push_back(level);
    this->currentLevel++;
}



/**
 * A merge function that deals with duplicate elements by keeping the higher one
 * @param higher The run with higher keys
 * @param lower The run with lower keys
 * @return A merged run with duplicates resolved
 */
run Levelling::merge(run higher, run lower) {

    // Initialize variables
    size_t i = 0;
    size_t j = 0;
    run resultSet;
    resultSet.reserve(higher.size() + lower.size());

    // Merge the two runs, resolving duplicates
    while (i < higher.size() && j < lower.size()) {
        if (higher.at(i).first == lower.at(j).first) {
            // Keep the higher of the two
            resultSet.push_back(higher.at(i++));
            if (lower.at(j).second.visible || lower.at(j).second.range <= 0) {
                // If it's not a range tombstone, move on to the next element in lower
                j++;
            }
        } else if (higher.at(i).first < lower.at(j).first) {
            // Add the element from higher to the result set
            resultSet.push_back(higher.at(i++));
        } else {
            // Add the element from lower to the result set
            resultSet.push_back(lower.at(j++));
        }
    }

    // Add any remaining elements to the result set
    if (i < higher.size()) {
        resultSet.insert(resultSet.end(), higher.begin() + i, higher.end());
    } else if (j < lower.size()) {
        resultSet.insert(resultSet.end(), higher.begin() + j, lower.end());
    }
    return std::move(resultSet);
}

/**
 * Scans the levelling data structure for all key-value pairs with keys between min_key and max_key (inclusive)
 * @param min_key The minimum key to scan for
 * @param max_key The maximum key to scan for
 * @return A vector of key-value pairs that match the scan criteria
 */
std::vector<Pair> Levelling::scan(int min_key, int max_key) {
    std::vector<Pair> resultSet;
    for (int i = this->currentLevel - 1; i >= 0; i--) {
        if ((this->mins.at(i) > max_key) || this->maxs.at(i) < min_key) {
            // Skip levels that don't contain any keys in the scan range
            continue;
        }
        std::vector<Pair> levelResult;
        for (auto &pair: this->levels.at(i)) {
            levelResult.push_back(pair);
        }
        // Merge the results from the current level with the overall result set
        this->merge(resultSet, levelResult);
    }
    return std::move(resultSet);
}

/**
 * Scans the entire levelling data structure for all key-value pairs
 * @return A vector of all key-value pairs in the data structure
 */
std::vector<Pair> Levelling::scan() {
    std::vector<Pair> resultSet;
    for (int i = this->currentLevel - 1; i >= 0; i--) {
        std::vector<Pair> levelResult;
        for (auto &pair: this->levels.at(i)) {
            levelResult.push_back(pair);
        }
        // Merge the results from the current level with the overall result set
        this->merge(resultSet, levelResult);
    }
    return std::move(resultSet);
}
