/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PERSON_ACTIONS_H
#define PERSON_ACTIONS_H

#include <QAbstractListModel>

#include <qqmlregistration.h>

class QAction;

namespace KPeople
{
class PersonActionsPrivate;

class PersonActionsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PersonActions)
    Q_PROPERTY(int count READ rowCount NOTIFY personChanged)
    Q_PROPERTY(QString personUri READ personUri WRITE setPersonUri NOTIFY personChanged)

public:
    enum Roles {
        IconNameRole = Qt::UserRole + 1,
        ActionRole,
        ActionTypeRole,
    };

    explicit PersonActionsModel(QObject *parent = nullptr);
    ~PersonActionsModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QString personUri() const;
    void setPersonUri(const QString &personUri);

    QList<QAction *> actions() const;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void triggerAction(int row) const;

Q_SIGNALS:
    void personChanged();

private:
    void resetActions();

    Q_DECLARE_PRIVATE(PersonActions)
    PersonActionsPrivate *const d_ptr;
};
}

#endif // PERSON_ACTIONS_H
