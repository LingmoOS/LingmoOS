#ifndef OS_RELEASE_H
#define OS_RELEASE_H

#include <string>
#include <map>

class OSRelease {
public:
    OSRelease();
    std::string get(const std::string& key) const;
    void load();

private:
    std::map<std::string, std::string> data;
    void parseLine(const std::string& line);
};

#endif // OS_RELEASE_H

