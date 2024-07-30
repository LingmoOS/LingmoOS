/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PARAMETERMAP_P_H
#define PARAMETERMAP_P_H

#include <QDataStream>
#include <QMap>
#include <QStringList>

#include <algorithm>
#include <vector>

namespace KContacts
{
struct ParameterData {
    QString param;
    QStringList paramValues;

    bool operator<(const ParameterData &other) const
    {
        return param < other.param;
    }
};

class ParameterMap : public std::vector<ParameterData>
{
public:
    inline const_iterator findParam(const QString &param) const
    {
        return std::find_if(cbegin(), cend(), [&param](const ParameterData &info) {
            return info.param == param;
        });
    }

    inline iterator findParam(const QString &param)
    {
        return std::find_if(begin(), end(), [&param](const ParameterData &info) {
            return info.param == param;
        });
    }

    /**
     * Inserts @p newdata and keeps the vector sorted, the latter needs to
     * be sorted for comparisons.
     */
    inline iterator insertParam(ParameterData &&newdata)
    {
        auto dIt = std::lower_bound(begin(), end(), newdata);
        return insert(dIt, newdata);
    }

    inline QString toString() const
    {
        if (empty()) {
            return {};
        }

        QString str;
        for (const auto &[p, list] : *this) {
            str += QStringLiteral("%1 %2").arg(p, list.join(QLatin1Char(',')));
        }

        return QLatin1String("    parameters: ") + str + QLatin1Char('\n');
    }

    static ParameterMap fromQMap(const QMap<QString, QStringList> &params)
    {
        ParameterMap vec;
        vec.reserve(params.size());
        for (auto it = params.begin(), endIt = params.cend(); it != endIt; ++it) {
            vec.push_back({it.key(), it.value()});
        }
        return vec;
    }

    inline QMap<QString, QStringList> toQMap() const
    {
        QMap<QString, QStringList> map;
        for (const auto &[name, list] : *this) {
            map.insert(name, list);
        }
        return map;
    }
};

inline bool operator==(const ParameterData &a, const ParameterData &b)
{
    return std::tie(a.param, a.paramValues) == std::tie(b.param, b.paramValues);
}

inline QDataStream &operator<<(QDataStream &s, const ParameterMap &map)
{
    s << static_cast<quint32>(map.size());
    for (const auto &[p, list] : map) {
        s << p << list;
    }
    return s;
}

inline QDataStream &operator>>(QDataStream &s, ParameterMap &map)
{
    map.clear();
    quint32 n;
    s >> n;
    for (quint32 i = 0; i < n; ++i) {
        QString p;
        QStringList lst;
        s >> p >> lst;
        if (s.status() != QDataStream::Ok) {
            map.clear();
            break;
        }
        map.push_back({p, lst});
    }

    return s;
}
}

#endif
