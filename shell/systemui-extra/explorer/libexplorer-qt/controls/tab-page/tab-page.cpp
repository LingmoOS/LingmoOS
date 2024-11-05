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

#include "tab-page.h"
#include "directory-view-container.h"
#include "directory-view-factory-manager.h"
#include "directory-view-plugin-iface.h"
#include "directory-view-widget.h"
#include "file-info.h"
#include "file-utils.h"

#include "file-launch-manager.h"

#include "properties-window.h"

#include <QTabBar>

#include <QUrl>

#include <QDebug>

using namespace Peony;

TabPage::TabPage(QWidget *parent) : QTabWidget(parent)
{
    m_double_click_limiter.setSingleShot(true);

    setMovable(true);
    setDocumentMode(true);
    setElideMode(Qt::ElideRight);
    setTabsClosable(true);
    setUsesScrollButtons(true);
    tabBar()->setExpanding(false);
    tabBar()->setAutoHide(true);

    connect(this, &QTabWidget::tabCloseRequested, [=](int index) {
        auto container = dynamic_cast<DirectoryViewContainer*>(widget(index));
        container->disconnect();
        container->deleteLater();
    });

    connect(this, &QTabWidget::currentChanged, [=](int index) {
        Q_UNUSED(index)
        Q_EMIT this->currentActiveViewChanged();

        this->rebindContainer();
    });
}

DirectoryViewContainer *TabPage::getActivePage()
{
    return qobject_cast<DirectoryViewContainer*>(currentWidget());
}

void TabPage::addPage(const QString &uri)
{
    auto container = new DirectoryViewContainer(this);
    auto view = container->getView();
    container->switchViewType(DirectoryViewFactoryManager2::getInstance()->getDefaultViewId());
    container->getView()->setDirectoryUri(uri);
    container->getView()->beginLocationChange();
    auto displayName = FileUtils::getFileDisplayName(uri);
    if (displayName.length() > ELIDE_TEXT_LENGTH)
    {
        int  charWidth = fontMetrics().averageCharWidth();
        displayName = fontMetrics().elidedText(displayName, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
    }

    if (displayName.contains("&")) {
        displayName = Peony::FileUtils::handleSpecialSymbols(displayName);
    }
    addTab(container,
           QIcon::fromTheme(FileUtils::getFileIconName(uri), QIcon::fromTheme("folder")),
           displayName);

    rebindContainer();
}

void TabPage::rebindContainer()
{
    for (int i = 0; i < this->count(); i++) {
        this->widget(i)->disconnect();
    }

    auto container = getActivePage();
    container->connect(container, &Peony::DirectoryViewContainer::viewDoubleClicked, [=](const QString &uri) {
        if (m_double_click_limiter.isActive())
            return;

        m_double_click_limiter.start(500);

        qDebug()<<"tab page double clicked"<<uri;
        //FIXME: replace BLOCKING api in ui thread.
        auto info = Peony::FileInfo::fromUri(uri);
        if (info->uri().startsWith("trash://")) {
            auto w = new PropertiesWindow(QStringList()<<uri);
            w->show();
            return;
        }
        if (info->isDir() || info->isVolume() || info->isVirtual()) {
            Q_EMIT this->updateWindowLocationRequest(uri);
        } else {
            FileLaunchManager::openAsync(uri, false, false);
        }
    });

    container->connect(container, &DirectoryViewContainer::updateWindowLocationRequest,
                       this, &TabPage::updateWindowLocationRequest);
    container->connect(container, &DirectoryViewContainer::directoryChanged,
                       this, &TabPage::currentLocationChanged);
    container->connect(container, &DirectoryViewContainer::selectionChanged,
                       this, &TabPage::currentSelectionChanged);
    container->connect(container, &DirectoryViewContainer::menuRequest,
                       this, &TabPage::menuRequest);

    container->connect(container, &DirectoryViewContainer::viewTypeChanged,
                       this, &TabPage::viewTypeChanged);
}

void TabPage::refreshCurrentTabText()
{
    auto uri = getActivePage()->getCurrentUri();
    setTabText(currentIndex(), FileUtils::getFileDisplayName(uri));
    setTabIcon(currentIndex(),
               QIcon::fromTheme(FileUtils::getFileIconName(uri),
                                QIcon::fromTheme("folder")));
}

void TabPage::stopLocationChange()
{
    auto view = getActivePage();
    view->stopLoading();
}
