#pragma once

#include "base/dbus.h"

#include <map>
#include <memory>
#include <optional>
#include <string>

class FileChooserRequest;

class FileChooserPortal
{
public:
    explicit FileChooserPortal(Lingmo::base::dbus::Connection *bus);
    ~FileChooserPortal();

    bool publish();

    uint32_t version() const { return 4; }

private:
    friend class FileChooserRequest;

    void choose(const std::string &mode, Lingmo::base::dbus::MethodCall &call);
    // Destroys the request on the next loop turn.
    void deleteLater(FileChooserRequest *request);

    Lingmo::base::dbus::Connection *m_bus;
    Lingmo::base::dbus::Object m_object;
    std::map<FileChooserRequest *, std::unique_ptr<FileChooserRequest>> m_requests;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

// A local file: URL as the chooser's answer may hold it, written the way the
// portal hands URLs on: a lower-case scheme, an empty authority, unreserved
// characters unescaped and everything outside the URL syntax escaped. Empty
// for another scheme, a host, or a relative path.
std::optional<std::string> normalizedFileUrl(std::string_view url);
