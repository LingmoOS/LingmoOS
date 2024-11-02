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

#include "kywlcom-thumbnail.h"
#include <QDebug>

class Thumbnail::Private
{
public:
    Private(Thumbnail *thumbnail);
    ~Private();
    void setup(kywc_context *ctx, Thumbnail::Type type, QString uuid, QString output_uuid, QString decoration);

    int32_t fd;
    uint32_t format;
    QSize size;
    uint32_t offset;
    uint32_t stride;
    uint64_t modifier;
    Thumbnail::BufferFlags flags;

    Type type;
    QString source_uuid;
    QString output_uuid;

    kywc_thumbnail *k_thumbnail = nullptr;

private:
    Thumbnail *q;
    static bool bufferHandle(kywc_thumbnail *thumbnail, const struct kywc_thumbnail_buffer *buffer,
                             void *data);
    static void destroyHandle(kywc_thumbnail *thumbnail, void *data);
    static struct kywc_thumbnail_interface thumbnail_impl;
};

Thumbnail::Private::Private(Thumbnail *thumbnail) : q(thumbnail) {}

Thumbnail::Private::~Private() {}

bool Thumbnail::Private::bufferHandle(kywc_thumbnail *thumbnail,
                                      const struct kywc_thumbnail_buffer *buffer, void *data)
{
    Thumbnail *thum = (Thumbnail *)data;
    switch (thumbnail->type) {
        case KYWC_THUMBNAIL_TYPE_OUTPUT:
            thum->d->type = Thumbnail::Type::Output;
            break;
        case KYWC_THUMBNAIL_TYPE_TOPLEVEL:
            thum->d->type = Thumbnail::Type::Toplevel;
            break;
        case KYWC_THUMBNAIL_TYPE_WORKSPACE:
            thum->d->type = Thumbnail::Type::Workspace;
            break;
    }

    thum->d->source_uuid = QString(thumbnail->source_uuid);
    thum->d->output_uuid = QString(thumbnail->output_uuid);

    if (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_DMABUF) {
        thum->d->flags |= Thumbnail::BufferFlag::Dmabuf;
    }

    if (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_REUSED) {
        thum->d->flags |= Thumbnail::BufferFlag::Reused;
    }

    thum->d->size = QSize(buffer->width, buffer->height);
    thum->d->offset = buffer->offset;
    thum->d->stride = buffer->stride;
    thum->d->fd = buffer->fd;

    thum->d->format = buffer->format;
    thum->d->modifier = buffer->modifier;

    emit thum->bufferUpdate();

    if (buffer->flags & KYWC_THUMBNAIL_BUFFER_IS_DMABUF) {
        return true;
    } else
        return false;
}

void Thumbnail::Private::destroyHandle(kywc_thumbnail *thumbnail, void *data)
{
    Thumbnail *thum = (Thumbnail *)data;
    thum->d->k_thumbnail = nullptr;
    emit thum->deleted();
}

struct kywc_thumbnail_interface Thumbnail::Private::thumbnail_impl {
        bufferHandle, destroyHandle,
};

void Thumbnail::Private::setup(kywc_context *ctx, Thumbnail::Type type, QString uuid,
                               QString output_uuid, QString decoration)
{
    QByteArray qByteArray_uuid = uuid.toUtf8();
    char *str = qByteArray_uuid.data();
    kywc_thumbnail *thumbnail = NULL;

    switch (type) {
        case Thumbnail::Type::Output:
            thumbnail = kywc_thumbnail_create_from_output(ctx, str, &thumbnail_impl, this->q);
            break;
        case Thumbnail::Type::Toplevel: {
            bool without_decoration = false;
            if (decoration == QLatin1String("true")) {
                without_decoration = true;
            }
            thumbnail = kywc_thumbnail_create_from_toplevel(ctx, str, without_decoration, &thumbnail_impl, this->q);
        } break;
        case Thumbnail::Type::Workspace: {
            QByteArray qByteArray_uuid = output_uuid.toUtf8();
            char *output = qByteArray_uuid.data();
            thumbnail = kywc_thumbnail_create_from_workspace(ctx, str, output, &thumbnail_impl, this->q);
        } break;

    }
    k_thumbnail = thumbnail;
}

Thumbnail::Thumbnail(QObject *parent) : d(new Private(this)) {}

Thumbnail::~Thumbnail() {
    if(d) {
        delete d;
        d = nullptr;
    }
}

void Thumbnail::setup(kywc_context *ctx, Thumbnail::Type type, QString uuid, QString output_uuid, QString decoration)
{
    d->setup(ctx, type, uuid, output_uuid, decoration);;
}

int32_t Thumbnail::fd() const
{
    return d->fd;
}

uint32_t Thumbnail::format() const
{
    return d->format;
}

QSize Thumbnail::size() const
{
    return d->size;
}

uint32_t Thumbnail::offset() const
{
    return d->offset;
}

uint32_t Thumbnail::stride() const
{
    return d->stride;
}

uint64_t Thumbnail::modifier() const
{
    return d->modifier;
}

Thumbnail::BufferFlags Thumbnail::flags() const
{
    return d->flags;
}

void Thumbnail::destory()
{
    if(d->k_thumbnail) {
        kywc_thumbnail_destroy(d->k_thumbnail);
        d->k_thumbnail = nullptr;
    }
}
