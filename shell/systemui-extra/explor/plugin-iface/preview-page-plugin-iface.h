/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef PREVIEWPAGEPLUGINIFACE_H
#define PREVIEWPAGEPLUGINIFACE_H

#include <QPluginLoader>
#include <QtPlugin>
#include <QString>
#include "plugin-iface.h"

#define PreviewPagePluginIface_iid "org.lingmo.explor-qt.plugin-iface.PreviewPagePluginInterface"

namespace Peony {

class PreviewPageIface;

class PreviewPagePluginIface : public PluginInterface
{
public:
    virtual ~PreviewPagePluginIface() {}

    virtual PreviewPageIface *createPreviewPage() = 0;
};

class PreviewPageIface
{
public:
    enum PreviewType {
        Attribute,
        Text,
        Pictrue,
        PDF,
        OfficeDoc,
        Other
    };

    virtual ~PreviewPageIface() {}

    virtual void prepare(const QString &uri, PreviewType type) = 0;
    virtual void prepare(const QString &uri) = 0;
    virtual void startPreview() = 0;
    virtual void cancel() = 0;
    virtual void closePreviewPage() = 0;
};

}

Q_DECLARE_INTERFACE(Peony::PreviewPagePluginIface, PreviewPagePluginIface_iid)

#endif // PREVIEWPAGEPLUGINIFACE_H
