// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "passwordsmodel.h"
#include "passwordprovider.h"
#include "otpprovider.h"

#include <QDebug>
#include <QPointer>

using namespace PlasmaPass;

static constexpr const char *passwordStoreDir = "PASSWORD_STORE_DIR";

struct PasswordsModel::Node {
    explicit Node() = default;
    Node(QString name, PasswordsModel::EntryType type, Node *nodeParent)
        : name(std::move(name))
        , type(type)
        , parent(nodeParent)
    {
        if (parent != nullptr) {
            parent->children.push_back(std::unique_ptr<Node>(this));
        }
    }

    Node(const Node &other) = delete;
    Node(Node &&other) = default;
    Node &operator=(const Node &other) = delete;
    Node &operator=(Node &&other) = delete;

    ~Node() = default;

    QString path() const
    {
        if (parent == nullptr) {
            return name;
        }

        QString fileName = name;
        if (type == PasswordsModel::PasswordEntry) {
            fileName += QStringLiteral(".gpg");
        }
        return parent->path() + QLatin1Char('/') + fileName;
    }

    QString fullName() const
    {
        if (!mFullName.isNull()) {
            return mFullName;
        }

        if (parent == nullptr) {
            return {};
        }
        const auto p = parent->fullName();
        if (p.isEmpty()) {
            mFullName = name;
        } else {
            mFullName = p + QLatin1Char('/') + name;
        }
        return mFullName;
    }

    QString name;
    PasswordsModel::EntryType type = PasswordsModel::FolderEntry;
    QPointer<PasswordProvider> provider;
    QPointer<OTPProvider> otpProvider;
    Node *parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;

private:
    mutable QString mFullName;
};

PasswordsModel::PasswordsModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mWatcher(this)
{
    if (qEnvironmentVariableIsSet(passwordStoreDir)) {
        mPassStore = QDir(QString::fromUtf8(qgetenv(passwordStoreDir)));
    } else {
        mPassStore = QDir(QStringLiteral("%1/.password-store").arg(QDir::homePath()));
    }

    // FIXME: Try to figure out what has actually changed and update the model
    // accordingly instead of reseting it
    connect(&mWatcher, &QFileSystemWatcher::directoryChanged, this, &PasswordsModel::populate);

    populate();
}

PasswordsModel::~PasswordsModel() = default;

PasswordsModel::Node *PasswordsModel::node(const QModelIndex &index)
{
    return static_cast<Node *>(index.internalPointer());
}

QHash<int, QByteArray> PasswordsModel::roleNames() const
{
    return {{NameRole, "name"},
            {EntryTypeRole, "type"},
            {FullNameRole, "fullName"},
            {PathRole, "path"},
            {HasPasswordRole, "hasPassword"},
            {PasswordRole, "password"},
            {OTPRole, "otp"},
            {HasOTPRole, "hasOtp"}};

}

int PasswordsModel::rowCount(const QModelIndex &parent) const
{
    const auto parentNode = parent.isValid() ? node(parent) : mRoot.get();
    return parentNode != nullptr ? static_cast<int>(parentNode->children.size()) : 0;
}

int PasswordsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QModelIndex PasswordsModel::index(int row, int column, const QModelIndex &parent) const
{
    const auto parentNode = parent.isValid() ? node(parent) : mRoot.get();
    if (parentNode == nullptr || row < 0 || static_cast<std::size_t>(row) >= parentNode->children.size() || column != 0) {
        return {};
    }

    return createIndex(row, column, parentNode->children.at(row).get());
}

QModelIndex PasswordsModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return {};
    }

    const auto childNode = node(child);
    if (childNode == nullptr || childNode->parent == nullptr) {
        return {};
    }
    const auto parentNode = childNode->parent;
    if (parentNode == mRoot.get()) {
        return {};
    }

    auto &children = parentNode->parent->children;
    const auto it = std::find_if(children.cbegin(), children.cend(), [parentNode](const auto &node) {
        return node.get() == parentNode;
    });
    Q_ASSERT(it != children.cend());
    return createIndex(std::distance(children.cbegin(), it), 0, parentNode);
}

QVariant PasswordsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    const auto node = this->node(index);
    if (node == nullptr) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return node->name;
    case EntryTypeRole:
        return node->type;
    case PathRole:
        return node->path();
    case FullNameRole:
        return node->fullName();
    case PasswordRole:
        if (node->provider == nullptr) {
            node->provider = new PasswordProvider(node->path());
        }
        return QVariant::fromValue(node->provider.data());
    case OTPRole:
        if (node->otpProvider == nullptr) {
            node->otpProvider = new OTPProvider(node->path());
        }
        return QVariant::fromValue(node->otpProvider.data());
    case HasPasswordRole:
        return !node->provider.isNull();
    case HasOTPRole:
        return !node->otpProvider.isNull();
    default:
        return {};
    }
}

void PasswordsModel::populate()
{
    beginResetModel();
    mRoot = std::make_unique<Node>();
    mRoot->name = mPassStore.absolutePath();
    populateDir(mPassStore, mRoot.get());
    endResetModel();
}

void PasswordsModel::populateDir(const QDir &dir, Node *parent)
{
    mWatcher.addPath(dir.absolutePath());
    auto entries = dir.entryInfoList({QStringLiteral("*.gpg")}, QDir::Files, QDir::NoSort);
    for (const auto &entry : qAsConst(entries)) {
        new Node(entry.completeBaseName(), PasswordEntry, parent);
    }
    entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    for (const auto &entry : qAsConst(entries)) {
        auto node = new Node(entry.fileName(), FolderEntry, parent);
        populateDir(entry.absoluteFilePath(), node);
    }
}

#include "moc_passwordsmodel.cpp"
