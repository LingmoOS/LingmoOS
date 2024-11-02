/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILELABELBOX_H
#define FILELABELBOX_H

#include <QListView>
#include <QProxyStyle>
#include <explor-core_global.h>
#include <QPropertyAnimation>

class PEONYCORESHARED_EXPORT FileLabelBox : public QListView
{
    Q_OBJECT
public:
    explicit FileLabelBox(QWidget *parent = nullptr);

    QSize sizeHint() const;

    int getTotalDefaultColor() {
        return TOTAL_DEFAULT_COLOR;
    }

    const int TOTAL_DEFAULT_COLOR = 7;
    void setFloatWidgetVisible(bool visible);

Q_SIGNALS:
    void leftClickOnBlank();
    void fileLabelVisible(bool checked);

protected:
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);

private:
    QPropertyAnimation *m_labelHeightAnimation = nullptr;
    bool m_isShow = false;
};

class LabelBoxStyle : public QProxyStyle
{
    static LabelBoxStyle *getStyle();

    friend class FileLabelBox;
    LabelBoxStyle() {}

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;
};

#endif // FILELABELBOX_H
