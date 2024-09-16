// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/dconf.h"

#include <QSharedPointer>
#include <QDebug>

Dconf::Dconf()
{

}

Dconf::~Dconf()
{

}

DConfig *Dconf::connectDconf(const QString &name, const QString &subpath, QObject *parent)
{
    // TODO: move it to dde-session
    DConfig *config = DConfig::create("dde-application-manager", name, subpath, parent);
    if (config == nullptr) {
        qDebug() << "Cannot find dconfigs, name:" << name;
        return nullptr;
    }
    if (!config->isValid()) {
        qDebug() << "Cannot find dconfigs, name:" << name;
        delete config;
        return nullptr;
    }

    return config;
}

const QVariant Dconf::GetValue(const QString &name, const QString &subPath, const QString &key, const QVariant &fallback)
{
    QSharedPointer<DConfig> config(connectDconf(name, subPath));
    if (config && config->isValid() && config->keyList().contains(key)) {
        QVariant v = config->value(key);
        return v;
    }

    qDebug() << "Cannot find dconfigs, name:" << name
             << " subPath:" << subPath << " key:" << key
             << "Use fallback value:" << fallback;
    return fallback;
}

bool Dconf::SetValue(const QString &name, const QString &subPath, const QString &key, const QVariant &value)
{
    QSharedPointer<DConfig> config(connectDconf(name, subPath));
    if (config && config->isValid() && config->keyList().contains(key)) {
        config->setValue(key, value);
        return true;
    }

    qDebug() << "Cannot find dconfigs, name:" << name
             << " subPath:" << subPath << " key:" << key;
    return false;
}
