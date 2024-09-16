// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QObject>

DS_BEGIN_NAMESPACE

class DApplet;

#define DAppletFactory_iid "org.deepin.ds.applet-factory"

/**
 * @brief 插件注册
 */
class DAppletFactoryPrivate;
class DS_SHARE DAppletFactory : public QObject
{
    Q_OBJECT
public:
    explicit DAppletFactory(QObject *parent = nullptr);
    virtual ~DAppletFactory() override;

    using CreateAppletFunction = std::function<DApplet *(QObject *)>;
    void registerInstance(CreateAppletFunction func);

    template<class T>
    static inline T *registerApplet(QObject *parent)
    {
        return new T(parent);
    }

    virtual DApplet *create(QObject *parent = nullptr);
private:
    DAppletFactoryPrivate *d = nullptr;
};

DS_END_NAMESPACE

Q_DECLARE_INTERFACE(DS_NAMESPACE::DAppletFactory, DAppletFactory_iid)

#define D_APPLET_CLASS(classname) \
 \
namespace { \
 \
class DS_SHARE classname##AppletFactory : public DS_NAMESPACE::DAppletFactory \
{ \
    Q_OBJECT \
    Q_PLUGIN_METADATA(IID DAppletFactory_iid) \
    Q_INTERFACES(DS_NAMESPACE::DAppletFactory) \
    public: \
    explicit classname##AppletFactory() \
        : DS_NAMESPACE::DAppletFactory() \
    { \
        registerInstance(registerApplet<classname>); \
    } \
}; \
}
