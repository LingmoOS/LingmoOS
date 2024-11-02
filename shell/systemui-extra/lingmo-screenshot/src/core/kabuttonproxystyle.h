/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USMPROXYSTYLE_H
#define USMPROXYSTYLE_H

#include <QProxyStyle>
#include <QStyleOption>

class KAButtonStyleOption : public QStyleOptionButton
{
public:
    enum ButtonPosition { OnlyOne = 0x00, Begin = 0x01, Middle = 0x02, End = 0x03 };
    enum ButtonType { BT_NORMAL = 0x00, BT_GROUP = 0x01, BT_TAB = 0x02 };

    ButtonPosition position;
    ButtonType katype;
    KAButtonStyleOption() {}
};

class KAButtonProxyStyle : public QProxyStyle
{
public:
    enum Corner {
        TopLeftCorner = 0x00001,
        TopRightCorner = 0x00002,
        BottomLeftCorner = 0x00004,
        BottomRightCorner = 0x00008
    };
    Q_DECLARE_FLAGS(Corners, Corner)

    KAButtonProxyStyle(QStyle *style = nullptr);
    KAButtonProxyStyle(const QString &key);

    /*!
     * \brief drawControl
     * \param element 比如按钮，对应CE枚举类型
     * \param option
     * \param painter
     * \param widget
     * \details
     * drawControl用于绘制基本控件元素，它本身一般只负责绘制控件的一部分或者一层。
     * 如果你想要知道控件具体如何绘制，你需要同时研究这个控件的源码和QStyle中的源码，
     * 因为它们都有可能改变控件的绘制流程。
     *
     * QStyle一般会遵循QCommonStyle的绘制流程，QCommenStyle是大部分主流style的最基类，
     * 它本身不能完全称之为一个主题，如果你直接使用它，你的控件将不能被正常绘制，因为它有可能只是
     * 在特定的时候执行了特定却未实现的绘制方法，它更像一个框架或者规范。
     */
    virtual void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const;
    virtual void drawItemPixmap(QPainter *painter, const QRect &rectangle, int alignment, const QPixmap &pixmap) const;
    virtual void drawItemText(QPainter *painter, const QRect &rectangle, int alignment, const QPalette &palette, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;

    /*!
     * \brief drawPrimitive
     * \param element 背景绘制，对应PE枚举类型
     * \param option
     * \param painter
     * \param widget
     * \details
     * drawPrimitive用于绘制控件背景，比如按钮和菜单的背景，
     * 我们一般需要判断控件的状态来绘制不同的背景，
     * 比如按钮的hover和点击效果。
     */
    virtual void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const;

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const override;

private:
    void drawRoundedRect(QPainter *pa, const QRect &rect, qreal xRadius, qreal yRadius, Corners corners, Qt::SizeMode mode = Qt::AbsoluteSize) const;
};

#endif // USMPROXYSTYLE_H
