#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <climits>
#include <cstdio>
#include "archiver.h"
#include "util.h"
#include "index.h"

Archive::Archive(std::string name) {
    this->name = name;
};

void Archive::make(std::vector<std::string> filenames) {
    std::ofstream archive(this->name); 
    // Write index table header to archive
    {
        int fileRelativeAddress = 0; // Represents beginning of files, ignoring index table at the beginning of the archive.
        for (std::string filename : filenames) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary); 
            archive << filename << "|" << fileRelativeAddress << ";"; // Uses the end of last file to assign address to current file.
            fileRelativeAddress += ((int) file.tellg()); // Sets address of next file.
        }
        archive << std::endl;
    }
    // Write files to archive
    {    
        for (std::string filename : filenames) {
            std::ifstream file(filename, std::ios::binary); // Opens the file with the cursor in its end (std::ios::ate).
            char buffer;
            file.get(buffer);
            while (!file.eof()) {
                archive << buffer; // Transfers from file to archive byte to byte.
                file.get(buffer);
            }
        }
    }
}

void Archive::listFiles() {
    for (Index index : getIndexTable())
        std::cout << index.filename << std::endl;
    std::cout << std::endl;
}

void Archive::extractFile(std::string filename, std::string newFilename) {
    int fileAddress = -1;
    int fileSize;
    // Get file address of specified filename and its size
    {    
        std::vector<Index> indexTable = getIndexTable();
        for (std::vector<Index>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            if (it->filename == filename) {
                fileAddress = it->fileAddress;
                it++; // Moves iterator to next row of indexTable.
                fileSize = it->fileAddress - fileAddress;
                break;
            }
        }
    }
    if (fileAddress < 0) // File does not exist in archive
        return;
    std::ifstream archive(this->name, std::ios::binary);
    skipLine(&archive);
    archive.seekg(fileAddress, std::ios::cur); // Skip to address
    std::ofstream file(newFilename, std::ios::binary);
    char buffer;
    archive.get(buffer);
    for (int i = 0; !archive.eof(); i++) {
        if (i >= fileSize) 
            break;
        file << buffer; // Copies from archive to file byte to byte.
        archive.get(buffer);
    }
}

void Archive::insertFile(std::string filename) {
    std::vector<Index> indexTable = getIndexTable();
    // Check if file is already in archive
    {
        for (std::vector<Index>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            if (it->filename == filename) {
                std::cout << "Your file has the exact same name of one who is already in the archive you specified." << std::endl;
                return;
            }
        }
    }
    int newAddress;
    // Get position for the file to be inserted
    {
        std::ifstream archive(this->name, std::ios::binary);
        skipLine(&archive);
        int firstLineSize = (int) archive.tellg();
        archive.seekg(0, std::ios::end);
        newAddress = ((int) archive.tellg()) - firstLineSize;
    }
    indexTable.push_back(Index(filename, newAddress)); // Adds name and position of the file in index table
    std::string stringBuffer = "";
    // Puts index table at the beginning of std::string buffer
    {
        for (Index index : indexTable) {
            stringBuffer += index.filename;
            stringBuffer += "|";
            stringBuffer += std::to_string(index.fileAddress);
            stringBuffer += ";";
        }
        stringBuffer += "\n";
    }
    // Copies the entire archive to std::string buffer
    {
        char buffer;
        std::ifstream archive(this->name, std::ios::binary);
        skipLine(&archive);

        archive.get(buffer);
        while (!archive.eof()) {
            stringBuffer += buffer; // Copies from archive to stringBuffer byte to byte.
            archive.get(buffer);
        }
    }
    // Copies file to be added to std::string buffer
    {   
        char buffer;
        std::ifstream file(filename, std::ios::binary);
        file.get(buffer);
        while (!file.eof()) {
            stringBuffer += buffer; // Copies from file to stringBuffer byte to byte.
            file.get(buffer);
        }
    }
    std::ofstream archive(this->name, std::ios::binary);
    archive << stringBuffer;
}

void Archive::removeFile(std::string filename) {
    int fileAddress = -1;
    int nextFileAddress;
    std::vector<Index> indexTable = getIndexTable();
    // Get addresses of specified file and the file after it.
    {
        for (std::vector<Index>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            if (it->filename == filename) {
                fileAddress = it->fileAddress;
                it++;
                nextFileAddress = it->fileAddress;
                it--;
                break;
            }
        }
    }
    if (fileAddress < 0) return; // File not found in archive.

    // Reconstructs the index table header
    {
        int current;
        int next;

        int nextAddress = -1;
        for (std::vector<Index>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            // This executes every iteration after specified file is found. 
            if (nextAddress >= 0) { // nextAddress is only modified when specified file is found.
                current = it->fileAddress;
                it++; // Moves iterator to next item of indexTable.
                next = it->fileAddress;
                it--; // Moves iterator back.
                int fileSize = next - current;

                it->fileAddress = nextAddress;
                std::cout << nextAddress << std::endl;
                nextAddress += fileSize;
            }

            if (it->filename == filename) {
                // If the file to be deleted is the last there is no 
                // need to rearrange the indexTable. Erasing it will be enough.
                if (it == --indexTable.end()) {
                    indexTable.pop_back();
                    break;
                } else
                    nextAddress = it->fileAddress;
            }
        }
    }
    // Erases the file to be deleted from index table std::vector
    {    for (std::vector<Index>::iterator it = indexTable.begin(); it != indexTable.end(); it++)
        if (it->filename == filename)
            indexTable.erase(it);
    }
    std::string stringBuffer;
    // Puts the now fixed index table header at the beginning of std::string buffer
    {
        for (Index index : indexTable) {
            stringBuffer += index.filename;
            stringBuffer += "|";
            stringBuffer += std::to_string(index.fileAddress);
            stringBuffer += ";";
        }
        stringBuffer += "\n";
    }
    char buffer;
    // Copies entire file except for the file to be deleted to std::string buffer 
    {   
        std::ifstream archive(this->name, std::ios::binary);
        skipLine(&archive);

        // Copies archive to std::string buffer until the start of file to be deleted.
        archive.get(buffer);
        for (int i = 0; i < fileAddress; i++) {
            stringBuffer += buffer; // Copies from archive to stringBuffer byte to byte.
            archive.get(buffer);
        }

        // Only try to copy the rest of the file if the file to be deleted is not the last. (or its !(nextFileAddress <= 0))
        if (nextFileAddress > 0) {
            archive.seekg(nextFileAddress - fileAddress - 1, std::ios::cur);
            archive.get(buffer);
            while (!archive.eof()) {
                stringBuffer += buffer;
                archive.get(buffer);
            }
        }
    }
    std::ofstream newOverwritedArchive(this->name, std::ios::binary);
    newOverwritedArchive << stringBuffer;
}

std::vector<Index> Archive::getIndexTable() {
    std::ifstream file(this->name); // Opens file in reading mode.

    std::string indexTableHeader;
    file >> indexTableHeader; // Reads first line of the archive, that contains the index table header.

    std::vector<Index> indexTable;

    for (std::string tuple : split(indexTableHeader, ';')) {
        std::vector<std::string> data = split(tuple, '|');
        indexTable.push_back(Index(data.at(0), stoi(data.at(1))));
    }

    return indexTable;
}
