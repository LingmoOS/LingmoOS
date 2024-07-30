/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 Piotr Henryk Dabrowski <phd@phd.re>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "renderersettings.h"

#include <QGuiApplication>
#include <QLibraryInfo>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSurfaceFormat>
#include <QVersionNumber>

/**
 * If QtQuick is configured (QQuickWindow::sceneGraphBackend()) to use the OpenGL backend,
 * check if it is supported or otherwise reconfigure QtQuick to fallback to software mode.
 * This function is called by init().
 *
 * @returns true if the selected backend is supported, false on fallback to software mode.
 */
static bool checkBackend(QOpenGLContext &checkContext)
{
    if (!QQuickWindow::sceneGraphBackend().isEmpty()) {
        return true; // QtQuick is not configured to use the OpenGL backend
    }

    // kwin wayland has it's own QPA, it is unable to create a GL context at this point.
    // KF6 TODO, drop this . The issue will be resolved in future kwin releases.
    QString platformName = qApp->platformName();
    if (platformName == QLatin1String("wayland-org.kde.kwin.qpa")) {
        return true;
    }

#ifdef QT_NO_OPENGL
    bool ok = false;
#else
    bool ok = checkContext.create();
#endif
    return ok;
}

void initializeRendererSessions()
{
    // This is loaded via Q_COREAPP_STARTUP_FUNCTION

    // Due to a quirk this gets called twice, see QTBUG-54479

    // The order of events is:
    // Q*Application constructor starts
    // We load the QPA
    // We load the QPT (The first arguably incorrect invocation triggers)
    // QPA gets initalised
    // QCoreApplication constructor ends
    // Second (correct) invocation
    // it's important that we run after the QPA is initalised'

    static bool firstCall = true;
    if (firstCall) {
        firstCall = false;
        return;
    }

    PlasmaQtQuickSettings::RendererSettings s;
    QOpenGLContext checkContext;

    QSGRendererInterface::GraphicsApi graphicsApi = QSGRendererInterface::Unknown;

    switch (s.sceneGraphBackend()) {
    case PlasmaQtQuickSettings::RendererSettings::software:
        graphicsApi = QSGRendererInterface::Software;
        break;
    case PlasmaQtQuickSettings::RendererSettings::opengl:
        graphicsApi = QSGRendererInterface::OpenGL;
        break;
    default:
        if (!checkBackend(checkContext)) {
            qWarning("Warning: fallback to QtQuick software backend.");
            graphicsApi = QSGRendererInterface::Software;
        }
    }

    if (graphicsApi != QSGRendererInterface::Unknown) {
        QQuickWindow::setSceneGraphBackend(graphicsApi);
    }

    if (!qEnvironmentVariableIsSet("QSG_RENDER_LOOP")) {
        if (s.renderLoop() == PlasmaQtQuickSettings::RendererSettings::basic) {
            qputenv("QSG_RENDER_LOOP", "basic");
        } else if (QGuiApplication::platformName() == QLatin1String("wayland")) {
#if QT_CONFIG(opengl)
            // Workaround for Bug 432062 / QTBUG-95817
            QOffscreenSurface surface;
            surface.create();
            if (checkContext.makeCurrent(&surface)) {
                const char *vendor = reinterpret_cast<const char *>(checkContext.functions()->glGetString(GL_VENDOR));
                if (qstrcmp(vendor, "NVIDIA Corporation") == 0) {
                    // Otherwise Qt Quick Windows break when resized
                    qputenv("QSG_RENDER_LOOP", "basic");
                }
            }
#endif
        }
    }
}

// Because this file gets loaded in the platform plugin, the QGuiApplication already exists
Q_COREAPP_STARTUP_FUNCTION(initializeRendererSessions)
