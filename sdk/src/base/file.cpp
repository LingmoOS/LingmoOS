// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/file.h"

#include <cerrno>
#include <csignal>
#include <pthread.h>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

namespace Lingmo::base::file {

std::optional<std::string> read(const std::string& path)
{
    const int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0)
        return std::nullopt;

    std::string contents;
    char buffer[16384];
    while (true) {
        const ssize_t size = ::read(fd, buffer, sizeof(buffer));
        if (size < 0 && errno == EINTR)
            continue;
        if (size < 0) {
            close(fd);
            return std::nullopt;
        }
        if (size == 0)
            break;
        contents.append(buffer, size_t(size));
    }
    close(fd);
    return contents;
}

// umask() can only be read by changing it, which races with other threads.
static mode_t currentUmask()
{
    const std::optional<std::string> status = read("/proc/self/status");
    const size_t start = status ? status->find("Umask:\t") : std::string::npos;
    if (start == std::string::npos)
        return 022;
    return mode_t(std::stoul(status->substr(start + 7, 4), nullptr, 8));
}

static bool writeAll(int fd, std::string_view contents)
{
    while (!contents.empty()) {
        const ssize_t written = ::write(fd, contents.data(), contents.size());
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            return false;
        contents.remove_prefix(size_t(written));
    }
    return true;
}

ssize_t writeWithoutSigpipe(int fd, const void* data, size_t size)
{
    sigset_t sigpipe;
    sigemptyset(&sigpipe);
    sigaddset(&sigpipe, SIGPIPE);
    sigset_t previous;
    pthread_sigmask(SIG_BLOCK, &sigpipe, &previous);
    const ssize_t n = ::write(fd, data, size);
    const int error = errno;
    if (n < 0 && error == EPIPE) {
        const timespec zero = {0, 0};
        while (sigtimedwait(&sigpipe, nullptr, &zero) == -1 && errno == EINTR) {
        }
    }
    pthread_sigmask(SIG_SETMASK, &previous, nullptr);
    errno = error;
    return n;
}

// The file a chain of links ends at: a rename over the link itself would
// replace the link.
static std::string linkTarget(const std::string& path)
{
    std::filesystem::path resolved(path);
    std::error_code error;
    // As the kernel's limit on links followed in one lookup.
    for (int hop = 0; hop < 40 && std::filesystem::is_symlink(resolved, error); ++hop) {
        const std::filesystem::path target = std::filesystem::read_symlink(resolved, error);
        if (error)
            break;
        resolved = target.is_absolute() ? target : resolved.parent_path() / target;
    }
    return resolved.string();
}

bool writeAtomically(const std::string& path, std::string_view contents)
{
    const std::string file = linkTarget(path);
    const std::filesystem::path target(file);
    std::error_code error;
    if (target.has_parent_path())
        std::filesystem::create_directories(target.parent_path(), error);

    std::string temporary = file + ".XXXXXX";
    const int fd = mkostemp(temporary.data(), O_CLOEXEC);
    if (fd < 0)
        return false;

    struct stat existing{};
    if (stat(file.c_str(), &existing) == 0)
        fchmod(fd, existing.st_mode & 07777);
    else
        fchmod(fd, 0666 & ~currentUmask());

    const bool written = writeAll(fd, contents) && fdatasync(fd) == 0;
    close(fd);
    if (!written || rename(temporary.c_str(), file.c_str()) != 0) {
        unlink(temporary.c_str());
        return false;
    }
    return true;
}

TemporaryDirectory::TemporaryDirectory(const std::string& parent, const std::string& prefix)
{
    std::string pattern = parent + "/" + prefix + "XXXXXX";
    if (mkdtemp(pattern.data()))
        m_path = std::move(pattern);
}

TemporaryDirectory::~TemporaryDirectory()
{
    if (m_autoRemove && !m_path.empty()) {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }
}

TemporaryFile::TemporaryFile(const std::string& directory, const std::string& prefix)
{
    std::string pattern = directory + "/" + prefix + "XXXXXX";
    m_fd = mkostemp(pattern.data(), O_CLOEXEC);
    if (m_fd >= 0)
        m_path = std::move(pattern);
}

TemporaryFile::~TemporaryFile()
{
    if (m_fd >= 0) {
        close(m_fd);
        unlink(m_path.c_str());
    }
}

bool TemporaryFile::write(std::string_view contents)
{
    while (!contents.empty() && m_fd >= 0) {
        const ssize_t written = ::write(m_fd, contents.data(), contents.size());
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            return false;
        contents.remove_prefix(size_t(written));
    }
    return m_fd >= 0;
}

} // namespace Lingmo::base::file
