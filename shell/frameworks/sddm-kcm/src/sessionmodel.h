/*
 * SPDX-FileCopyrightText: 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef SDDM_SESSIONMODEL_H
#define SDDM_SESSIONMODEL_H

#include <QAbstractListModel>
#include <QHash>

class SessionModelPrivate;

class SessionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(SessionModel)

    enum SessionType {
        SessionTypeX,
        SessionTypeWayland,
    };

public:
    enum SessionRole {
        NameRole = Qt::DisplayRole,
        FileRole = Qt::UserRole,
        ExecRole,
        CommentRole,
    };

    explicit SessionModel(QObject *parent = nullptr);
    ~SessionModel() Q_DECL_OVERRIDE;

    void loadDir(const QString &path, SessionType type);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int indexOf(const QString &sessionId) const;

private:
    SessionModelPrivate *d{nullptr};
};

#endif // SDDM_SESSIONMODEL_H
