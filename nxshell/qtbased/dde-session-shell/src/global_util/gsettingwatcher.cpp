// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gsettingwatcher.h"

#include <QGSettings>
#include <QVariant>
#include <QWidget>

/* convert 'some-key' to 'someKey' or 'SomeKey'.
 * the second form is needed for appending to 'set' for 'setSomeKey'
 */
inline QString qtify_name(const char *name)
{
    bool next_cap = false;
    QString result;

    while (*name) {
        if (*name == '-') {
            next_cap = true;
        } else if (next_cap) {
            result.append(QChar(*name).toUpper().toLatin1());
            next_cap = false;
        } else {
            result.append(*name);
        }

        name++;
    }

    return result;
}

GSettingWatcher::GSettingWatcher(QObject *parent)
    : QObject(parent)
    , m_gsettings(new QGSettings("com.deepin.dde.session-shell", QByteArray(), this))
{
    connect(m_gsettings, &QGSettings::changed, this, &GSettingWatcher::onStatusModeChanged);
}

GSettingWatcher *GSettingWatcher::instance()
{
    static GSettingWatcher w;
    return &w;
}

void GSettingWatcher::bind(const QString &gsettingsName, QWidget *binder)
{
    m_map.insert(gsettingsName, binder);

    setStatus(gsettingsName, binder);
}

void GSettingWatcher::erase(const QString &gsettingsName)
{
    if (m_map.isEmpty() || !m_map.contains(gsettingsName))
        return;

    m_map.remove(gsettingsName);
}

void GSettingWatcher::erase(const QString &gsettingsName, QWidget *binder)
{
    if (m_map.isEmpty() || !m_map.contains(gsettingsName))
        return;

    m_map.remove(gsettingsName, binder);
}

void GSettingWatcher::setStatus(const QString &gsettingsName, QWidget *binder)
{
    if (!binder || !m_gsettings->keys().contains(qtify_name(gsettingsName.toUtf8().data())))
        return;

    const QString setting = m_gsettings->get(gsettingsName).toString();

    if ("Enabled" == setting)
        binder->setEnabled(true);
    else if ("Disabled" == setting)
        binder->setEnabled(false);

    binder->setVisible("Hiden" != setting);
}

const QString GSettingWatcher::getStatus(const QString &gsettingsName)
{
    if (!m_gsettings->keys().contains(qtify_name(gsettingsName.toUtf8().data())))
        return QString();

    return m_gsettings->get(gsettingsName).toString();
}

void GSettingWatcher::onStatusModeChanged(const QString &key)
{
    if (m_map.isEmpty() || !m_map.contains(key))
        return;

    // 重新设置控件对应的显示类型
    for (auto mapUnit = m_map.begin(); mapUnit != m_map.end(); ++mapUnit) {
        if (key == mapUnit.key()) {
            setStatus(key, mapUnit.value());
            emit enableChanged(key, mapUnit.value()->isEnabled());
        }
    }
}
