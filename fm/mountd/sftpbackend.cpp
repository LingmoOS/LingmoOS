// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sftpbackend.h"

#include "base/i18n.h"

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/time.h>

using Lingmo::i18n::tr;

// OpenSSH drops the session on requests much larger than the 32 KiB the protocol draft
// promises to accept.
static constexpr size_t kChunk = 32768;

struct SftpFile
{
    sftp_file file = nullptr;
    uint64_t generation = 0;
};

static SftpFile *fileFor(uint64_t handle)
{
    return reinterpret_cast<SftpFile *>(handle);
}

static void fillStat(sftp_attributes attributes, struct stat *st)
{
    std::memset(st, 0, sizeof(*st));

    mode_t type = S_IFREG;
    switch (attributes->type) {
    case SSH_FILEXFER_TYPE_DIRECTORY:
        type = S_IFDIR;
        break;
    case SSH_FILEXFER_TYPE_SYMLINK:
        type = S_IFLNK;
        break;
    case SSH_FILEXFER_TYPE_SPECIAL:
        type = S_IFIFO;
        break;
    default:
        break;
    }

    // Some servers send the type bits in permissions too; the type field decides.
    st->st_mode = type | (attributes->permissions & 07777);
    st->st_nlink = type == S_IFDIR ? 2 : 1;
    st->st_uid = attributes->uid;
    st->st_gid = attributes->gid;
    st->st_size = attributes->size;
    st->st_blksize = 32768;
    st->st_blocks = (attributes->size + 511) / 512;
    st->st_atim.tv_sec = attributes->atime64 ? attributes->atime64 : attributes->atime;
    st->st_mtim.tv_sec = attributes->mtime64 ? attributes->mtime64 : attributes->mtime;
    st->st_ctim = st->st_mtim;
}

SftpBackend::SftpBackend(const RemoteUrl &url)
    : m_url(url)
{
}

SftpBackend::~SftpBackend()
{
    close();
}

void SftpBackend::close()
{
    if (m_sftp) {
        sftp_free(m_sftp);
        m_sftp = nullptr;
    }

    if (m_ssh) {
        ssh_disconnect(m_ssh);
        ssh_free(m_ssh);
        m_ssh = nullptr;
    }
}

int SftpBackend::error(int fallback)
{
    if (!m_sftp)
        return -ENOTCONN;

    // A fatal error leaves the session unusable; the next call connects again.
    if (ssh_get_error_code(m_ssh) == SSH_FATAL) {
        close();
        return -EIO;
    }

    switch (sftp_get_error(m_sftp)) {
    case SSH_FX_OK:
    case SSH_FX_EOF:
        return 0;
    case SSH_FX_NO_SUCH_FILE:
    case SSH_FX_NO_SUCH_PATH:
        return -ENOENT;
    case SSH_FX_PERMISSION_DENIED:
        return -EACCES;
    case SSH_FX_FILE_ALREADY_EXISTS:
        return -EEXIST;
    case SSH_FX_WRITE_PROTECT:
        return -EROFS;
    case SSH_FX_OP_UNSUPPORTED:
        return -ENOTSUP;
    case SSH_FX_NO_CONNECTION:
    case SSH_FX_CONNECTION_LOST:
        return -ENOTCONN;
    case SSH_FX_NO_MEDIA:
        return -ENOMEDIUM;
    default:
        return -fallback;
    }
}

ConnectResult SftpBackend::connect(const Credentials &credentials)
{
    const ConnectResult result = establish(credentials);
    if (result.status == ConnectResult::Connected)
        m_credentials = credentials;

    return result;
}

ConnectResult SftpBackend::establish(const Credentials &credentials)
{
    close();

    ConnectResult result;
    m_ssh = ssh_new();
    if (!m_ssh) {
        result.message = tr("Cannot start an SSH session");
        return result;
    }

    ssh_options_set(m_ssh, SSH_OPTIONS_HOST, m_url.host.c_str());
    // ~/.ssh/config may name another host, port, user or key for this one.
    ssh_options_parse_config(m_ssh, nullptr);

    const std::string &user = !credentials.user.empty() ? credentials.user : m_url.user;
    if (!user.empty())
        ssh_options_set(m_ssh, SSH_OPTIONS_USER, user.c_str());

    if (m_url.port > 0) {
        const unsigned int port = m_url.port;
        ssh_options_set(m_ssh, SSH_OPTIONS_PORT, &port);
    }

    const long timeout = 15;
    ssh_options_set(m_ssh, SSH_OPTIONS_TIMEOUT, &timeout);
    // Each request waits for its answer; Nagle would hold every one back for a delayed ACK.
    const int noDelay = 1;
    ssh_options_set(m_ssh, SSH_OPTIONS_NODELAY, &noDelay);

    if (ssh_connect(m_ssh) != SSH_OK) {
        result.message = std::string(ssh_get_error(m_ssh));
        close();
        return result;
    }

    ssh_key key = nullptr;
    unsigned char *hash = nullptr;
    size_t hashLength = 0;
    std::string fingerprint;
    if (ssh_get_server_publickey(m_ssh, &key) == SSH_OK
        && ssh_get_publickey_hash(key, SSH_PUBLICKEY_HASH_SHA256, &hash, &hashLength) == 0) {
        char *text = ssh_get_fingerprint_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hashLength);
        fingerprint = text;
        ssh_string_free_char(text);
        ssh_clean_pubkey_hash(&hash);
    }
    ssh_key_free(key);

    const auto known = ssh_session_is_known_server(m_ssh);
    if (known != SSH_KNOWN_HOSTS_OK) {
        if (known == SSH_KNOWN_HOSTS_ERROR || fingerprint.empty()) {
            result.message = std::string(ssh_get_error(m_ssh));
            close();
            return result;
        }

        if (credentials.acceptedHostKey != fingerprint) {
            result.status = ConnectResult::UnknownHostKey;
            result.fingerprint = fingerprint;
            result.hostKeyChanged = known == SSH_KNOWN_HOSTS_CHANGED
                    || known == SSH_KNOWN_HOSTS_OTHER;
            close();
            return result;
        }

        ssh_session_update_known_hosts(m_ssh);
    }

    // Keys from the agent or ~/.ssh first, then the password.
    int auth = ssh_userauth_none(m_ssh, nullptr);
    const int methods = ssh_userauth_list(m_ssh, nullptr);

    if (auth != SSH_AUTH_SUCCESS && (methods & SSH_AUTH_METHOD_PUBLICKEY))
        auth = ssh_userauth_publickey_auto(m_ssh, nullptr, nullptr);

    if (auth != SSH_AUTH_SUCCESS && !credentials.password.empty()) {
        const std::string &password = credentials.password;

        if (methods & SSH_AUTH_METHOD_PASSWORD)
            auth = ssh_userauth_password(m_ssh, nullptr, password.c_str());

        // PAM servers often take the password only as a keyboard-interactive answer.
        if (auth != SSH_AUTH_SUCCESS && (methods & SSH_AUTH_METHOD_INTERACTIVE)) {
            auth = ssh_userauth_kbdint(m_ssh, nullptr, nullptr);
            while (auth == SSH_AUTH_INFO) {
                const int prompts = ssh_userauth_kbdint_getnprompts(m_ssh);
                for (int i = 0; i < prompts; ++i) {
                    char echo = 0;
                    ssh_userauth_kbdint_getprompt(m_ssh, i, &echo);
                    ssh_userauth_kbdint_setanswer(m_ssh, i, echo ? "" : password.c_str());
                }
                auth = ssh_userauth_kbdint(m_ssh, nullptr, nullptr);
            }
        }
    }

    if (auth != SSH_AUTH_SUCCESS) {
        const bool canAsk = methods & (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_INTERACTIVE);
        result.status = canAsk ? ConnectResult::NeedPassword : ConnectResult::Failed;
        result.message = std::string(ssh_get_error(m_ssh));
        close();
        return result;
    }

    m_sftp = sftp_new(m_ssh);
    if (!m_sftp || sftp_init(m_sftp) != SSH_OK) {
        result.message = std::string(ssh_get_error(m_ssh));
        close();
        return result;
    }

    m_readChunk = kChunk;
    m_writeChunk = kChunk;
    if (sftp_limits_t limits = sftp_limits(m_sftp)) {
        if (limits->max_read_length)
            m_readChunk = std::min<size_t>(kChunk, limits->max_read_length);
        if (limits->max_write_length)
            m_writeChunk = std::min<size_t>(kChunk, limits->max_write_length);
        sftp_limits_free(limits);
    }

    if (char *home = sftp_canonicalize_path(m_sftp, ".")) {
        m_home = home;
        ssh_string_free_char(home);
    }

    ++m_generation;
    result.status = ConnectResult::Connected;
    return result;
}

bool SftpBackend::ensureConnected()
{
    if (m_ssh && m_sftp && ssh_is_connected(m_ssh))
        return true;

    Credentials credentials = m_credentials;
    return establish(credentials).status == ConnectResult::Connected;
}

std::string SftpBackend::initialPath() const
{
    if (!m_url.path.empty() && m_url.path != "/")
        return m_url.path;

    return m_home;
}

int SftpBackend::stat(const std::string &path, struct stat *st)
{
    if (!ensureConnected())
        return -ENOTCONN;

    // Links are shown as what they point to; one that points nowhere as itself.
    sftp_attributes attributes = sftp_stat(m_sftp, path.c_str());
    if (!attributes)
        attributes = sftp_lstat(m_sftp, path.c_str());
    if (!attributes)
        return error();

    fillStat(attributes, st);
    sftp_attributes_free(attributes);
    return 0;
}

int SftpBackend::list(const std::string &path, std::vector<DirectoryEntry> *entries)
{
    if (!ensureConnected())
        return -ENOTCONN;

    sftp_dir directory = sftp_opendir(m_sftp, path.c_str());
    if (!directory)
        return error();

    const std::string base = path == "/" ? path : path + '/';
    std::vector<std::string> links;

    while (sftp_attributes attributes = sftp_readdir(m_sftp, directory)) {
        const std::string name = attributes->name ? attributes->name : "";
        if (!name.empty() && name != "." && name != "..") {
            DirectoryEntry entry;
            entry.name = name;
            fillStat(attributes, &entry.st);
            if (S_ISLNK(entry.st.st_mode))
                links.push_back(name);
            entries->push_back(std::move(entry));
        }
        sftp_attributes_free(attributes);
    }

    const bool complete = sftp_dir_eof(directory);
    sftp_closedir(directory);
    if (!complete)
        return error();

    for (DirectoryEntry &entry : *entries) {
        if (!S_ISLNK(entry.st.st_mode))
            continue;
        if (sftp_attributes target = sftp_stat(m_sftp, (base + entry.name).c_str())) {
            fillStat(target, &entry.st);
            sftp_attributes_free(target);
        }
    }

    return 0;
}

int SftpBackend::open(const std::string &path, int flags, mode_t mode, uint64_t *handle)
{
    if (!ensureConnected())
        return -ENOTCONN;

    const int access = flags & O_ACCMODE;
    const int openFlags = access | (flags & (O_CREAT | O_EXCL | O_TRUNC | O_APPEND));
    sftp_file file = sftp_open(m_sftp, path.c_str(), openFlags, mode & 07777);
    if (!file)
        return error();

    *handle = reinterpret_cast<uint64_t>(new SftpFile {file, m_generation});
    return 0;
}

// The chunks of one call are all requested before the first answer is awaited, so a
// call costs one round trip instead of one per chunk.
int SftpBackend::read(uint64_t handle, char *buffer, size_t size, off_t offset)
{
    SftpFile *file = fileFor(handle);
    if (file->generation != m_generation)
        return -EIO;

    if (sftp_seek64(file->file, offset) < 0)
        return error();

    struct Request
    {
        sftp_aio aio;
        size_t start;
        size_t length;
    };

    std::vector<Request> requests;
    for (size_t requested = 0; requested < size;) {
        sftp_aio aio = nullptr;
        const ssize_t length = sftp_aio_begin_read(file->file, std::min(size - requested, m_readChunk),
                                                   &aio);
        if (length <= 0)
            break;
        requests.push_back({aio, requested, size_t(length)});
        requested += length;
    }

    if (requests.empty())
        return error();

    // Every request is waited for, even past a short one, so libssh frees its answer.
    size_t done = 0;
    bool ended = false;
    int failure = 0;
    for (Request &request : requests) {
        const ssize_t count = sftp_aio_wait_read(&request.aio, buffer + request.start,
                                                 request.length);
        if (ended)
            continue;

        if (count < 0) {
            failure = error();
            ended = true;
        } else {
            done += count;
            // A short answer is the end of the file.
            ended = size_t(count) < request.length;
        }
    }

    return done ? int(done) : failure;
}

int SftpBackend::write(uint64_t handle, const char *buffer, size_t size, off_t offset)
{
    SftpFile *file = fileFor(handle);
    if (file->generation != m_generation)
        return -EIO;

    // Some servers (proftpd) fail on empty writes.
    if (size == 0)
        return 0;

    if (sftp_seek64(file->file, offset) < 0)
        return error();

    std::vector<std::pair<sftp_aio, size_t>> requests;
    for (size_t sent = 0; sent < size;) {
        sftp_aio aio = nullptr;
        const ssize_t length = sftp_aio_begin_write(file->file, buffer + sent,
                                                    std::min(size - sent, m_writeChunk), &aio);
        if (length <= 0)
            break;
        requests.emplace_back(aio, length);
        sent += length;
    }

    if (requests.empty())
        return error();

    size_t done = 0;
    int failure = 0;
    for (auto &[aio, length] : requests) {
        const ssize_t count = sftp_aio_wait_write(&aio);
        if (count < 0 && !failure)
            failure = error();
        else if (count >= 0 && !failure)
            done += count;
    }

    return failure && !done ? failure : int(done);
}

int SftpBackend::release(uint64_t handle)
{
    SftpFile *file = fileFor(handle);
    int result = 0;
    if (file->generation == m_generation && sftp_close(file->file) != SSH_NO_ERROR)
        result = error();

    delete file;
    return result;
}

int SftpBackend::truncate(const std::string &path, off_t size, const uint64_t *)
{
    if (!ensureConnected())
        return -ENOTCONN;

    sftp_attributes_struct attributes = {};
    attributes.flags = SSH_FILEXFER_ATTR_SIZE;
    attributes.size = size;
    return sftp_setstat(m_sftp, path.c_str(), &attributes) == 0 ? 0 : error();
}

int SftpBackend::remove(const std::string &path)
{
    if (!ensureConnected())
        return -ENOTCONN;

    return sftp_unlink(m_sftp, path.c_str()) == 0 ? 0 : error();
}

int SftpBackend::makeDirectory(const std::string &path, mode_t mode)
{
    if (!ensureConnected())
        return -ENOTCONN;

    if (sftp_mkdir(m_sftp, path.c_str(), mode & 07777) == 0)
        return 0;

    // Version 3 servers answer a generic failure when the name is taken.
    if (sftp_get_error(m_sftp) == SSH_FX_FAILURE) {
        if (sftp_attributes existing = sftp_lstat(m_sftp, path.c_str())) {
            sftp_attributes_free(existing);
            return -EEXIST;
        }
    }

    return error();
}

int SftpBackend::removeDirectory(const std::string &path)
{
    if (!ensureConnected())
        return -ENOTCONN;

    if (sftp_rmdir(m_sftp, path.c_str()) == 0)
        return 0;

    // A folder that still exists after a generic failure was not empty.
    if (sftp_get_error(m_sftp) == SSH_FX_FAILURE) {
        if (sftp_attributes existing = sftp_lstat(m_sftp, path.c_str())) {
            sftp_attributes_free(existing);
            return -ENOTEMPTY;
        }
    }

    return error();
}

int SftpBackend::rename(const std::string &from, const std::string &to, unsigned flags)
{
    if (!ensureConnected())
        return -ENOTCONN;

    if (flags & RENAME_EXCHANGE)
        return -EINVAL;

    sftp_attributes existing = sftp_lstat(m_sftp, to.c_str());
    const bool exists = existing;
    const bool directory = existing && existing->type == SSH_FILEXFER_TYPE_DIRECTORY;
    sftp_attributes_free(existing);

    if (exists && (flags & RENAME_NOREPLACE))
        return -EEXIST;

    if (sftp_rename(m_sftp, from.c_str(), to.c_str()) == 0)
        return 0;

    // Version 3 rename never replaces; without the POSIX rename extension the old file
    // goes first, as other clients do.
    const int code = sftp_get_error(m_sftp);
    if (!exists || (code != SSH_FX_FAILURE && code != SSH_FX_FILE_ALREADY_EXISTS))
        return error();

    const int removed = directory ? sftp_rmdir(m_sftp, to.c_str()) : sftp_unlink(m_sftp, to.c_str());
    if (removed != 0)
        return directory ? -ENOTEMPTY : error();

    return sftp_rename(m_sftp, from.c_str(), to.c_str()) == 0 ? 0 : error();
}

int SftpBackend::setMode(const std::string &path, mode_t mode)
{
    if (!ensureConnected())
        return -ENOTCONN;

    return sftp_chmod(m_sftp, path.c_str(), mode & 07777) == 0 ? 0 : error();
}

int SftpBackend::setTimes(const std::string &path, const struct timespec times[2])
{
    if (!ensureConnected())
        return -ENOTCONN;

    sftp_attributes current = nullptr;
    struct timeval values[2];
    for (int i = 0; i < 2; ++i) {
        if (times[i].tv_nsec == UTIME_NOW) {
            gettimeofday(&values[i], nullptr);
        } else if (times[i].tv_nsec == UTIME_OMIT) {
            if (!current && !(current = sftp_stat(m_sftp, path.c_str())))
                return error();
            values[i].tv_sec = i == 0 ? current->atime : current->mtime;
            values[i].tv_usec = 0;
        } else {
            values[i].tv_sec = times[i].tv_sec;
            values[i].tv_usec = times[i].tv_nsec / 1000;
        }
    }

    if (current)
        sftp_attributes_free(current);

    return sftp_utimes(m_sftp, path.c_str(), values) == 0 ? 0 : error();
}

int SftpBackend::readLink(const std::string &path, std::string *target)
{
    if (!ensureConnected())
        return -ENOTCONN;

    char *value = sftp_readlink(m_sftp, path.c_str());
    if (!value)
        return error();

    *target = value;
    ssh_string_free_char(value);
    return 0;
}

int SftpBackend::makeLink(const std::string &target, const std::string &path)
{
    if (!ensureConnected())
        return -ENOTCONN;

    return sftp_symlink(m_sftp, target.c_str(), path.c_str()) == 0 ? 0 : error();
}

int SftpBackend::statfs(const std::string &path, struct statvfs *st)
{
    if (!ensureConnected())
        return -ENOTCONN;

    std::memset(st, 0, sizeof(*st));
    st->f_bsize = 4096;
    st->f_frsize = 4096;
    st->f_namemax = 255;

    if (!sftp_extension_supported(m_sftp, "statvfs@openssh.com", "2"))
        return 0;

    sftp_statvfs_t info = sftp_statvfs(m_sftp, path.c_str());
    if (!info)
        return 0;

    st->f_bsize = info->f_bsize;
    st->f_frsize = info->f_frsize ? info->f_frsize : info->f_bsize;
    st->f_blocks = info->f_blocks;
    st->f_bfree = info->f_bfree;
    st->f_bavail = info->f_bavail;
    st->f_files = info->f_files;
    st->f_ffree = info->f_ffree;
    st->f_favail = info->f_favail;
    st->f_namemax = info->f_namemax;
    sftp_statvfs_free(info);
    return 0;
}
