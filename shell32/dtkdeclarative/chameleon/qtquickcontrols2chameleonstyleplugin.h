// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QTQUICKCONTROLS2CHAMELEONSTYLEPLUGIN_H
#define QTQUICKCONTROLS2CHAMELEONSTYLEPLUGIN_H

#ifdef USE_QQuickStylePlugin
#include <private/qquickstyleplugin_p.h>
#else
#include <QQmlExtensionPlugin>
#endif

class QtQuickControls2ChameleonStylePlugin
#ifdef USE_QQuickStylePlugin
        : public QQuickStylePlugin
#else
        : public QQmlExtensionPlugin
#endif
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    ~QtQuickControls2ChameleonStylePlugin() override;
#ifdef USE_QQuickStylePlugin
    QString name() const override;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual void initializeTheme(QQuickTheme *) {}
#endif
    void registerTypes(const char *uri) override;
};

#endif // QTQUICKCONTROLS2CHAMELEONSTYLEPLUGIN_H
