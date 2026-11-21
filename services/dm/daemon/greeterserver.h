#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

#include "base/local_socket.h"

#include "../common/wire.h"

namespace dm {

// GREETD_SOCK with greetd's protocol; one request is answered at a time.
class GreeterServer
{
public:
    GreeterServer();
    ~GreeterServer();

    GreeterServer(const GreeterServer &) = delete;
    GreeterServer &operator=(const GreeterServer &) = delete;

    // Only `owner` may connect; the socket is chowned to it with mode 0600.
    bool listen(const std::string &path, uid_t owner);
    std::string path() const { return m_path; }

    // Answers the request in flight.
    void respond(const Json &response);
    static Json success();
    static Json error(bool authentication, const std::string &description);
    static Json authMessage(const std::string &style, const std::string &text);

    std::function<void(const std::string &username)> onCreateSession;
    std::function<void(const Json &response)> onPostAuthMessageResponse;
    // `session`, a wayland-sessions id, is our extension; greetd greeters send cmd.
    std::function<void(const std::vector<std::string> &command, const std::vector<std::string> &environment,
                       const std::string &session)>
            onStartSession;
    std::function<void()> onCancelSession;
    // The client went away; a new greeter starts from scratch.
    std::function<void()> onDisconnected;

private:
    void accept(int fd);
    void readRequests();
    void dropClient();
    void dispatch(const Json &request);

    Lingmo::base::LocalServer m_server;
    std::unique_ptr<Lingmo::base::LocalSocket> m_client;
    FrameReader m_reader;
    std::deque<Json> m_queued;
    std::string m_path;
    uid_t m_owner = 0;
    bool m_waiting = false;
};

} // namespace dm
