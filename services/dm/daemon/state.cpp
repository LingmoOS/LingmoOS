#include "state.h"

#include "base/settings.h"

namespace dm {

State::State(std::string path)
    : m_path(std::move(path))
{
}

void State::remember(const std::string &user, const std::string &session)
{
    // Writing creates the directory as well.
    Lingmo::base::Settings settings(m_path);
    settings.setValue("Last/User", user);
    settings.setValue("Sessions/" + user, session);
    settings.sync();
}

} // namespace dm
