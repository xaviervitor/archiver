#pragma once
#include <fstream>
#include <vector>
#include <string>

/* Separates s into n strings and returns a vector<string> with size n containing those strings. */
const std::vector<std::string> split(const std::string& s, const char& delimiter); 

/* Skips a line from current stream position indicator. */
void skipLine(std::ifstream* stream);
