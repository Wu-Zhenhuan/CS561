
// Header guard to prevent multiple inclusion of the same header
#ifndef TEMPLATEDB_LEVELLING_H
#define TEMPLATEDB_LEVELLING_H

#include "Value.h"
#include "BloomFilter/BloomFilter.h"

// Alias for templatedb::Pair & templatedb::run
typedef templatedb::Pair Pair;
typedef templatedb::run run;

// Class definition for Levelling
class Levelling {
private:
    int currentLevel; // The current level of the levelling data structure
    std::vector<run> levels; // Vector of runs stored in each level of the data structure
    std::vector<BF::BloomFilter> bloomFilters;// Vector of bloom filters for each run in the data structure
    std::vector<int> mins, maxs; // Minimum and maximum keys stored in each run

public:
// Constructor
    explicit Levelling();
    // Methods for accessing and modifying data in the levelling data structure
    templatedb::Value get(int key);           // Get the value associated with a given key
    void put(int key, templatedb::Value val); // Insert or update a key-value pair in the data structure
    std::vector<Pair> scan();                // Scan the entire data structure and return all key-value pairs
    std::vector<Pair> scan(int min_key, int max_key); // Scan a specific range of keys and return matching key-value pairs
    void del(int key);                       // Delete the key-value pair associated with a given key
    void del(int min_key, int max_key);       // Delete all key-value pairs in a given key range
    void flushIn(std::tuple<run, int, int> buffer); // Flush a buffer of key-value pairs to the data structure

// Utility methods for managing the levels and runs in the levelling data structure
    int levelCapacity(int l);                 // Compute the capacity of a level based on its level number
    run merge(run higher, run lower);         // Merge two runs, resolving conflicts by keeping the higher key-value pair
    void newLevel();                          // Add a new level to the levelling data structure

};


#endif //TEMPLATEDB_LEVELLING_H
