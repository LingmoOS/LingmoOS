/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "actions.h"
namespace TaskManager {
class ActionPrivate
{
public:
    ActionPrivate();
    ~ActionPrivate();
    QString m_name;
    Action::Type m_type =  Action::Type::Invalid;
    QVariant m_param;
    QAction *m_action = nullptr;
};

ActionPrivate::ActionPrivate()
{
    m_action = new QAction;
}

ActionPrivate::~ActionPrivate()
{
    if(m_action) {
        delete m_action;
        m_action = nullptr;
    }
}

Action::Action(QObject *parent) : QObject(parent), d(new ActionPrivate())
{
    initConnection();
}
Action::Action(const QString &name, const QString &displayName, const QIcon &icon, Type type,  const QVariant &param, QObject *parent)
        : QObject(parent), d(new ActionPrivate())
{
    d->m_name = name;
    d->m_type = type;
    d->m_param = param;
    d->m_action->setText(displayName);
    d->m_action->setIcon(icon);
    initConnection();
}

Action::Action(const Action &other)
{
    d = other.d;
    initConnection();
}

Action::~Action() = default;

QString Action::name() const
{
    return d->m_name;
}

Action::Type Action::type() const
{
    return d->m_type;
}

void Action::setName(const QString &name)
{
    d->m_name = name;
}

QString Action::text() const
{
    return d->m_action->text();
}

void Action::setText(const QString &name)
{
    d->m_action->setText(name);
}

QIcon Action::icon() const
{
    return d->m_action->icon();
}

void Action::setIcon(const QIcon &icon)
{
    d->m_action->setIcon(icon);
}

void Action::setType(const Action::Type &type)
{
    d->m_type = type;
}

bool Action::operator==(const Action &other) const
{
    return(d->m_name == other.name()
    && d->m_type == other.type());
}

QVariant Action::param()
{
    return d->m_param;
}

void Action::setParam(const QVariant &param)
{
    d->m_param = param;
}

void Action::initConnection()
{
    connect(d->m_action, &QAction::triggered, this, [&](){
        Q_EMIT actionTriggered(d->m_param);
    });
}

QAction *Action::internalAction() const
{
    return d->m_action;
}

bool Action::enabled() const
{
    return d->m_action->isEnabled();
}

void Action::setEnabled(bool enabled)
{
    d->m_action->setEnabled(enabled);
}
}
