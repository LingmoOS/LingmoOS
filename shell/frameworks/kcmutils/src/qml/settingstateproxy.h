/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SETTINGSTATEPROXY_H
#define SETTINGSTATEPROXY_H

#include <QObject>
#include <QPointer>
#include <QQmlEngine>

#include <KCoreConfigSkeleton>

/**
 * This element allows to represent in a declarative way the
 * state of a particular setting in a config object.
 *
 * @since 5.73
 */
class SettingStateProxy : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * The config object which will be monitored for setting state changes
     */
    Q_PROPERTY(KCoreConfigSkeleton *configObject READ configObject WRITE setConfigObject NOTIFY configObjectChanged)

    /**
     * The name of the setting in the config object
     */
    Q_PROPERTY(QString settingName READ settingName WRITE setSettingName NOTIFY settingNameChanged)

    /**
     * Indicates if the setting is marked as immutable
     */
    Q_PROPERTY(bool immutable READ isImmutable NOTIFY immutableChanged)

    /**
     * Indicates if the setting differs from its default value
     */
    Q_PROPERTY(bool defaulted READ isDefaulted NOTIFY defaultedChanged)

public:
    using QObject::QObject;

    KCoreConfigSkeleton *configObject() const;
    void setConfigObject(KCoreConfigSkeleton *configObject);

    QString settingName() const;
    void setSettingName(const QString &settingName);

    bool isImmutable() const;
    bool isDefaulted() const;

Q_SIGNALS:
    void configObjectChanged();
    void settingNameChanged();

    void immutableChanged();
    void defaultedChanged();

private Q_SLOTS:
    void updateState();

private:
    void connectSetting();

    QPointer<KCoreConfigSkeleton> m_configObject;
    QString m_settingName;
    bool m_immutable = false;
    bool m_defaulted = true;
};

#endif
