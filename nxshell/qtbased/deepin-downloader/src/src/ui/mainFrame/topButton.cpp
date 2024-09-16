// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file topButton.cpp
 *
 * @brief 主界面顶部按钮加搜索框
 *
 * @date 2020-06-09 10:00
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "topButton.h"
#include <QDebug>
#include <DToolTip>
#include <QTimer>
#include <QCursor>
#include <QMouseEvent>
#include <QCompleter>
#include <QAbstractItemView>
#include <DGuiApplicationHelper>
#include "global.h"

DWIDGET_USE_NAMESPACE

TopButton::TopButton(QWidget *parent)
    : QWidget(parent)
{
    Init();
    InitConnections();
    setObjectName("toolBox");
}

QPoint TopButton::getSearchEditPosition()
{
    return m_searchEdit->geometry().bottomLeft();
}

void TopButton::Init()
{
    QHBoxLayout *mainHlayout = new QHBoxLayout(this);

    mainHlayout->setContentsMargins(0, 5, 0, 5);
    mainHlayout->setSpacing(10);
    m_iconLable = new DLabel;
    QIcon logo_icon = QIcon(":icons/icon/downloader5.svg");
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        m_iconLable->setPixmap(logo_icon.pixmap(32, 32));
    } else {
        m_iconLable->setPixmap(logo_icon.pixmap(32*Global::compactMode_ratio, 32*Global::compactMode_ratio));
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            m_iconLable->setPixmap(logo_icon.pixmap(32, 32));
        } else {
            m_iconLable->setPixmap(logo_icon.pixmap(32*Global::compactMode_ratio, 32*Global::compactMode_ratio));
        }
    });
#else
    m_iconLable->setPixmap(logo_icon.pixmap(32, 32));
#endif
    m_searchEdit = new SearchWidget();
    m_searchEdit->setMinimumWidth(380);
    m_searchEdit->lineEdit()->setMaxLength(256);

    // searchEdit->setFixedSize(350,36);
    m_newDownloadBtn = new DIconButton(this);
    m_newDownloadBtn->setObjectName("newTaskBtn");
    m_newDownloadBtn->setAccessibleName("newTaskBtn");
    m_newDownloadBtn->setIcon(QIcon::fromTheme("dcc_newdownload"));
    m_newDownloadBtn->setToolTip(tr("New task"));

    m_pauseDownloadBtn = new DIconButton(this);
    m_pauseDownloadBtn->setObjectName("pauseDownloadBtn");
    m_pauseDownloadBtn->setAccessibleName("pauseDownloadBtn");
    m_pauseDownloadBtn->setIcon(QIcon::fromTheme("dcc_list_icon_pause"));
    m_pauseDownloadBtn->setEnabled(false);
    m_pauseDownloadBtn->setToolTip(tr("Pause"));

    m_startDownloadBtn = new DIconButton(this);
    m_startDownloadBtn->setObjectName("startDownloadBtn");
    m_startDownloadBtn->setAccessibleName("startDownloadBtn");
    m_startDownloadBtn->setIcon(QIcon::fromTheme("dcc_icon_start"));
    m_startDownloadBtn->setEnabled(false);
    m_startDownloadBtn->setToolTip(tr("Resume"));

    m_deleteDownloadBtn = new DIconButton(this);
    m_deleteDownloadBtn->setObjectName("deleteBtn");
    m_deleteDownloadBtn->setAccessibleName("deleteBtn");
    m_deleteDownloadBtn->setIcon(QIcon::fromTheme("dcc_list_icon_delete"));
    m_deleteDownloadBtn->setEnabled(false);
    m_deleteDownloadBtn->setToolTip(tr("Delete"));

    mainHlayout->addSpacing(10);
    mainHlayout->addWidget(m_iconLable);
    mainHlayout->addSpacing(7);
    mainHlayout->addWidget(m_pauseDownloadBtn);
    mainHlayout->addWidget(m_startDownloadBtn);

    mainHlayout->addWidget(m_deleteDownloadBtn);
    mainHlayout->addWidget(m_newDownloadBtn);
    mainHlayout->addStretch();
    mainHlayout->addWidget(m_searchEdit);
    mainHlayout->addStretch();

    // mainHlayout->addStretch();
    qDebug() << "asdwasdw";
}

void TopButton::InitConnections()
{
    connect(m_newDownloadBtn, &DIconButton::clicked, this, &TopButton::newDownloadBtnClicked);
    connect(m_pauseDownloadBtn, &DIconButton::clicked, this, &TopButton::pauseDownloadBtnClicked);
    connect(m_startDownloadBtn, &DIconButton::clicked, this, &TopButton::startDownloadBtnClicked);
    connect(m_deleteDownloadBtn, &DIconButton::clicked, this, &TopButton::deleteDownloadBtnClicked);
    connect(m_searchEdit, &DSearchEdit::focusChanged, this, &TopButton::SearchEditFocus);
    connect(m_searchEdit, &DSearchEdit::textChanged, this, &TopButton::SearchEditTextChange);
    connect(m_searchEdit, &SearchWidget::keyPressed, this, &TopButton::SearchEditKeyPressed);
}

void TopButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }
    QWidget::mousePressEvent(event);
}

void TopButton::onTableChanged(int index)
{
    if (index == 2) {
        m_startDownloadBtn->setIcon(QIcon::fromTheme("dcc_recycel_delete"));
        m_pauseDownloadBtn->setIcon(QIcon::fromTheme("dcc_recycel_restore"));
        m_deleteDownloadBtn->setIcon(QIcon::fromTheme("dcc_list_icon_delete"));

        m_startDownloadBtn->setToolTip(tr("Empty"));
        m_pauseDownloadBtn->setToolTip(tr("Restore"));
        m_deleteDownloadBtn->setToolTip(tr("Delete"));
    } else if (index == 1) {
        m_startDownloadBtn->setIcon(QIcon::fromTheme("dcc_finish_openfolder"));
        m_pauseDownloadBtn->setIcon(QIcon::fromTheme("dcc_finish_openfile"));
        m_deleteDownloadBtn->setIcon(QIcon::fromTheme("dcc_list_icon_delete"));

        m_startDownloadBtn->setToolTip(tr("Open folder"));
        m_pauseDownloadBtn->setToolTip(tr("Open"));
        m_deleteDownloadBtn->setToolTip(tr("Delete"));
    } else {
        m_startDownloadBtn->setIcon(QIcon::fromTheme("dcc_icon_start"));
        m_pauseDownloadBtn->setIcon(QIcon::fromTheme("dcc_list_icon_pause"));
        m_deleteDownloadBtn->setIcon(QIcon::fromTheme("dcc_list_icon_delete"));

        m_startDownloadBtn->setToolTip(tr("Resume"));
        m_pauseDownloadBtn->setToolTip(tr("Pause"));
        m_deleteDownloadBtn->setToolTip(tr("Delete"));

        m_startDownloadBtn->setEnabled(false);
        m_pauseDownloadBtn->setEnabled(false);
        m_deleteDownloadBtn->setEnabled(false);
    }
}

SearchWidget::SearchWidget(QWidget *parent)
     : DTK_WIDGET_NAMESPACE::DSearchEdit(parent)
{
    connect(lineEdit(), &QLineEdit::editingFinished, this, [&](){
                emit keyPressed(Qt::Key_Enter);
    });
}

bool SearchWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        switch (ke->key()) {
        case Qt::Key_Up: {
            emit keyPressed(Qt::Key_Up);
            return true;
        }
        case Qt::Key_Down: {
            emit keyPressed(Qt::Key_Down);
            return true;
        }
        case Qt::Key_Enter: {
            emit keyPressed(Qt::Key_Enter);
            return true;
        }
        default:
            break;
        }
    }
    return QWidget::eventFilter(o, e);
}
