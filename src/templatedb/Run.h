//
// Created by Zhenhuan Wu on 4/26/23.
//

#ifndef TEMPLATEDB_RUN_H
#define TEMPLATEDB_RUN_H
#include "Value.h"

class Run {
public:
    explicit Run(int capacity);

private:
    int capacity;
    std::vector<std::vector<templatedb::Pair>> run;
};


#endif //TEMPLATEDB_RUN_H
