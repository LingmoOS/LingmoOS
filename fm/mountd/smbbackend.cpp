// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smbbackend.h"

#include "base/i18n.h"
#include "base/url.h"
#include "base/uuid.h"

#include <libsmbclient.h>
#include <unicode/uchar.h>
#include <unicode/ustring.h>

#include <cstring>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/time.h>
#include <unistd.h>

using Lingmo::i18n::tr;

// Equal under full Unicode case folding.
static bool sameIgnoringCase(const std::string &a, const std::string &b)
{
    auto toUtf16 = [](const std::string &text) {
        std::u16string out(text.size() + 1, u'\0');
        int32_t length = 0;
        UErrorCode status = U_ZERO_ERROR;
        u_strFromUTF8Lenient(reinterpret_cast<UChar *>(out.data()), int32_t(out.size()), &length,
                             text.data(), int32_t(text.size()), &status);
        out.resize(U_SUCCESS(status) ? size_t(length) : 0);
        return out;
    };
    const std::u16string first = toUtf16(a);
    const std::u16string second = toUtf16(b);
    UErrorCode error = U_ZERO_ERROR;
    return u_strCaseCompare(reinterpret_cast<const UChar *>(first.data()), int32_t(first.size()),
                            reinterpret_cast<const UChar *>(second.data()), int32_t(second.size()),
                            U_FOLD_CASE_DEFAULT, &error)
            == 0;
}

// 32 hexadecimal digits without hyphens.
static std::string hexUuid()
{
    std::string text = Lingmo::base::Uuid::generate().toString();
    std::erase(text, '-');
    return text;
}

static SMBCFILE *fileFor(uint64_t handle)
{
    return reinterpret_cast<SMBCFILE *>(handle);
}

static void copyField(char *field, int length, const std::string &value)
{
    if (length <= 0)
        return;

    const int count = std::min<int>(value.size(), length - 1);
    std::memcpy(field, value.data(), count);
    field[count] = '\0';
}

SmbBackend::SmbBackend(const RemoteUrl &url)
    : m_url(url)
    , m_uid(::getuid())
    , m_gid(::getgid())
{
}

SmbBackend::~SmbBackend()
{
    if (m_context)
        smbc_free_context(m_context, 1);
}

void SmbBackend::authenticate(SMBCCTX *context, const char *, const char *, char *workgroup,
                              int workgroupLength, char *user, int userLength, char *password,
                              int passwordLength)
{
    auto *self = static_cast<SmbBackend *>(smbc_getOptionUserData(context));
    const Credentials &credentials = self->m_credentials;

    if (credentials.anonymous) {
        copyField(user, userLength, std::string());
        copyField(password, passwordLength, std::string());
        return;
    }

    copyField(user, userLength, credentials.user);
    copyField(password, passwordLength, credentials.password);
    if (!credentials.domain.empty())
        copyField(workgroup, workgroupLength, credentials.domain);
}

SMBCCTX *SmbBackend::createContext(const Credentials &credentials)
{
    SMBCCTX *context = smbc_new_context();
    if (!context)
        return nullptr;

    smbc_setOptionUserData(context, this);
    smbc_setFunctionAuthDataWithContext(context, authenticate);
    // An empty user keeps libsmbclient from signing in as the local account by itself.
    smbc_setUser(context, "");
    smbc_setOptionUseKerberos(context, 1);
    smbc_setOptionFallbackAfterKerberos(context, !credentials.password.empty());
    // Without a password only a Kerberos ticket can sign in; with one, a missing ticket
    // cache must not fail the attempt.
    smbc_setOptionUseCCache(context, credentials.password.empty());
    smbc_setOptionNoAutoAnonymousLogin(context, !credentials.anonymous);

    if (!smbc_init_context(context)) {
        smbc_free_context(context, 0);
        return nullptr;
    }

    return context;
}

std::string SmbBackend::urlFor(const std::string &path) const
{
    std::string url = "smb://" + Lingmo::base::url::percentEncoded(m_url.hostForUrl(), "[]:");
    if (m_url.port > 0)
        url += ':' + std::to_string(m_url.port);

    // libsmbclient decodes %XX in every name, so a literal % must be encoded.
    url += Lingmo::base::url::percentEncoded(path, "/!$&'()*+,;=:@ ~");
    return url;
}

bool SmbBackend::isRoot(const std::string &path)
{
    return path.empty() || path == "/";
}

bool SmbBackend::isShare(const std::string &path)
{
    return !isRoot(path) && path.find('/', 1) == std::string::npos;
}

void SmbBackend::fillStat(struct stat *st) const
{
    // Owners are made up and the mode bits carry DOS attributes; only read-only counts.
    if (S_ISDIR(st->st_mode)) {
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
    } else {
        st->st_mode = S_IFREG | ((st->st_mode & S_IWUSR) ? 0644 : 0444);
        st->st_nlink = 1;
    }

    st->st_uid = m_uid;
    st->st_gid = m_gid;
    st->st_ino = 0;
}

static ConnectResult failure(int code)
{
    ConnectResult result;
    switch (code) {
    case ENOENT:
    case ENODEV:
        result.message = tr("The share does not exist.");
        break;
    case EHOSTUNREACH:
    case EHOSTDOWN:
    case ECONNREFUSED:
    case ETIMEDOUT:
    case ENETUNREACH:
        result.message = tr("The server could not be reached.");
        break;
    default:
        result.message = std::strerror(code);
        break;
    }
    return result;
}

ConnectResult SmbBackend::connect(const Credentials &credentials)
{
    m_credentials = credentials;
    if (m_context)
        smbc_free_context(m_context, 1);

    m_context = createContext(credentials);
    if (!m_context)
        return failure(errno ? errno : EIO);

    // A share in the address is what has to open; otherwise the server's list of shares.
    const std::string &path = m_url.path;
    const size_t start = path.find_first_not_of('/');
    const std::string share = start == std::string::npos ? std::string()
                                                         : path.substr(start, path.find('/', start) - start);
    const std::string target = urlFor(share.empty() ? "/" : '/' + share);

    // Opening a share's folder may succeed before the share is connected; the first
    // entry read is what proves access.
    errno = 0;
    SMBCFILE *directory = smbc_getFunctionOpendir(m_context)(m_context, target.c_str());
    if (directory) {
        errno = 0;
        const bool readable = smbc_getFunctionReaddir(m_context)(m_context, directory) || !errno;
        const int readError = errno;
        smbc_getFunctionClosedir(m_context)(m_context, directory);
        if (readable) {
            ConnectResult result;
            result.status = ConnectResult::Connected;
            return result;
        }
        errno = readError;
    }

    const int code = errno;
    // Kerberos without a ticket fails with EINVAL; a password is what is missing then.
    if (code == EACCES || code == EPERM || (code == EINVAL && credentials.password.empty())) {
        ConnectResult result;
        result.status = ConnectResult::NeedPassword;
        result.needsDomain = true;
        result.anonymousAllowed = true;
        return result;
    }

    return failure(code ? code : EIO);
}

std::string SmbBackend::initialPath() const
{
    return m_url.path.empty() ? std::string("/") : m_url.path;
}

int SmbBackend::listShares(std::vector<DirectoryEntry> *entries)
{
    const std::string url = urlFor("/");
    SMBCFILE *directory = smbc_getFunctionOpendir(m_context)(m_context, url.c_str());
    if (!directory)
        return -errno;

    auto readdir = smbc_getFunctionReaddir(m_context);
    while (struct smbc_dirent *entry = readdir(m_context, directory)) {
        const std::string name(entry->name);
        // Printers, IPC and the administrative shares ending in $ are no folders to browse.
        if (entry->smbc_type != SMBC_FILE_SHARE || name.empty() || name.back() == '$')
            continue;

        DirectoryEntry share;
        share.name = name;
        share.st.st_mode = S_IFDIR | 0755;
        fillStat(&share.st);
        entries->push_back(std::move(share));
    }

    smbc_getFunctionClosedir(m_context)(m_context, directory);
    return 0;
}

int SmbBackend::stat(const std::string &path, struct stat *st)
{
    std::memset(st, 0, sizeof(*st));

    if (isRoot(path)) {
        st->st_mode = S_IFDIR | 0755;
        fillStat(st);
        return 0;
    }

    const std::string url = urlFor(path);
    if (smbc_getFunctionStat(m_context)(m_context, url.c_str(), st) == 0) {
        fillStat(st);
        return 0;
    }

    // A share the account cannot stat may still be listed; show it as a folder.
    const int code = errno;
    if (isShare(path) && (code == EACCES || code == EPERM)) {
        std::memset(st, 0, sizeof(*st));
        st->st_mode = S_IFDIR;
        fillStat(st);
        st->st_mode = S_IFDIR | 0500;
        return 0;
    }

    return -(code ? code : EIO);
}

int SmbBackend::list(const std::string &path, std::vector<DirectoryEntry> *entries)
{
    if (isRoot(path))
        return listShares(entries);

    const std::string url = urlFor(path);
    SMBCFILE *directory = smbc_getFunctionOpendir(m_context)(m_context, url.c_str());
    if (!directory)
        return -(errno ? errno : EIO);

    auto readdir = smbc_getFunctionReaddirPlus2(m_context);
    struct stat st;
    while (const struct libsmb_file_info *info = readdir(m_context, directory, &st)) {
        const std::string name(info->name ? info->name : "");
        if (name.empty() || name == "." || name == "..")
            continue;
        if (!S_ISDIR(st.st_mode) && !S_ISREG(st.st_mode))
            continue;

        DirectoryEntry entry;
        entry.name = name;
        entry.st = st;
        fillStat(&entry.st);
        entries->push_back(std::move(entry));
    }

    smbc_getFunctionClosedir(m_context)(m_context, directory);
    return 0;
}

int SmbBackend::open(const std::string &path, int flags, mode_t mode, uint64_t *handle)
{
    if (isRoot(path) || isShare(path))
        return -EISDIR;

    // Seeking a file opened for appending is broken in libsmbclient; the kernel passes
    // the end of the file as each write's offset anyway.
    const int openFlags = flags & (O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC);
    const std::string url = urlFor(path);

    errno = 0;
    SMBCFILE *file = smbc_getFunctionOpen(m_context)(m_context, url.c_str(), openFlags, mode);
    if (!file) {
        // Some servers fail opening a folder as a file without saying why.
        return -(errno ? errno : ENOTDIR);
    }

    *handle = reinterpret_cast<uint64_t>(file);
    return 0;
}

int SmbBackend::read(uint64_t handle, char *buffer, size_t size, off_t offset)
{
    SMBCFILE *file = fileFor(handle);
    if (smbc_getFunctionLseek(m_context)(m_context, file, offset, SEEK_SET) < 0)
        return -errno;

    size_t done = 0;
    auto read = smbc_getFunctionRead(m_context);
    while (done < size) {
        const ssize_t count = read(m_context, file, buffer + done, size - done);
        if (count < 0)
            return done ? int(done) : -errno;
        if (count == 0)
            break;
        done += count;
    }

    return int(done);
}

int SmbBackend::write(uint64_t handle, const char *buffer, size_t size, off_t offset)
{
    SMBCFILE *file = fileFor(handle);
    if (smbc_getFunctionLseek(m_context)(m_context, file, offset, SEEK_SET) < 0)
        return -errno;

    size_t done = 0;
    auto write = smbc_getFunctionWrite(m_context);
    while (done < size) {
        const ssize_t count = write(m_context, file, buffer + done, size - done);
        if (count <= 0)
            return done ? int(done) : -(errno ? errno : EIO);
        done += count;
    }

    return int(done);
}

int SmbBackend::release(uint64_t handle)
{
    return smbc_getFunctionClose(m_context)(m_context, fileFor(handle)) == 0 ? 0 : -errno;
}

int SmbBackend::truncate(const std::string &path, off_t size, const uint64_t *handle)
{
    if (handle)
        return smbc_getFunctionFtruncate(m_context)(m_context, fileFor(*handle), size) == 0
                ? 0
                : -errno;

    uint64_t opened = 0;
    if (const int result = open(path, O_WRONLY, 0, &opened); result < 0)
        return result;

    const int result = smbc_getFunctionFtruncate(m_context)(m_context, fileFor(opened), size) == 0
            ? 0
            : -errno;
    release(opened);
    return result;
}

int SmbBackend::remove(const std::string &path)
{
    if (isRoot(path) || isShare(path))
        return -EPERM;

    const std::string url = urlFor(path);
    return smbc_getFunctionUnlink(m_context)(m_context, url.c_str()) == 0 ? 0 : -errno;
}

int SmbBackend::makeDirectory(const std::string &path, mode_t mode)
{
    if (isRoot(path) || isShare(path))
        return -EPERM;

    const std::string url = urlFor(path);
    return smbc_getFunctionMkdir(m_context)(m_context, url.c_str(), mode) == 0 ? 0 : -errno;
}

int SmbBackend::removeDirectory(const std::string &path)
{
    if (isRoot(path) || isShare(path))
        return -EPERM;

    const std::string url = urlFor(path);
    if (smbc_getFunctionRmdir(m_context)(m_context, url.c_str()) == 0)
        return 0;

    // ENOTEMPTY is not reliably reported: a folder still there was not empty.
    const int code = errno;
    struct stat st;
    if (smbc_getFunctionStat(m_context)(m_context, url.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        return code == EACCES || code == EPERM ? -code : -ENOTEMPTY;

    return -(code ? code : EIO);
}

int SmbBackend::rename(const std::string &from, const std::string &to, unsigned flags)
{
    if (isRoot(from) || isShare(from) || isRoot(to) || isShare(to))
        return -EPERM;
    if (flags & RENAME_EXCHANGE)
        return -EINVAL;

    // A move between shares needs another connection.
    if (from.substr(0, from.find('/', 1)) != to.substr(0, to.find('/', 1)))
        return -EXDEV;

    const std::string source = urlFor(from);
    const std::string target = urlFor(to);

    // libsmbclient does not reliably report an existing target; look first, unless only
    // the case changes on a server that ignores case.
    struct stat existing;
    const bool caseOnly = sameIgnoringCase(from, to);
    const auto renameUrl = smbc_getFunctionRename(m_context);
    if (!caseOnly && smbc_getFunctionStat(m_context)(m_context, target.c_str(), &existing) == 0) {
        if (flags & RENAME_NOREPLACE)
            return -EEXIST;

        if (S_ISDIR(existing.st_mode)) {
            if (smbc_getFunctionRmdir(m_context)(m_context, target.c_str()) != 0)
                return -ENOTEMPTY;
        } else {
            // The file replaced waits aside until the rename lands: a server that refuses
            // the rename after allowing the delete would otherwise lose it.
            const std::string aside = urlFor(to.substr(0, to.rfind('/') + 1) + ".lingmo-replaced-"
                                             + hexUuid());
            if (renameUrl(m_context, target.c_str(), m_context, aside.c_str()) != 0)
                return -(errno ? errno : EIO);
            if (renameUrl(m_context, source.c_str(), m_context, target.c_str()) != 0) {
                const int code = errno;
                renameUrl(m_context, aside.c_str(), m_context, target.c_str());
                return -(code ? code : EIO);
            }
            smbc_getFunctionUnlink(m_context)(m_context, aside.c_str());
            return 0;
        }
    }

    if (renameUrl(m_context, source.c_str(), m_context, target.c_str()) == 0)
        return 0;

    return -(errno ? errno : EIO);
}

int SmbBackend::setTimes(const std::string &path, const struct timespec times[2])
{
    if (isRoot(path) || isShare(path))
        return 0;

    const std::string url = urlFor(path);
    struct stat current = {};
    bool known = false;

    struct timeval values[2];
    for (int i = 0; i < 2; ++i) {
        if (times[i].tv_nsec == UTIME_NOW) {
            gettimeofday(&values[i], nullptr);
        } else if (times[i].tv_nsec == UTIME_OMIT) {
            if (!known && smbc_getFunctionStat(m_context)(m_context, url.c_str(), &current) != 0)
                return -errno;
            known = true;
            values[i].tv_sec = i == 0 ? current.st_atime : current.st_mtime;
            values[i].tv_usec = 0;
        } else {
            values[i].tv_sec = times[i].tv_sec;
            values[i].tv_usec = times[i].tv_nsec / 1000;
        }
    }

    return smbc_getFunctionUtimes(m_context)(m_context, url.c_str(), values) == 0 ? 0 : -errno;
}

int SmbBackend::statfs(const std::string &path, struct statvfs *st)
{
    std::memset(st, 0, sizeof(*st));
    st->f_bsize = 4096;
    st->f_frsize = 4096;
    st->f_namemax = 255;

    if (isRoot(path))
        return 0;

    const std::string url = urlFor(path);
    struct statvfs info = {};
    if (smbc_getFunctionStatVFS(m_context)(m_context, const_cast<char *>(url.c_str()), &info) != 0)
        return 0;

    // Servers disagree on what the fragment size means; blocks of f_bsize are counted.
    const unsigned long blockSize = info.f_bsize * (info.f_frsize ? info.f_frsize : 1);
    if (!blockSize || !info.f_blocks)
        return 0;

    st->f_bsize = blockSize;
    st->f_frsize = blockSize;
    st->f_blocks = info.f_blocks;
    st->f_bfree = info.f_bfree;
    st->f_bavail = info.f_bavail ? info.f_bavail : info.f_bfree;
    if (info.f_flag & SMBC_VFS_FEATURE_RDONLY)
        st->f_flag |= ST_RDONLY;
    return 0;
}
