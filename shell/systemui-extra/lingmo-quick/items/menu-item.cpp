/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

#include "menu-item.h"
#include "icon-helper.h"

namespace LingmoUIQuick {

MenuItem::MenuItem(QObject *parent)
{
    setAction(new QAction(this));
}

QAction *MenuItem::action() const
{
    return m_action;
}

void MenuItem::setAction(QAction *action)
{
    if (m_action != action) {
        if (m_action) {
            disconnect(m_action, nullptr, this, nullptr);
            if (m_action->parent() == this) {
                delete m_action;
                m_action = nullptr;
            }
        }

        if (action) {
            m_action = action;
        } else {
            m_action = new QAction(this);
            m_action->setVisible(false);
        }

        setEnabled(m_action->isEnabled());

        connect(m_action, &QAction::changed, this, &MenuItem::textChanged);
        connect(m_action, &QAction::changed, this, &MenuItem::checkableChanged);
        connect(m_action, &QAction::changed, this, &MenuItem::enabledChanged);
        connect(m_action, &QAction::toggled, this, &MenuItem::toggled);
        connect(m_action, &QAction::triggered, this, &MenuItem::clicked);
        connect(m_action, &QObject::destroyed, this, [this] {
            if (m_action->parent() != this) {
                m_action = new QAction(this);
                m_action->setVisible(false);
                Q_EMIT actionChanged();
            }
        });
        connect(this, &QObject::destroyed, this, &MenuItem::deleteLater);
        Q_EMIT actionChanged();
    }
}

QString MenuItem::icon() const
{
    return m_action->icon().name();
}

void MenuItem::setIcon(const QString &icon)
{
    m_action->setIcon(IconHelper::loadIcon(icon));
}

QString MenuItem::text() const
{
    return m_action->text();
}

void MenuItem::setText(const QString &text)
{
    if (m_action->text() != text) {
        m_action->setText(text);
    }
}

bool MenuItem::checkable() const
{
    return m_action->isCheckable();
}

void MenuItem::setCheckable(bool checkable)
{
    m_action->setCheckable(checkable);
}

bool MenuItem::checked() const
{
    return m_action->isChecked();
}

void MenuItem::setChecked(bool checked)
{
    m_action->setChecked(checked);
}

bool MenuItem::isEnabled() const
{
    return m_action->isEnabled();
}

void MenuItem::setEnabled(bool enabled)
{
    m_action->setEnabled(enabled);
}

}
