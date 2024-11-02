/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
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
 */
#ifndef FONTSIZE_COLOR_CHOSE2_H
#define FONTSIZE_COLOR_CHOSE2_H

#include <QWidget>
#include <QFrame>
#include "src/tools/capturecontext.h"
class FontSize_Color_Chose2 : public QWidget
{
    Q_OBJECT
public:
        explicit FontSize_Color_Chose2(QWidget *parent = nullptr);
        QVector<QRect> m_colorAreaList;
        QVector<QColor> m_colorList =
        {
            QColor(219,2,15),
            QColor(36,87,217),
            QColor(85,209,57),
            QColor(211,154,44),
            QColor(251,66,136),
            QColor(25,25,25),
            QColor(98,102,113),
            QColor(255,255,255)
        };
        //设置小三角的起始位置
        void setStartPos(double startX);
        //设置小三角的宽和高
        void setTriangleInfo(double width,double height );
       //设置中间区域
        void setCenterWidget(QWidget* widget = nullptr);
        //void mouseMoveEvent(QMouseEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
protected:
        void paintEvent(QPaintEvent *event) ;
signals:
        void  colorSelected2(QColor color);
        void  font_size_change2(int thickness);
private:
        CaptureContext context;
        //小三角的起始位置
        double m_startx;
        //小三角的宽
        double m_triangleWidth;
        //小三角的高
        double m_triangleHeight;
        int  extrasize = 6;
        double Rect_h,Rect_w;
        double radius;
        double Rect_x,Rect_y;
        double Start_x, Start_y;
        QFrame *line1;
public:
        QColor color;
        QRect color_rect;
};

#endif // FONTSIZE_COLOR_CHOSE2_H
