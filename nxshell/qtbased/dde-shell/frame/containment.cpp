// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "containment.h"
#include "private/containment_p.h"
#include "appletitemmodel.h"

#include "pluginloader.h"

#include <QLoggingCategory>

DS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

DContainment::DContainment(QObject *parent)
    : DContainment(*new DContainmentPrivate(this), parent)
{
}

DContainment::DContainment(DContainmentPrivate &dd, QObject *parent)
    : DApplet(dd, parent)
{

}

DContainment::~DContainment()
{

}

DApplet *DContainment::createApplet(const DAppletData &data)
{
    D_D(DContainment);
    if (!DPluginMetaData::isRootPlugin(this->pluginId())) {
        const auto children = DPluginLoader::instance()->childrenPlugin(this->pluginId());
        if (!children.contains(DPluginLoader::instance()->plugin(data.pluginId()))) {
            return nullptr;
        }
    }
    auto applet = DPluginLoader::instance()->loadApplet(data);
    if (!applet) {
        return nullptr;
    }

    applet->setParent(this);

    QObject::connect(applet, &DApplet::rootObjectChanged, this, [this, applet]() {
        if (auto object = applet->rootObject()) {
            D_D(DContainment);
            d->m_model->append(object);
            QObject::connect(object, &QObject::destroyed, this, [this, object]() {
                D_D(DContainment);
                d->m_model->remove(object);
            });
        }
    });

    d->m_applets.append(applet);
    return applet;
}

void DContainment::removeApplet(DApplet *applet)
{
    Q_ASSERT(applet);
    D_D(DContainment);
    if (d->m_applets.contains(applet)) {
        d->m_applets.removeOne(applet);
    }
    if (auto view = applet->rootObject()) {
        d->m_model->remove(view);
    }
    applet->deleteLater();
}

QList<DApplet *> DContainment::applets() const
{
    D_DC(DContainment);
    return d->m_applets;
}

QList<QObject *> DContainment::appletItems()
{
    D_D(DContainment);

    return d->m_model->rootObjects();
}

DAppletItemModel *DContainment::appletItemModel() const
{
    D_DC(DContainment);

    return d->m_model;
}

DApplet *DContainment::applet(const QString &id) const
{
    D_DC(DContainment);
    for (auto item : d->m_applets) {
        if (item->id() == id)
            return item;
    }
    return nullptr;
}

bool DContainment::load()
{
    D_D(DContainment);

    return DApplet::load();
}

bool DContainment::init()
{
    D_D(DContainment);

    return DApplet::init();
}

DS_END_NAMESPACE
