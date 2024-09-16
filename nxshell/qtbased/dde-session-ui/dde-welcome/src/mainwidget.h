// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "fullscreenbackground.h"

#include "org_deepin_dde_imageblur1.h"

#include <QWidget>

using ImageBlur = org::deepin::dde::ImageBlur1;

class MainWidget : public FullscreenBackground
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void onBlurWallpaperFinished(const QString &source, const QString &blur, bool status);

private:
    ImageBlur *m_blurImageInter;
    QString m_wallpaper;
};

#endif // MAINWIDGET_H
