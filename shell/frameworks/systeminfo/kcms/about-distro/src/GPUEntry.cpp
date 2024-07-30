/*
    SPDX-FileCopyrightText: 2016 Rohan Garg <rohan@kde.org>
    SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "GPUEntry.h"

#include <QDebug>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <KLocalizedString>

#include "FancyString.h"

GPUEntry::GPUEntry()
    : Entry(ki18n("Graphics Processor:"), QString())
{
    QOpenGLContext context;
    QOffscreenSurface surface;
    surface.create();
    if (!context.create()) {
        qWarning() << "Failed create QOpenGLContext";
        return;
    }

    if (context.makeCurrent(&surface)) {
        m_value = QString::fromUtf8(reinterpret_cast<const char *>(context.functions()->glGetString(GL_RENDERER)));
        m_value = FancyString::fromUgly(m_value);
        // It seems the renderer value may have excess information in parentheses ->
        // strip that. Elide would probably be nicer, a bit meh with QWidgets though.
        m_value = m_value.mid(0, m_value.indexOf('('));
        // Leads to trailing space in my case, don't know whether that is happening
        // everywhere, though. Thus removing trailing spaces separately.
        m_value = m_value.trimmed();
        context.doneCurrent();
    } else {
        qWarning() << "Failed to make QOpenGLContext current";
        return;
    }
}
