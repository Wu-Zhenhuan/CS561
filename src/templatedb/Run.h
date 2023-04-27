//
// Created by Zhenhuan Wu on 4/26/23.
//

#ifndef TEMPLATEDB_RUN_H
#define TEMPLATEDB_RUN_H
#include "db.hpp"

class Run {
public:
    explicit Run(int capacity);

private:
    int capacity;
    std::vector<std::vector<pair>> run;
};


#endif //TEMPLATEDB_RUN_H
