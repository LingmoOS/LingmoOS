/*
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KSambaShareData>
#include <QAbstractListModel>

/**
 * Model of KSambaShareData.
 */
class KSambaShareModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role { Name = Qt::UserRole + 1, Path, ShareUrl, Comment };
    Q_ENUM(Role)

    explicit KSambaShareModel(QObject *parent = nullptr);
    ~KSambaShareModel() override;

    int rowCount(const QModelIndex &parent) const final;
    QVariant data(const QModelIndex &index, int intRole) const final;
    bool hasChildren(const QModelIndex &parent) const final;
    Q_INVOKABLE void showPropertiesDialog(int index);

    QHash<int, QByteArray> roleNames() const final;

public Q_SLOTS:
    void reloadData();

private:
    QList<KSambaShareData> m_list;
    QString m_fqdn;
};
