// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERPREFERENCE_H
#define USERPREFERENCE_H

#include "preferenceitem.h"

#include <QVariantHash>
#include <QSharedPointer>
#include <QReadWriteLock>

namespace GrandSearch {

typedef QSharedPointer<class UserPreference> UserPreferencePointer;

class UserPreference
{
public:
    explicit UserPreference(const QVariantHash &datas);
    virtual UserPreferencePointer group(const QString &group) const;

    template<typename T>
    T value(const QString &name, const T &def = T()) const
    {
        QVariant var;
        if (innerValue(name, var)) {
            return var.value<T>();
        }

        return def;
    }
    void setValue(const QString &name, const QVariant &var);
protected:
    virtual bool innerValue(const QString &name, QVariant &var) const;
private:
    mutable QReadWriteLock m_rwLock;
    QVariantHash m_datas;
};

}

Q_DECLARE_METATYPE(GrandSearch::UserPreferencePointer)
#endif // USERPREFERENCE_H
