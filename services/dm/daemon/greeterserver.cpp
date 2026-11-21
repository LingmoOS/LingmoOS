#include "greeterserver.h"

#include "base/event_loop.h"
#include "base/i18n.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Lingmo;
using Lingmo::i18n::tr;

namespace dm {

GreeterServer::GreeterServer()
{
    m_server.onNewConnection = [this](int fd) { accept(fd); };
}

GreeterServer::~GreeterServer()
{
    if (m_client)
        m_client->abort();
    m_server.close();
}

bool GreeterServer::listen(const std::string &path, uid_t owner)
{
    m_owner = owner;
    m_path = path;

    if (!m_server.listen(path))
        return false;

    return ::chown(path.c_str(), owner, gid_t(-1)) == 0 && ::chmod(path.c_str(), 0600) == 0;
}

void GreeterServer::accept(int fd)
{
    ucred peer {};
    socklen_t length = sizeof(peer);
    const bool trusted = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &peer, &length) == 0
            && (peer.uid == m_owner || peer.uid == 0);
    if (!trusted) {
        ::close(fd);
        return;
    }

    // A new login screen replaces whatever was connected before, and the login
    // that one was creating goes with it.
    if (m_client) {
        m_client->abort();
        m_client.reset();
        if (auto callback = onDisconnected)
            callback();
    }

    m_client = std::make_unique<base::LocalSocket>();
    m_client->adopt(fd);
    m_reader = FrameReader();
    m_queued.clear();
    m_waiting = false;

    m_client->onReadyRead = [this] { readRequests(); };
    m_client->onDisconnected = [this] { dropClient(); };
    m_client->onError = [this] { dropClient(); };
}

void GreeterServer::dropClient()
{
    // From inside the socket's own callback: it goes on the next loop turn.
    base::EventLoop::current()->post([client = std::shared_ptr<base::LocalSocket>(std::move(m_client))] {});
    if (auto callback = onDisconnected)
        callback();
}

void GreeterServer::readRequests()
{
    if (!m_client)
        return;

    std::string bytes = m_client->readAll();
    m_reader.append(bytes);
    wipe(bytes);

    while (auto request = m_reader.next())
        m_queued.push_back(std::move(*request));
    if (m_reader.failed()) {
        m_client->abort();
        dropClient();
        return;
    }

    if (!m_waiting && !m_queued.empty()) {
        Json request = std::move(m_queued.front());
        m_queued.pop_front();
        dispatch(request);
    }
}

void GreeterServer::dispatch(const Json &request)
{
    m_waiting = true;

    const std::string type = request["type"].toString();
    if (type == "create_session") {
        if (auto callback = onCreateSession)
            callback(request["username"].toString());
    } else if (type == "post_auth_message_response") {
        if (auto callback = onPostAuthMessageResponse)
            callback(request["response"]);
    } else if (type == "start_session") {
        std::vector<std::string> command;
        std::vector<std::string> environment;
        if (request["cmd"].isArray()) {
            for (const Json &value : request["cmd"].toArray())
                command.push_back(value.toString());
        }
        if (request["env"].isArray()) {
            for (const Json &value : request["env"].toArray())
                environment.push_back(value.toString());
        }
        if (auto callback = onStartSession)
            callback(command, environment, request["session"].toString());
    } else if (type == "cancel_session") {
        if (auto callback = onCancelSession)
            callback();
    } else {
        respond(error(false, tr("Unknown request")));
    }
}

void GreeterServer::respond(const Json &response)
{
    if (!m_waiting)
        return;

    m_waiting = false;
    if (m_client)
        m_client->write(encodeFrame(response));

    if (!m_queued.empty()) {
        Json request = std::move(m_queued.front());
        m_queued.pop_front();
        dispatch(request);
    }
}

Json GreeterServer::success()
{
    return Json::object({{"type", "success"}});
}

Json GreeterServer::error(bool authentication, const std::string &description)
{
    return Json::object({{"type", "error"},
                         {"error_type", authentication ? "auth_error" : "error"},
                         {"description", description}});
}

Json GreeterServer::authMessage(const std::string &style, const std::string &text)
{
    return Json::object({{"type", "auth_message"}, {"auth_message_type", style}, {"auth_message", text}});
}

} // namespace dm
