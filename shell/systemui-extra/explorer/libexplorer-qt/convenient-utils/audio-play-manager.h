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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#ifndef AUDIOPLAYMANAGER_H
#define AUDIOPLAYMANAGER_H

#include <QObject>
#include "explorer-core_global.h"
#include <gio/gio.h>

//play audio lib head file
#include <canberra.h>

namespace Peony {

class PEONYCORESHARED_EXPORT AudioPlayManager : public QObject
{
    Q_OBJECT
public:
    static void init();
    static AudioPlayManager *getInstance();
    void close();

    void playWarningAudio();

protected:
    const gchar* m_event_id = "dialog-warning";
    ca_context *m_ca_context;

private:
    explicit AudioPlayManager(QObject *parent = nullptr);
    ~AudioPlayManager();
};

}

#endif // AUDIOPLAYMANAGER_H
