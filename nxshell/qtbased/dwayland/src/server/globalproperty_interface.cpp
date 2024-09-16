// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "globalproperty_interface.h"
#include "display.h"
#include "surface_interface_p.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

#include "qwayland-server-dde-globalproperty.h"

namespace KWaylandServer
{
static const quint32 s_version = 1;
class GlobalPropertyInterfacePrivate : public QtWaylandServer::dde_globalproperty
{
public:
    GlobalPropertyInterfacePrivate(GlobalPropertyInterface *q, Display *display);

    GlobalPropertyInterface *q;

private:
    void dde_globalproperty_set_property(Resource *resource, const QString &module, const QString &function, struct ::wl_resource *surface, int32_t type, const QString &data) override;
    void dde_globalproperty_get_property(Resource *resource, const QString &data) override;
};

GlobalPropertyInterfacePrivate::GlobalPropertyInterfacePrivate(GlobalPropertyInterface *_q, Display *display)
    : QtWaylandServer::dde_globalproperty(*display, s_version)
    , q(_q)
{

}

void GlobalPropertyInterfacePrivate::dde_globalproperty_set_property(Resource *resource, const QString &module, const QString &function, struct ::wl_resource *surface, int32_t type, const QString &data)
{
    SurfaceInterface *si = SurfaceInterface::get(surface);
    if (!si) {
        wl_resource_post_error(resource->handle, 0, "Invalid surface");
        return;
    }

    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(data.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Failed to parse data" << error.errorString();
        return;
    }
    QMap<QString, QVariant> ret;
    const QJsonObject rootObj = doc.object();
    for (const auto &key : rootObj.keys()) {
        const QVariant &value = rootObj[key].toVariant();
        ret[key] = value;
    }
    emit q->windowDecoratePropertyChanged(si, ret);
}

void GlobalPropertyInterfacePrivate::dde_globalproperty_get_property(Resource *resource, const QString &data)
{

}

GlobalPropertyInterface::GlobalPropertyInterface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new GlobalPropertyInterfacePrivate(this, display))
{

}

GlobalPropertyInterface::~GlobalPropertyInterface()
{

}

}
