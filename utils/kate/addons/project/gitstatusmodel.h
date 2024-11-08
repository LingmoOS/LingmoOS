/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QAbstractItemModel>

#include "git/gitstatus.h"

class GitStatusModel : public QAbstractItemModel
{
public:
    explicit GitStatusModel(QObject *parent);

    enum ItemType {
        NodeStage = 0,
        NodeChanges,
        NodeConflict,
        NodeUntrack,
        NodeFile,
    };
    Q_ENUM(ItemType)

    enum Role { TreeItemType = Qt::UserRole + 1, FileNameRole, GitItemType };

public:
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void setStatusItems(GitUtils::GitParsedStatus status);

    const QList<GitUtils::StatusItem> &untrackedFiles() const
    {
        return m_nodes[3];
    }

    const QList<GitUtils::StatusItem> &stagedFiles() const
    {
        return m_nodes[0];
    }

    const QList<GitUtils::StatusItem> &changedFiles() const
    {
        return m_nodes[1];
    }

    QModelIndex getModelIndex(ItemType type) const
    {
        return createIndex(type, 0, 0xFFFFFFFF);
    }

    QModelIndex indexForFilename(const QString &file);

private:
    QList<GitUtils::StatusItem> m_nodes[4];
    QSet<QString> m_nonUniqueFileNames;
};
