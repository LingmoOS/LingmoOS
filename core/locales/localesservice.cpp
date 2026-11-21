// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localesservice.h"

#include "base/event_loop.h"
#include "base/file.h"
#include "base/logging.h"
#include "base/strings.h"
#include "common/detached_call.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <locale.h>
#include <map>
#include <optional>
#include <tuple>
#include <unistd.h>

namespace dbus = Lingmo::Base::dbus;
namespace strings = Lingmo::Base::strings;

static const Lingmo::Base::LogCategory kLog("lingmo.locales");

static constexpr char kAction[] = "org.lingmo.locales.generate";

// polkit's Subject: a kind and its details.
using PolkitSubject = std::tuple<std::string, dbus::VariantMap>;

static bool isLetters(std::string_view text, bool upper)
{
    for (const char c : text) {
        if (upper ? !(c >= 'A' && c <= 'Z') : !(c >= 'a' && c <= 'z'))
            return false;
    }
    return !text.empty();
}

// ll_CC.UTF-8 or lll_CC.UTF-8, with an optional @modifier.
static bool isWellFormed(const std::string &name)
{
    const size_t underscore = name.find('_');
    const size_t dot = name.find('.');
    if (underscore == std::string::npos || dot != underscore + 3)
        return false;
    const std::string_view view(name);
    const std::string_view language = view.substr(0, underscore);
    const std::string_view territory = view.substr(underscore + 1, 2);
    std::string_view rest = view.substr(dot);
    if (language.size() < 2 || language.size() > 3 || !isLetters(language, false) || !isLetters(territory, true))
        return false;
    if (!rest.starts_with(".UTF-8"))
        return false;
    rest.remove_prefix(6);
    return rest.empty() || (rest.front() == '@' && isLetters(rest.substr(1), false));
}

// The locale's source: the name without its codeset, "sr_RS@latin" for
// "sr_RS.UTF-8@latin".
static std::string sourceName(const std::string &name)
{
    return strings::replaceAll(name, ".UTF-8", "");
}

LocalesService::LocalesService()
    : LocalesService(Paths())
{
}

LocalesService::LocalesService(Paths paths, std::chrono::milliseconds idleTimeout)
    : m_paths(std::move(paths))
{
    m_idleTimer.setSingleShot(true);
    m_idleTimer.setInterval(idleTimeout);
    m_idleTimer.onTimeout = [this] {
        if (m_jobs.empty() && !m_pendingAuthorizations && onIdle)
            onIdle();
    };
    m_idleTimer.start();
}

LocalesService::~LocalesService() = default;

bool LocalesService::exportTo(dbus::Connection *bus)
{
    m_bus = bus;
    m_object = std::make_unique<dbus::Object>(bus, "/org/lingmo/Locales", "org.lingmo.Locales");
    m_object->addMethod(
            "generate", "as", "", [this](dbus::MethodCall &call) { generate(call); }, {{"names"}, {}});
    return m_object->publish();
}

bool LocalesService::isSupported(const std::string &name) const
{
    if (!isWellFormed(name))
        return false;
    const std::optional<std::string> supported = Lingmo::Base::file::read(m_paths.supported);
    if (!supported)
        return false;
    bool listed = false;
    for (const std::string &line : strings::split(*supported, '\n')) {
        if (strings::trimmed(line) == name + " UTF-8") {
            listed = true;
            break;
        }
    }
    return listed && ::access((m_paths.sources + '/' + sourceName(name)).c_str(), R_OK) == 0;
}

bool LocalesService::isGenerated(const std::string &name) const
{
    const std::string directory = strings::replaceAll(name, ".UTF-8", ".utf8");
    if (::access((m_paths.output + '/' + directory + "/LC_CTYPE").c_str(), F_OK) == 0)
        return true;
    locale_t locale = newlocale(LC_ALL_MASK, name.c_str(), nullptr);
    if (!locale)
        return false;
    freelocale(locale);
    return true;
}

bool LocalesService::enableInLocaleGen(const std::string &name) const
{
    const std::optional<std::string> contents = Lingmo::Base::file::read(m_paths.localeGen);
    if (!contents)
        return ::access(m_paths.localeGen.c_str(), F_OK) != 0;

    const std::string entry = name + " UTF-8";
    std::vector<std::string> lines = strings::split(*contents, '\n');
    if (!lines.empty() && lines.back().empty())
        lines.pop_back();
    bool found = false;
    for (std::string &line : lines) {
        const std::string text(strings::trimmed(line));
        if (text == entry)
            return true;
        if (!found && text.starts_with('#') && strings::trimmed(std::string_view(text).substr(1)) == entry) {
            line = entry;
            found = true;
        }
    }
    if (!found)
        lines.push_back(entry);

    // Replaced whole, so a reader never sees half of it.
    const std::string temporary = m_paths.localeGen + ".lingmo-new";
    const int fd = ::open(temporary.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0) {
        LM_WARNING(kLog) << "lingmo-locales: cannot write" << temporary << std::strerror(errno);
        return false;
    }
    const std::string text = strings::join(lines, "\n") + '\n';
    const bool written = ::write(fd, text.data(), text.size()) == ssize_t(text.size()) && ::fsync(fd) == 0;
    ::close(fd);
    if (!written || ::rename(temporary.c_str(), m_paths.localeGen.c_str()) != 0) {
        LM_WARNING(kLog) << "lingmo-locales: cannot replace" << m_paths.localeGen << std::strerror(errno);
        ::unlink(temporary.c_str());
        return false;
    }
    return true;
}

void LocalesService::generate(dbus::MethodCall &call)
{
    m_idleTimer.start();

    std::vector<std::string> requested;
    call.read(requested);
    std::vector<std::string> names;
    for (const std::string &name : requested) {
        if (!isSupported(name)) {
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Not a locale that can be generated: " + name);
            return;
        }
        if (!isGenerated(name))
            names.push_back(name);
    }
    if (names.empty()) {
        call.reply();
        return;
    }

    dbus::MethodCall request = call;
    dbus::Message check = m_bus->newMethodCall("org.freedesktop.PolicyKit1", "/org/freedesktop/PolicyKit1/Authority",
                                               "org.freedesktop.PolicyKit1.Authority", "CheckAuthorization");
    const PolkitSubject subject {"system-bus-name", {{"name", dbus::Variant(call.message().sender())}}};
    // 0: no interaction; a caller the policy does not allow outright is refused.
    check.append(subject, std::string(kAction), std::map<std::string, std::string>(), uint32_t(0), std::string());

    ++m_pendingAuthorizations;
    Lingmo::Core::callDetached(
            m_bus, check,
            [this, request, names](const dbus::Reply &reply) mutable {
                --m_pendingAuthorizations;
                m_idleTimer.start();

                bool authorized = false;
                std::tuple<bool, bool, std::map<std::string, std::string>> result;
                if (reply.read(result))
                    authorized = std::get<0>(result);
                else
                    LM_WARNING(kLog) << "lingmo-locales: polkit check failed:" << reply.error().message;

                if (!authorized) {
                    request.replyError("org.freedesktop.DBus.Error.AccessDenied", "Not authorized to generate locales");
                    return;
                }
                m_jobs.push_back({request, names});
                if (m_jobs.size() == 1)
                    runNext();
            },
            std::chrono::seconds(30));
}

void LocalesService::runNext()
{
    if (m_jobs.empty())
        return;
    Job &job = m_jobs.front();
    if (job.names.empty()) {
        finishJob(true);
        return;
    }

    const std::string name = job.names.back();
    job.names.pop_back();
    if (!enableInLocaleGen(name)) {
        finishJob(false);
        return;
    }

    std::vector<std::string> arguments {"--no-archive", "-i", sourceName(name), "-c", "-f", "UTF-8"};
    if (::access(m_paths.aliases.c_str(), R_OK) == 0) {
        arguments.push_back("-A");
        arguments.push_back(m_paths.aliases);
    }
    arguments.push_back(name);

    m_localedef = std::make_unique<Lingmo::Base::Process>();
    // localedef exits non-zero for warnings it was told to ignore; what it
    // wrote decides.
    m_localedef->onFinished = [this, name](int, bool) {
        Lingmo::Base::EventLoop::current()->post([this, name] {
            m_localedef.reset();
            if (!isGenerated(name)) {
                LM_WARNING(kLog) << "lingmo-locales: localedef did not generate" << name;
                finishJob(false);
                return;
            }
            runNext();
        });
    };
    if (!m_localedef->start(m_paths.localedef, arguments)) {
        m_localedef.reset();
        finishJob(false);
    }
}

void LocalesService::finishJob(bool succeeded)
{
    Job job = std::move(m_jobs.front());
    m_jobs.pop_front();
    if (succeeded)
        job.call.reply();
    else
        job.call.replyError("org.lingmo.Locales.Error.Failed", "The locales could not be generated");
    m_idleTimer.start();
    runNext();
}
