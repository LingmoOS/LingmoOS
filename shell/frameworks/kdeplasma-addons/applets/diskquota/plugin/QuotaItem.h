/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef PLASMA_QUOTA_ITEM_H
#define PLASMA_QUOTA_ITEM_H

#include <QMetaType>
#include <QString>

/**
 * Class that holds all quota info for one mount point.
 */
class QuotaItem
{
public:
    QuotaItem();

    QString mountPoint() const;
    void setMountPoint(const QString &mountPoint);

    int usage() const;
    void setUsage(int usage);

    QString iconName() const;
    void setIconName(const QString &name);

    QString mountString() const;
    void setMountString(const QString &mountString);

    QString usedString() const;
    void setUsedString(const QString &usedString);

    QString freeString() const;
    void setFreeString(const QString &freeString);

    bool operator==(const QuotaItem &other) const;
    bool operator!=(const QuotaItem &other) const;

private:
    QString m_iconName;
    QString m_mountPoint;
    int m_usage;
    QString m_mountString;
    QString m_usedString;
    QString m_freeString;
};

Q_DECLARE_METATYPE(QuotaItem)

#endif // PLASMA_QUOTA_ITEM_H
