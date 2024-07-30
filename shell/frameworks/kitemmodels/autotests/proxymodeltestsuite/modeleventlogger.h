/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODELEVENTLOGGER_H
#define MODELEVENTLOGGER_H

#include <QAbstractItemModel>

#include "proxymodeltestsuite_export.h"

class ModelDumper;

class PROXYMODELTESTSUITE_EXPORT PersistentChange : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString oldPath READ getOldPath)
    Q_PROPERTY(QString newPath READ getNewPath)

public:
    PersistentChange(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    QString getPath(const QList<int> &path) const
    {
        QString result(QStringLiteral("QList<int>()"));
        for (const int part : path) {
            result.append(QLatin1String(" << "));
            result.append(QString::number(part));
        }

        return result;
    }
    QString getOldPath() const
    {
        return getPath(oldPath);
    }
    QString getNewPath() const
    {
        return getPath(newPath);
    }

    QList<int> oldPath;
    QList<int> newPath;
};

class PROXYMODELTESTSUITE_EXPORT ModelEvent : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Init,
        RowsInserted,
        RowsRemoved,
        DataChanged,
        LayoutChanged,
        ModelReset,
    };

private:
    // TODO: See if Q_ENUMS does this:
    //   Q_PROPERTY(Type type READ type)

    Q_PROPERTY(QString type READ type)

    Q_PROPERTY(int start READ start)
    Q_PROPERTY(int end READ end)
    // TODO: custom grantlee plugin.
    //   Q_PROPERTY(QList<int> rowAncestors READ rowAncestors)

    Q_PROPERTY(QString rowAncestors READ rowAncestors)
    Q_PROPERTY(bool hasInterpretString READ hasInterpretString)
    Q_PROPERTY(QString interpretString READ interpretString)
    Q_PROPERTY(QVariantList changes READ changes)

public:
    ModelEvent(QObject *parent = nullptr);

    //   Type type() const;
    QString type() const;
    void setType(Type type);

    int start() const;
    void setStart(int start);

    int end() const;
    void setEnd(int end);

    QString rowAncestors() const;
    //   QList<int> rowAncestors() const;
    void setRowAncestors(QList<int> rowAncestors);

    bool hasInterpretString() const;

    QString interpretString() const;
    void setInterpretString(const QString &interpretString);

    void setChanges(const QList<PersistentChange *> &changes)
    {
        m_changedPaths = changes;
    }
    QVariantList changes() const
    {
        QVariantList list;
        for (PersistentChange *change : std::as_const(m_changedPaths)) {
            list.append(QVariant::fromValue(static_cast<QObject *>(change)));
        }
        return list;
    }

private:
    Type m_type;
    int m_start;
    int m_end;
    QList<int> m_rowAncestors;
    QString m_interpretString;
    QList<PersistentChange *> m_changedPaths;
};

/**
 * @brief A logger for QAbstractItemModel events.
 *
 * The log creates the same structure as the @p model, and can be compiled
 * to reproduce failure cases.
 *
 * The log is written to a file, see writeLog for the file naming.
 */
class PROXYMODELTESTSUITE_EXPORT ModelEventLogger : public QObject
{
    Q_OBJECT
public:
    ModelEventLogger(QAbstractItemModel *model, QObject *parent = nullptr);
    void writeLog();
    ~ModelEventLogger() override;

private:
    void persistChildren(const QModelIndex &parent);

private Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void layoutAboutToBeChanged();
    void layoutChanged();
    void modelReset();

private:
    const QAbstractItemModel *const m_model;
    ModelDumper *const m_modelDumper;
    QVariant m_initEvent;
    QVariantList m_events;
    QList<QPersistentModelIndex> m_persistentIndexes;
    QList<QList<int>> m_oldPaths;
    int m_numLogs;
    QString m_modelName;
};

#endif
