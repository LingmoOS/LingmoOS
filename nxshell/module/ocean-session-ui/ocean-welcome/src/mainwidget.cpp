// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwidget.h"
#include "updatecontent.h"
#include "utils.h"
#include "org_lingmo_ocean_imageblur1.h"

#include <QVariant>
#include <QKeyEvent>
#include <QApplication>

#include <DConfig>

#include "dtkcore_global.h"
#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
#include <DDBusSender>
#else
#include <QProcess>
#endif

using ImageBlur = org::lingmo::ocean::ImageBlur1;

MainWidget::MainWidget(QWidget *parent)
    : FullscreenBackground(parent)
{
    auto blurImageInter = new ImageBlur("org.lingmo.ocean.ImageBlur1",
                                        "/org/lingmo/ocean/ImageBlur1",
                                        QDBusConnection::systemBus(), this);
    setAccessibleName("MainWidget");
    connect(blurImageInter, &ImageBlur::BlurDone, this, &MainWidget::onBlurWallpaperFinished);
    connect(qApp, &QApplication::aboutToQuit, this, [=] {
        DDBusSender()
            .service("org.lingmo.ocean.Zone1")
            .interface("org.lingmo.ocean.Zone1")
            .path("/org/lingmo/ocean/Zone1")
            .method("EnableZoneDetected")
            .arg(true)
            .call();

    });

    auto dConfig = Dtk::Core::DConfig::create("org.lingmo.ocean.appearance", "org.lingmo.ocean.appearance", QString(), this);
    if (!dConfig) {
        qWarning() << "Failed to create org.lingmo.ocean.appearance DConfig";
        return;
    }

    const QStringList list = dConfig->value("Background-Uris").toStringList();
    m_wallpaper = list.first();

    const QUrl url(m_wallpaper);
    m_wallpaper = url.isLocalFile() ? url.path() : m_wallpaper;

    UpdateContent *content = new UpdateContent(GetSystemVersion(), this);
    content->setAccessibleName("UpdateContent");
    setContent(content);
    content->hide();

    // blur wallpaper
    const QString &w = blurImageInter->Get(m_wallpaper);

    updateBackground(w.isEmpty() ? m_wallpaper : w);

    DDBusSender()
        .service("org.lingmo.ocean.Zone1")
        .interface("org.lingmo.ocean.Zone1")
        .path("/org/lingmo/ocean/Zone1")
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
