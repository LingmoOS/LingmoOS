// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QAbstractListModel>

class BugzillaManager;

class PlatformModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        Name,
    };
    Q_ENUM(Role)

    using QAbstractListModel::QAbstractListModel;

    Q_PROPERTY(BugzillaManager *manager MEMBER m_manager WRITE setManager NOTIFY managerChanged)
    BugzillaManager *m_manager = nullptr;
    void setManager(BugzillaManager *manager);
    Q_SIGNAL void managerChanged();

    Q_PROPERTY(int detectedPlatformRow READ detectedPlatformRow NOTIFY detectedPlatformRowChanged)
    [[nodiscard]] int detectedPlatformRow();
    Q_SIGNAL void detectedPlatformRowChanged();

    Q_PROPERTY(QString error MEMBER m_error NOTIFY errorChanged)
    QString m_error;
    Q_SIGNAL void errorChanged();

    [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int intRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    QStringList m_list;
};
