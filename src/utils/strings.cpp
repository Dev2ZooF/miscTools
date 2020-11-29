#include "include/strings.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

const bool utils::strings::isStrEqual(const string& stra, const string& strb)
{
    return (strcmp(stra.c_str(), strb.c_str()) == 0);
}

const bool utils::strings::isStringANumber(const string& str)
{
    return !str.empty() && find_if(str.begin(), 
        str.end(), [](unsigned char c) { return !std::isdigit(c); }) == str.end();
}

vector<string> utils::strings::split(const string& str, const char& separator)
{
    vector<string> elements;
    stringstream ss(str);
    string subStr;
    while (getline(ss, subStr, separator))
    {
        elements.push_back(subStr);
    }

    return elements;
}