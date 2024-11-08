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

#ifndef SHAREPAGE_H
#define SHAREPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <properties-window-tab-iface.h>
#include <memory>
#include <file-info.h>
#include <QCheckBox>
#include <QFuture>
#include <explorer-qt/usershare-manager.h>
#include <explorer-qt/file-utils.h>

#include "kswitchbutton.h"
using namespace kdk;

//#include "libexplorer-qt/usershare-manager.h"
using namespace Peony;

class SharePage : public Peony::PropertiesWindowTabIface
{
Q_OBJECT
public:
    explicit SharePage(const QString &uri, QWidget *parent = nullptr);

    void addSeparate()
    {
        QFrame * separate = new QFrame(this);
        separate->setContentsMargins(0, 0, 0, 0);
        separate->setFrameShape(QFrame::HLine);
        QPalette palette = separate->palette();
        palette.setColor(QPalette::WindowText,QColor("#F0F0F0FF"));
        separate->setPalette(palette);
        m_layout->addWidget(separate);
    }

    void init();

    void initFloorOne();

    void initFloorTwo();

    void initFloorThree();

    /*!
     * 响应确定按钮
     * \brief
     */
    void saveAllChange() override;

    bool checkAuthorization();

    bool checkSpecialSymbols(const QString& displayName);

private:
    QFutureWatcher<void> *m_theFutureWatcher = nullptr;

    Peony::ShareInfo       m_shareInfo;
    QVBoxLayout*    m_layout = nullptr;

    std::shared_ptr<Peony::FileInfo> m_fileInfo = nullptr;

    //floor1
    QLabel      *m_iconLabel   = nullptr;
    QLabel      *m_folderName  = nullptr;
    QLabel      *m_sharedState = nullptr;

    //floor2
    KSwitchButton *m_switchButton = nullptr;
    //floor3
    QFrame    *m_floor3        = nullptr;
    QLineEdit *m_shareNameEdit = nullptr;
    QLineEdit *m_commentEdit   = nullptr;
    QPushButton *m_advanceShareButton = nullptr;

    QCheckBox *m_allowGuestModify   = nullptr;
    QCheckBox *m_shareAllowGuestCheckBox = nullptr;

    bool m_ret = false;
    bool m_has_unix_mode = false;
    QString m_usershareAcl;
    QMap<QString, QString> m_userInfoCache;
};

#endif // SHAREPAGE_H
