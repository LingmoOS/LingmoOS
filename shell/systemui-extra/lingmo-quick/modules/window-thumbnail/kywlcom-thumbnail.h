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

#ifndef LINGMO_QUICK_KYWLCOM_THUMBNAIL_H
#define LINGMO_QUICK_KYWLCOM_THUMBNAIL_H

#include <QObject>
#include <QSize>
#include <libkywc.h>
class Thumbnail : public QObject
{
    Q_OBJECT
public:
    enum Type{
        Output,
        Toplevel,
        Workspace,
    };

    enum BufferFlag {
        Dmabuf = 1 << 0,
        Reused = 1 << 1,
    };
    Q_DECLARE_FLAGS(BufferFlags, BufferFlag);

    explicit Thumbnail(QObject *parent = nullptr);
    ~Thumbnail();

    void setup(kywc_context *ctx, Thumbnail::Type type, QString uuid, QString output_uuid, QString decoration);
    void destory();

    int32_t fd() const;
    uint32_t format() const;
    QSize size() const;
    uint32_t offset() const;
    uint32_t stride() const;
    uint64_t modifier() const;
    Thumbnail::BufferFlags flags() const;

Q_SIGNALS:
    void bufferUpdate();
    void deleted();

private:
    class Private;
    Private *d;
};


#endif //LINGMO_QUICK_KYWLCOM_THUMBNAIL_H
