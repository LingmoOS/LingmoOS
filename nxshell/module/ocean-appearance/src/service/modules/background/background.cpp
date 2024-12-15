// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "background.h"
#include "../api/utils.h"
#include "dbus/appearancedbusproxy.h"

#include <QDBusInterface>
#include <pwd.h>

Background::Background()
{
}

Background::~Background()
{
}

void Background::setId(QString id)
{
    this->id = id;
}

QString Background::getId() const
{
    return id;
}

bool Background::getDeleteable()
{
    return deletable;
}
void Background::setDeletable(bool deletable)
{
    this->deletable = deletable;
}

void Background::Delete()
{
    if (!deletable) {
        return;
    }

    struct passwd *user = getpwuid(getuid());
    if (user == nullptr) {
        return;
    }

    QString uri = utils::deCodeURI(id);

    AppearanceDBusProxy::DeleteCustomWallPaper(user->pw_name, uri);
    AppearanceDBusProxy::Delete(uri);
    AppearanceDBusProxy::Delete("all", uri);
}

QString Background::Thumbnail()
{
    return "";
}
