#pragma once 

#include <string>
#include "index.h"

class Archive {
private:    
    std::string name;
    std::vector<Index> getIndexTable();
public: 
    Archive(std::string name) 
        : name(name)
    {
    }

    void make(std::vector<std::string> filenames);
    void listFiles();
    void extractFile(std::string filename, std::string newFilename); 
    void insertFile(std::string filename);
    void removeFile(std::string filename);
};
