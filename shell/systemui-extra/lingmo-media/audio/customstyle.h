/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef CUSTOMSTYLE_H
#define CUSTOMSTYLE_H

#include <QProxyStyle>
#include <QDebug>
#include "ukmedia_slider_tip_label_helper.h"
/*!
 * \brief The CustomStyle class
 * \details
 * 自定义QStyle
 * 基于QProxyStyle，默认使用QProxyStyle的实例绘制控件，你需要针对某一个控件重新实现若干对应的接口。
 * QProxyStyle可以从现有的qt style实例化，我们只需要知道这个style的名字即可。
 * 这种做法带来了不错的扩展性和自由度，因为我们不需要将某个style的代码直接引入我们的项目中，
 * 也能够“继承”这个style类进行二次开发。
 *
 * 下面的方法展现了QStyle的所有的接口，使用QStyle进行控件的绘制使得qt应用能够进行风格的切换，
 * 从而达到不修改项目源码却对应用外观产生巨大影响的效果。
 *
 * \note
 * 需要注意QStyle与QSS并不兼容，因为QSS本身其实上也是QStyle的一种实现，对一个控件而言，本身理论上只能
 * 在同一时间调用唯一一个QStyle进行绘制。
 */
class CustomStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit CustomStyle(const QString &proxyStyleName = "windows", QObject *parent = nullptr);
    ~CustomStyle();
    /*!
     * \brief drawComplexControl
     * \param control 比如ScrollBar，对应CC枚举类型
     * \param option
     * \param painter
     * \param widget
     * \details
     * drawComplexControl用于绘制具有子控件的复杂控件，它本身一般不直接绘制控件，
     * 而是通过QStyle的其它方法将复杂控件分解成子控件再调用其它的draw方法绘制。
     * 如果你需要重新实现一个复杂控件的绘制方法，首先考虑的应该是在不改变它原有的绘制流程的情况下，
     * 对它调用到的其它方法进行重写。
     *
     * 如果你不想使用原有的绘制流程，那你需要重写这个接口，然后自己实现一切，
     * 包括背景的绘制，子控件的位置和状态计算，子控件的绘制等。
     * 所以，你需要对这个控件有足够的了解之后再尝试直接重写这个接口。
     */
    virtual void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const;
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
    virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const;
    virtual QStyle::SubControl hitTestComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, const QPoint &position, const QWidget *widget = nullptr) const;
    virtual QRect itemPixmapRect(const QRect &rectangle, int alignment, const QPixmap &pixmap) const;
    virtual QRect itemTextRect(const QFontMetrics &metrics, const QRect &rectangle, int alignment, bool enabled, const QString &text) const;
    //virtual int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation, const QStyleOption *option, const QWidget *widget);
    virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const;

    /*!
     * \brief polish
     * \param widget
     * \details
     * polish用于对widget进行预处理，一般我们可以在polish中修改其属性，
     * 另外，polish是对动画和特效实现而言十分重要的一个方法，
     * 通过polish我们能够使widget和特效和动画形成对应关系。
     */
    virtual void polish(QWidget *widget);
    virtual void polish(QApplication *application);
    virtual void polish(QPalette &palette);
    virtual void unpolish(QWidget *widget);
    virtual void unpolish(QApplication *application);

    virtual QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget = nullptr) const;
    virtual QIcon standardIcon(QStyle::StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const;
    virtual QPalette standardPalette() const;
    /*!
     * \brief styleHint
     * \param hint 对应的枚举是SH
     * \param option
     * \param widget
     * \param returnData
     * \return
     * \details
     * styleHint比较特殊，通过它我们能够改变一些控件的绘制流程或者方式，比如说QMenu是否可以滚动。
     */
    virtual int	styleHint(QStyle::StyleHint hint, const QStyleOption *option = nullptr, const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const;
    /*!
     * \brief subControlRect
     * \param control
     * \param option
     * \param subControl
     * \param widget
     * \return
     * \details
     * subControlRect返回子控件的位置和大小信息，这个方法一般在内置流程中调用，
     * 如果我们要重写某个绘制方法，可能需要用到它
     */
    virtual QRect subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget = nullptr) const;
    /*!
     * \brief subElementRect
     * \param element
     * \param option
     * \param widget
     * \return
     * \details
     * 与subControlRect类似
     */
    virtual QRect subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget = nullptr) const;

Q_SIGNALS:

public Q_SLOTS:
private:
    SliderTipLabelHelper *m_helpTip;
};

#endif // CUSTOMSTYLE_H
