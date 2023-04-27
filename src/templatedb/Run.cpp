//
// Created by Zhenhuan Wu on 4/26/23.
//

#include "Run.h"

Run::Run(int capacity) : capacity(capacity) {
    this->run = std::vector<std::vector<pair>>(capacity)
};
