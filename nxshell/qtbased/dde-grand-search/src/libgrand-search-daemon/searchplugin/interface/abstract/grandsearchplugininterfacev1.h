// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRANDSEARCHPLUGININTERFACEV1_H
#define GRANDSEARCHPLUGININTERFACEV1_H

#include <QObject>

//! 扩展搜索插件需实现的dbus接口规范,该文件仅定义接口用于生成dbus接口描述的xml文件。
//! 插件开发者可使用xml文件生成adaptor实现接口
//! 插件调用者可使用xml文件生成interface调用

namespace GrandSearch {
namespace SearchPluginV1 {

class GrandSearchPluginInterface : QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.GrandSearch.SearchPlugin")
public slots:
    QString Search(const QString &json);
    bool Action(const QString &json);
    bool Stop(const QString &json);
private:
    explicit GrandSearchPluginInterface(QObject *parent = nullptr);
};

}
}

#endif // GRANDSEARCHPLUGININTERFACEV1_H
