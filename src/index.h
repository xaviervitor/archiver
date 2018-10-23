#pragma once 

#include <string>

struct Index {
    
    std::string filename;
    int fileAddress;

    Index() {
    };

    Index(std::string filename, int fileAddress) 
        : filename(filename), fileAddress(fileAddress) 
    {
    }
};
