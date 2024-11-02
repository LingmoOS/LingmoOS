/*
 *
 * Copyright: 2010-2011 Razor team
 *            2020 KylinSoft Co., Ltd.
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
#include "font_options.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QAbstractItemView>
#include <QFontDatabase>
#include <QDebug>
#include <QVector4D>

#define SHADOW_WIDTH  5     // 小三角的阴影宽度
#define TRIANGLE_WIDTH 16    // 小三角的宽度
#define TRIANGLE_HEIGHT 8    // 小三角的高度
//#define BORDER_RADIUS 6     // 窗口边角弧度
Font_Options::Font_Options(QWidget *parent) :
    QWidget(parent),
    m_startx(12),
    m_triangleWidth(TRIANGLE_WIDTH),
    m_triangleHeight(TRIANGLE_HEIGHT),
    radius(6),
    Delete(false),
    Underline(false),
    bold(false),
    italic(false)
{
    setCursor(Qt::ArrowCursor);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setProperty("useCustomShadow", true); // 启用协议
    setProperty("customShadowDarkness", 1.0); // 阴影暗度
    setProperty("customShadowWidth", 30); // 阴影边距大小
    setProperty("customShadowRadius", QVector4D(1, 1, 1, 1)); // 阴影圆角，必须大于0，这个值应该和frameless窗口本身绘制的形状吻合
    setProperty("customShadowMargins", QVector4D(30, 30, 30, 30)); // 阴影与窗口边缘的距离，一般和customShadowWidth保持一致
    this->setMouseTracking(true);
    Start_x = 231;
    Start_y = 32;
    for (int i = 0; i < 4; i++) {
        m_colorAreaList.append(QRect(Start_x, Start_y, 14, 14));
        Start_x += 24;
    }
    Start_x = 231;
    Start_y = 53;
    for (int i = 4; i < 8; i++) {
        m_colorAreaList.append(QRect(Start_x, Start_y, 14, 14));
        Start_x += 24;
    }
    context.style_settings = new QGSettings("org.lingmo.style");
    context.style_name = context.style_settings->get("style-name").toString();
}

void Font_Options::setStartPos(double startX)
{
    m_startx = startX;
}

void Font_Options::setTriangleInfo(double width, double height)
{
    m_triangleWidth = width;
    m_triangleHeight = height;
}

void Font_Options::setCenterWidget(QWidget *widget)
{
    QFont f;
    f.setPixelSize(10);
    Font_type = new  QComboBox(this);
    QFontDatabase database;
    foreach(const QString &family, database.families())
    {
        Font_type->addItem(family);
    }
    Font_type->setFont(f);
    Font_type->view()->setFixedWidth(240);
    Font_type->setStyleSheet(
        "QComboBox QAbstractItemView::item { min-height: 40px; min-width: 60px; }");
    Font_type->setGeometry(13, 21, 134, 26);
    Font_size = new  QSpinBox(this);
    Font_size->setMinimum(6);
    Font_size->setStyleSheet("font:10px;");
    Font_size->setValue(2);
    Font_size->setGeometry(153, 21, 56, 26);

    font_widget = new QWidget(this);

    delete_btn = new QPushButton(font_widget);
    delete_btn->setCheckable(true);
    delete_btn->setChecked(Delete);
    delete_btn->setToolTip(tr("StrikeOut"));
    delete_btn->setFixedSize(48, 22);

    Underline_btn = new QPushButton(font_widget);
    Underline_btn->setCheckable(true);
    Underline_btn->setChecked(Underline);
    Underline_btn->setToolTip(tr("Underline"));
    Underline_btn->setFixedSize(48, 22);

    bold_btn = new QPushButton(font_widget);
    bold_btn->setCheckable(true);
    bold_btn->setChecked(bold);
    bold_btn->setToolTip(tr("Bold"));
    bold_btn->setFixedSize(48, 22);

    Italic_btn = new QPushButton(font_widget);
    Italic_btn->setToolTip(tr("Italic"));
    Italic_btn->setIconSize(QSize(12, 12));
    Italic_btn->setCheckable(true);
    Italic_btn->setChecked(italic);
    Italic_btn->setFixedSize(48, 22);

    bold_btn->setIconSize(QSize(12, 12));
    Underline_btn->setIconSize(QSize(12, 12));
    delete_btn->setIconSize(QSize(12, 12));
    styleTheme = "QPushButton{border-image:url(%1%2)}";

    if(IsDarkTheme()){
        font_widget->setStyleSheet( "QWidget{border:none;border-radius:4px;}"
                                    "QPushButton{ border:none;border-radius:0px;"
                                    "background-color: rgba(50, 50, 50, 1);}"
                                    "QPushButton:checked {border:none; border-radius: 0px;"
                                    "background-color: rgba(0, 98, 240, 1);}");
    }else{
        font_widget->setStyleSheet( "QWidget{border:none;border-radius:4px;}"
                                    "QPushButton{ border:none;border-radius:0px;"
                                    "background-color: rgba(255, 255, 255, 0.57);}"
                                    "QPushButton:checked {border:none; border-radius: 0px;"
                                    "background-color: rgba(0, 98, 240, 1);}");
    }
    // 设置圆角
    Italic_btn->setStyleSheet(QStringLiteral(
                                  "QPushButton{"
                                  "    border-top-right-radius: 4px;"
                                  "    border-bottom-right-radius: 4px;"
                                  "}"
                                  "QPushButton:checked {"
                                  "    border-top-right-radius: 4px;"
                                  "    border-bottom-right-radius: 4px;"
                                  "}"
                                  ));
    delete_btn->setStyleSheet(QStringLiteral(
                                  "QPushButton{"
                                  "    border-top-left-radius: 4px;"
                                  "    border-bottom-left-radius: 4px;"
                                  "}"
                                  "QPushButton:checked {"
                                  "    border-top-left-radius: 4px;"
                                  "    border-bottom-left-radius: 4px;"
                                  "}"
                                  ));

    font_widget->setGeometry(Font_type->x(), Font_type->y()+Font_type->height()+6, delete_btn->width()*4+3, delete_btn->height());
    font_layout = new QHBoxLayout(font_widget);
    font_layout->addWidget(delete_btn, 0, Qt::AlignCenter);
    font_layout->addWidget(Underline_btn, 0, Qt::AlignCenter);
    font_layout->addWidget(bold_btn, 0, Qt::AlignCenter);
    font_layout->addWidget(Italic_btn, 0, Qt::AlignCenter);
    font_layout->setSpacing(1);
    font_layout->setContentsMargins(0,0,0,0);

    line1 = new QFrame(this);
    line1->setFrameShape(QFrame::VLine);
    line1->setFixedSize(1, 16);
    line1->move(font_widget->x()+font_widget->width()+10,font_widget->y()-12);
    line1->setStyleSheet("background-color: black;");

    connect(Font_size, SIGNAL(valueChanged(int)),
            this, SLOT(font_size_change(int)));
    connect(Font_type, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index)
    {
        emit font_type_Selete(QFont(Font_type->currentText()));
    });
    connect(delete_btn, &QPushButton::clicked,
            this, &Font_Options::font_delete_selete);
    connect(Underline_btn, &QPushButton::clicked,
            this, &Font_Options::font_underline_selete);
    connect(Italic_btn, &QPushButton::clicked,
            this, &Font_Options::font_italic_selete);
    connect(bold_btn, &QPushButton::clicked,
            this, &Font_Options::font_bold_selete);
}

void Font_Options::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    QPolygon trianglePolygon;
    trianglePolygon << QPoint(m_startx, m_triangleHeight + SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startx+m_triangleWidth/2, SHADOW_WIDTH);
    trianglePolygon << QPoint(m_startx + m_triangleWidth, m_triangleHeight + SHADOW_WIDTH);
    QPainterPath drawPath;
    int borderRadius =  "classical" == context.style_settings->get("widgetThemeName").toString() ? 0 : 6;
    drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH,
                                  width()-SHADOW_WIDTH *2,
                                  height() -SHADOW_WIDTH *2 -m_triangleHeight),
                            borderRadius, borderRadius, Qt::AbsoluteSize);
    drawPath.addPolygon(trianglePolygon);
    if (IsDarkTheme()) {
        painter.setBrush(QColor(25, 25, 25, 180));
        painter.drawPath(drawPath);
    } else {
        painter.setBrush(QColor(225, 225, 225, 180));
        painter.drawPath(drawPath);
    }

    for (int i = 0; i < 8; i++) {
        QRect rect = m_colorAreaList.at(i);
        QColor color1 = m_colorList.at(i);
        if (color == color1) {
            Rect_h = rect.height()+4;
            Rect_w = rect.width() +4;
            painter.setBrush(QColor(255, 255, 255));
            painter.drawRoundedRect(QRect(rect.x()-2,
                                          rect.y()-2, Rect_h, Rect_w), 2, 2, Qt::AbsoluteSize);
        }
        painter.setBrush(color1);
        painter.drawRoundedRect(m_colorAreaList.at(i), 2, 2, Qt::AbsoluteSize);
    }
    is_font_bold_selete(bold);
    is_font_underline_selete(Underline);
    is_font_delete_selete(Delete);
    is_font_italic_selete(italic);
}

void Font_Options::mousePressEvent(QMouseEvent *e)
{
    for (int i = 0; i < 8; i++) {
        if (m_colorAreaList.at(i).contains(e->pos())) {
            color = m_colorList.at(i);
            emit colorSelected(color);
            update();
            break;
        }
    }
}

void Font_Options::font_size_change(int i)
{
    qDebug()<<"text font thickness changed";
    emit font_size_Selete(i);
}

void Font_Options::font_bold_selete()
{
    bold = !bold;
    emit font_bold_change(bold);
    is_font_bold_selete(bold);
}

void Font_Options::font_delete_selete()
{
    Delete = !Delete;
    emit font_delete_change(Delete);
    is_font_delete_selete(Delete);
}

void Font_Options::font_underline_selete()
{
    Underline = !Underline;
    emit font_underline_change(Underline);
    is_font_underline_selete(Underline);
}

void Font_Options::font_italic_selete()
{
    italic = !italic;
    emit font_italic_change(italic);
    is_font_italic_selete(italic);
}

void Font_Options::is_font_bold_selete(bool selected)
{
    // bold_btn->setPosition(KAButtonStyleOption::Middle);
    if(selected){
        bold_btn->setIcon(QIcon(":/img/material/font/black_bold.png"));
    }else{
        if (IsDarkTheme()) {
            bold_btn->setIcon(QIcon(":/img/material/font/black_bold.png"));
        } else {
            bold_btn->setIcon(QIcon(":/img/material/font/bold.svg"));
        }
    }
}

void Font_Options::is_font_delete_selete(bool selected)
{
    // delete_btn->setPosition(KAButtonStyleOption::Begin);
    if(selected){
        delete_btn->setIcon(QIcon(":/img/material/font/black_delete.png"));
    }else{
        if (IsDarkTheme()) {
            delete_btn->setIcon(QIcon(":/img/material/font/black_delete.png"));
        } else {
            delete_btn->setIcon(QIcon(":/img/material/font/delete.svg"));
        }
    }
}

void Font_Options::is_font_underline_selete(bool selected)
{
    // Underline_btn->setPosition(KAButtonStyleOption::Middle);
    if(selected){
        Underline_btn->setIcon(QIcon(":/img/material/font/black_underline.png"));
    }else{
        if (IsDarkTheme()) {
            Underline_btn->setIcon(QIcon(":/img/material/font/black_underline.png"));
        } else {
            Underline_btn->setIcon(QIcon(":/img/material/font/underline.svg"));
        }
    }
}

void Font_Options::is_font_italic_selete(bool selected)
{
    // Italic_btn->setPosition(KAButtonStyleOption::End);
    if(selected){
        Italic_btn->setIcon(QIcon(":/img/material/font/black_italic.png"));
    }else{
        if (IsDarkTheme()) {
            Italic_btn->setIcon(QIcon(":/img/material/font/black_italic.png"));
        } else {
            Italic_btn->setIcon(QIcon(":/img/material/font/italic.svg"));
        }
    }
}

bool Font_Options::IsDarkTheme()
{
    bool isDrak;
    context.style_settings = new QGSettings("org.lingmo.style");
    context.style_name = context.style_settings->get("style-name").toString();
    if ((context.style_name.compare("lingmo-dark") == 0)
        || (context.style_name.compare("lingmo-black") == 0)) {
        isDrak = true;
    } else {
        isDrak = false;
    }
    return isDrak;
}
