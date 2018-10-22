#include <iostream>
#include <vector>
#include <string>
#include "archiver.h"

/*void openArchive(Archive& archive, const std::string name) {
    archive = new Archive(name);
    std::cout << "\"" << name << "\" opened." << std::endl;
}

void closeArchive(Archive& archive) { 
    archive->name = ;
    std::cout << "\"" << archive->name << "\" closed." << std::endl;
}*/

int main(int argc, char **argv) {
    Archive* archive;
    int option;
    do {
        std::cout << "1. Open archive" << std::endl;
        std::cout << "2. Close archive" << std::endl;
        std::cout << "3. Create archive" << std::endl;
        std::cout << "4. List files in archive" << std::endl;
        std::cout << "5. Extract from archive" << std::endl;
        std::cout << "6. Insert in archive" << std::endl;
        std::cout << "7. Remove from archive" << std::endl;
        std::cout << "0. Leave" << std::endl;
        std::cin >> option;
        if (option == 1)  { // Open
        /*    std::string archiveName;
            std::cout << "Name of the archive:" << std::endl;
            std::cin >> archiveName;
            //test if this is actually an archive
            openArchive(archive, archiveName);*/
        } else if (option == 2) { // Close
            /*closeArchive(archive);*/
        } else if (option == 3) { // Create 
            std::vector<std::string> filenames;
            std::string userInput;
            std::cout << std::endl << "*Note: the created archive will stay opened until you close it." << std::endl; 
            std::cout << "Which files do you want in archive? (0 to finish.)" << std::endl;
            while (1) {
                std::cin >> userInput;
                if (userInput == "0") 
                    break;
                filenames.push_back(userInput);
                std::cout << "Got it." << std::endl;
            }
            std::string archiveName;
            std::cout << "Name of the archive:" << std::endl;
            std::cin >> archiveName;
            archive = new Archive(archiveName);
            archive->make(filenames);
            std::cout << "Archive created." << std::endl << std::endl;
        }else if (option == 4) { // List files 
            archive->listFiles();
        } else if (option == 5) { // Extract 
            std::string filename;
            std::cout << std::endl << "What file?" << std::endl;
            std::cin >> filename;
            archive->extractFile(filename, filename);
            std::cout << "File extracted." << std::endl << std::endl;
        } else if (option == 6) { // Insert 
            std::string filename;
            std::cout << std::endl << "What file?" << std::endl;
            std::cin >> filename;
            archive->insertFile(filename);
            std::cout << "File inserted." << std::endl << std::endl;
        } else if (option == 7) { // Remove
            std::string filename;
            std::cout << std::endl << "What file?" << std::endl;
            std::cin >> filename;
            archive->removeFile(filename);
            std::cout << "File removed." << std::endl << std::endl;
        }
    } while (option != 0);
    return 0;
}
