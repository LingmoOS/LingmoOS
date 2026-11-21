// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The core's sources manage Qt objects (QProcess, QSettings) and name them
// with QString, while the lingmo-sdk's base library takes std::string. These
// are the conversions between the two, so a call site can stay Qt-flavoured
// without a cast at every argument. Included by the targets that link Qt.
#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include <string>
#include <utility>
#include <vector>

#include "base/logging.h"

// Qt defines `emit` as an empty macro; the toolkit's signals have an emit()
// method, and the toolkit's own headers save and restore the macro around
// their declarations.
#undef emit

// The toolkit's log stream has no QString overload; log the UTF-8 text. The
// LM_* macros build the stream as a temporary, so both value categories are
// needed.
inline Lingmo::base::LogStream &operator<<(Lingmo::base::LogStream &stream, const QString &value)
{
    return stream << value.toStdString();
}

inline Lingmo::base::LogStream &&operator<<(Lingmo::base::LogStream &&stream, const QString &value)
{
    stream << value.toStdString();
    return std::move(stream);
}

inline Lingmo::base::LogStream &operator<<(Lingmo::base::LogStream &stream, const QStringList &values)
{
    return stream << values.join(QLatin1Char(' ')).toStdString();
}

inline Lingmo::base::LogStream &&operator<<(Lingmo::base::LogStream &&stream, const QStringList &values)
{
    stream << values.join(QLatin1Char(' ')).toStdString();
    return std::move(stream);
}

inline std::string toStdString(const QString &value)
{
    return value.toStdString();
}

inline std::vector<std::string> toStdStrings(const QStringList &values)
{
    std::vector<std::string> strings;
    strings.reserve(values.size());
    for (const QString &value : values)
        strings.push_back(value.toStdString());
    return strings;
}

inline QStringList toQStringList(const std::vector<std::string> &values)
{
    QStringList strings;
    for (const std::string &value : values)
        strings.append(QString::fromStdString(value));
    return strings;
}

inline std::vector<uint32_t> toStdVector(const QList<uint32_t> &values)
{
    return std::vector<uint32_t>(values.begin(), values.end());
}
