#include <vector>
#include <string>
#include <fstream>

const std::vector<std::string> split(const std::string& s, const char& c) {
    std::string buffer{""};
    std::vector<std::string> v;

    for (auto n : s) {
        if (n != c)
            buffer += n;
        else if (n == c && buffer != "") {
            v.push_back(buffer);
            buffer = "";
        }
    }

    if (buffer != "")
        v.push_back(buffer);

    return v;
}

void skipLine(std::ifstream* archive) {
    std::string dummyLine;
    getline(*archive, dummyLine);
}
