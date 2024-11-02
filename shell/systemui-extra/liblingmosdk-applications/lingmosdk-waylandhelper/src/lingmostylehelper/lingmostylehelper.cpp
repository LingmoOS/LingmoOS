/*
 * liblingmosdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "lingmostylehelper.h"
#include "../waylandhelper.h"
#include "lingmo-decoration-manager.h"
#include "lingmo-shell-manager.h"
#include "xatom-helper.h"
#include "../windowmanager/lingmowaylandinterface.h"
#include <QWidget>
#include <QGuiApplication>

namespace kdk {

static LingmoUIStyleHelper *global_instance = nullptr;

LingmoUIStyleHelper *LingmoUIStyleHelper::self()
{
    if(global_instance)
        return global_instance;
    else
    {
        global_instance = new LingmoUIStyleHelper();
        return global_instance;
    }
}

void LingmoUIStyleHelper::removeHeader(QWidget* widget)
{
    if(!widget)
        return;
    QString platform = QGuiApplication::platformName();
    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
    {
        m_widget = widget;
        m_widget->installEventFilter(this);
    }
    else
    {
        MotifWmHints hints1;
        hints1.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints1.functions = MWM_FUNC_ALL;
        hints1.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(widget->winId(), hints1);
    }
}

bool LingmoUIStyleHelper::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj == m_widget && (ev->type() == QEvent::PlatformSurface || ev->type() == QEvent::Show || ev->type() == QEvent::UpdateRequest))
    {
        #ifdef USE_LINGMO_PROTOCOL
            if(LingmoUIShellManager::getInstance()->lingmoshellReady())
                LingmoUIShellManager::getInstance()->removeHeaderBar(m_widget->windowHandle());
            else
                LINGMODecorationManager::getInstance()->removeHeaderBar(m_widget->windowHandle());
        #else
            LINGMODecorationManager::getInstance()->removeHeaderBar(m_widget->windowHandle());
        #endif
    }
    return QObject::eventFilter(obj,ev);
}

LingmoUIStyleHelper::LingmoUIStyleHelper()
{

}

}
