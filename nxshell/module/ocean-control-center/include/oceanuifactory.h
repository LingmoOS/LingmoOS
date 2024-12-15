// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCFACTORY_H
#define DCCFACTORY_H

#include <QObject>

namespace oceanuiV25 {
#define OceanUIFactory_iid "org.lingmo.ocean.oceanui-factory/v1.0"
class OceanUIObject;

class OceanUIFactory : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    // 作为数据返回，会导出为oceanuiData供main.qml使用
    virtual QObject *create(QObject * = nullptr) { return nullptr; }

    // 未提供qml的，可在此自己加载qml返回OceanUIObject对象
    virtual OceanUIObject *oceanuiObject(QObject * = nullptr) { return nullptr; }
};
} // namespace oceanuiV25
Q_DECLARE_INTERFACE(oceanuiV25::OceanUIFactory, OceanUIFactory_iid)

#define DCC_FACTORY_CLASS(classname)                                      \
    namespace {                                                           \
    class Q_DECL_EXPORT classname##OceanUIFactory : public oceanuiV25::OceanUIFactory \
    {                                                                     \
        Q_OBJECT                                                          \
        Q_PLUGIN_METADATA(IID OceanUIFactory_iid)                             \
        Q_INTERFACES(oceanuiV25::OceanUIFactory)                                  \
    public:                                                               \
        using oceanuiV25::OceanUIFactory::OceanUIFactory;                             \
        QObject *create(QObject *parent = nullptr) override               \
        {                                                                 \
            return new classname(parent);                                 \
        }                                                                 \
    };                                                                    \
    }
#endif // DCCFACTORY_H
