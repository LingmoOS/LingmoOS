/*
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "waylandxdgforeignv2_p.h"

#include <QGuiApplication>

WaylandXdgForeignExportedV2::WaylandXdgForeignExportedV2(::zxdg_exported_v2 *object)
    : QObject()
    , QtWayland::zxdg_exported_v2(object)
{
}

WaylandXdgForeignExportedV2::~WaylandXdgForeignExportedV2()
{
    if (qGuiApp) {
        destroy();
    }
}

QString WaylandXdgForeignExportedV2::handle() const
{
    return m_handle;
}

void WaylandXdgForeignExportedV2::zxdg_exported_v2_handle(const QString &handle)
{
    m_handle = handle;
    Q_EMIT handleReceived(handle);
}

WaylandXdgForeignExporterV2::WaylandXdgForeignExporterV2()
    : QWaylandClientExtensionTemplate<WaylandXdgForeignExporterV2>(1)
{
    initialize();
}

WaylandXdgForeignExporterV2::~WaylandXdgForeignExporterV2()
{
    if (qGuiApp && isActive()) {
        destroy();
    }
}

WaylandXdgForeignExporterV2 &WaylandXdgForeignExporterV2::self()
{
    static WaylandXdgForeignExporterV2 s_instance;
    return s_instance;
}

WaylandXdgForeignExportedV2 *WaylandXdgForeignExporterV2::exportToplevel(wl_surface *surface)
{
    return new WaylandXdgForeignExportedV2(export_toplevel(surface));
}

WaylandXdgForeignImportedV2::WaylandXdgForeignImportedV2(const QString &handle, ::zxdg_imported_v2 *object)
    : QObject()
    , QtWayland::zxdg_imported_v2(object)
    , m_handle(handle)
{
}

WaylandXdgForeignImportedV2::~WaylandXdgForeignImportedV2()
{
    if (qGuiApp) {
        destroy();
    }
}

void WaylandXdgForeignImportedV2::zxdg_imported_v2_destroyed()
{
    delete this;
}

QString WaylandXdgForeignImportedV2::handle() const
{
    return m_handle;
}

WaylandXdgForeignImporterV2::WaylandXdgForeignImporterV2()
    : QWaylandClientExtensionTemplate<WaylandXdgForeignImporterV2>(1)
{
    initialize();
}

WaylandXdgForeignImporterV2::~WaylandXdgForeignImporterV2()
{
    if (qGuiApp && isActive()) {
        destroy();
    }
}

WaylandXdgForeignImporterV2 &WaylandXdgForeignImporterV2::self()
{
    static WaylandXdgForeignImporterV2 s_instance;
    return s_instance;
}

WaylandXdgForeignImportedV2 *WaylandXdgForeignImporterV2::importToplevel(const QString &handle)
{
    return new WaylandXdgForeignImportedV2(handle, import_toplevel(handle));
}

#include "moc_waylandxdgforeignv2_p.cpp"
