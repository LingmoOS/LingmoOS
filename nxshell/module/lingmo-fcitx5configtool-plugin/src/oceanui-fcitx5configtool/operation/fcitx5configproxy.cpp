// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fcitx5configproxy.h"
#include "private/fcitx5configproxy_p.h"

#include <dbusprovider.h>
#include <varianthelper.h>

#include <QDBusPendingCallWatcher>

using namespace lingmo::fcitx5configtool;

Fcitx5ConfigProxyPrivate::Fcitx5ConfigProxyPrivate(Fcitx5ConfigProxy *parent,
                                                 fcitx::kcm::DBusProvider *dbus,
                                                 const QString &path)
    : q(parent), dbusprovider(dbus), path(path)
{
    timer = new QTimer(q);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, q, &Fcitx5ConfigProxy::save);
}

QStringList Fcitx5ConfigProxyPrivate::formatKey(const QString &shortcut) {
    QStringList list;
    for (const auto &key : shortcut.split("+")) {
        if (key.trimmed().toLower() == "control")
            list << "Ctrl";
        else if (key.trimmed().toLower() == "super")
            list << "Meta";
        else if (key.trimmed().toLower() == "backspace")
            list << "Backspace";
        else if (key.trimmed().toLower() == "space")
            list << "Space";
        else
            list << key.trimmed();
    }
    if (list.size() == 3 && list.contains("Ctrl") && list.contains("Meta")) {
        if (list.contains("Control_L")) {
            list.removeAll("Control_L");
        } else {
            list.removeAll("Control_R");
        }
    } else if (list.size() == 3 && list.contains("Shift") && list.contains("Meta")) {
        if (list.contains("Shift_L")) {
            list.removeAll("Shift_L");
        } else {
            list.removeAll("Shift_R");
        }
    } else if (list.size() == 3 && list.contains("Alt") && list.contains("Meta")) {
        if (list.contains("Alt_L")) {
            list.removeAll("Alt_L");
        } else {
            list.removeAll("Shift_R");
        }
    }
    return list;
}

QString Fcitx5ConfigProxyPrivate::formatKeys(const QStringList &keys) {
    QStringList list;   
    for (const auto &key : keys) {
        if (key.trimmed().toLower() == "ctrl")
            list << "Control";
        else if (key.trimmed().toLower() == "meta")
            list << "Super";
        else if (key.trimmed().toLower() == "backspace")
            list << "BackSpace";
        else if (key.trimmed().toLower() == "space")
            list << "space";
        else
            list << key.trimmed();
    }
    if (list.size() == 2 && list.contains("Shift") && list.contains("Super")) {
        list.append("Shift_L");
    } else if (list.size() == 2 && list.contains("Control") && list.contains("Super")) {
        list.append("Control_L");
    } else if (list.size() == 2 && list.contains("Alt") && list.contains("Super")) {
        list.append("Alt_L");
    }

    return list.join("+");
}

QVariant Fcitx5ConfigProxyPrivate::readDBusValue(const QVariant &value) {
    if (value.canConvert<QDBusArgument>()) {
        auto argument = qvariant_cast<QDBusArgument>(value);
        QVariantMap map;
        argument >> map;
        
        QVariantMap resultMap;
        for (auto iter = map.begin(); iter != map.end(); ++iter) {
            resultMap[iter.key()] = readDBusValue(iter.value());
        }
        return resultMap;
    }
    
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (value.typeId() == QMetaType::QVariantMap) {
#else
    if (value.type() == QVariant::Map) {
#endif
        QVariantMap resultMap;
        const auto map = value.toMap();
        for (auto iter = map.begin(); iter != map.end(); ++iter) {
            resultMap[iter.key()] = readDBusValue(iter.value());
        }
        return resultMap;
    }
    
    return value;
}

Fcitx5ConfigProxy::Fcitx5ConfigProxy(fcitx::kcm::DBusProvider *dbus, const QString &path, QObject *parent)
    : QObject(parent), d(new Fcitx5ConfigProxyPrivate(this, dbus, path))
{
}

Fcitx5ConfigProxy::~Fcitx5ConfigProxy() = default;

void Fcitx5ConfigProxy::clear()
{
    d->configValue.clear();
    d->configTypes.clear();
    Q_EMIT requestConfigFinished();
}

QVariantList Fcitx5ConfigProxy::globalConfigTypes() const
{
    QVariantList list;
    for (const auto &type : d->configTypes) {
        if (type.name() == "GlobalConfig") {
            for (const auto &option : type.options()) {
                QVariantMap item;
                item["name"] = option.name();
                item["description"] = option.description();
                list.append(item);
            }
            break;
        }
    }
    return list;
}

QVariantList Fcitx5ConfigProxy::globalConfigOptions(const QString &type, bool all) const
{
    QVariantList list;
    QString currentType = type+"$"+type+"Config";
    for (const auto &configType : d->configTypes) {
        if (configType.name() != currentType)
            continue;
        for (const auto &option : configType.options()) {
            // Don't display TriggerKeys and EnumerateForwardKeys
            if (!all && (option.name() == "TriggerKeys" || option.name() == "EnumerateForwardKeys")) {
                continue;
            }

            QVariantMap item;
            item["name"] = option.name();
            item["type"] = option.type();
            item["description"] = option.description();
            auto variant = value(type+"/"+option.name());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (variant.typeId() == QMetaType::QVariantMap) {
#else
            if (variant.type() == QVariant::Map) {
#endif
                QVariantMap map = variant.toMap();
                if (map.contains("0")) {
                    item["value"] = d->formatKey(map["0"].toString());
                }
            } else {
                item["value"] = variant;
            }

            QVariantMap properties = option.properties();
            if (!properties.isEmpty()) {
                auto iterator = properties.begin();
                while (iterator != properties.constEnd()) {
                    if (iterator.key() == "Enum") {
                        auto argument = qvariant_cast<QDBusArgument>(iterator.value());
                        QVariantMap map;
                        argument >> map;
                        QVariantList enumStrings = map.values().toList();
                        if (!enumStrings.isEmpty()) {
                            item["properties"] = enumStrings;
                        }
                        break;
                    }
                    ++iterator;
                }
                iterator = properties.begin();
                while (iterator != properties.constEnd()) {
                    if (iterator.key() == "EnumI18n") {
                        auto argument = qvariant_cast<QDBusArgument>(iterator.value());
                        QVariantMap map;
                        argument >> map;
                        QVariantList enumStrings = map.values().toList();
                        if (!enumStrings.isEmpty()) {
                            item["propertiesI18n"] = enumStrings;
                        }
                        break;
                    }
                    ++iterator;
                }
            }
            list.append(item);
        }
        break;
    }
    return list;
}

QVariant Fcitx5ConfigProxy::globalConfigOption(const QString &type, const QString &optionName) const
{
    QVariantMap item;
    QString currentType = type+"$"+type+"Config";
    for (const auto &configType : d->configTypes) {
        if (configType.name() != currentType)
            continue;
        for (const auto &option : configType.options()) {
            // Don't display TriggerKeys and EnumerateForwardKeys
            if (option.name() != optionName) {
                continue;
            }

            item["name"] = option.name();
            item["type"] = option.type();
            item["description"] = option.description();
            auto variant = value(type+"/"+option.name());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (variant.typeId() == QMetaType::QVariantMap) {
#else
            if (variant.type() == QVariant::Map) {
#endif
                QVariantMap map = variant.toMap();
                if (map.contains("0")) {
                    item["value"] = d->formatKey(map["0"].toString());
                }
            } else {
                item["value"] = variant;
            }

            QVariantMap properties = option.properties();
            if (!properties.isEmpty()) {
                auto iterator = properties.begin();
                while (iterator != properties.constEnd()) {
                    if (iterator.key() == "Enum") {
                        auto argument = qvariant_cast<QDBusArgument>(iterator.value());
                        QVariantMap map;
                        argument >> map;
                        QVariantList enumStrings = map.values().toList();
                        if (!enumStrings.isEmpty()) {
                            item["properties"] = enumStrings;
                        }
                        break;
                    }
                    ++iterator;
                }
                iterator = properties.begin();
                while (iterator != properties.constEnd()) {
                    if (iterator.key() == "EnumI18n") {
                        auto argument = qvariant_cast<QDBusArgument>(iterator.value());
                        QVariantMap map;
                        argument >> map;
                        QVariantList enumStrings = map.values().toList();
                        if (!enumStrings.isEmpty()) {
                            item["propertiesI18n"] = enumStrings;
                        }
                        break;
                    }
                    ++iterator;
                }
            }
            break;
        }
        break;
    }
    return item;
}

void Fcitx5ConfigProxy::restoreDefault(const QString &type)
{
    QString currentType = type+"$"+type+"Config";
    for (const auto &configType : d->configTypes) {
        if (configType.name() != currentType)
            continue;
        for (const auto &option : configType.options()) {
            auto defaultValue = option.defaultValue();
            setValue(type+"/"+option.name(), d->readDBusValue(defaultValue.variant()));
        }
        break;
    }
}

void Fcitx5ConfigProxy::requestConfig(bool sync)
{
    if (!d->dbusprovider->controller()) {
        return;
    }
    auto call = d->dbusprovider->controller()->GetConfig(d->path);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher,
            &QDBusPendingCallWatcher::finished,
            this,
            &Fcitx5ConfigProxy::onRequestConfigFinished);
    if (sync) {
        watcher->waitForFinished();
    }
}
 
void Fcitx5ConfigProxy::onRequestConfigFinished(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<QDBusVariant, fcitx::FcitxQtConfigTypeList> reply = *watcher;
    if (reply.isError()) {
        qWarning() << reply.error();
        return;
    }
    d->configTypes = reply.argumentAt<1>(); 

    auto value = reply.argumentAt<0>().variant();
    QVariantMap allMap;
    allMap = d->readDBusValue(value).toMap();
    std::swap(d->configValue, allMap);

    Q_EMIT requestConfigFinished();
}

QVariant Fcitx5ConfigProxy::value(const QString &path) const
{
    return fcitx::kcm::readVariant(d->configValue, path);
}

void Fcitx5ConfigProxy::setValue(const QString &path, const QVariant &value, bool isKey)
{
    if (value == this->value(path))
        return;
    if (isKey) {
        auto keys = d->formatKeys(value.toStringList());
        fcitx::kcm::writeVariant(d->configValue, path, keys);
    } else {
        fcitx::kcm::writeVariant(d->configValue, path, value);
    }
    d->timer->start();
}

void Fcitx5ConfigProxy::save()
{
    if (!d->dbusprovider->controller()) {
        return;
    }

    QDBusVariant var(d->configValue);
    d->dbusprovider->controller()->SetConfig(d->path, var);
    requestConfig(false);
}
