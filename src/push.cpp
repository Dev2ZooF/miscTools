#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sys/stat.h>
#include <experimental/filesystem>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include "push/Project.hpp"

namespace fs = std::experimental::filesystem;
using namespace rapidjson;
using namespace std;

enum Action
{
    act_nothingToDo,
    act_help,
    act_init,
    act_show,
    act_push
};

const bool pathExists(const string& name);
const bool isStrEqual(const string& stra, const string& strb);
const bool isStringANumber(const string& str);
const bool mapKeyExist(map<string, vector<Project>>& elMap, const string& elKey);
const Action handleArgs(const vector<string>& args);

void printHelp();
void initialize(const string& CONFIG_FILE_PATH, string& projPath);
void showProjects(const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects);
void pushProjects(const vector<string>& args, const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects);
void scanProjects(const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects);
void ntd();

string& parsePath(string& s);
void searchForRepos(vector<string>& repoPathes, const string& pathToScan, const int& projPathSize);
vector<string> split(const string& str, char separator);

int main(int argc, char **argv)
{
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

    const string CONFIG_FILE_PATH(parsePath(home) + "/Dev2ZooF/config/push.json");
    string projPath("");
    vector<Project> projects;
    vector<string> args;

    Action action(act_nothingToDo);

    if (!pathExists(CONFIG_FILE_PATH))
    {
        action = act_init;
    }
    else if (argc > 1)
    {
        args = vector<string>(argv + 1, argv + argc);
        action = handleArgs(args);
    }

    switch (action)
    {
    case act_help:
        printHelp();
        break;
    // if no config file found or --init arg is passed, have to create a new config file
    case act_init:
        initialize(CONFIG_FILE_PATH, projPath);
        break;
    case act_show:
        showProjects(CONFIG_FILE_PATH, projPath, projects);
        break;
    case act_push:
        pushProjects(args, CONFIG_FILE_PATH, projPath, projects);
        break;
    default:
        ntd();
    }
}

const bool pathExists(const string& name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

const bool isStrEqual(const string& stra, const string& strb)
{
    return (strcmp(stra.c_str(), strb.c_str()) == 0);
}

const bool isStringANumber(const string& str)
{
    return !str.empty() && std::find_if(str.begin(), 
        str.end(), [](unsigned char c) { return !std::isdigit(c); }) == str.end();
}

const bool mapKeyExist(map<string, vector<Project>>& elMap, const string& elKey)
{
    map<string, vector<Project>>::iterator it = elMap.find(elKey);
    return it != elMap.end();
}

const Action handleArgs(const vector<string>& args)
{
    const string ARG_HELP("--help");
    const string ARG_INIT("--init");
    const string ARG_SHOW("--show");

    if (isStrEqual(args[0], ARG_HELP))
    {
        return act_help;
    }

    if (args.size() == 1 && isStrEqual(args[0], ARG_INIT))
    {
        return act_init;
    }

    if (args.size() == 1 && isStrEqual(args[0], ARG_SHOW))
    {
        return act_show;
    }

    return act_push;
}

void printHelp()
{
    // TODO : print help
    cout << "I will print help one day!" << endl;
}

void initialize(const string& CONFIG_FILE_PATH, string& projPath)
{
    bool init = true;

    // if a previous config file is found, ask for confirmation
    if (pathExists(CONFIG_FILE_PATH))
    {
        cout << "Previous config file found in " << CONFIG_FILE_PATH << "." << endl;

        // continue to ask until user type y or n (case non-sensitive)
        string answer("");
        while (!isStrEqual(answer, "Y") && !isStrEqual(answer, "N"))
        {
            cout << "Do you want to erase it ? (y/n)" << endl;
            getline(cin, answer);
            transform(answer.begin(), answer.end(), answer.begin(), ::toupper);
        }

        if (answer[0] == 'Y')
        {
            if (fs::remove(CONFIG_FILE_PATH) == 0)
            {
                init = false;
                cerr << "Error : cannot remove the existing config file." << endl;
                exit(-1);
            }
            else
            {
                cout << "Config file successfuly removed." << endl;
            }
        }
        else
        {
            init = false;
            cout << "Initialization canceled by user." << endl;
        }
    }
    else
    {
        cout << "No previous config file found." << endl;
    }

    if (init)
    {
        cout << "Initializing config file in " << CONFIG_FILE_PATH << "." << endl;

        // Create folders which will hold the config file if it doesn't already exists
        vector<string> splittedConfigPath = split(CONFIG_FILE_PATH, '/');
        if (!splittedConfigPath.empty() && splittedConfigPath.size() > 2) {
            string pathToTest(splittedConfigPath[0]);
            for (vector<string>::iterator it = splittedConfigPath.begin() + 1; it != splittedConfigPath.end() - 1; it++) {
                pathToTest += '/' + *it;
                if (!pathExists(pathToTest)) {
                    if (!fs::create_directories(pathToTest)) {
                        cerr << "Error : cannot create dir " << pathToTest << "which will contain the config file." << endl;
                        exit(-1);
                    }
                }
            }
        }

        Document d;
        string answer("");
        bool userAnswered = false;
        bool userConfirmed = false;

        // Create the config file and asks the user to type path to repositories...
        if (FILE *fp = fopen(CONFIG_FILE_PATH.c_str(), "wb"))
        {

            do
            {
                do
                {
                    cout << "Enter path to the root folder of your git projects :" << endl;
                    getline(cin, projPath);
                } while (isStrEqual(projPath, ""));

                projPath = parsePath(projPath);

                // continue to ask until user type y or n (case non-sensitive)
                do
                {
                    cout << "Do you confirm this path : \"" << projPath << "\" ? (y/n or a to abort)" << endl;
                    getline(cin, answer);
                    transform(answer.begin(), answer.end(), answer.begin(), ::toupper);
                } while (!isStrEqual(answer, "Y") && !isStrEqual(answer, "N") && !isStrEqual(answer, "A"));

                if (answer[0] == 'Y')
                {
                    userAnswered = true;
                    userConfirmed = true;
                }

                if (answer[0] == 'A')
                {
                    userAnswered = true;
                }

            } while (!userAnswered);

            if (userConfirmed)
            {
                cout << "Writing path in config file..." << endl;

                d.SetObject();
                d.AddMember("projPath", projPath, d.GetAllocator());

                char writeBuffer[65536];
                FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

                PrettyWriter<FileWriteStream> writer(os);
                d.Accept(writer);

                cout << "Done writing." << endl;
            }

            fclose(fp);
        }
        else
        {
            cerr << "Error : can't write in " << CONFIG_FILE_PATH << "." << endl;
            exit(-1);
        }

        if (!userConfirmed)
        {
            cout << "Initialization canceled by user." << endl;
            if (fs::remove(CONFIG_FILE_PATH) == 0)
            {
                init = false;
                cerr << "Error : cannot remove the initialized config file." << endl;
                exit(-1);
            }
        }
    }
}

void showProjects(const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects) {
    scanProjects(CONFIG_FILE_PATH, projPath, projects);
    if (!projects.empty()) {
        cout << "Found those projects in " << projPath << " :" << endl;
        for (Project project : projects) {
            cout << " - Project \"" << project.getName() << "\" from " << project.getPath() << endl;
        }
    } else {
        cout << "No projects found in " << projPath << "." << endl;
    }
}

void pushProjects(const vector<string>& args, const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects)
{
    map<string, vector<Project>> projectsToPush;
    vector<string> ignoredArgs;

    scanProjects(CONFIG_FILE_PATH, projPath, projects);

    for (string arg : args)
    {
        bool isArgAProjectName(false);
        int i = 0;
        vector<int> idsToErase;
        for (vector<Project>::iterator it = projects.begin(); it != projects.end(); it++, i++)
        {
            if (isStrEqual(arg, it->getName()))
            {
                isArgAProjectName = true;
                idsToErase.push_back(i);
                if (mapKeyExist(projectsToPush, arg))
                {
                    projectsToPush.at(arg).push_back(*it);
                }
                else
                {
                    vector<Project> argProjects;
                    argProjects.push_back(*it);
                    projectsToPush.insert(make_pair(arg, argProjects));
                }
            }
        }
        if (isArgAProjectName)
        {
            for (vector<int>::reverse_iterator it = idsToErase.rbegin(); it != idsToErase.rend(); it++)
            {
                projects.erase(projects.begin() + *it);
            }
        }
        else
        {
            ignoredArgs.push_back(arg);
        }
    }

    if (!ignoredArgs.empty())
    {
        cout << "Ignoring ";
        for (string arg : ignoredArgs)
        {
            cout << "\"" << arg << "\" ";
        }
        cout << "argument" << (ignoredArgs.size() > 1 ? "s." : ".") << endl;
    }

    for (const auto &elem : projectsToPush)
    {
        string projectName = elem.first;
        int nbOfProjects = elem.second.size();
        if (nbOfProjects > 1)
        {
            string answer;
            int i(0);
            do
            {
                cout << "There are " << nbOfProjects << " projects with the name \"" << projectName << "\"." << endl;
                cout << "Select the project you want to push in the following list :" << endl;
                cout << "(0) : if you don't want to push at all " << projectName << endl;
                i = 1;
                for (auto it = elem.second.begin(); it != elem.second.end(); it++, i++)
                {
                    cout << '(' << i << ") : \"" << projectName << "\" from " << it->getPath() << endl;
                }
                cout << "(a) : if you want to push all at once" << endl;
                getline(cin, answer);
                transform(answer.begin(), answer.end(), answer.begin(), ::toupper);
            } while ((!isStringANumber(answer) && !isStrEqual(answer, "A")) || (isStringANumber(answer) && (stoi(answer) < 0 || stoi(answer) > i - 1)));

            if (!isStrEqual(answer, "A"))
            {
                int projIdx = stoi(answer) - 1;
                vector<Project> projVector;
                if (projIdx >= 0)
                {
                    projVector.push_back(*(elem.second.begin() + projIdx));
                }
                projectsToPush.at(projectName) = projVector;
            }
        }

        for (auto project : elem.second)
        {
            cout << "Pushing \"" << projectName << "\" from " << project.getPath() << " ..." << endl;
            string cmd("bash " + projPath + project.getPath() + "/push.sh");
            system(cmd.c_str());
            cout << "Done pushing." << endl;
        }

    }
    cout << "All push is done!" << endl;

}

void scanProjects(const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects)
{
    vector<string> repoPathes;

    if (FILE *fp = fopen(CONFIG_FILE_PATH.c_str(), "rb"))
    {

        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));

        Document d;
        d.ParseStream(is);

        projPath = d["projPath"].GetString();

        fclose(fp);

        cout << "Scanning " << projPath << " ..." << endl;

        searchForRepos(repoPathes, projPath, projPath.size());
        cout << "Done scanning." << endl;

        for (const string &repo : repoPathes)
        {
            vector<string> splittedRepo = split(repo, '/');
            string firstFolder(splittedRepo[1]);
            string lastFolder(splittedRepo[splittedRepo.size() - 1]);
            Project project(lastFolder, repo);
            projects.push_back(project);
        }

    }
    else
    {
        cerr << "Error : scan failed, the config file cannot be read." << endl;
        exit(-1);
    }
}

void ntd()
{
    // TODO : print hint
    cout << "Nothing to do." << endl;
}

string& parsePath(string &s)
{
    replace(s.begin(), s.end(), '\\', '/');
    return s;
}

void searchForRepos(vector<string>& repoPathes, const string& pathToScan, const int& projPathSize)
{
    if (pathExists(pathToScan + "/.git"))
    {
        if (pathExists(pathToScan + "/push.sh"))
        {
            string s = pathToScan.substr(projPathSize);
            repoPathes.push_back(parsePath(s));
        }
    }
    else
    {
        for (const auto &entry : fs::directory_iterator(pathToScan))
        {
            if (fs::is_directory(entry))
            {
                searchForRepos(repoPathes, entry.path().u8string(), projPathSize);
            }
        }
    }
}

vector<string> split(const string& str, char separator)
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