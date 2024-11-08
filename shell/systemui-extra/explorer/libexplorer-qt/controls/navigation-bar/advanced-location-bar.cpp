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

#include "advanced-location-bar.h"
#include "path-edit.h"
#include "location-bar.h"
#include "search-vfs-uri-parser.h"
#include "search-bar-container.h"
#include "global-settings.h"
#include "trash-cleaned-watcher.h"

#include <QStackedLayout>
#include <QDebug>
#include <QToolButton>
#include <QResizeEvent>
using namespace Peony;

AdvancedLocationBar::AdvancedLocationBar(QWidget *parent) : QWidget(parent)
{
    QStackedLayout *layout = new QStackedLayout(this);
    m_layout = layout;

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetDefaultConstraint);
    m_bar = new Peony::LocationBar(this);
    m_edit = new Peony::PathEdit(this);
    m_search_bar = new Peony::SearchBarContainer(this);

    m_bar->connect(m_bar, &Peony::LocationBar::blankClicked, [=]() {
        auto curUri = m_bar->getCurentUri();
        //不允许编辑的路径，新增搜索目录，关联bug#94229 改动
        if (! curUri.startsWith("trash:///") && ! curUri.startsWith("recent:///") && ! curUri.startsWith("search:///"))
        {
            layout->setCurrentWidget(m_edit);
            m_edit->setFocus();
            m_edit->setUri(curUri);
        }
    });

    m_edit->connect(m_edit, &Peony::PathEdit::uriChangeRequest, [=](const QString uri) {
        //qDebug() << "uriChangeRequest:" <<uri;
        QString targetUri = uri;
        //fix bug 38942
        while (targetUri.endsWith("/") && targetUri != "file:///")
        {
            targetUri = targetUri.left(targetUri.lastIndexOf("/"));
        }
        m_bar->setRootUri(targetUri);
        layout->setCurrentWidget(m_bar);
        Q_EMIT this->updateWindowLocationRequest(targetUri);
        m_text = m_edit->text();
        if (! m_text.startsWith("search://"))
            m_last_non_search_path = m_text;
    });

    m_bar->connect(m_bar, &LocationBar::groupChangedRequest, [=](const QString &uri) {
        if (m_text == uri) {
            Q_EMIT this->refreshRequest();
            return;
        }
        Q_EMIT this->updateWindowLocationRequest(uri);
        m_text = uri;
        if (! uri.startsWith("search://"))
            m_last_non_search_path = uri;
    });

    m_edit->connect(m_edit, &Peony::PathEdit::editCancelled, [=]() {
        layout->setCurrentWidget(m_bar);
    });

    m_search_bar->connect(m_search_bar, &Peony::SearchBarContainer::returnPressed, [=]() {
        auto key = m_search_bar->text();
        key = processSpecialChar(key);
        qDebug() << "search key:" <<key <<m_last_key;
        if (key != m_last_key)
        {
            Q_EMIT searchRequest(m_last_non_search_path, key);
            m_last_key = key;
            if (key == "") {
                m_search_bar->updateSearchProgress(false);
            } else {
                m_search_bar->updateSearchProgress(true);
            }
        }
    });

    m_search_bar->connect(m_search_bar, &Peony::SearchBarContainer::filterUpdate, [=](const int &index)
    {
        Q_EMIT this->updateFileTypeFilter(index);
    });

    connect(this, &Peony::AdvancedLocationBar::updateSearchProgress, m_search_bar, &Peony::SearchBarContainer::updateSearchProgress);

    bool is_intel = (QString::compare("V10SP1-edu", GlobalSettings::getInstance()->getProjectName(), Qt::CaseInsensitive) == 0);
    if (is_intel) {
        QToolButton* indicator = m_bar->findChild<QToolButton*>("explorer_location_bar_indicator");
        if (indicator) {
            indicator->move(0, 1);
        }
    }

    layout->addWidget(m_bar);
    layout->addWidget(m_edit);
    layout->addWidget(m_search_bar);

    setLayout(layout);
    auto iscleaned = Peony::TrashCleanedWatcher::getInstance();
    connect(iscleaned,&TrashCleanedWatcher::updateTrashIcon,m_bar,&LocationBar::updateTrashIcon);
}

QString AdvancedLocationBar::processSpecialChar(QString key)
{
    if (key.length() == 0)
        return key;
    //qDebug() << "enter processSpecialChar:" <<key;
    for(auto mchar : SPECIAL_CHARS)
    {
        if (key.contains(mchar))
        {
            QString tmp = mchar;
            tmp.replace("\\", "");
            key = key.replace(tmp, "\\" + mchar);
        }
    }

    //qDebug() << "ret processSpecialChar:" <<key;
    return key;
}

void AdvancedLocationBar::updateLocation(const QString &uri)
{
    m_bar->setRootUri(uri);
    m_edit->setUri(uri);
    m_text = uri;
    //qDebug() << "m_edit visible:"<<isEditing();
    if (! uri.startsWith("search://"))
    {
        m_last_non_search_path = uri;
        //from search mode go to other non search path, stop search
        //fix bug#97807, change path in search mode crash issue
        if (m_last_key != "")
            clearSearchBox();
    }
    Q_EMIT this->refreshRequest();
}
void AdvancedLocationBar::setAnimationMode(bool isAnimation)
{
    m_bar->setAnimationMode(isAnimation);
}

bool AdvancedLocationBar::isEditing()
{
    return m_edit->isVisible();
}

void AdvancedLocationBar::startEdit()
{
    m_edit->setVisible(true);
    m_layout->setCurrentWidget(m_edit);
    m_edit->setFocus();
    m_edit->setUri(m_bar->getCurentUri());
}

void AdvancedLocationBar::cancelEdit()
{
    m_edit->cancelList();
}

void AdvancedLocationBar::finishEdit()
{
    Q_EMIT m_edit->returnPressed();
}

void AdvancedLocationBar::switchEditMode(bool bSearchMode)
{
    if (bSearchMode)
    {
        m_edit->setVisible(false);
        m_layout->setCurrentWidget(m_search_bar);
        m_search_bar->setPlaceholderText(tr("Search Content..."));
        m_search_bar->setFocus();
        m_in_search_mode = true;
    }
    else if(m_in_search_mode)
    {
        //quit search mode, show non search contents
        // bug#94229 改动，不需要额外更新信号，退出搜索模式正常
//        if (m_last_key.length() > 0)
//        {
//            Q_EMIT this->updateWindowLocationRequest(m_last_non_search_path, false);
//        }

        //在文件保护箱中搜索时，清空搜索内容会导致刷新，路径更新需要授权，导致2次弹框
        //屏蔽清空搜索内容代码，解决bug#76431, 概率性闪退问题
        QString curUri = m_bar->getCurentUri();
        if (! curUri.startsWith("filesafe:///"))
            m_search_bar->setText("");
        m_layout->setCurrentWidget(m_bar);
        m_in_search_mode = false;
    }
}

void AdvancedLocationBar::clearSearchBox()
{
    m_search_bar->clearSearchBox();
    m_last_key = "";
}

void AdvancedLocationBar::deselectSearchBox()
{
    m_search_bar->deselectSearchBox();
    m_last_key = "";
}

void AdvancedLocationBar::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    m_edit->setFixedHeight(e->size().height());
    m_search_bar->setFixedHeight(e->size().height());
    m_bar->setFixedHeight(e->size().height());
}

