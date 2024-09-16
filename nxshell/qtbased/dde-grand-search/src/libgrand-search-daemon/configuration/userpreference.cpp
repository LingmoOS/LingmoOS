// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userpreference.h"

using namespace GrandSearch;

UserPreference::UserPreference(const QVariantHash &datas)
    : m_datas(datas)
{

}

UserPreferencePointer UserPreference::group(const QString &group) const
{
    QReadLocker lk(&m_rwLock);
    if (!m_datas.contains(group))
        return nullptr;

    auto var = m_datas.value(group);
    return var.value<UserPreferencePointer>();
}

void UserPreference::setValue(const QString &name, const QVariant &var)
{
    if (name.isEmpty())
        return;
    QWriteLocker lk(&m_rwLock);
    m_datas.insert(name, var);
}

bool UserPreference::innerValue(const QString &name, QVariant &var) const
{
    QReadLocker lk(&m_rwLock);
    if (m_datas.contains(name)) {
        var = m_datas.value(name);
        return true;
    }

    return false;
}
