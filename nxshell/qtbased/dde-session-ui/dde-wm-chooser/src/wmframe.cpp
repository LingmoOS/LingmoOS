// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wmframe.h"

#include <QScreen>
#include <QGSettings>

WMFrame::WMFrame(QWidget *parent)
    : FullscreenBackground(parent)
    , m_blurImageInter(new ImageBlur("org.deepin.dde.ImageBlur1",
                                     "/org/deepin/dde/ImageBlur1",
                                     QDBusConnection::systemBus(), this))
{
    setAccessibleName("WMFrame");
    m_wmchooser = new WMChooser(this);
    m_wmchooser->setAccessibleName("WMChooser");
    setContent(m_wmchooser);

    QGSettings gsettings("com.deepin.dde.appearance", "", this);
    const QStringList list = gsettings.get("background-uris").toStringList();
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
