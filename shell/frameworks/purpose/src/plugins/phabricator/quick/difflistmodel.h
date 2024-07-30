/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DIFFLISTMODEL_H
#define DIFFLISTMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QList>

class KJob;
class QTemporaryDir;

class DiffListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus)
public:
    DiffListModel(QObject *parent = nullptr);

    void refresh();

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    QString status() const
    {
        return m_status;
    }

    void setStatus(const QString &status);

    void receivedDiffRevs(KJob *job);
    Q_SCRIPTABLE QVariant get(int row, const QByteArray &role);

private:
    struct Value {
        QVariant summary;
        QVariant id;
        QVariant status;
#ifndef QT_NO_DEBUG_STREAM
        operator QString() const
        {
            QString ret = QStringLiteral("DiffListModel::Value{summary=\"%1\" id=\"%2\" status=\"%3\"}");
            return ret.arg(this->summary.toString()).arg(this->id.toString()).arg(this->status.toInt());
        }
#endif
    };
    QList<Value> m_values;

    QString m_status;
    QString m_initialDir;
    QTemporaryDir *m_tempDir;
};

#endif
