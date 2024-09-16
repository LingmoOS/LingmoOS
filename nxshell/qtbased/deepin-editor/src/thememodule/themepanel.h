// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THEMEPANEL_H
#define THEMEPANEL_H

#include <QWidget>
#include <QPainterPath>
#include "themelistview.h"
#include "themelistmodel.h"
#include "themeitemdelegate.h"

class Window;
class ThemePanel : public QWidget
{
    Q_OBJECT

public:
    ThemePanel(QWidget *parent = nullptr);
    ~ThemePanel();

    void setBackground(const QString &color);
    void popup();
    void hide();

    void setFrameColor(const QString &selectedColor, const QString &normalColor);
    void setSelectionTheme(const QString &path);

signals:
    void themeChanged(const QString &path);

protected:
    void paintEvent(QPaintEvent *);

private:
    ThemeListView *m_themeView;
    ThemeListModel *m_themeModel;
    Window *m_window;

    QColor m_frameLightColor = QColor("#000000");
    QColor m_frameDarkColor = QColor("#FFFFFF");
    QColor m_backgroundColor;
    QColor m_frameColor;
};

#endif
