/*
 * Peony-Qt's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "xdg-portal-helper.h"
#include <gio/gio.h>

#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

using namespace Peony;

static XdgPortalHelper *global_instance = nullptr;
static bool is_flatpak_context = false;
static bool is_init_with_gtk_use_portal_env = false;

static QMutex portal_mutex;

XdgPortalHelper *XdgPortalHelper::getInstance()
{
    if (!global_instance)
        global_instance = new XdgPortalHelper;
    return global_instance;
}

bool XdgPortalHelper::tryUnusePortal()
{
    if (is_flatpak_context)
        return false;
    if (!is_init_with_gtk_use_portal_env)
        return true;

    if (!portal_mutex.tryLock()) {
        qWarning()<<__FUNCTION__<<"can not get portal mutex control";
        return false;
    }
    g_unsetenv("GTK_USE_PORTAL");
    portal_mutex.unlock();
    return true;
}

bool XdgPortalHelper::tryResetPortal()
{
    if (is_flatpak_context)
        return true;
    if (!is_init_with_gtk_use_portal_env)
        return true;
    if (!portal_mutex.tryLock()) {
        qWarning()<<__FUNCTION__<<"can not get portal mutex control";
        return false;
    }
    g_setenv("GTK_USE_PORTAL", "1", true);
    portal_mutex.unlock();
    return true;
}

XdgPortalHelper::XdgPortalHelper(QObject *parent)
    : QObject{parent}
{
    is_flatpak_context = g_file_test("/.flatpak-info", G_FILE_TEST_EXISTS);
    auto env = g_getenv("GTK_USE_PORTAL");
    is_init_with_gtk_use_portal_env = (env && env[0] == '1');
}
