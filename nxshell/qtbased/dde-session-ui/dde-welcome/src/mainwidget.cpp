// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwidget.h"
#include "updatecontent.h"
#include "utils.h"

#include <QGSettings/QGSettings>
#include <QVariant>
#include <QKeyEvent>
#include <QApplication>

#include "dtkcore_global.h"
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

MainWidget::MainWidget(QWidget *parent)
    : FullscreenBackground(parent)
    , m_blurImageInter(new ImageBlur("org.deepin.dde.ImageBlur1",
                                     "/org/deepin/dde/ImageBlur1",
                                     QDBusConnection::systemBus(), this))
{
    setAccessibleName("MainWidget");
    connect(m_blurImageInter, &ImageBlur::BlurDone, this, &MainWidget::onBlurWallpaperFinished);
    connect(qApp, &QApplication::aboutToQuit, this, [=] {
        DDBusSender()
            .service("org.deepin.dde.Zone1")
            .interface("org.deepin.dde.Zone1")
            .path("/org/deepin/dde/Zone1")
            .method("EnableZoneDetected")
            .arg(true)
            .call();

    });

    QGSettings gsettings("com.deepin.dde.appearance", "", this);
    const QStringList list = gsettings.get("background-uris").toStringList();
    m_wallpaper = list.first();

    const QUrl url(m_wallpaper);
    m_wallpaper = url.isLocalFile() ? url.path() : m_wallpaper;

    UpdateContent *content = new UpdateContent(GetSystemVersion(), this);
    content->setAccessibleName("UpdateContent");
    setContent(content);
    content->hide();

    // blur wallpaper
    const QString &w = m_blurImageInter->Get(m_wallpaper);

    updateBackground(w.isEmpty() ? m_wallpaper : w);

    DDBusSender()
        .service("org.deepin.dde.Zone1")
        .interface("org.deepin.dde.Zone1")
        .path("/org/deepin/dde/Zone1")
        .method("EnableZoneDetected")
        .arg(false)
        .call();
}

void MainWidget::onBlurWallpaperFinished(const QString &source, const QString &blur, bool status)
{
    const QString &sourcePath = QUrl(source).isLocalFile() ? QUrl(source).toLocalFile() : source;

    if (status && m_wallpaper == sourcePath)
        updateBackground(blur);
}

void MainWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
#ifdef  QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
#endif
    case Qt::Key_Enter:
    case Qt::Key_Return:
        qApp->quit();
    default:;
    }

    FullscreenBackground::keyPressEvent(event);
}
