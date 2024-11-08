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

#ifndef PREVIEWPAGEFACTORYMANAGER_H
#define PREVIEWPAGEFACTORYMANAGER_H

#include "explorer-core_global.h"
#include <QObject>
#include <QMap>

namespace Peony {

class PreviewPagePluginIface;

/*!
 * \brief The PreviewPageFactoryManager class
 * \details
 * In explorer-qt, we have a optional extra preview view at
 * right of the window.
 * PreviewPage is provided by an inplement of PreviewPagePluginInterface.
 * Actually, the interface is represent a factory template for
 * create a preview page.
 * We use this class manage all the factories of preview page.
 * When the manger instance init, the factories will be loaded
 * and registered in manager. Then we can call the createPreviewPage()
 * function of them when we need. The registered factory can be
 * listed in a GUI, and we can disable/enable them thourgh the
 * setEnable() method.
 * \note
 * The manager is single and global, and factory too.
 * Do not try newing or deleting them.
 */
class PEONYCORESHARED_EXPORT PreviewPageFactoryManager : public QObject
{
    Q_OBJECT
public:
    static PreviewPageFactoryManager *getInstance();

    bool registerFactory(const QString &name, PreviewPagePluginIface* plugin);
    bool unregisterFactory(const QString &name, PreviewPagePluginIface* plugin);

    const QStringList getPluginNames();
    PreviewPagePluginIface *getPlugin(const QString &name);

    const QString getLastPreviewPageId();

private:
    explicit PreviewPageFactoryManager(QObject *parent = nullptr);
    ~PreviewPageFactoryManager();

    QMap<QString, PreviewPagePluginIface*> *m_map = nullptr;
    QString m_last_preview_page_id = nullptr;
};

}

#endif // PREVIEWPAGEFACTORYMANAGER_H
