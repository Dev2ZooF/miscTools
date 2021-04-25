#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <experimental/filesystem>
#include "utils/include/files.hpp"
#include "utils/include/strings.hpp"
#include "push/Project.hpp"

namespace fs = std::experimental::filesystem;
using namespace rapidjson;
using namespace std;


// Handle actions depending on command arguments
enum Action
{
    act_nothingToDo,
    act_help,
    act_init,
    act_show,
    act_push
};

// Check if specified key exists in specified map
const bool mapKeyExist(map<string, vector<Project>>& elMap, const string& elKey);

// Defines what to do by reading passed arguments
const Action handleArgs(const vector<string>& args);

// --help : display help
void printHelp();
// --init (or no config file found) : (re)init config file
void initialize(const string& CONFIG_FILE_PATH, string& projPath);
// --show : display all repositories found in project path
void showProjects(const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects);
// Calls "push.sh" of all specified repositories
void pushProjects(const vector<string>& args, const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects);
// Read config file and initialize the search of repositories
void scanProjects(const string& CONFIG_FILE_PATH, string& projPath, vector<Project>& projects);
// Recursive function, stores in repoPathes all directories in pathToScan which contains ".git" folder and "push.sh" file.
void searchForRepos(vector<string>& repoPathes, const string& pathToScan, const int& projPathSize);
// Called when nothing to do.
void ntd();

int main(int argc, char **argv)
{
    // Choose a path to store the config file depending on which global vars are defined on the system
    string configHome("");
    if (getenv("HOME")) {
        configHome = getenv("HOME");
        configHome.append("/.config");
    } else if (getenv("APPDATA")) {
        configHome = getenv("APPDATA");
    } else if (getenv("HOMEDRIVE") && getenv("HOMEPATH")) {
        string homedrive(getenv("HOMEDRIVE")), homepath(getenv("HOMEPATH"));
        configHome = homedrive + homepath;
    } else {
        cerr << "Error : no home path discovered..." << endl;
        exit(-1);
    }
    const string CONFIG_FILE_PATH(utils::files::parsePath(configHome) + "/Dev2ZooF/push/config.json");

    string projPath("");
    vector<Project> projects;
    vector<string> args;

    Action action(act_nothingToDo);

    // You have to have a config file in order to use this program
    if (!utils::files::pathExists(CONFIG_FILE_PATH))
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

const bool mapKeyExist(map<string, vector<Project>>& elMap, const string& elKey)
{
    map<string, vector<Project>>::iterator it = elMap.find(elKey);
    return it != elMap.end();
}

const Action handleArgs(const vector<string>& args)
{
    // List of possible args
    const string ARG_HELP("--help");
    const string ARG_INIT("--init");
    const string ARG_SHOW("--show");

    // If first arg is "--help" : display help text.
    if (utils::strings::isStrEqual(args[0], ARG_HELP))
    {
        return act_help;
    }

    // If just one arg and arg is "--init" : initialize config file
    if (args.size() == 1 && utils::strings::isStrEqual(args[0], ARG_INIT))
    {
        return act_init;
    }

    // If just one arg and arg is "--show" : scan project path and display all repositories found
    if (args.size() == 1 && utils::strings::isStrEqual(args[0], ARG_SHOW))
    {
        return act_show;
    }

    // In any other case, try to push repositories whose name is in passed arguments.
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

    // If a previous config file is found, ask for confirmation to delete and replace it
    if (utils::files::pathExists(CONFIG_FILE_PATH))
    {
        cout << "Previous config file found in " << CONFIG_FILE_PATH << "." << endl;

        // Continue to ask until user types y or n (case non-sensitive)
        string answer("");
        while (!utils::strings::isStrEqual(answer, "Y") && !utils::strings::isStrEqual(answer, "N"))
        {
            cout << "Do you want to erase it ? (y/n)" << endl;
            getline(cin, answer);
            transform(answer.begin(), answer.end(), answer.begin(), ::toupper);
        }

        if (answer[0] == 'Y')
        {
            // Try to remove previous config file
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
        vector<string> splittedConfigPath = utils::strings::split(CONFIG_FILE_PATH, '/');
        if (!splittedConfigPath.empty() && splittedConfigPath.size() > 2) {
            string pathToTest(splittedConfigPath[0]);
            for (vector<string>::iterator it = splittedConfigPath.begin() + 1; it != splittedConfigPath.end() - 1; it++) {
                pathToTest += '/' + *it;
                if (!utils::files::pathExists(pathToTest)) {
                    if (!fs::create_directories(pathToTest)) {
                        cerr << "Error : cannot create dir " << pathToTest << "which would have contained the config file." << endl;
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
            // Repeat this block until the user confirm the new configuration or cancel
            do
            {
                //  Repeat this block until the user type something
                do
                {
                    cout << "Enter path to the root folder of your git projects :" << endl;
                    getline(cin, projPath);
                } while (utils::strings::isStrEqual(projPath, ""));

                projPath = utils::files::parsePath(projPath);

                // Continue to ask until user types y or n (case non-sensitive)
                do
                {
                    cout << "Do you confirm this path : \"" << projPath << "\" ? (y/n or a to abort)" << endl;
                    getline(cin, answer);
                    transform(answer.begin(), answer.end(), answer.begin(), ::toupper);
                } while (!utils::strings::isStrEqual(answer, "Y") && !utils::strings::isStrEqual(answer, "N") && !utils::strings::isStrEqual(answer, "A"));

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

            // If user confirmed, update the config file
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

        // If user canceled, try to remove the initialized config file
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

    // Scanning all passed args :
    //  - If arg is a known project name :
    //        add the project name to the list of project to push (map projectsToPush)
    //  - If it's not a known project name :
    //        add the arg in the list of ignored args (vector ignoredArgs) in order to display it to the user
    // (no doubles allowed)
    for (string arg : args)
    {
        bool isArgAProjectName(false);
        int i = 0;
        vector<int> idsToErase;
        for (vector<Project>::iterator it = projects.begin(); it != projects.end(); it++, i++)
        {
            if (utils::strings::isStrEqual(arg, it->getName()))
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
        // For performance : remove a project name to the list of projects when it becomes a project to push
        // The less iteration on projects name for each args the better
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

    // Display ignored args if any
    if (!ignoredArgs.empty())
    {
        cout << "Ignoring ";
        for (string arg : ignoredArgs)
        {
            cout << "\"" << arg << "\" ";
        }
        cout << "argument" << (ignoredArgs.size() > 1 ? "s." : ".") << endl;
    }

    // Try to push projects which names matched passed arguments
    for (const auto &elem : projectsToPush)
    {
        // If a name passed in arg match several projects, prompt the user to select which project specifically to push (or all, or cancel)
        string projectName = elem.first;
        int nbOfProjects = elem.second.size();
        if (nbOfProjects > 1)
        {
            string answer;
            int i(0);
            // Asks until the user types a valid answer
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
            } while ((!utils::strings::isStringANumber(answer) && !utils::strings::isStrEqual(answer, "A")) || (utils::strings::isStringANumber(answer) && (stoi(answer) < 0 || stoi(answer) > i - 1)));

            if (!utils::strings::isStrEqual(answer, "A"))
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

        // Call "push.sh" files in each project directory to push project
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

    // Try to read JSON config file in order to pass the projects path to the recursive scanning function
    // then build the found projects list (vector)
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
            vector<string> splittedRepo = utils::strings::split(repo, '/');
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

void searchForRepos(vector<string>& repoPathes, const string& pathToScan, const int& projPathSize)
{
    // If a ".git" folder in a directory is found, there's no need to go further in subfolders.
    if (utils::files::pathExists(pathToScan + "/.git"))
    {
        if (utils::files::pathExists(pathToScan + "/push.sh"))
        {
            string s = pathToScan.substr(projPathSize);
            repoPathes.push_back(utils::files::parsePath(s));
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

void ntd()
{
    // TODO : print hint
    cout << "Nothing to do." << endl;
}