// Copyright (C) 2023 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "outputmanagement.h"

#include "impl/output_manager_impl.h"

#include <wglobal.h>
#include <woutput.h>
#include <wserver.h>

#include <qwdisplay.h>

#include <QDebug>
#include <QQmlInfo>

extern "C" {
#include <wayland-server-core.h>
}

PrimaryOutputV1::PrimaryOutputV1(QObject *parent)
    : QObject(parent)
{
}

const char *PrimaryOutputV1::primaryOutput()
{
    return m_handle->primary_output_name;
}

bool PrimaryOutputV1::setPrimaryOutput(const char *name)
{
    if (name == nullptr) {
        if (m_outputs.empty()) { // allow null when output list is empty
            m_handle->set_primary_output(nullptr);
            Q_EMIT primaryOutputChanged();
            return true;
        } else {
            return false;
        }
    }
    if (m_handle->primary_output_name != nullptr
        && strcmp(m_handle->primary_output_name, name) == 0)
        return true;
    for (auto *output : m_outputs)
        if (strcmp(output->nativeHandle()->name, name) == 0) {
            m_handle->set_primary_output(output->nativeHandle()->name);
            Q_EMIT primaryOutputChanged();
            return true;
        }
    qmlWarning(this) << QString("Try to set unknown output(%1) as primary output!").arg(name);
    return false;
}

void PrimaryOutputV1::newOutput(WAYLIB_SERVER_NAMESPACE::WOutput *output)
{
    m_outputs.append(output);
    if (m_handle->primary_output_name == nullptr)
        setPrimaryOutput(output->nativeHandle()->name);
}

void PrimaryOutputV1::removeOutput(WAYLIB_SERVER_NAMESPACE::WOutput *output)
{
    m_outputs.removeOne(output);

    if (m_handle->primary_output_name == output->nativeHandle()->name) {
        if (m_outputs.isEmpty()) {
            setPrimaryOutput(nullptr);
        } else {
            setPrimaryOutput(m_outputs.first()->nativeHandle()->name);
        }
    }
}

void PrimaryOutputV1::create(WServer *server)
{
    m_handle = treeland_output_manager_v1::create(server->handle());

    connect(m_handle,
            &treeland_output_manager_v1::requestSetPrimaryOutput,
            this,
            &PrimaryOutputV1::setPrimaryOutput);
}

void PrimaryOutputV1::destroy(WServer *server) { }

wl_global *PrimaryOutputV1::global() const
{
    return m_handle->global;
}
