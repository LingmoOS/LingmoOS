/*
    SPDX-FileCopyrightText: 2017 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_XDGFOREIGN_P_H
#define KWAYLAND_CLIENT_XDGFOREIGN_P_H

#include "xdgforeign.h"
#include <QObject>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN XdgExporter::Private
{
public:
    Private();
    virtual ~Private();

    virtual XdgExported *exportTopLevelV2(Surface *surface, QObject *parent) = 0;

    virtual void setupV2(zxdg_exporter_v2 *arg) = 0;
    virtual zxdg_exporter_v2 *exporterV2() = 0;

    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

    EventQueue *queue = nullptr;
};

class Q_DECL_HIDDEN XdgImporter::Private
{
public:
    Private();
    virtual ~Private();

    virtual XdgImported *importTopLevelV2(const QString &handle, QObject *parent) = 0;

    virtual void setupV2(zxdg_importer_v2 *arg) = 0;
    virtual zxdg_importer_v2 *importerV2() = 0;

    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

    EventQueue *queue = nullptr;
};

class Q_DECL_HIDDEN XdgExported::Private
{
public:
    Private(XdgExported *q);
    virtual ~Private();

    virtual void setupV2(zxdg_exported_v2 *) = 0;
    virtual zxdg_exported_v2 *exportedV2() = 0;

    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

    QString handle;

protected:
    XdgExported *q;
};

class Q_DECL_HIDDEN XdgImported::Private
{
public:
    Private(XdgImported *q);
    virtual ~Private();

    virtual void setupV2(zxdg_imported_v2 *) = 0;
    virtual zxdg_imported_v2 *importedV2() = 0;

    virtual void setParentOf(Surface *surface) = 0;
    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

protected:
    XdgImported *q;
};

}
}

#endif
