// Copyright (C) 2024 WenHao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "personalization_manager_impl.h"
#include "personalizationmanager.h"

#include <qwglobal.h>
#include <qwoutput.h>
#include <qwsignalconnector.h>

#include <QHash>

QW_USE_NAMESPACE

class PersonalizationCursorContextPrivate : public QWObjectPrivate
{
public:
    PersonalizationCursorContextPrivate(personalization_cursor_context_v1 *handle,
                                           bool isOwner,
                                           PersonalizationCursorContext *qq)
        : QWObjectPrivate(handle, isOwner, qq)
    {
        Q_ASSERT(!map.contains(handle));
        map.insert(handle, qq);
        sc.connect(&handle->events.destroy, this, &PersonalizationCursorContextPrivate::on_destroy);
        sc.connect(&handle->events.commit, this,
                   &PersonalizationCursorContextPrivate::on_commit);
        sc.connect(&handle->events.get_size, this,
                   &PersonalizationCursorContextPrivate::on_get_size);
        sc.connect(&handle->events.get_theme, this,
                   &PersonalizationCursorContextPrivate::on_get_theme);
    }

    ~PersonalizationCursorContextPrivate()
    {
        if (!m_handle)
            return;
        destroy();
        if (isHandleOwner)
            personalization_cursor_context_v1_destroy(q_func()->handle());
    }

    inline void destroy()
    {
        Q_ASSERT(m_handle);
        Q_ASSERT(map.contains(m_handle));
        Q_EMIT q_func()->beforeDestroy(q_func());
        map.remove(m_handle);
        sc.invalidate();
    }

    void on_destroy(void *);
    void on_commit(void *);
    void on_get_size(void *);
    void on_get_theme(void *);

    static QHash<void *, PersonalizationCursorContext *> map;
    QW_DECLARE_PUBLIC(PersonalizationCursorContext)
    QWSignalConnector sc;
};

QHash<void *, PersonalizationCursorContext *> PersonalizationCursorContextPrivate::map;

void PersonalizationCursorContextPrivate::on_destroy(void *)
{
    destroy();
    m_handle = nullptr;
    delete q_func();
}

void PersonalizationCursorContextPrivate::on_commit(void *data)
{
    Q_EMIT q_func()->commit(reinterpret_cast<personalization_cursor_context_v1*>(data));
}

void PersonalizationCursorContextPrivate::on_get_size(void *data)
{
    Q_EMIT q_func()->get_size(reinterpret_cast<personalization_cursor_context_v1*>(data));
}

void PersonalizationCursorContextPrivate::on_get_theme(void *data)
{
    Q_EMIT q_func()->get_theme(reinterpret_cast<personalization_cursor_context_v1*>(data));
}

PersonalizationCursorContext::PersonalizationCursorContext(
    personalization_cursor_context_v1 *handle, bool isOwner)
    : QObject(nullptr)
    , QWObject(*new PersonalizationCursorContextPrivate(handle, isOwner, this))
{
}

PersonalizationCursorContext *PersonalizationCursorContext::get(
    personalization_cursor_context_v1 *handle)
{
    return PersonalizationCursorContextPrivate::map.value(handle);
}

PersonalizationCursorContext *PersonalizationCursorContext::from(
    personalization_cursor_context_v1 *handle)
{
    if (auto o = get(handle))
        return o;
    return new PersonalizationCursorContext(handle, false);
}

PersonalizationCursorContext *PersonalizationCursorContext::create(
    personalization_cursor_context_v1 *handle)
{
    if (!handle)
        return nullptr;
    return new PersonalizationCursorContext(handle, true);
}
