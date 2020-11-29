#include "include/files.hpp"
#include <string>
#include <sys/stat.h>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
using namespace std;

const bool utils::files::pathExists(const string& name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

string& utils::files::parsePath(string &s)
{
    replace(s.begin(), s.end(), '\\', '/');
    return s;
}