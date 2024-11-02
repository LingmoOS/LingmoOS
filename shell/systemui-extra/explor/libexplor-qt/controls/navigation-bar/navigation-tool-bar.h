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

#ifndef NAVIGATIONTOOLBAR_H
#define NAVIGATIONTOOLBAR_H

#include <QToolBar>
#include <QStack>

#include "explor-core_global.h"

namespace Peony {

class DirectoryViewContainer;

class PEONYCORESHARED_EXPORT NavigationToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit NavigationToolBar(QWidget *parent = nullptr);
    bool canGoBack();
    bool canGoForward();
    bool canCdUp();

Q_SIGNALS:
    void updateWindowLocationRequest(const QString &uri, bool addHistory, bool forceUpdate = false);
    void refreshRequest();

public Q_SLOTS:
    void setCurrentContainer(DirectoryViewContainer *container);

    void updateActions();

    void onGoBack();
    void onGoForward();
    void onGoToUri(const QString &uri, bool addHistory, bool forceUpdate = false);

    void clearHistory();

private:
    DirectoryViewContainer *m_current_container = nullptr;

    QAction *m_back_action;
    QAction *m_forward_action;
    QAction *m_history_action;
    QAction *m_cd_up_action;
    QAction *m_refresh_action;
};

}

#endif // NAVIGATIONTOOLBAR_H
