/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"
#ifndef UI_KYFILEDIALOG_H
#define UI_KYFILEDIALOG_H
#include <QApplication>
#include <QComboBox>
#include <QTreeView>
#include <QToolButton>
#include <QLineEdit>
#include <QFrame>
#include <QSplitter>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include "sidebar.h"
#include <QSizePolicy>
#include "kyfiledialog.h"
#include <explor-qt/controls/directory-view/directory-view-container.h>
#include <explor-qt/controls/navigation-bar/advanced-location-bar.h>
#include "pathbar.h"
#include <QPushButton>
#include <QToolBar>
#include <QObject>
#include <QDialog>
#include "menutoolbutoon.h"

namespace Peony {
class DirectoryViewContainer;
}
namespace LINGMOFileDialog {
class Ui_KyFileDialog: public QWidget
{
    QOBJECT_H
    public:
        explicit Ui_KyFileDialog(QWidget *parent = nullptr);
    ~Ui_KyFileDialog();

    QGridLayout *m_gridLayout = nullptr;
    QHBoxLayout *m_hHeaderLayout = nullptr;
    QHBoxLayout *m_hLineEditLayout = nullptr;
    QHBoxLayout *m_hBtnLayout = nullptr;
    FileDialogPathBar *m_pathbarWidget = nullptr;
    Peony::AdvancedLocationBar *m_pathbar = nullptr;
    QToolButton *m_searchBtn = nullptr;
    //QWidget *m_pathbar = nullptr;
    QToolButton *m_backButton = nullptr;
    QToolButton *m_forwardButton = nullptr;
    QToolButton *m_toParentButton = nullptr;
    MenuToolButton *m_modeButton = nullptr;
    MenuToolButton *m_sortButton = nullptr;
    QToolButton *m_maximizeAndRestore = nullptr;
    QToolButton *m_closeButton = nullptr;
    QMenu *m_modeMenu = nullptr;
    QMenu *m_sortMenu = nullptr;
    QAction *m_listModeAction = nullptr;
    QAction *m_iconModeAction = nullptr;
    FileDialogSideBar *m_sider = nullptr;
    QFrame *m_frame = nullptr;
    QSplitter *m_splitter = nullptr;
    QVBoxLayout *vboxLayout = nullptr;
    QTreeView *m_treeView = nullptr;
    QLabel *m_fileNameLabel = nullptr;
    QLineEdit *m_fileNameEdit = nullptr;
    FileDialogComboBox *m_fileTypeCombo = nullptr;
    QPushButton *m_newFolderButton = nullptr;
    QPushButton *m_acceptButton = nullptr;
    QPushButton *m_rejectButton = nullptr;
    QFrame *m_upSeperate = nullptr;
    QFrame *m_downSeperate = nullptr;

    QWidget *m_widget = nullptr;
    Peony::DirectoryViewContainer *m_container = nullptr;

    QWidget *m_siderWidget = nullptr;
    QVBoxLayout *m_siderLayout = nullptr;

    QActionGroup *m_sortTypeGroup = nullptr;
    QAction *m_fileName = nullptr;
    QAction *m_modifiedDate = nullptr;
    QAction *m_fileType = nullptr;
    QAction *m_fileSize = nullptr;
    QAction *m_originalPath = nullptr;
    QActionGroup *m_sortOrderGroup = nullptr;
    QAction *m_descending = nullptr;
    QAction *m_ascending = nullptr;
    QAction *m_useGlobalSortAction = nullptr;

public:
    void setupUi(QDialog *mKyFileDialog);

private:
    void initSortMenu(QDialog *mKyFileDialog);
    void initModeMenu(QDialog *mKyFileDialog);
    void initSiderBar(QDialog *mKyFileDialog);
    void initHeaderBar(QDialog *mKyFileDialog);
    void initLineEditLayout(QDialog *mKyFileDialog);
    void intiBtnLayout(QDialog *mKyFileDialog);
};
}
#endif // UI_KYFILEDIALOG_H
