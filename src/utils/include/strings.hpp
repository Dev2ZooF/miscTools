#ifndef STRINGS_HPP
#define STRINGS_HPP

#include <string>
#include <vector>

using namespace std;

namespace utils {
    namespace strings {
        const bool isStrEqual(const string& stra, const string& strb);
        const bool isStringANumber(const string& str);
        vector<string> split(const string& str, const char& separator);
    }
}

#endif // STRINGS_HPP