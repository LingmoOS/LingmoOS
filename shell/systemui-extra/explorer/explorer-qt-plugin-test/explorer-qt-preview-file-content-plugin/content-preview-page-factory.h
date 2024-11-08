/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef CONTENTPREVIEWPAGEFACTORY_H
#define CONTENTPREVIEWPAGEFACTORY_H

#include "explorer-qt-preview-file-content-plugin_global.h"
#include "preview-page-plugin-iface.h"

/*!
 * \brief The ContentPreviewPageFactory class
 *
 * \details This example is showing you how to
 * implement a preview page plugin for explorer-qt.
 * We should start at implement the PreviewPagePluginIface
 * with your own factory.
 *
 * You have to declare your factory
 * as PreviewPagePluginIface derived class
 * and override all virtual method both
 * Peony::PreviewPagePluginIface and Peony::PluginIface provided.
 *
 * \note
 * You don't need declare the PreviewPageIface as a plugin interface,
 * But you should aslo implement your own PreviewPage
 * by deriving Peony::PreviewPageIface for the return value
 * as this class's createPreviewPage() method.
 */
class PEONYQTPREVIEWFILECONTENTPLUGINSHARED_EXPORT ContentPreviewPageFactory : public QObject, public Peony::PreviewPagePluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewPagePluginIface_iid)
    Q_INTERFACES(Peony::PreviewPagePluginIface)
public:
    explicit ContentPreviewPageFactory(QObject *parent = nullptr);
    ~ContentPreviewPageFactory() override;

    PluginInterface::PluginType pluginType() override {
        return PluginInterface::PreviewPagePlugin;
    }
    const QString name() override {
        return "Preview Page Plugin";
    }
    const QString description() override {
        return "This plugin is a factory plugin providing the content preview page for FMWindow";
    }
    const QIcon icon() override {
        return QIcon::fromTheme("lingmo-preview-file-details");
    }
    void setEnable(bool enable) override {
        m_enable = enable;
    }
    bool isEnable() override {
        return m_enable;
    }

    Peony::PreviewPageIface *createPreviewPage() override;

private:
    bool m_enable = true;

};

#endif // CONTENTPREVIEWPAGEFACTORY_H
