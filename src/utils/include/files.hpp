#ifndef FILES_HPP
#define FILES_HPP

#include <string>

using namespace std;

namespace utils {
    namespace files {

        // Check if specified file or directory exists
        const bool pathExists(const string& name);

        string& parsePath(string& s);
    }
}


#endif // FILES_HPP