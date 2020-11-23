#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <string>

class Project {
    public:

    Project(const std::string& name, const std::string& path);

    const std::string& getName() const;
    const std::string& getPath() const;

    void setName(const std::string& name);
    void setPath(const std::string& path);

    private:

    std::string m_name;
    std::string m_path;
};

#endif