#pragma once

#include <string>

namespace dm {

// Who logged in last and with which session, kept for the login screen to
// preselect. The file is world-readable: the greeter reads it as its own user.
class State
{
public:
    explicit State(std::string path);

    void remember(const std::string &user, const std::string &session);

private:
    std::string m_path;
};

} // namespace dm
