// Copyright 2022  diguoliang <diguoliang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QObject>
#include <QString>
#include <QMap>

#include <DWayland/Server/kwaylandserver_export.h>

struct wl_resource;

namespace KWaylandServer
{
class Display;
class wl_surface;
class SurfaceInterface;;
class GlobalPropertyInterfacePrivate;

struct PropertyData{
    QString module;
    QString function;
    QMap<QString, QVariant> propetyData;
};

class KWAYLANDSERVER_EXPORT GlobalPropertyInterface : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(QString propertyData READ propertyData WRITE setPropertyData NOTIFY WindowDecoratePropertyChanged)
public:
    explicit GlobalPropertyInterface(Display *display, QObject *parent = nullptr);
    virtual ~GlobalPropertyInterface();

Q_SIGNALS:
    void windowDecoratePropertyChanged(SurfaceInterface *, QMap<QString, QVariant> &) const;

private:
    QScopedPointer<GlobalPropertyInterfacePrivate> d;
};

}