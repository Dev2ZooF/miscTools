#include <string>
#include <iostream>

using namespace std;

int main() {
    string home("");
    if (getenv("HOME")) {
        home = getenv("HOME");
    } else if (getenv("HOMEPATH")) {
        string homedrive(getenv("HOMEDRIVE")), homepath(getenv("HOMEPATH"));
        home = homedrive + homepath;
    } else {
        cerr << "Error : no home path discovered..." << endl;
        exit(-1);
    }
    cout << home << endl;

    return 0;
}