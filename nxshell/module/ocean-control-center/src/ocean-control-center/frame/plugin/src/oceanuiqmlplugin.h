// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCQMLPLUGIN_H
#define DCCQMLPLUGIN_H

#include <QQmlExtensionPlugin>

namespace oceanuiV25 {
class OceanUIQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
};
} // namespace oceanuiV25
#endif // QMLPLUGIN_H
