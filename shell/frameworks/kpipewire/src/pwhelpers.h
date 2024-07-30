/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "pipewiresourcestream.h"
#include <QByteArray>
#include <epoxy/egl.h>
#include <kpipewire_export.h>

typedef unsigned int GLenum;

/**
 * The @class PipeWireFrameCleanupFunction is used to track the lifetime of a pipewire frame.
 *
 * It is used to have a ref-counted class that will call the cleanup function when
 * it's left with no references.
 * This is useful so that it can be passed to QImage() if necessary without having to
 * track if the QImage itself outlives the buffer.
 */
class PipeWireFrameCleanupFunction
{
    Q_DISABLE_COPY(PipeWireFrameCleanupFunction)
public:
    PipeWireFrameCleanupFunction(std::function<void()> cleanup)
        : m_ref(0)
        , m_cleanup(cleanup)
    {
    }

    void ref()
    {
        m_ref++;
    }
    static void unref(void *x)
    {
        if (!x) {
            return;
        }
        auto self = static_cast<PipeWireFrameCleanupFunction *>(x);
        self->m_ref--;
        if (self->m_ref == 0) {
            self->m_cleanup();
            delete self;
        }
    }

private:
    QAtomicInt m_ref;
    std::function<void()> m_cleanup;
};

namespace PWHelpers
{

KPIPEWIRE_EXPORT QImage
SpaBufferToQImage(const uchar *data, int width, int height, qsizetype bytesPerLine, spa_video_format format, PipeWireFrameCleanupFunction *cleanup);
}
