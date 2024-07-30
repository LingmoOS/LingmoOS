// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PASSWORDSMODEL_H_
#define PASSWORDSMODEL_H_

#include <QAbstractItemModel>
#include <QDir>
#include <QFileSystemWatcher>

#include <memory>

namespace PlasmaPass
{
class PasswordsModel : public QAbstractItemModel
{
    Q_OBJECT

    struct Node;

public:
    enum EntryType {
        FolderEntry,
        PasswordEntry,
    };
    Q_ENUM(EntryType)

    enum Roles {
        NameRole = Qt::DisplayRole,
        EntryTypeRole = Qt::UserRole,
        FullNameRole,
        PathRole,
        PasswordRole,
        OTPRole,
        HasPasswordRole,
        HasOTPRole
    };

    explicit PasswordsModel(QObject *parent = nullptr);
    ~PasswordsModel() override;

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    QVariant data(const QModelIndex &index, int role) const override;

private:
    void populate();
    void populateDir(const QDir &dir, Node *parent);

    static Node *node(const QModelIndex &index);

    QFileSystemWatcher mWatcher;
    QDir mPassStore;

    std::unique_ptr<Node> mRoot;
};

}
#endif
