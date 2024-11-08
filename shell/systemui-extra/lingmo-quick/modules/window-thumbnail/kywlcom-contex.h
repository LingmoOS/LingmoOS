/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_KYWLCOM_CONTEX_H
#define LINGMO_QUICK_KYWLCOM_CONTEX_H

#include <QObject>
#include "kywlcom-thumbnail.h"
class Context : public QObject
{
    Q_OBJECT
public:
    enum class Capability {
        Output = 1 << 0,
        Toplevel = 1 << 1,
        Workspace = 1 << 2,
        Thumbnail = 1 << 3,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    explicit Context(struct wl_display *display, Capabilities caps, QObject *parent = nullptr);
    ~Context();

    void start();
    void thumbnail_init(Thumbnail *thumbnail, Thumbnail::Type type, QString uuid, QString output_uuid, QString decoration);
    void dispatch();
    bool valid();

Q_SIGNALS:
    void aboutToTeardown();

    void created();
    void destroyed();
    void validChanged(bool);

private Q_SLOTS:
    void onContextReady();

private:
    class Private;
    Private *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Context::Capabilities)


#endif //LINGMO_QUICK_KYWLCOM_CONTEX_H
