/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_UTIL_H
#define KUSERFEEDBACK_CONSOLE_UTIL_H

#include <QMetaEnum>
#include <QMetaType>

class QString;

namespace KUserFeedback {
namespace Console {
namespace Util
{
    template<typename Enum, typename Table, std::size_t N>
    Enum stringToEnum(const QString &s, const Table(&lookupTable)[N])
    {
        for (std::size_t i = 0; i < N; ++i) {
            if (s == QLatin1String(lookupTable[i].name))
                return lookupTable[i].type;
        }
        return Enum();
    }

    template<typename Enum>
    QByteArray enumToString(Enum v)
    {
        const auto mo = QMetaType::metaObjectForType(qMetaTypeId<Enum>());
        if (!mo)
            return {};

        const QByteArray typeName = QMetaType::typeName(qMetaTypeId<Enum>());
        const auto idx = typeName.lastIndexOf("::");
        if (idx <= 0)
            return {};

        const auto enumName = typeName.mid(idx + 2);
        const auto enumIdx = mo->indexOfEnumerator(enumName.constData());
        if (enumIdx < 0)
            return {};

        const auto me = mo->enumerator(enumIdx);
        return me.valueToKeys(v);
    }
}
}
}

#endif // KUSERFEEDBACK_CONSOLE_UTIL_H
