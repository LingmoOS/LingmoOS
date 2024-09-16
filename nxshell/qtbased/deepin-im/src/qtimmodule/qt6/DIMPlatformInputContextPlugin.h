// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIMPLATFORMINPUTCONTEXTPLUGIN_H
#define DIMPLATFORMINPUTCONTEXTPLUGIN_H

#include <qpa/qplatforminputcontextplugin_p.h>

class DIMPlatformInputContextPlugin : public QPlatformInputContextPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "plugin.json")

public:
    explicit DIMPlatformInputContextPlugin(QObject *parent = nullptr);
    ~DIMPlatformInputContextPlugin() = default;

    QPlatformInputContext *create(const QString &key, const QStringList &paramList) override;
};

#endif //! DIMPLATFORMINPUTCONTEXTPLUGIN_H
