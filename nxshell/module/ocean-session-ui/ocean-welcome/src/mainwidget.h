// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "fullscreenbackground.h"

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
    QString m_wallpaper;
};

#endif // MAINWIDGET_H
