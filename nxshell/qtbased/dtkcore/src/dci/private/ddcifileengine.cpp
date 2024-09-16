// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <filesystem>  //Avoid changing the access control of the standard library
#endif

#define private public
#define protected public
#include <private/qfile_p.h>
#undef private
#undef protected

#include "ddcifileengine_p.h"
#include "dci/ddcifile.h"

#include <QBuffer>
#include <QLoggingCategory>

DCORE_BEGIN_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logFE, "dtk.dci.fileengine")
#else
Q_LOGGING_CATEGORY(logFE, "dtk.dci.fileengine", QtInfoMsg)
#endif

#define DCI_FILE_SCHEME "dci:"
#define DCI_FILE_SUFFIX ".dci"

QAbstractFileEngine *DDciFileEngineHandler::create(const QString &fileName) const
{
    if (!fileName.startsWith(QStringLiteral(DCI_FILE_SCHEME)))
        return nullptr;

    DDciFileEngine *engine = new DDciFileEngine(fileName);
    if (!engine->isValid()) {
        delete engine;
        return nullptr;
    }

    return engine;
}

// 共享同个线程内的同个 DDciFile
static thread_local QHash<QString, QWeakPointer<DDciFile>> sharedDciFile;
static void doDeleteSharedDciFile(const QString &path, DDciFile *file) {
    int count = sharedDciFile.remove(path);
    Q_ASSERT(count > 0);
    delete file;
}

static DDciFileShared getDciFile(const QString &dciFilePath, bool usePath = true)
{
    if (auto shared = sharedDciFile.value(dciFilePath)) {
        return shared.toStrongRef();
    }

    DDciFileShared shared(usePath ? new DDciFile(dciFilePath) : new DDciFile(),
                          std::bind(doDeleteSharedDciFile, dciFilePath,
                                    std::placeholders::_1));
    sharedDciFile[dciFilePath] = shared.toWeakRef();
    return shared;
}

DDciFileEngineIterator::DDciFileEngineIterator(QDir::Filters filters, const QStringList &nameFilters)
    : QAbstractFileEngineIterator(filters, nameFilters)
{

}

QString DDciFileEngineIterator::next()
{
    current = nextValid;
    return DDciFileEngineIterator::currentFileName();
}

bool DDciFileEngineIterator::hasNext() const
{
    if (!file) {
        const auto paths = DDciFileEngine::resolvePath(path());
        if (paths.first.isEmpty()
                || paths.second.isEmpty())
            return false;

        file = getDciFile(paths.first);
        list = file->list(paths.second);
    }

    for (int i = current + 1; i < list.count(); ++i) {
        // 先检查文件类型
        const auto filters = this->filters();
        const auto fileType = file->type(list.at(i));
        if (fileType == DDciFile::Directory) {
            if (!filters.testFlag(QDir::Files))
                continue;
        } else if (fileType == DDciFile::File) {
            if (!filters.testFlag(QDir::Files))
                continue;
        } else if (fileType == DDciFile::Symlink) {
            if (filters.testFlag(QDir::NoSymLinks))
                continue;
        } else { // DDciFile::UnknowFile
            continue;
        }

        // 按名称进行过滤
        if (!nameFilters().isEmpty() && !QDir::match(nameFilters(), list.at(i)))
            continue;

        nextValid = i;
        return true;
    }

    return false;
}

QString DDciFileEngineIterator::currentFileName() const
{
    return file->name(list.at(current));
}

DDciFileEngine::DDciFileEngine(const QString &fullPath)
{
    setFileName(fullPath);
}

DDciFileEngine::~DDciFileEngine()
{
    close();
}

bool DDciFileEngine::isValid() const
{
    return file && file->isValid();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
bool DDciFileEngine::open(QIODevice::OpenMode openMode, std::optional<QFile::Permissions> permissions)
#else
bool DDciFileEngine::open(QIODevice::OpenMode openMode)
#endif
{
    if (fileBuffer) {
        setError(QFile::OpenError, "The file is opened");
        return false;
    }

    if (!file->isValid()) {
        setError(QFile::OpenError, "The DCI file is invalid");
        return false;
    }

    if (file->type(subfilePath) == DDciFile::Directory) {
        setError(QFile::OpenError, "Can't open a directory");
        return false;
    }

    if (file->type(subfilePath) == DDciFile::Symlink) {
        if (!file->exists(file->symlinkTarget(subfilePath))) {
            setError(QFile::OpenError, "The symlink target is not existed");
            return false;
        }
    }

    if (openMode & QIODevice::Text) {
        setError(QFile::OpenError, "Not supported open mode");
        return false;
    }

    if (openMode & QIODevice::NewOnly) {
        if (file->exists(subfilePath)) {
            setError(QFile::OpenError, "The file is existed");
            return false;
        }
    }

    if ((openMode & QIODevice::ExistingOnly)
            || !(openMode & QIODevice::WriteOnly)) {
        if (!file->exists(subfilePath)) {
            setError(QFile::OpenError, "The file is not exists");
            return false;
        }
    }

    // 此时当文件不存在时应当创建它
    if (openMode & QIODevice::WriteOnly) {
        realDciFile.setFileName(dciFilePath);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
        auto success = permissions ? realDciFile.open(openMode, permissions.value()) : realDciFile.open(openMode);
        if (!success)
            return false;
#else
        if (!realDciFile.open(openMode)) {
            return false;
        }
#endif

        // 不存在时尝试新建
        if (!file->exists(subfilePath)
                && !file->writeFile(subfilePath, QByteArray())) {
            return false;
        }
    }

    // 加载数据
    fileData = file->dataRef(subfilePath);
    fileBuffer = new QBuffer(&fileData);
    bool ok = fileBuffer->open(openMode);
    Q_ASSERT(ok);
    if (Q_UNLIKELY(!ok)) {
        delete fileBuffer;
        fileBuffer = nullptr;
        return false;
    }

    return true;
}

bool DDciFileEngine::close()
{
    if (!fileBuffer) {
        return false;
    }

    fileBuffer->close();
    delete fileBuffer;
    fileBuffer = nullptr;

    bool ok = flush();
    realDciFile.close();
    return ok;
}

bool DDciFileEngine::flushToFile(QFile *target, bool writeFile) const
{
    if (target->isWritable()) {
        if (writeFile && !file->writeFile(subfilePath, fileData, true))
            return false;
        if (!target->resize(0))
            return false;
        const QByteArray &data = file->toData();
        if (target->write(data) != data.size())
            return false;
        return true;
    }

    return false;
}

bool DDciFileEngine::flush()
{
    if (!flushToFile(&realDciFile, true))
        return false;

    return realDciFile.flush();
}

bool DDciFileEngine::syncToDisk()
{
    if (!flush())
        return false;
    return realDciFile.d_func()->engine()->syncToDisk();
}

qint64 DDciFileEngine::size() const
{
    if (fileBuffer) {
        return fileData.size();
    }

    return file->dataRef(subfilePath).size();
}

qint64 DDciFileEngine::pos() const
{
    return fileBuffer->size();
}

bool DDciFileEngine::seek(qint64 pos)
{
    return fileBuffer->seek(pos);
}

bool DDciFileEngine::isSequential() const
{
    return false;
}

bool DDciFileEngine::remove()
{
    return file->isValid() && file->remove(subfilePath) && forceSave();
}

bool DDciFileEngine::copy(const QString &newName)
{
    if (!file->isValid())
        return false;
    // 解析出新的 dci 内部文件路径
    const auto paths = resolvePath(newName, dciFilePath);
    if (paths.second.isEmpty())
        return false;

    return file->copy(subfilePath, paths.second) && forceSave();
}

bool DDciFileEngine::rename(const QString &newName)
{
    if (!file->isValid())
        return false;
    // 解析出新的 dci 内部文件路径
    const auto paths = resolvePath(newName, dciFilePath);
    if (paths.second.isEmpty())
        return false;

    return file->rename(subfilePath, paths.second, false) && forceSave();
}

bool DDciFileEngine::renameOverwrite(const QString &newName)
{
    if (!file->isValid())
        return false;
    // 解析出新的 dci 内部文件路径
    const auto paths = resolvePath(newName, dciFilePath);
    if (paths.second.isEmpty())
        return false;

    return file->rename(subfilePath, paths.second, true) && forceSave();
}

bool DDciFileEngine::link(const QString &newName)
{
    if (!file->isValid())
        return false;

    // 解析出新的 dci 内部文件路径
    const auto paths = resolvePath(newName, dciFilePath);
    const QString &linkPath = paths.second.isEmpty() ? newName : paths.second;

    return file->link(subfilePath, linkPath) && forceSave();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
bool DDciFileEngine::mkdir(const QString &dirName,
                           bool createParentDirectories,
                           std::optional<QFile::Permissions> permissions) const
{
    Q_UNUSED(permissions)
#else
bool DDciFileEngine::mkdir(const QString &dirName, bool createParentDirectories) const
{
#endif
    if (!file->isValid())
        return false;
    // 解析出新的 dci 内部文件路径
    const auto paths = resolvePath(dirName, dciFilePath);
    if (paths.second.isEmpty())
        return false;

    if (!createParentDirectories)
        return file->mkdir(paths.second) && forceSave();

    const QStringList dirItems = paths.second.split('/');
    QString currentPath;
    for (const QString &newDir : dirItems) {
        if (newDir.isEmpty())
            continue;
        currentPath += ("/" + newDir);
        if (file->exists(currentPath)) {
            continue;
        }
        // 创建此路径
        if (!file->mkdir(currentPath))
            return false;
    }

    return forceSave();
}

bool DDciFileEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const
{
    if (!file->isValid())
        return false;
    // 解析出新的 dci 内部文件路径
    const auto paths = resolvePath(dirName, dciFilePath);
    if (paths.second.isEmpty())
        return false;

    if (!file->remove(paths.second))
        return false;
    if (!recurseParentDirectories)
        return forceSave();

    // 查找空的父目录
    QDir dir(paths.second);

    while (dir.cdUp()) {
        // 不删除根
        if (dir.isRoot())
            break;

        if (file->childrenCount(dir.absolutePath()) > 0)
            continue;
        if (!file->remove(dir.absolutePath()))
            return false;
    }

    return forceSave();
}

bool DDciFileEngine::setSize(qint64 size)
{
    if (!fileBuffer) {
        fileData = file->dataRef(subfilePath);
    }

    // 确保新数据填充为 0
    if (size > fileData.size()) {
        fileData.append(size - fileData.size(), '\0');
    } else {
        fileData.resize(size);
    }

    return fileBuffer ? true : forceSave(true);
}

bool DDciFileEngine::caseSensitive() const
{
    return true;
}

bool DDciFileEngine::isRelativePath() const
{
    return !subfilePath.startsWith('/');
}

QByteArray DDciFileEngine::id() const
{
    return fileName().toUtf8();
}

uint DDciFileEngine::ownerId(QAbstractFileEngine::FileOwner owner) const
{
    QFileInfo info(dciFilePath);
    return owner == OwnerUser ? info.ownerId() : info.groupId();
}

QString DDciFileEngine::owner(QAbstractFileEngine::FileOwner owner) const
{
    QFileInfo info(dciFilePath);
    return owner == OwnerUser ? info.owner() : info.group();
}

QAbstractFileEngine::FileFlags DDciFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    auto flags = QAbstractFileEngine::FileFlags();

    if (!file->isValid())
        return flags;

    if (type & TypesMask) {
        const auto fileType = file->type(subfilePath);

        if (fileType == DDciFile::Directory) {
            flags |= DirectoryType;
        } else if (fileType == DDciFile::File) {
            flags |= FileType;
        } else if (fileType == DDciFile::Symlink) {
            flags |= LinkType;
        }
    }

    if ((type & FlagsMask)) {
        if (file->exists(subfilePath))
            flags |= ExistsFlag;

        if (subfilePath == QLatin1Char('/'))
            flags |= RootFlag;
    }

    if ((type & PermsMask) && file->exists(subfilePath)) {
        flags |= static_cast<FileFlags>(static_cast<int>(QFileInfo(dciFilePath).permissions()));
    }

    return flags;
}

QString DDciFileEngine::fileName(QAbstractFileEngine::FileName file) const
{
    switch (file) {
    case AbsoluteName:
    case CanonicalName:
    case DefaultName:
        return QDir::cleanPath(DCI_FILE_SCHEME + dciFilePath + subfilePath);
    case AbsolutePathName:
        return QDir::cleanPath(DCI_FILE_SCHEME + dciFilePath);
    case BaseName:
        return QFileInfo(subfilePath).baseName();
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    case AbsoluteLinkTarget:
#else
    case LinkName:
#endif
        return this->file->type(subfilePath) == DDciFile::Symlink
                ? this->file->symlinkTarget(subfilePath)
                : QString();
    default:
        break;
    }

    return QString();
}

void DDciFileEngine::setFileName(const QString &fullPath)
{
    // 销毁旧的内容
    close();
    file.reset(nullptr);
    dciFilePath.clear();
    subfilePath.clear();

    const auto paths = resolvePath(fullPath, QString(), false);
    if (paths.first.isEmpty()
            || paths.second.isEmpty())
        return;

    dciFilePath = paths.first;
    subfilePath = paths.second;
    file = getDciFile(dciFilePath, QFile::exists(dciFilePath));
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 1)
QDateTime DDciFileEngine::fileTime(QFile::FileTime time) const
{
    return QFileInfo(dciFilePath).fileTime(time);
}
#else
QDateTime DDciFileEngine::fileTime(QAbstractFileEngine::FileTime time) const
{
    return QFileInfo(dciFilePath).fileTime(static_cast<QFile::FileTime>(time));
}
#endif

DDciFileEngine::Iterator *DDciFileEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
    return new DDciFileEngineIterator(filters, filterNames);
}

DDciFileEngine::Iterator *DDciFileEngine::endEntryList()
{
    return nullptr;
}

qint64 DDciFileEngine::read(char *data, qint64 maxlen)
{
    return fileBuffer->read(data, maxlen);
}

qint64 DDciFileEngine::write(const char *data, qint64 len)
{
    return fileBuffer->write(data, len);
}

bool DDciFileEngine::extension(QAbstractFileEngine::Extension extension,
                               const QAbstractFileEngine::ExtensionOption *option,
                               QAbstractFileEngine::ExtensionReturn *output)
{
    Q_UNUSED(option)
    Q_UNUSED(output)
    return extension == AtEndExtension && fileBuffer->atEnd();
}

bool DDciFileEngine::supportsExtension(QAbstractFileEngine::Extension extension) const
{
    return extension == AtEndExtension;
}

bool DDciFileEngine::cloneTo(QAbstractFileEngine *target)
{
    const QByteArray &data = file->dataRef(subfilePath);
    return target->write(data.constData(), data.size()) == data.size();
}

bool DDciFileEngine::forceSave(bool writeFile) const
{
    QFile file(dciFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    return flushToFile(&file, writeFile);
}

QPair<QString, QString> DDciFileEngine::resolvePath(const QString &fullPath,
                                                    const QString &realFilePath,
                                                    bool needRealFileExists)
{
    if (!fullPath.startsWith(QStringLiteral(DCI_FILE_SCHEME) + realFilePath))
        return {};

    qCDebug(logFE(), "Resolve the path: \"%s\"", qPrintable(fullPath));
    // 此路径来源于调用方，将其格式化为标准格式，尾部添加 "/" 以确保下文中得到的
    // subfilePath 绝对不为空
    QString formatFullPath = QDir::cleanPath(fullPath) + "/";
    QString dciFilePath = realFilePath, subfilePath;
    const int schemeLength = qstrlen(DCI_FILE_SCHEME);
    const int suffixLength = qstrlen(DCI_FILE_SUFFIX);

    if (dciFilePath.isEmpty()) {
        // 尾部加 "/" 是确保 ".dci" 为一个文件的结尾
        int dciSuffixIndex = formatFullPath.indexOf(DCI_FILE_SUFFIX "/", schemeLength);

        while (dciSuffixIndex > 0) {
            dciSuffixIndex += suffixLength;
            dciFilePath = formatFullPath.mid(schemeLength, dciSuffixIndex - schemeLength);
            // 查找一个有效的后缀名是 ".dci" 的文件
            if (needRealFileExists) {
                if (QFileInfo(dciFilePath).isFile())
                    break;
            } else {
                QFileInfo info(dciFilePath);
                // 不存在的文件允许被新建
                if (!info.exists() && !info.isSymLink())
                    break;
            }

            dciSuffixIndex = dciFilePath.indexOf(DCI_FILE_SUFFIX, dciSuffixIndex + 1);
        }
    } else {
        qCDebug(logFE(), "The base file path of user is: \"%s\"", qPrintable(realFilePath));
    }

    // 未找到有效的 dci 文件
    if (dciFilePath.isEmpty())
        return {};

    subfilePath = QDir::cleanPath(formatFullPath.mid(schemeLength + dciFilePath.length()));
    qCDebug(logFE(), "The DCI file path is: \"%s\", the subfile path is: \"%s\"",
            qPrintable(dciFilePath), qPrintable(subfilePath));
    Q_ASSERT(!subfilePath.isEmpty());

    return qMakePair(dciFilePath, subfilePath);
}

DCORE_END_NAMESPACE
