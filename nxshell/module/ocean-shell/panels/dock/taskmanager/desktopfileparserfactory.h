// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstractwindow.h"

#include <QMap>
#include <QObject>
#include <QPointer>

namespace dock {
class DesktopfileAbstractParser;

template<typename... Args>
class DesktopfileParserFactory;

template <typename First, typename... Rest>
class DesktopfileParserFactory<First, Rest...>
{
public:
    [[nodiscard]] static QSharedPointer<DesktopfileAbstractParser> createByWindow(QPointer<AbstractWindow> window)
    {
        auto desktopFileParser = DesktopfileParserFactory<First>::createByWindow(window);
        return desktopFileParser.isNull() ?
            DesktopfileParserFactory<Rest...>::createByWindow(window) : desktopFileParser;
    }

    [[nodiscard]] static QSharedPointer<DesktopfileAbstractParser> createById(const QString& id, const QString& type)
    {
        if (type == First::identifyType()) {
            return DesktopfileParserFactory<First>::createById(id);
        }

        return DesktopfileParserFactory<Rest...>::createById(id, type);
    }
};

template <typename T>
class DesktopfileParserFactory<T>
{
    static_assert(std::is_base_of<DesktopfileAbstractParser, T>::value, "T must be a subclass of DesktopfileAbstractParser");

public:
    [[nodiscard]] static QSharedPointer<DesktopfileAbstractParser> createByWindow(QPointer<AbstractWindow> window)
    {
        auto id = identifyWindow(window);
        if (id.isEmpty()) return nullptr;
        return createById(id);
    }

    [[nodiscard]] static QSharedPointer<DesktopfileAbstractParser> createById(const QString& id, const QString& type = "")
    {
        Q_UNUSED(type)
        auto desktopFileParser = m_desktopFileParsers.value(id, QWeakPointer<T>(nullptr)).toStrongRef();
        if (desktopFileParser.isNull()) {
            desktopFileParser = QSharedPointer<T>(new T(id));
            m_desktopFileParsers.insert(id, desktopFileParser.toWeakRef());
        }
        return desktopFileParser;
    }

private:
    static inline QString identifyWindow(QPointer<AbstractWindow> window)
    {
        return T::identifyWindow(window);
    }

private:
    static inline QMap<QString, QWeakPointer<T>> m_desktopFileParsers;
};
}
