/*
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WAYLANDXDGFOREIGNV2_P_H
#define WAYLANDXDGFOREIGNV2_P_H

#include "qwayland-xdg-foreign-unstable-v2.h"

#include <QObject>
#include <QtWaylandClient/QWaylandClientExtension>

class WaylandXdgForeignExportedV2 : public QObject, public QtWayland::zxdg_exported_v2
{
    Q_OBJECT

public:
    explicit WaylandXdgForeignExportedV2(::zxdg_exported_v2 *object);
    ~WaylandXdgForeignExportedV2() override;

    QString handle() const;

Q_SIGNALS:
    void handleReceived(const QString &handle);

protected:
    void zxdg_exported_v2_handle(const QString &handle) override;

private:
    QString m_handle;
};

class WaylandXdgForeignExporterV2 : public QWaylandClientExtensionTemplate<WaylandXdgForeignExporterV2>, public QtWayland::zxdg_exporter_v2
{
public:
    ~WaylandXdgForeignExporterV2() override;

    static WaylandXdgForeignExporterV2 &self();

    WaylandXdgForeignExportedV2 *exportToplevel(struct ::wl_surface *surface);

private:
    WaylandXdgForeignExporterV2();
};

class WaylandXdgForeignImportedV2 : public QObject, public QtWayland::zxdg_imported_v2
{
public:
    explicit WaylandXdgForeignImportedV2(const QString &handle, ::zxdg_imported_v2 *object);
    ~WaylandXdgForeignImportedV2() override;

    QString handle() const;

protected:
    void zxdg_imported_v2_destroyed() override;

private:
    QString m_handle;
};

class WaylandXdgForeignImporterV2 : public QWaylandClientExtensionTemplate<WaylandXdgForeignImporterV2>, public QtWayland::zxdg_importer_v2
{
public:
    ~WaylandXdgForeignImporterV2() override;

    static WaylandXdgForeignImporterV2 &self();

    WaylandXdgForeignImportedV2 *importToplevel(const QString &handle);

private:
    WaylandXdgForeignImporterV2();
};

#endif // WAYLANDXDGFOREIGNV2_P_H
