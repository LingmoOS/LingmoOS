/*
 * Copyright:  2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@cs2c.com.cn.com>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#ifndef FONT_OPTIONS2_H
#define FONT_OPTIONS2_H

#include <QWidget>
#include <QFrame>
#include <QMouseEvent>
#include <QComboBox>
#include <QSpinBox>
#include  <QPushButton>
#include <QHBoxLayout>
#include "src/tools/capturecontext.h"
#include "kpressbutton.h"

using namespace kdk;

class Font_Options2 : public QWidget
{
    Q_OBJECT
public:
    explicit Font_Options2(QWidget *parent = nullptr);
    QVector<QRect> m_colorAreaList;
    QVector<QColor> m_colorList =
    {
        QColor(219, 2, 15),
        QColor(36, 87, 217),
        QColor(85, 209, 57),
        QColor(211, 154, 44),
        QColor(251, 66, 136),
        QColor(25, 25, 25),
        QColor(98, 102, 113),
        QColor(255, 255, 255)
    };
    // 设置小三角的起始位置
    void setStartPos(double startX);
    // 设置小三角的宽和高
    void setTriangleInfo(double width, double height);
    // 设置中间区域
    void setCenterWidget(QWidget *widget = nullptr);
    // void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
protected:
    void paintEvent(QPaintEvent *event);
signals:
    void  colorSelected(QColor color);
    void  font_size_Selete(int i);
    void  font_type_Selete(QFont);
    void  font_bold_change(bool);
    void  font_delete_change(bool);
    void  font_underline_change(bool);
    void  font_italic_change(bool);
public  slots:
    void font_size_change(int thickness);
    void font_bold_selete();
    void font_delete_selete();
    void font_underline_selete();
    void font_italic_selete();

private:
    CaptureContext context;
    // 小三角的起始位置
    double m_startx;
    // 小三角的宽
    double m_triangleWidth;
    // 小三角的高
    double m_triangleHeight;
    int extrasize = 6;
    double Rect_h, Rect_w;
    double radius;
    double Rect_x, Rect_y;
    double Start_x, Start_y;
    QFrame *line1;
    QString styleTheme;
    bool  IsDarkTheme();
    void is_font_bold_selete(bool selected);
    void is_font_delete_selete(bool selected);
    void is_font_underline_selete(bool selected);
    void is_font_italic_selete(bool selected);
public:
    QColor color;
    QRect color_rect;
    QComboBox *Font_type;
    QFont font_type;
    QSpinBox *Font_size;
    int font_size;
    QPushButton *delete_btn, *Underline_btn, *bold_btn, *Italic_btn;
    bool Delete, Underline, bold, italic;
    QWidget *font_widget;   //delete_btn...四个按钮的容器窗口
    QHBoxLayout *font_layout;

};

#endif // FONT_OPTIONS2_H
