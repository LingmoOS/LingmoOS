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

#ifndef DEFAULTPREVIEWPAGEFACTORY_H
#define DEFAULTPREVIEWPAGEFACTORY_H

#include "explorer-core_global.h"
#include "preview-page-plugin-iface.h"
#include <QObject>

namespace Peony {

/*!
 * \brief The DefaultPreviewPageFactory class
 * \details
 * This class is used to create the preview page of explorer-qt.
 * \note
 * This is a interanl interface implement, so it is not a real plugin.
 * The interface implement is not so strict than a plugin one.
 */
class PEONYCORESHARED_EXPORT DefaultPreviewPageFactory : public QObject, public PreviewPagePluginIface
{
    Q_OBJECT
public:
    static DefaultPreviewPageFactory *getInstance();

    PluginType pluginType() override {
        return PluginType::PreviewPagePlugin;
    }
    const QString name() override {
        return tr("Default Preview");
    }
    const QString description() override {
        return tr("This is the Default Preview of explorer-qt");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("lingmo-preview-file-symbolic", QIcon::fromTheme("lingmo-preview-file", QIcon::fromTheme("privew-file")));
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    PreviewPageIface *createPreviewPage() override;

private:
    explicit DefaultPreviewPageFactory(QObject *parent = nullptr);
    ~DefaultPreviewPageFactory() override;

    bool m_enable = true;
};

}

#endif // DEFAULTPREVIEWPAGEFACTORY_H
