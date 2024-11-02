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
#include "kywlcom-contex.h"
#include <QSocketNotifier>
#include <utility>
#include <libkywc.h>
#include <QDebug>

class Context::Private
{
public:
    explicit Private(Context *context);
    ~Private();
    kywc_context *setup(uint32_t capability);

    kywc_context *k_context = nullptr;
    QSocketNotifier *notifier = nullptr;
    struct wl_display *display = nullptr;
    uint32_t capabilities = 0;
    bool valid = false;

private:
    Context *q;
    static void createHandle(kywc_context *context, void *data);
    static void destroyHandle(kywc_context *context, void *data);
    static void newOutput(kywc_context *context, kywc_output *output, void *data);
    static void newToplevel(kywc_context *context, kywc_toplevel *toplevel, void *data);
    static void newWorkspace(kywc_context *context, kywc_workspace *workspace, void *data);
    static struct kywc_context_interface context_impl;
};

Context::Private::Private(Context *context) : q(context) {}

Context::Private::~Private() {}

void Context::Private::createHandle(kywc_context *context, void *data)
{
    auto *p = (Context::Private *)data;
    Q_EMIT p->q->created();
}

void Context::Private::destroyHandle(kywc_context *context, void *data)
{
    auto *p = (Context::Private *)data;
    Q_EMIT p->q->destroyed();
}

void Context::Private::newOutput(kywc_context *context, kywc_output *output, void *data)
{
}

void Context::Private::newToplevel(kywc_context *context, kywc_toplevel *toplevel, void *data)
{
}

void Context::Private::newWorkspace(kywc_context *context, kywc_workspace *workspace, void *data)
{
}

struct kywc_context_interface Context::Private::context_impl = {
        createHandle, destroyHandle, newOutput, newToplevel, newWorkspace,
};


kywc_context *Context::Private::setup(uint32_t capabilities)
{
    if (!display)
        k_context = kywc_context_create(nullptr, capabilities, &context_impl, this);
    else
        k_context = kywc_context_create_by_display(display, capabilities, &context_impl, this);
    return k_context;
}

Context::Context( struct wl_display *display, Capabilities caps, QObject *parent)
        : QObject{parent}
        , d(new Private(this))
{
    d->display = display;

    uint32_t capabilities = 0;
    if (caps & Context::Capability::Output)
        capabilities |= KYWC_CONTEXT_CAPABILITY_OUTPUT;
    if (caps & Context::Capability::Toplevel)
        capabilities |= KYWC_CONTEXT_CAPABILITY_TOPLEVEL;
    if (caps & Context::Capability::Workspace)
        capabilities |= KYWC_CONTEXT_CAPABILITY_WORKSPACE;
    if (caps & Context::Capability::Thumbnail)
        capabilities |= KYWC_CONTEXT_CAPABILITY_THUMBNAIL;
    d->capabilities = capabilities;
}

Context::~Context() {
    kywc_context_destroy(d->k_context);
    if(d) {
        delete d;
        d = nullptr;
    }
}

void Context::start()
{
    kywc_context *ctx = nullptr;

    if (!d->display) {
        ctx = d->setup(d->capabilities);
        if (!ctx) {
            return;
        }
        d->notifier = new QSocketNotifier(kywc_context_get_fd(ctx), QSocketNotifier::Read, this);
        connect(d->notifier, &QSocketNotifier::activated, this, &Context::onContextReady);

        kywc_context_process(ctx);
    } else {
        ctx = d->setup(d->capabilities);
        if (!ctx) {
            return;
        }
    }
    d->valid = true;
    Q_EMIT validChanged(d->valid);
}

void Context::onContextReady()
{
    if (kywc_context_process(d->k_context) < 0) {
        disconnect(d->notifier, &QSocketNotifier::activated, this, &Context::onContextReady);
        Q_EMIT aboutToTeardown();
        d->valid  = false;
        Q_EMIT validChanged(d->valid);
    }
}

void Context::dispatch()
{
    kywc_context_dispatch(d->k_context);
}

void Context::thumbnail_init(Thumbnail *thumbnail, Thumbnail::Type type, QString uuid, QString output_uuid, QString decoration)
{
    thumbnail->setup(d->k_context, type, std::move(uuid), std::move(output_uuid), std::move(decoration));
}

bool Context::valid()
{
    return d->valid;
}
