// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wmframe.h"

#include <QScreen>

#include <DConfig>

WMFrame::WMFrame(QWidget *parent)
    : FullscreenBackground(parent)
    , m_blurImageInter(new ImageBlur("org.lingmo.ocean.ImageBlur1",
                                     "/org/lingmo/ocean/ImageBlur1",
                                     QDBusConnection::systemBus(), this))
{
    setAccessibleName("WMFrame");
    m_wmchooser = new WMChooser(this);
    m_wmchooser->setAccessibleName("WMChooser");
    setContent(m_wmchooser);

    auto dConfig = Dtk::Core::DConfig::create("org.lingmo.ocean.appearance", "org.lingmo.ocean.appearance", QString(), this);
    if (!dConfig) {
        qWarning() << "Failed to create DConfig: org.lingmo.ocean.appearance";
        return;
    }
    const QStringList list = dConfig->value("Background-Uris").toStringList();
    QString wallpaper = list.first();

    const QUrl url(wallpaper);
    wallpaper = url.isLocalFile() ? url.path() : url.url();

    // blur wallpaper
    const QString &w = m_blurImageInter->Get(wallpaper);

    updateBackground(w.isEmpty() ? wallpaper : w);
}

void WMFrame::setConfigPath(const QString &path)
{
    m_wmchooser->setConfigPath(path);
}
