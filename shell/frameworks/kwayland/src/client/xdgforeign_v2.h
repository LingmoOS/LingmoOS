/*
    SPDX-FileCopyrightText: 2017 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_XDGFOREIGN_V2_H
#define KWAYLAND_CLIENT_XDGFOREIGN_V2_H

#include "surface.h"
#include "xdgforeign.h"

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct zxdg_exporter_v2;
struct zxdg_importer_v2;
struct zxdg_exported_v2;
struct zxdg_imported_v2;

namespace KWayland
{
namespace Client
{
class XdgExportedUnstableV2;
class XdgImportedUnstableV2;

/**
 * @short Wrapper for the zxdg_exporter_v2 interface.
 *
 * This class provides a convenient wrapper for the zxdg_exporter_v2 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the  interface:
 * @code
 *  *c = registry->create(name, version);
 * @endcode
 *
 * This creates the  and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 *  *c = new ;
 * c->setup(registry->bind(name, version));
 * @endcode
 *
 * The  can be used as a drop-in replacement for any zxdg_exporter_v2
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class Q_DECL_HIDDEN XdgExporterUnstableV2 : public XdgExporter
{
    Q_OBJECT
public:
    /**
     * Creates a new .
     * Note: after constructing the  it is not yet valid and one needs
     * to call setup. In order to get a ready to use  prefer using
     * Registry::create.
     **/
    explicit XdgExporterUnstableV2(QObject *parent = nullptr);
    ~XdgExporterUnstableV2() override;

private:
    class Private;
};

/**
 * @short Wrapper for the zxdg_importer_v2 interface.
 *
 * This class provides a convenient wrapper for the zxdg_importer_v2 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the  interface:
 * @code
 *  *c = registry->create(name, version);
 * @endcode
 *
 * This creates the  and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 *  *c = new ;
 * c->setup(registry->bind(name, version));
 * @endcode
 *
 * The  can be used as a drop-in replacement for any zxdg_importer_v2
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class Q_DECL_HIDDEN XdgImporterUnstableV2 : public XdgImporter
{
    Q_OBJECT
public:
    /**
     * Creates a new .
     * Note: after constructing the  it is not yet valid and one needs
     * to call setup. In order to get a ready to use  prefer using
     * Registry::create.
     **/
    explicit XdgImporterUnstableV2(QObject *parent = nullptr);
    ~XdgImporterUnstableV2() override;

private:
    class Private;
};

class Q_DECL_HIDDEN XdgExportedUnstableV2 : public XdgExported
{
    Q_OBJECT
public:
    ~XdgExportedUnstableV2() override;

private:
    friend class XdgExporterUnstableV2;
    explicit XdgExportedUnstableV2(QObject *parent = nullptr);
    class Private;
};

class Q_DECL_HIDDEN XdgImportedUnstableV2 : public XdgImported
{
    Q_OBJECT
public:
    ~XdgImportedUnstableV2() override;

private:
    friend class XdgImporterUnstableV2;
    explicit XdgImportedUnstableV2(QObject *parent = nullptr);
    class Private;
};

}
}

#endif
