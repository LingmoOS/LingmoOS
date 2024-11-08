/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef LINGMOCONFIGSTYLE_H
#define LINGMOCONFIGSTYLE_H

#include <QProxyStyle>
#include <QFontMetrics>
#include <QStyleOption>
#include <private/qtextengine_p.h>
#include <qmath.h>
#include <QGSettings>
#include "widget-parameters/control-parameters.h"


class QStyleOptionViewItem;
class QDBusInterface;

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
#include <private/qfusionstyle_p.h>
#define Style QFusionStyle
#else
#define Style QProxyStyle
#endif


class BlurHelper;
class GestureHelper;
class WindowManager;

//class ApplicationStyleSettings;
//QT_BEGIN_NAMESPACE
//namespace LINGMOConfigStyleSpace { class LINGMOConfigStyle; }
//QT_END_NAMESPACE
/*!
 * \brief The LINGMOConfigStyle class
 * \details
 * This class provide the default lingmo style in LINGMO desktop enviroment.
 */
namespace LINGMOConfigStyleSpace {
class ConfigTabWidgetAnimationHelper;
class ConfigScrollBarAnimationHelper;
class ConfigButtonAnimationHelper;
class ConfigBoxAnimationHelper;
class ConfigProgressBarAnimationHelper;
class ConfigShadowHelper;
class ConfigRadioButtonAnimationHelper;
class ConfigCheckBoxAnimationHelper;
class ConfigSliderAnimationHelper;
class ConfigTreeAnimationHelper;
class LINGMOConfigStyleParameters;
}
class LINGMOConfigStyle : public Style
{
    Q_OBJECT
public:
    explicit LINGMOConfigStyle(QString name);
    ~LINGMOConfigStyle();

    bool shouldBeTransparent(const QWidget *w) const;

    //debuger
    bool eventFilter(QObject *obj, QEvent *e);

    int styleHint(StyleHint hint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const;

    void polish(QWidget *widget);
    void unpolish(QWidget *widget);
    void polish(QPalette& palette) ;
    void polish(QApplication *app);


    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
                           const QWidget *widget = nullptr) const override;

    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;

    QPixmap testGeneratedSVGPixmap(QPixmap pixmap) const;

    QPalette standardPalette() const;

    void drawPrimitive(QStyle::PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget = nullptr) const;

    void drawComplexControl(QStyle::ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const;

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    int pixelMetric(QStyle::PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const;

    QRect subControlRect(QStyle::ComplexControl control,
                         const QStyleOptionComplex *option,
                         QStyle::SubControl subControl,
                         const QWidget *widget = nullptr) const;

    void drawItemPixmap(QPainter *painter,
                        const QRect &rect,
                        int alignment,
                        const QPixmap &pixmap) const;

    QRect subElementRect(SubElement element,
                                 const QStyleOption *option,
                                 const QWidget *widget = nullptr) const;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const;


    QPixmap drawColorPixmap(QPainter *p, QColor color, QPixmap &pixmap) const;

protected:

    void realSetWindowSurfaceFormatAlpha(const QWidget *widget) const;
    void realSetMenuTypeToMenu(const QWidget *widget) const;
    QRect centerRect(const QRect &rect, int width, int height) const;

private Q_SLOTS:
    void updateTabletModeValue(bool isTabletMode);
    QRect scrollbarSliderRec(QWidget *w);
    void setScrollbarSliderRec(QWidget *w, QRect rect) const;
    QRect sliderHandleRec(QWidget *w);
    void setSliderHandleRec(QWidget *w,QRect rect) const;

private:
    bool isUseDarkPalette() const;

    QColor button_Click(const QStyleOption *option) const;
    QColor button_Hover(const QStyleOption *option) const;
    QColor transparentButton_Click(const QStyleOption *option) const;
    QColor transparentButton_Hover(const QStyleOption *option) const;
    QColor highLight_Click(const QStyleOption *option) const;
    QColor highLight_Hover(const QStyleOption *option) const;

    void setThemeColor(QColor color, QPalette &palette) const;

    // view
    QString calculateElidedText(const QString &text, const QTextOption &textOption,
                                    const QFont &font, const QRect &textRect, const Qt::Alignment valign,
                                    Qt::TextElideMode textElideMode, int flags,
                                    bool lastVisibleLineShouldBeElided, QPointF *paintStartPosition) const;
    void viewItemDrawText(QPainter *painter, const QStyleOptionViewItem *option, const QRect &rect) const;
    void viewItemLayout(const QStyleOptionViewItem *option,  QRect *checkRect, QRect *pixmapRect, QRect *textRect, bool sizehint) const;
    QSize viewItemSize(const QStyleOptionViewItem *option, int role) const;

    void getDefaultControlParameters(QApplication *app);

private:
    LINGMOConfigStyleSpace::ConfigTabWidgetAnimationHelper *m_tab_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigScrollBarAnimationHelper *m_scrollbar_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigButtonAnimationHelper*  m_button_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigBoxAnimationHelper* m_combobox_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigShadowHelper *m_shadow_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigProgressBarAnimationHelper *m_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigRadioButtonAnimationHelper *m_radiobutton_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigCheckBoxAnimationHelper *m_checkbox_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigSliderAnimationHelper *m_slider_animation_helper = nullptr;
    LINGMOConfigStyleSpace::ConfigTreeAnimationHelper *m_tree_animation_helper = nullptr;

    bool m_drak_palette = false;
    bool m_default_palette = false;
    QString m_widget_theme = "lingmo-default";
    QString m_style_name = "lingmo-default";

    bool m_is_tablet_mode = false;
    QDBusInterface *m_statusManagerDBus = nullptr;

    // LINGMOConfigStyleParameters
    LINGMOConfigStyleSpace::LINGMOConfigStyleParameters *sp = nullptr;

    mutable QHash<QWidget*, QRect> m_scrollBarSliderRec;
    mutable QHash<QWidget*, QRect>  m_sliderHandleRec;

    BlurHelper *m_blur_helper = nullptr;
    GestureHelper *m_gesture_helper = nullptr;
    WindowManager *m_window_manager = nullptr;

//    ApplicationStyleSettings *m_app_style_settings = nullptr;

    bool m_blink_cursor = true;
    int m_blink_cursor_time = 1200;
};

#endif // LINGMOCONFIGSTYLE_H
