#include <string>
#include "Project.hpp"

using namespace std;

Project::Project(const std::string &name, const std::string &path) : m_name(name),
                                                                      m_path(path)
{
}

const string &Project::getName() const
{
    return m_name;
}

const string &Project::getPath() const
{
    return m_path;
}

void Project::setName(const string &name)
{
    m_name = name;
}

void Project::setPath(const string &path)
{
    m_path = path;
}