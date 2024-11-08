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

#ifndef DIRECTORYVIEWMENU_H
#define DIRECTORYVIEWMENU_H

#include <QMenu>
#include <memory>
#include "explorer-core_global.h"

#include "directory-view-plugin-iface2.h"

namespace Peony {

class FMWindowIface;
class FileInfo;

/*!
 * \brief The DirectoryViewMenu class
 *
 * \todo
 * add create template function.
 */
class PEONYCORESHARED_EXPORT DirectoryViewMenu : public QMenu
{
    Q_OBJECT
public:
    /*!
     * \brief DirectoryViewMenu
     * \param directoryView
     * \param parent
     * \deprecated
     */
    explicit DirectoryViewMenu(DirectoryViewWidget *directoryView, QWidget *parent = nullptr);
    explicit DirectoryViewMenu(FMWindowIface *window, QWidget *parent = nullptr);

    const QStringList &urisToEdit() {
        return m_uris_to_edit;
    }

    void setHiddenActionsByObjectName(const QStringList &actionNames);

protected:
    void fillActions();
    const QList<QAction *> constructOpenOpActions();
    const QList<QAction *> constructCreateTemplateActions();
    const QList<QAction *> constructViewOpActions();
    const QList<QAction *> constructFileOpActions();
    const QList<QAction *> constructMenuPluginActions(); //directory view menu extension.
    const QList<QAction *> constructFilePropertiesActions();
    const QList<QAction *> constructComputerActions();
    const QList<QAction *> constructTrashActions();
    const QList<QAction *> constructSearchActions();
    const QList<QAction *> constructMultiSelectActions();
    void keyPressEvent(QKeyEvent *e) override;

private:
    bool isMultFile(std::shared_ptr<FileInfo> info);
    void isMobileFile(const QString &uri);

private:
    FMWindowIface *m_top_window;

    DirectoryViewWidget *m_view;
    QString m_directory;
    QString m_version;
    QStringList m_selections;

    bool m_is_cd = false;
    bool m_is_ftp = false;
    bool m_is_computer = false;
    bool m_is_network = false;
    bool m_is_trash = false;
    bool m_is_search = false;
    bool m_is_recent = false;
    bool m_is_favorite = false;
    bool m_can_delete = true;           // if contains computer:/// trash:/// recent:/// and desktop path
    bool m_is_kydroid = false;
    bool m_is_filesafe = false;
    bool m_is_filebox_file = false;     //filesafe path files operation flag
    bool m_is_smb_file = false;
    bool m_is_mobile_file = false;
    bool m_is_boxpath = false;
    bool m_is_mtp_ptp = false;
    bool m_is_label_model = false;

    const int ELIDE_TEXT_LENGTH = 16;

    QStringList m_uris_to_edit;
};

}

#endif // DIRECTORYVIEWMENU_H
