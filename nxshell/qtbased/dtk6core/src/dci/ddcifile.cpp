// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddcifile.h"
#include "private/ddcifileengine_p.h"

#ifndef DTK_NO_PROJECT
#include <DObjectPrivate>
#else
#define D_D(class)
#define D_DC(class)
#endif
#include <QDataStream>
#include <QFile>
#include <QLoggingCategory>
#include <QtEndian>
#include <QSaveFile>
#include <QDir>
#include <QBuffer>
#include <QCollator>

DCORE_BEGIN_NAMESPACE

#define MAGIC "DCI"

#define MAGIC_SIZE 4
#define VERSION_SIZE 1
#define FILE_COUNT_SIZE 3
#define FILE_META_SIZE 72

#define FILE_TYPE_SIZE 1
#define FILE_NAME_SIZE 63
#define FILE_DATA_SIZE 8

#define FILE_TYPE_FILE DDciFile::FileType::File
#define FILE_TYPE_DIR DDciFile::FileType::Directory
#define FILE_TYPE_SYMLINK DDciFile::FileType::Symlink

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logDF, "dtk.dci.file")
#else
Q_LOGGING_CATEGORY(logDF, "dtk.dci.file", QtInfoMsg)
#endif

class DDciFilePrivate
#ifndef DTK_NO_PROJECT
    : public DObjectPrivate
#endif
{
public:
    DDciFilePrivate(DDciFile *qq)
#ifndef DTK_NO_PROJECT
        : DObjectPrivate(qq)
    {
#else
    {
    Q_UNUSED(qq)
#endif

    }
    ~DDciFilePrivate();

    void setErrorString(const QString &message);

    void load(const QString &fileName);
    void load(const QByteArray &data);

    QString errorMessage;
    struct Node {
        qint8 type = DDciFile::UnknowFile;
        QString name;

        Node *parent = nullptr;
        QVector<Node*> children; // for directory
        QByteArray data; // for file

        ~Node() {
            qDeleteAll(children);
        }

        QString path() const {
            QString p = name;
            Node *current = parent;
            while (current) {
                p.prepend(current->name + "/");
                current = current->parent;
            }

            return p;
        }

        QString linkPath() const {
            const QString &path = QString::fromUtf8(data);
            QStringView pathView{path};
            if (pathView.startsWith('/'))
                return path;
            // 转为绝对路径
            auto pNode = parent;
            int pathStart = 0;
            while (pathStart < pathView.size()) {
                if (pathView.mid(pathStart, 3) == QLatin1String("../")) {
                    pathStart += 3;
                    pNode = pNode->parent;
                    if (!pNode)
                        return QString();
                } else if (pathView.mid(pathStart, 2) == QLatin1String("./")) {
                    pathStart += 2;
                } else {
                    break;
                }
            }
            Q_ASSERT(pNode);
            return pNode->path() + QLatin1Char('/') + path.mid(pathStart);
        }
    };

    qint64 writeMetaDataForNode(QIODevice *device, Node *node, qint64 dataSize) const;
    qint64 writeDataForNode(QIODevice *device, Node *node) const;
    qint64 writeNode(QIODevice *device, Node *node) const;

    Node *mkNode(const QString &filePath);
    void removeNode(Node *node);
    void copyNode(const Node *from, Node *to);

    bool loadDirectory(Node *directory,
                       const QByteArray &data, qint64 &begin, qint64 end,
                       QHash<QString, Node *> &pathToNode);

    // 按标准中规定的文件排序计算此 name 在这个列表中的位置
    static int getOrderedIndexOfNodeName(const decltype(Node::children) &list, const QString &name);

    qint8 version = 0;
    QScopedPointer<Node> root;
    QHash<QString, Node*> pathToNode;
    QByteArray rawData;
};

DDciFilePrivate::~DDciFilePrivate()
{

}

void DDciFilePrivate::setErrorString(const QString &message)
{
    qCDebug(logDF, "%s", qPrintable(message));
    errorMessage = message;
}

void DDciFilePrivate::load(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        setErrorString(file.errorString());
        return;
    }

    return load(file.readAll());
}

void DDciFilePrivate::load(const QByteArray &data)
{
    // check magic
    if (!data.startsWith("DCI")) {
        setErrorString(QString("Expect value is \"DCI\", "
                               "but actually value is \"%1\"")
                       .arg(QString::fromLatin1(data.left(3))));
        return;
    }

    qint8 version = data.at(MAGIC_SIZE);
    if (version != 1) {
        setErrorString(QString("Not supported version: %1").arg(version));
        return;
    }

    char fileCountData[4];
    int fileCountOffset = MAGIC_SIZE + VERSION_SIZE;
    memcpy(fileCountData, data.constData() + fileCountOffset, FILE_COUNT_SIZE);
    fileCountData[3] = 0;
    int fileCount = qFromLittleEndian<qint32>(fileCountData);

    if (fileCount < 0) {
        setErrorString(QString("Invalid file count: %1").arg(fileCount));
        return;
    }

    qint64 offset = MAGIC_SIZE + VERSION_SIZE + FILE_COUNT_SIZE;
    Node *root = new Node;
    root->type = FILE_TYPE_DIR;
    root->parent = nullptr;

    QHash<QString, Node*> pathToNode;

    if (!loadDirectory(root, data, offset, data.size() - 1, pathToNode)
            || fileCount != root->children.count()) {
        delete root;
        return;
    }

    this->version = version;
    this->root.reset(root);
    this->pathToNode = pathToNode;
    this->pathToNode["/"] = root;
    // Node 中保存的文件数据仅是此数据的引用，因此要确保此数据一直存在
    this->rawData = data;
}

qint64 DDciFilePrivate::writeMetaDataForNode(QIODevice *device, DDciFilePrivate::Node *node, qint64 dataSize) const
{
    qint64 size = 0;
    device->putChar(static_cast<char>(node->type));
    size += FILE_TYPE_SIZE;

    const QByteArray rawName = node->name.toUtf8().left(FILE_NAME_SIZE - 1);
    size += device->write(rawName);
    // 填充未使用的部分
    size += device->write(QByteArray(FILE_NAME_SIZE - rawName.size(), '\0'));

    char int64[FILE_DATA_SIZE];
    qToLittleEndian<qint64>(dataSize, int64);
    size += device->write(int64, FILE_DATA_SIZE);
    Q_ASSERT(size == FILE_META_SIZE);

    return size;
}

qint64 DDciFilePrivate::writeDataForNode(QIODevice *device, DDciFilePrivate::Node *node) const
{
    if (node->type == FILE_TYPE_FILE
            ||  node->type == FILE_TYPE_SYMLINK) {
        return device->write(node->data);
    } else if (node->type == FILE_TYPE_DIR) {
        qint64 dataSize = 0;
        for (Node *child : node->children) {
            dataSize += writeNode(device, child);
        }
        return dataSize;
    }

    return 0;
}

qint64 DDciFilePrivate::writeNode(QIODevice *device, DDciFilePrivate::Node *node) const
{
    const qint64 metaDataPos = device->pos();
    device->seek(metaDataPos + FILE_META_SIZE);
    const qint64 dataSize = writeDataForNode(device, node);
    device->seek(metaDataPos);
    const qint64 metaDataSize = writeMetaDataForNode(device, node, dataSize);
    device->seek(device->pos() + dataSize);
    return metaDataSize + dataSize;
}

DDciFilePrivate::Node *DDciFilePrivate::mkNode(const QString &filePath)
{
    qCDebug(logDF, "Request create a node");

    if (pathToNode.contains(filePath)) {
        setErrorString(QString("The \"%1\" is existed").arg(filePath));
        return nullptr;
    }

    const QFileInfo info(filePath);
    qCDebug(logDF, "The parent directory is \"%s\"", qPrintable(info.path()));

    if (Node *parentNode = pathToNode.value(info.path())) {
        if (parentNode->type != FILE_TYPE_DIR) {
            setErrorString(QString("The \"%1\" is not a directory").arg(info.path()));
            return nullptr;
        }

        // 检查文件名长度，避免溢出
        if (info.fileName().toUtf8().size() > FILE_NAME_SIZE - 1) {
            setErrorString(QString("The file name size must less then %1 bytes").arg(FILE_NAME_SIZE));
            return nullptr;
        }

        Node *newNode = new Node;
        newNode->name = info.fileName();
        newNode->parent = parentNode;

        const int index = getOrderedIndexOfNodeName(parentNode->children, newNode->name);
        parentNode->children.insert(index, newNode);
        pathToNode[newNode->path()] = newNode;

        return newNode;
    } else {
        setErrorString("The parent directory is not exists");
        return nullptr;
    }
}

void DDciFilePrivate::removeNode(DDciFilePrivate::Node *node)
{
    Q_ASSERT(node != root.data());

    node->parent->children.removeOne(node);
    auto removedNode = pathToNode.take(node->path());
    Q_ASSERT(removedNode == node);

    for (Node *child : node->children) {
        Q_ASSERT(child->parent == node);
        auto removedNode = pathToNode.take(child->path());
        Q_ASSERT(removedNode == child);
    }

    delete node;
}

void DDciFilePrivate::copyNode(const DDciFilePrivate::Node *from, DDciFilePrivate::Node *to)
{
    QList<QPair<const Node*, Node*>> copyPendingList;
    copyPendingList << qMakePair(from, to);

    for (int i = 0; i < copyPendingList.size(); ++i) {
        auto f = copyPendingList.at(i).first;
        auto t = copyPendingList.at(i).second;

        t->type = f->type;
        t->data = f->data;

        for (const auto child : f->children) {
            if (child == to)
                continue;

            Node *newChild = new Node;
            newChild->parent = t;
            newChild->name = child->name;
            pathToNode[newChild->path()] = newChild;

            const int index = getOrderedIndexOfNodeName(t->children, newChild->name);
            t->children.insert(index, newChild);
            copyPendingList << qMakePair(child, newChild);
        }
    }
}

bool DDciFilePrivate::loadDirectory(DDciFilePrivate::Node *directory,
                                    const QByteArray &data, qint64 &offset, qint64 end,
                                    QHash<QString, DDciFilePrivate::Node *> &pathToNode)
{
    // load files
    while (offset < end) {
        Node *node = new Node;

        node->parent = directory;
        node->type = data.at(offset);
        offset += FILE_TYPE_SIZE;
        // 计算文件名的长度
        const int nameLength = data.indexOf('\0', offset) - offset;
        if (nameLength <= 0 || nameLength >= FILE_NAME_SIZE) {
            setErrorString(QString("Invalid file name, the data offset: %1").arg(offset));
            delete node;
            return false;
        }
        node->name = QString::fromUtf8(data.constData() + offset, nameLength);
        offset += FILE_NAME_SIZE;

        const qint64 dataSize = qFromLittleEndian<qint64>(data.constData() + offset);
        offset += FILE_DATA_SIZE;

        // 无失败时调用 break
        do {
            if (node->type == FILE_TYPE_DIR) {
                if (loadDirectory(node, data, offset, offset + dataSize - 1, pathToNode)) {
                    break;
                }
            } else if (node->type == FILE_TYPE_FILE
                       || node->type == FILE_TYPE_SYMLINK) {
                // 跳过文件内容
                node->data = QByteArray::fromRawData(data.constData() + offset, dataSize);

                if (node->data.size() == dataSize) {
                    offset += dataSize;
                    break;
                } else {
                    setErrorString(QString("Invalid data size of \"%1\" file").arg(node->path()));
                }
            } else {
                setErrorString(QString("Invalid file type: %1").arg(node->type));
            }

            delete node;
            return false;
        } while (false);

        directory->children << node;
        pathToNode[node->path()] = node;
    }

    return true;
}

int DDciFilePrivate::getOrderedIndexOfNodeName(const decltype(Node::children) &list, const QString &name)
{
    QCollator collator(QLocale::English);
    collator.setNumericMode(true);
    for (int i = 0; i < list.count(); ++i) {
        const Node *node = list.at(i);
        if (collator.compare(name, node->name) < 0)
            return i;
    }

    return list.count();
}

void DDciFile::registerFileEngine()
{
    // 在 QAbstractFileEngineHandler 的构造函数中会注册自己，后续
    // 在使用 QFile 时会调用 DDciFileEngineHandler::create
    static DDciFileEngineHandler globalHandler;
    Q_UNUSED(globalHandler);
}

#ifndef DTK_NO_PROJECT
DDciFile::DDciFile()
    : DObject(*new DDciFilePrivate(this))
{
    d_func()->load(QByteArrayLiteral("DCI\0\1\0\0\0"));
}

DDciFile::DDciFile(const QString &fileName)
    : DObject(*new DDciFilePrivate(this))
{
    d_func()->load(fileName);
}

DDciFile::DDciFile(const QByteArray &data)
    : DObject(*new DDciFilePrivate(this))
{
    d_func()->load(data);
}
#else
DDciFile::DDciFile()
    : d(new DDciFilePrivate(this))
{
    d->load(QByteArrayLiteral("DCI\0\1\0\0\0"));
}

DDciFile::DDciFile(const QString &fileName)
    : d(new DDciFilePrivate(this))
{
    d->load(fileName);
}

DDciFile::DDciFile(const QByteArray &data)
    : d(new DDciFilePrivate(this))
{
    d->load(data);
}
#endif

bool DDciFile::isValid() const
{
    D_DC(DDciFile);
    return !!(d->root);
}

QString DDciFile::lastErrorString() const
{
    D_DC(DDciFile);
    return d->errorMessage;
}

bool DDciFile::writeToFile(const QString &fileName) const
{
    QSaveFile sf(fileName);
    do {
        if (!sf.open(QIODevice::WriteOnly)) {
            break;
        }
        if (!writeToDevice(&sf))
            return false;
        if (!sf.commit())
            break;
        return true;
    } while (false);

    qCDebug(logDF, "Failed on write to file \"%s\", error message is: \"%s\"",
            qPrintable(fileName), qPrintable(sf.errorString()));
    return false;
}

bool DDciFile::writeToDevice(QIODevice *device) const
{
    Q_ASSERT(isValid());
    D_DC(DDciFile);

    // magic
    device->write(QByteArrayLiteral("DCI\0"));
    // version
    device->putChar(static_cast<char>(d->version));
    char fileCountData[sizeof(int)];
    qToLittleEndian<int>(d->root->children.count(), fileCountData);
    // file count
    device->write(fileCountData, FILE_COUNT_SIZE);
    d->writeDataForNode(device, d->root.data());

    return device->size() >= metadataSizeV1()
            + (d->pathToNode.count() - 1) * FILE_META_SIZE;
}

QByteArray DDciFile::toData() const
{
    if (!isValid())
        return QByteArray();

    D_DC(DDciFile);

    qint64 allFilesContentSize = 0;
    for (auto node : d->pathToNode) {
        if (node->type == FILE_TYPE_FILE
                || node->type == FILE_TYPE_SYMLINK)
            allFilesContentSize += node->data.size();
    }

    QByteArray data;
    // -1 是排除根目录
    data.resize(metadataSizeV1() + (d->pathToNode.count() - 1) * FILE_META_SIZE
                 + allFilesContentSize);
    QBuffer buffer(&data);

    if (!buffer.open(QIODevice::WriteOnly) || !writeToDevice(&buffer))
        return QByteArray();

    return data;
}

constexpr int DDciFile::metadataSizeV1()
{
    return MAGIC_SIZE + VERSION_SIZE + FILE_COUNT_SIZE;
}

QStringList DDciFile::list(const QString &dir, bool onlyFileName) const
{
    if (!isValid())
        return {};

    D_DC(DDciFile);

    auto dirNode = d->pathToNode.value(dir);
    if (!dirNode) {
        qCDebug(logDF, "The \"%s\" is not exists", qPrintable(dir));
        return {};
    }

    if (dirNode->type != FILE_TYPE_DIR) {
        qCWarning(logDF, "The \"%s\" is not a directory", qPrintable(dir));
        return {};
    }

    QStringList children;
    for (auto child : dirNode->children) {
        children << (onlyFileName ? child->name : QDir(dir).filePath(child->name));
    }

    return children;
}

int DDciFile::childrenCount(const QString &dir) const
{
    if (!isValid())
        return 0;

    D_DC(DDciFile);

    auto dirNode = d->pathToNode.value(dir);
    if (!dirNode) {
        return 0;
    }

    return dirNode->children.count();
}

bool DDciFile::exists(const QString &filePath) const
{
    if (!isValid())
        return false;

    D_DC(DDciFile);
    return d->pathToNode.contains(filePath);
}

DDciFile::FileType DDciFile::type(const QString &filePath) const
{
    if (!isValid())
        return UnknowFile;

    D_DC(DDciFile);

    auto node = d->pathToNode.value(filePath);
    if (!node) {
        qCDebug(logDF, "The \"%s\" is not exists", qPrintable(filePath));
        return DDciFile::UnknowFile;
    }

    return static_cast<DDciFile::FileType>(node->type);
}

QByteArray DDciFile::dataRef(const QString &filePath) const
{
    if (!isValid())
        return QByteArray();

    D_DC(DDciFile);

    auto node = d->pathToNode.value(filePath);
    if (!node) {
        qCDebug(logDF, "The \"%s\" is not exists", qPrintable(filePath));
        return QByteArray();
    }

    if (node->type == FILE_TYPE_SYMLINK) {
        return dataRef(node->linkPath());
    }

    return node->data;
}

QString DDciFile::name(const QString &filePath) const
{
    if (!isValid())
        return QString();

    D_DC(DDciFile);
    if (auto node = d->pathToNode.value(filePath)) {
        return node->name;
    }

    return QString();
}

QString DDciFile::symlinkTarget(const QString &filePath, bool originData) const
{
    if (!isValid())
        return QString();

    D_DC(DDciFile);
    if (auto node = d->pathToNode.value(filePath)) {
        if (node->type != FILE_TYPE_SYMLINK)
            return QString();

        if (originData) {
            return QString::fromUtf8(node->data);
        }

        const QString &linkPath = node->linkPath();
        const auto targetNode = d->pathToNode.value(linkPath);

        // 链接的目标只能是“不存在的路径”、“文件”、“链接”，不可是目录
        if (!targetNode || targetNode->type == FILE_TYPE_FILE
                || targetNode->type == FILE_TYPE_SYMLINK)
            return linkPath;
    }

    return QString();
}

bool DDciFile::mkdir(const QString &filePath)
{
    Q_ASSERT(isValid());
    D_D(DDciFile);

    qCDebug(logDF, "Request create the \"%s\" directory", qPrintable(filePath));
    auto node = d->mkNode(filePath);
    if (!node)
        return false;
    node->type = FILE_TYPE_DIR;
    return true;
}

bool DDciFile::writeFile(const QString &filePath, const QByteArray &data, bool override)
{
    Q_ASSERT(isValid());
    D_D(DDciFile);

    qCDebug(logDF, "Request create the \"%s\" file", qPrintable(filePath));
    // 先删除旧的数据
    if (auto node = d->pathToNode.value(filePath)) {
        if (override) {
            if (node->type == FILE_TYPE_SYMLINK) {
                const QString &linkPath = node->linkPath();
                qCDebug(logDF(), "Follow the symlink to \"%s\"", qPrintable(linkPath));

                if (!d->pathToNode.contains(linkPath)) {
                    qCDebug(logDF(), "Can't write to a symlink target file if it is not existed");
                    return false;
                }

                return writeFile(linkPath, data, override);
            }

            qCDebug(logDF, "Try override the file");
            if (node->type != FILE_TYPE_FILE) {
                qCWarning(logDF, "The \"%s\" is existed and it is not a file", qPrintable(filePath));
                return false;
            }

            node->data = data;
            return true;
        } else {
            d->setErrorString("No the \"override\" flag and the file is existed, can't write");
            return false;
        }
    }

    auto node = d->mkNode(filePath);
    if (!node)
        return false;

    node->type = FILE_TYPE_FILE;
    node->data = data;
    return true;
}

bool DDciFile::remove(const QString &filePath)
{
    Q_ASSERT(isValid());
    D_D(DDciFile);

    if (auto node = d->pathToNode.value(filePath)) {
        if (node == d->root.data()) {
            for (auto child : d->root->children)
                d->removeNode(child);
            d->root->children.clear();
        } else {
            d->removeNode(node);
        }
        return true;
    } else {
        d->setErrorString("The file is not exists");
        return false;
    }
}

bool DDciFile::rename(const QString &filePath, const QString &newFilePath, bool override)
{
    Q_ASSERT(isValid());
    D_D(DDciFile);

    qCDebug(logDF, "Rename from \"%s\" to \"%s\"", qPrintable(filePath), qPrintable(newFilePath));
    if (filePath == newFilePath)
        return false;

    if (newFilePath.toUtf8().size() >= FILE_META_SIZE) {
        d->setErrorString(QString("The new name size must less then %1 bytes").arg(FILE_NAME_SIZE));
        return false;
    }

    if (!override && d->pathToNode.contains(newFilePath)) {
        d->setErrorString("The target file is existed");
        return false;
    }
    auto overrideNode = override ? d->pathToNode.take(newFilePath) : nullptr;

    if (auto node = d->pathToNode.take(filePath)) {
        QFileInfo info(newFilePath);
        if (auto parent = d->pathToNode.value(info.absolutePath())) {
            node->name = info.fileName();

            // 从旧节点删除添加到新的节点
            if (node->parent != parent) {
                bool ok = node->parent->children.removeOne(node);
                Q_ASSERT(ok);
                const int index = d->getOrderedIndexOfNodeName(parent->children, node->name);
                parent->children.insert(index, node);
                node->parent = parent;
            }

            d->pathToNode[info.absoluteFilePath()] = node;
            Q_ASSERT(node->path() == info.absoluteFilePath());

            // 删除被覆盖的节点
            if (overrideNode) {
                Q_ASSERT(overrideNode->parent == parent);
                overrideNode->parent->children.removeOne(overrideNode);
                delete overrideNode;
            }

            return true;
        } else {
            d->setErrorString(QString("The \"%1\" directory is not exists").arg(info.absolutePath()));
            return false;
        }
    } else {
        d->setErrorString("The file is not exists");
        return false;
    }
}

bool DDciFile::copy(const QString &from, const QString &to)
{
    Q_ASSERT(isValid());
    D_D(DDciFile);

    const auto fromNode = d->pathToNode.value(from);
    if (!fromNode) {
        d->setErrorString(QString("The \"%1\" is not exists").arg(from));
        return false;
    }

    auto toNode = d->mkNode(to);
    if (!toNode) {
        return false;
    }

    d->copyNode(fromNode, toNode);
    return true;
}

bool DDciFile::link(const QString &source, const QString &to)
{
    Q_ASSERT(isValid());
    D_D(DDciFile);

    if (source == to || source.isEmpty())
        return false;

    auto toNode = d->mkNode(to);
    if (!toNode)
        return false;
    toNode->type = FILE_TYPE_SYMLINK;
    toNode->data = source.toUtf8();

    return true;
}

DCORE_END_NAMESPACE
