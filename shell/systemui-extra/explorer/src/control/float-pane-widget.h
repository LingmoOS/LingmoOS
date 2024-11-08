/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FLOATPANEWIDGET_H
#define FLOATPANEWIDGET_H

#include <QWidget>

class QVariantAnimation;

class FloatPaneWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FloatPaneWidget(QWidget *mainWidget, QWidget *floatWidget, QWidget *parent = nullptr);

    void setFloatWidgetWidthHint(int widthHint);
    QSize sizeHint() const override;

public Q_SLOTS:
    void setFloatWidgetVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QWidget *m_mainWidget = nullptr;
    QWidget *m_floatWidget = nullptr;
    QVariantAnimation *m_floatWidgetWidthAnimation = nullptr;

    int m_floatWidgetWidthHint = 200;
    const int FLOAT_WIDGET_MIN_WIDTH = 596/*296*/;
};

#endif // FLOATPANEWIDGET_H
