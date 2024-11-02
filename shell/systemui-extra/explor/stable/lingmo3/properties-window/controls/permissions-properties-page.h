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

#ifndef STABLEPERMISSIONSPROPERTIESPAGE_H
#define STABLEPERMISSIONSPROPERTIESPAGE_H
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QMutex>
#include <QMessageBox>

#include "properties-window-tab-iface.h"
#include "explor-core_global.h"
#include "systemd-bus-accounts.h"
#include "usershare-manager.h"

#include <memory>
#include <gio/gio.h>

class QLabel;
class QTableWidget;
class QCheckBox;

namespace Peony {

class FileWatcher;

class PEONYCORESHARED_EXPORT PermissionsPropertiesPage : public PropertiesWindowTabIface
{
    Q_OBJECT
public:
    explicit PermissionsPropertiesPage(const QStringList &uris, QWidget *parent = nullptr);
    ~PermissionsPropertiesPage();

    /*!
     * init the main Widget
     * \brief initTabWidget
     */
    void initTableWidget();

    /**
     * \brief 创建一个自定义的表格单元组件，带图标和文字
     * \param parent
     * \param icon
     * \param text
     * \return
     */
    static QWidget* createCellWidget(QWidget* parent,QIcon icon, QString text);

    void savePermissions();

    void updateCheckBox();

    void addAdvancedLayout();

    void updateLabelShow(const QString &str);

protected:
    static GAsyncReadyCallback async_query_permisson_callback(GObject *obj,
            GAsyncResult *res,
            PermissionsPropertiesPage *p_this);

    void queryPermissionsAsync(const QString&, const QString &uri);

protected Q_SLOTS:
    void changePermission(int row, int column, bool checked);

Q_SIGNALS:
    void checkBoxChanged(int row, int column, bool checked);

private:
    QString m_uri;
    std::shared_ptr<FileWatcher> m_watcher;

    QVBoxLayout *m_layout = nullptr;

    QLabel *m_label         = nullptr;
    QLabel *m_message       = nullptr;
    QTableWidget *m_table   = nullptr;
    QPushButton *m_advancedBtn = nullptr;

    //防止错误修改权限
    bool m_enable = false;

    bool m_permissions[3][3];

    //unixmode能力，标识修改权限是否可使用gio接口
    bool m_has_unix_mode = false;

    bool m_isShow = false;

public:
    void thisPageChanged() override;

    // PropertiesWindowTabIface interface
public:
    void saveAllChange();
};

class AdvancedPermissionsPage : public QWidget {
    Q_OBJECT
public:
    explicit AdvancedPermissionsPage(const QString &uri, QWidget *parent = nullptr);
    ~AdvancedPermissionsPage();

    void init();
    void initTableWidget();
    void initListWidget();
    void initFloorOne();
    void initFloorTwo();
    void initFloorThree();
    void initFloorFour();
    void addSeparate();

    void getUserInfo();
    void parseUserInfoAcl(QString strAcl);
    bool updateCheckBox(int col, QString &name);
    void updateUserInfo(int row, int col, bool checked);
    void checkInheritsBoxInfo();
    void initCheckState();
    void saveAclPermissions();

Q_SIGNALS:
    void updatePermissions();

protected Q_SLOTS:
    void updateDelAclBtn(int row, int col);

private:
    QString m_uri;
    QString m_defaultAcl;
    SystemDbusAccounts *m_sysAccounts = nullptr;
    QMap<QString, QString>  m_userInfo;
    QMutex m_mutex;
    bool m_thisPageChanged = false;

    QLabel *m_label = nullptr;
    QLabel *m_listLabel = nullptr;
    QTableWidget *m_tabWidget = nullptr;
    QListWidget *m_listWidget = nullptr;
    QPushButton *m_saveBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
    QPushButton *m_addUserBtn = nullptr;
    QPushButton *m_delUserBtn = nullptr;
    QCheckBox *m_inheritsBox = nullptr;
    QVBoxLayout *m_layout = nullptr;
};

}

#endif // PERMISSIONSPROPERTIESPAGE_H
