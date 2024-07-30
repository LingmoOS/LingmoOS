// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later
#include "action.h"

#include <QIcon>

namespace KRunner
{
class ActionPrivate
{
public:
    explicit ActionPrivate(const QString id, const QString text, const QString iconName)
        : m_id(id)
        , m_text(text)
        , m_iconSource(iconName)
    {
    }
    explicit ActionPrivate() = default;
    explicit ActionPrivate(const ActionPrivate &action) = default;
    const QString m_id;
    const QString m_text;
    const QString m_iconSource;
};

Action::Action(const QString &id, const QString &iconName, const QString &text)
    : d(new ActionPrivate(id, text, iconName))
{
}
Action::Action(const Action &action)
    : d(new ActionPrivate(*action.d))
{
}
Action::Action()
    : d(new ActionPrivate())
{
}

Action::~Action() = default;
Action &Action::operator=(const Action &other)
{
    d.reset(new ActionPrivate(*other.d));
    return *this;
}

QString Action::id() const
{
    return d->m_id;
}
QString Action::text() const
{
    return d->m_text;
}
QString Action::iconSource() const
{
    return d->m_iconSource;
}
}

#include "moc_action.cpp"
