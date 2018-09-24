#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <climits>
#include <cstdio>

using namespace std;

/* Same as split() from Java or explode() for PHP */
const vector<string> split(const string& s, const char& c) {
    string buff{""};
    vector<string> v;

    for (auto n : s) {
        if (n != c)
            buff += n;
        else if (n == c && buff != "") {
            v.push_back(buff);
            buff = "";
        }
    }

    if (buff != "")
        v.push_back(buff);

    return v;
}

/* Skips a line from current stream position indicator */
void skipLine(ifstream* archive) {
    string dummyLine;
    getline(*archive, dummyLine);
}

struct IndexTableRow {
    string filename;
    int fileAddress;

    IndexTableRow() {
    };

    IndexTableRow(string filename, int fileAddress) {
        this->filename = filename;
        this->fileAddress = fileAddress;
    };
};

void makeArchive(string archiveName, vector<string> filenames) {
    ofstream archive(archiveName); // Opens the destiny archive file.

    /* writing index table header to archive */
    {
        int fileAddressByte = 0; // File relative address. This ignores the index at the beginning of the archive.
        for (string filename : filenames) {
            ifstream file(filename, ios::ate | ios::binary); // Opens file at its end
            archive << filename << "|" << fileAddressByte << ";"; // Uses the end of last file to assign address to current file.
            fileAddressByte += ((int) file.tellg()); // Address of next file is after the end of this file.
        }
        archive << endl;
    }

    /* writing files to archive */
    {
        for (string filename : filenames) {
            ifstream file(filename, ios::binary); // Opens the file with the cursor in its end (ios::ate).

            char buffer;
            file.get(buffer);
            while (!file.eof()) {
                archive << buffer; // Transfers from file to archive byte to byte.
                file.get(buffer);
            }
        }
    }
}

vector<IndexTableRow> getIndexTable(string archiveName) {
    ifstream file(archiveName); // Opens file in reading mode.

    string indexTableHeader;
    file >> indexTableHeader; // Reads first line of the archive, that contains the index table header.

    vector<IndexTableRow> indexTable; // Actual index table structure.

    for (string tuple : split(indexTableHeader, ';')) {
        vector<string> data = split(tuple, '|');
        indexTable.push_back(IndexTableRow(data.at(0), stoi(data.at(1))));
    }

    return indexTable;
}

const vector<string> getFilenames(string archiveName) {
    vector<string> filenames;
    for (IndexTableRow index : getIndexTable(archiveName))
        filenames.push_back(index.filename);
    return filenames;
}

void extractFromArchive(string archiveName, string filename, string newFilename) {
    int fileAddress = -1;
    int fileSize;

    /* Get file address of specified filename and its size */
    {
        vector<IndexTableRow> indexTable = getIndexTable(archiveName);
        for (vector<IndexTableRow>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            if (it->filename == filename) {
                fileAddress = it->fileAddress;
                it++; // Moves iterator to next row of indexTable.
                fileSize = it->fileAddress - fileAddress;
                break;
            }
        }
        //fileSize = (fileSize < 0) ? INT_MAX : fileSize;
    }

    if (fileAddress < 0) return; // File does not exist in archive

    ifstream archive(archiveName, ios::binary);

    skipLine(&archive);

    archive.seekg(fileAddress, ios::cur); // Skip to address

    ofstream file(newFilename, ios::binary);

    char buffer;
    archive.get(buffer);
    for (int i = 0; !archive.eof(); i++) {
        if (i >= fileSize) break;
        file << buffer; // Copies from archive to file byte to byte.
        archive.get(buffer);
    }
}

void insertFileToArchive(string archiveName, string filename) {
    vector<IndexTableRow> indexTable = getIndexTable(archiveName);

    /* Check if file is already in archive */
    {
        for (vector<IndexTableRow>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            if (it->filename == filename) {
                cout << "Your file is already in the archive you specified." << endl;
                return;
            }
        }
    }

    int newAddress;
    /* Get position for the file to be inserted */
    {
        ifstream archive(archiveName, ios::binary);
        skipLine(&archive);
        int firstLineSize = (int) archive.tellg();
        archive.seekg(0, ios::end);
        newAddress = ((int) archive.tellg()) - firstLineSize;
    }

    indexTable.push_back(IndexTableRow(filename, newAddress)); // Adds name and position of the file in index table

    string stringBuffer = "";
    /* Puts index table at the beginning of string buffer */
    {
        for (IndexTableRow index : indexTable) {
            stringBuffer += index.filename;
            stringBuffer += "|";
            stringBuffer += to_string(index.fileAddress);
            stringBuffer += ";";
        }
        stringBuffer += "\n";
    }

    /* Copies the entire archive to string buffer */
    {
        char buffer;
        ifstream archive(archiveName, ios::binary);
        skipLine(&archive);

        archive.get(buffer);
        while (!archive.eof()) {
            stringBuffer += buffer; // Copies from archive to stringBuffer byte to byte.
            archive.get(buffer);
        }
    }

    /* Copies file to be added to string buffer */
    {
        char buffer;
        ifstream file(filename, ios::binary);
        file.get(buffer);
        while (!file.eof()) {
            stringBuffer += buffer; // Copies from file to stringBuffer byte to byte.
            file.get(buffer);
        }
    }

    ofstream archive(archiveName, ios::binary);
    archive << stringBuffer;
}

void removeFileFromArchive(string archiveName, string filename) {

    int fileAddress = -1;
    int nextFileAddress;
    vector<IndexTableRow> indexTable = getIndexTable(archiveName);
    /* Get addresses of specified file and the file after it. */
    {
        for (vector<IndexTableRow>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            if (it->filename == filename) {
                fileAddress = it->fileAddress;
                it++; // Moves iterator to next item of indexTable.
                nextFileAddress = it->fileAddress;
                it--;
                break;
            }
        }
    }

    if (fileAddress < 0) return; // File not found in archive.

    /* Reconstructs the index table header */
    {
        int current;
        int next;

        int nextAddress = -1;
        for (vector<IndexTableRow>::iterator it = indexTable.begin(); it != indexTable.end(); it++) {
            // This executes every iteration after specified file is found. 
            if (nextAddress >= 0) { // nextAddress is only modified when specified file is found.
                current = it->fileAddress;
                it++; // Moves iterator to next item of indexTable.
                next = it->fileAddress;
                it--; // Moves iterator back.
                int fileSize = next - current;

                it->fileAddress = nextAddress;
                cout << nextAddress << endl;
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

    /* Erases the file to be deleted from index table vector */
    {
        for (vector<IndexTableRow>::iterator it = indexTable.begin(); it != indexTable.end(); it++)
            if (it->filename == filename)
                indexTable.erase(it);
    }

    string stringBuffer;
    /* Puts the now fixed index table header at the beginning of string buffer */
    {
        for (IndexTableRow index : indexTable) {
            stringBuffer += index.filename;
            stringBuffer += "|";
            stringBuffer += to_string(index.fileAddress);
            stringBuffer += ";";
        }
        stringBuffer += "\n";
    }

    char buffer;
    /* Copies entire file except for the file to be deleted to string buffer */
    {
        ifstream archive(archiveName, ios::binary);
        skipLine(&archive);

        // Copies archive to string buffer until the start of file to be deleted.
        archive.get(buffer);
        for (int i = 0; i < fileAddress; i++) {
            stringBuffer += buffer; // Copies from archive to stringBuffer byte to byte.
            archive.get(buffer);
        }

        // Only try to copy the rest of the file if the file to be deleted is not the last. (or its !(nextFileAddress <= 0))
        if (nextFileAddress > 0) {
            archive.seekg(nextFileAddress - fileAddress - 1, ios::cur);
            archive.get(buffer);
            while (!archive.eof()) {
                stringBuffer += buffer;
                archive.get(buffer);
            }
        }
    }

    ofstream newOverwritedArchive(archiveName, ios::binary);
    newOverwritedArchive << stringBuffer;
}

int main(int argc, char **argv) {
    int opcao;
    do {
        cout << "0. Leave" << endl;
        cout << "1. Create archive" << endl;
        cout << "2. List files in archive" << endl;
        cout << "3. Extract from archive" << endl;
        cout << "4. Insert in archive" << endl;
        cout << "5. Remove from archive" << endl;

        cin >> opcao;

        if (opcao == 1) {
            vector<string> filenames;
            string userInput;
            cout << endl << "Which files do you want in archive? (0 to finish.)" << endl;
            while (1) {
                cin >> userInput;
                if (userInput == "0") break;
                filenames.push_back(userInput);
                cout << "Got it." << endl;
            }
            string archiveName;
            cout << "Name of the archive:" << endl;
            cin >> archiveName;

            makeArchive(archiveName, filenames);

            cout << "Archive created." << endl << endl;
        } else if (opcao == 2) {
            string userInput;
            cout << endl << "What archive?" << endl;
            cin >> userInput;

            for (string index : getFilenames(userInput))
                cout << index << endl;
            cout << endl;
        } else if (opcao == 3) {
            string archive;
            string filename;
            cout << endl << "What archive and file?" << endl << "Archive: ";
            cin >> archive;
            cout << "File: ";
            cin >> filename;
            extractFromArchive(archive, filename, filename);
            cout << "File extracted." << endl << endl;
        } else if (opcao == 4) {
            string archive;
            string filename;
            cout << endl << "What archive and file?" << endl << "Archive: ";
            cin >> archive;
            cout << "File: ";
            cin >> filename;
            insertFileToArchive(archive, filename);
            cout << "File inserted." << endl << endl;
        } else if (opcao == 5) {
            string archive;
            string filename;
            cout << endl << "What archive and file?" << endl << "Archive: ";
            cin >> archive;
            cout << "File: ";
            cin >> filename;
            removeFileFromArchive(archive, filename);
            cout << "File removed." << endl << endl;
        }
    } while (opcao != 0);

    return 0;
}
