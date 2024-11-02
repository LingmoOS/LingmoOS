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

#include "lingmo-config-style.h"

#include "config-style-helper.h"

#include "settings/lingmo-style-settings.h"
#include "animations/tabwidget/lingmo-tabwidget-default-slide-animator.h"

#include <QSharedPointer>
#include <QStyleOption>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>

#include <QPaintDevice>

#include "animations/config-tab-widget-animation-helper.h"
#include "animations/config-scrollbar-animation-helper.h"
#include "animations/config-button-animation-helper.h"
#include "animations/config-button-animator.h"
#include "animations/config-box-animation-helper.h"
#include "animations/animator-iface.h"
#include "animations/animation-helper.h"
#include "animations/config-progressbar-animation-helper.h"
#include "animations/config-progressbar-animation.h"
#include "animations/config-radiobutton-animation-helper.h"
#include "animations/config-checkbox-animation-helper.h"
#include "animations/config-slider-animation-helper.h"
#include "animations/config-tree-animation-helper.h"
#include "config-shadow-helper.h"

#include "../blur-helper.h"
#include "../gesture-helper.h"
#include "../window-manager.h"
#include "../platformthemedebug.h"
#include "settings/application-style-settings.h"
#include "settings/lingmo-style-settings.h"
#include "effects/highlight-effect.h"

#include "lingmo-config-style-parameters.h"
#include "settings/black-list.h"


#include <QIcon>
#include <QStyleOptionViewItem>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTreeView>
#include <QListView>
#include <QMenu>
#include <QToolButton>
#include <QtPlatformHeaders/QXcbWindowFunctions>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QEvent>
#include <QDebug>
#include <QPixmapCache>
#include <QStyleOptionButton>
#include <QStyleOptionMenuItem>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QApplication>
#include <QToolTip>
#include <QRadioButton>
#include <QCheckBox>
#include <KWindowEffects>
#include <KWindowSystem>
#include <QtConcurrent/QtConcurrent>

#include <private/qlineedit_p.h>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <qdrawutil.h>
#include <QWizardPage>
#include <QStyleHints>
#include <QLibrary>
#include <QProxyStyle>
#include <QDialogButtonBox>

using namespace LINGMOConfigStyleSpace;

const char* GESTURE_LIB = "libqt5-gesture-extensions.so.1";

#define DBUS_STATUS_MANAGER_IF "com.lingmo.statusmanager.interface"

#define COMMERCIAL_VERSION true
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

//static QWindow *qt_getWindow(const QWidget *widget)
//{
//    return widget ? widget->window()->windowHandle() : 0;
//}


#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
LINGMOConfigStyle::LINGMOConfigStyle(QString name) : QFusionStyle()
#else
LINGMOConfigStyle::LINGMOConfigStyle(bool dark, bool useDefault, QString type) : QProxyStyle("fusion")
#endif
{
    auto settings = LINGMOStyleSettings::globalInstance();
    if (settings) {
        m_blink_cursor = settings->get("cursorBlink").toBool();
        m_blink_cursor_time = settings->get("cursorBlinkTime").toInt();
        qApp->styleHints()->setCursorFlashTime(m_blink_cursor_time);

        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "cursorBlink") {
                m_blink_cursor = settings->get("cursorBlink").toBool();
                if (qApp->activeWindow()) {
                    qApp->activeWindow()->update();
                }
                if (qApp->activeModalWidget()) {
                    qApp->activeModalWidget()->update();
                }
                if (qApp->activePopupWidget()) {
                    qApp->activePopupWidget()->update();
                }
            }
            if (key == "cursorBlinkTime") {
                m_blink_cursor_time = settings->get("cursorBlinkTime").toInt();
                qApp->styleHints()->setCursorFlashTime(m_blink_cursor_time);
            }
        });
    } else {
        qWarning("org.lingmo.style is null!");
    }

    m_blur_helper = new BlurHelper(this);

//    m_gesture_helper = new GestureHelper(this);

    m_window_manager = new WindowManager(this);

//    m_app_style_settings = ApplicationStyleSettings::getInstance();
//    if (m_app_style_settings) {
//        connect(m_app_style_settings, &ApplicationStyleSettings::colorStretageChanged, [=](const ApplicationStyleSettings::ColorStretagy &stretagy) {
//          ////todo implemet palette switch.
//            switch (stretagy) {
//            case ApplicationStyleSettings::System: {
//                break;
//            }
//            case ApplicationStyleSettings::Bright: {
//                break;
//            }
//            case ApplicationStyleSettings::Dark: {
//                break;
//            }
//            default:
//                break;
//            }
//        });
//    } else {
//        qWarning("org.lingmo.style is null!");
//    }

    if (QGSettings::isSchemaInstalled("org.lingmo.peripherals-mouse")) {
        QGSettings *settings = new QGSettings("org.lingmo.peripherals-mouse");
        if (settings->keys().contains("doubleClick")) {
            int mouse_double_click_time = settings->get("doubleClick").toInt();
            if (mouse_double_click_time != qApp->doubleClickInterval()) {
                qApp->setDoubleClickInterval(mouse_double_click_time);
            }
            connect(settings, &QGSettings::changed, qApp, [=] (const QString &key) {
                if (key == "doubleClick") {
                    int mouse_double_click_time = settings->get("doubleClick").toInt();
                    if (mouse_double_click_time != qApp->doubleClickInterval()) {
                        qApp->setDoubleClickInterval(mouse_double_click_time);
                    }
                }
            });
        } else {
            qWarning("org.lingmo.peripherals-mouse no doubleClick keys!");
        }
//        settings->deleteLater();
//        settings = nullptr;
    } else {
        qWarning("org.lingmo.peripherals-mouse is null!");
    }

    if (auto settings = LINGMOStyleSettings::globalInstance()) {
        m_widget_theme = settings->get("widgetThemeName").toString();
        m_style_name = settings->get("styleName").toString();
    }

    if (m_style_name == "lingmo") {
        m_style_name = "lingmo-default";
    } else if (m_style_name == "lingmo-black") {
        m_style_name = "lingmo-dark";
    } else if (m_style_name == "lingmo-white") {
        m_style_name = "lingmo-light";
    }

    if (qApp->property("customStyleName").isValid()) {
        if (qApp->property("customStyleName").toString() == "lingmo-light") {
            m_style_name = "lingmo-light";
        } else if (qApp->property("customStyleName").toString() == "lingmo-dark") {
            m_style_name = "lingmo-dark";
        }
    }

    if (m_style_name == "lingmo-default") {
        m_default_palette = true;
    } else if (m_style_name == "lingmo-dark") {
        m_drak_palette = true;
    }

    m_tab_animation_helper = new ConfigTabWidgetAnimationHelper(this);
    m_scrollbar_animation_helper = new ConfigScrollBarAnimationHelper(this);
    m_button_animation_helper = new ConfigButtonAnimationHelper(this);
    m_combobox_animation_helper = new ConfigBoxAnimationHelper(this);
    m_animation_helper = new ConfigProgressBarAnimationHelper(this);
    m_shadow_helper = new ConfigShadowHelper(this);
    m_radiobutton_animation_helper = new ConfigRadioButtonAnimationHelper(this);
    m_checkbox_animation_helper = new ConfigCheckBoxAnimationHelper(this);
    m_slider_animation_helper = new ConfigSliderAnimationHelper(this);
    m_tree_animation_helper = new ConfigTreeAnimationHelper(this);

    //dbus
    m_statusManagerDBus = new QDBusInterface(DBUS_STATUS_MANAGER_IF, "/" ,DBUS_STATUS_MANAGER_IF,QDBusConnection::sessionBus(),this);
    if (m_statusManagerDBus) {
        QDBusReply<bool> message_a = m_statusManagerDBus->call("get_current_tabletmode");
        if (message_a.isValid()) {
            m_is_tablet_mode = message_a.value();
        }

        if (m_statusManagerDBus->isValid()) {
            //平板模式切换
            connect(m_statusManagerDBus, SIGNAL(mode_change_signal(bool)), this, SLOT(updateTabletModeValue(bool)));
        }
    }

    if(m_widget_theme == "")
        m_widget_theme == "default";

    sp = new LINGMOConfigStyleParameters(this, isUseDarkPalette(), m_widget_theme);

    setProperty("maxRadius", sp->radiusProperty().maxRadius);
    setProperty("normalRadius", sp->radiusProperty().normalRadius);
    setProperty("minRadius", sp->radiusProperty().minRadius);
}

LINGMOConfigStyle::~LINGMOConfigStyle()
{
    if(m_blur_helper){
        m_blur_helper->deleteLater();
        m_blur_helper = nullptr;
    }
    if(m_window_manager){
        m_window_manager->deleteLater();
        m_window_manager = nullptr;
    }
    if(m_tab_animation_helper){
        m_tab_animation_helper->deleteLater();
        m_tab_animation_helper = nullptr;
    }
    if(m_scrollbar_animation_helper){
        m_scrollbar_animation_helper->deleteLater();
        m_scrollbar_animation_helper = nullptr;
    }
    if(m_button_animation_helper){
        m_button_animation_helper->deleteLater();
        m_button_animation_helper = nullptr;
    }
    if(m_combobox_animation_helper){
        m_combobox_animation_helper->deleteLater();
        m_combobox_animation_helper = nullptr;
    }
    if(m_animation_helper){
        m_animation_helper->deleteLater();
        m_animation_helper = nullptr;
    }
    if(m_shadow_helper){
        m_shadow_helper->deleteLater();
        m_shadow_helper = nullptr;
    }
    if(m_statusManagerDBus){
        m_statusManagerDBus->deleteLater();
        m_statusManagerDBus = nullptr;
    }
    if(sp){
        sp->deleteLater();
        sp = nullptr;
    }

}

bool LINGMOConfigStyle::shouldBeTransparent(const QWidget *w) const
{
    bool should = false;

    if (w && w->inherits("QComboBoxPrivateContainer"))
        return true;

    if (w && w->inherits("QTipLabel")) {
        return true;
    }

    return should;
}

bool LINGMOConfigStyle::eventFilter(QObject *obj, QEvent *e)
{
    /*!
      \bug
      There is a bug when use fusion as base style when in qt5 assistant's
      HelperView. ScrollBar will not draw with our overrided function correctly,
      and then it will trigger QEvent::StyleAnimationUpdate. by some how it will let
      HelperView be hidden.
      I eat this event to aviod this bug, but the scrollbar in HelperView still
      display with old fusion style, and the animation will be ineffective.

      I don't know why HelperView didn't use our function drawing scrollbar but fusion, that
      makes me stranged.
      */
    if (e->type() == QEvent::StyleAnimationUpdate) {
        return true;
    }

//    //button animation cancel temporary
//    if (qobject_cast<QPushButton *>(obj) || qobject_cast<QToolButton *>(obj)) {
//        if (e->type() == QEvent::Hide) {
//            if (QWidget *w = qobject_cast<QWidget *>(obj)) {
//                auto animator = m_button_animation_helper->animator(w);
//                if (animator) {
//                    animator->stopAnimator("SunKen");
//                    animator->stopAnimator("MouseOver");
//                    animator->setAnimatorCurrentTime("SunKen", 0);
//                    animator->setAnimatorCurrentTime("MouseOver", 0);
//                }
//            }
//        }
//        return false;
//    }

    if (obj != nullptr && obj->inherits("QTipLabel"))
    {
        auto label = qobject_cast<QLabel *>(obj);
        if(label != nullptr && !(label->alignment() & Qt::AlignVCenter)){
            label->setAlignment(Qt::AlignVCenter);
        }
    }

    if (qobject_cast<QSlider *>(obj)) {
        if (e->type() == QEvent::Wheel) {
            if (QWidget *w = qobject_cast<QWidget *>(obj)) {
                if (!w->hasFocus()) {
                    return true;
                }
            }
        }
        auto mousePressEvent = static_cast<QMouseEvent*>(e);

        if((e->type() ==QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonDblClick) && (proxy()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons) == mousePressEvent->button() ||
                proxy()->styleHint(QStyle::SH_Slider_PageSetButtons) == mousePressEvent->button())){
            aSliderDebug << "e->type:" << e->type() << mousePressEvent->button();
            if(obj->property("animation").isValid() && !obj->property("animation").toBool())
                return false;

            auto *slider = qobject_cast<QSlider *>(obj);
            if(slider == nullptr || !slider->isEnabled())
                return false;

            auto animator = m_slider_animation_helper->animator(slider);
            if(!animator)
                return false;

            const bool horizontal = slider->orientation() == Qt::Horizontal;

            aSliderDebug << "MouseButtonPress............";
            QRect handleRect = sliderHandleRec(slider);
            if(horizontal){
                aSliderDebug << "mousepressevent1111 pos...." << mousePressEvent->pos().x() << handleRect.x();

                if(mousePressEvent->pos().x() <= handleRect.x()){
                    animator->setExtraProperty("addValue", false);

                }
                else if(mousePressEvent->pos().x() >= handleRect.x() + handleRect.width()){
                    animator->setExtraProperty("addValue", true);
                }
                else
                    return false;
            }
            else{
                aSliderDebug << "mousepressevent1111 pos...." << mousePressEvent->pos().y() << handleRect.y();
                if(mousePressEvent->pos().y() <= handleRect.y()){
                    animator->setExtraProperty("addValue", true);

                }
                else if(mousePressEvent->pos().y() >= handleRect.y() + handleRect.height()){
                    animator->setExtraProperty("addValue", false);
                }
                else
                    return false;
            }

            if(!animator->isRunning("move_position")){
                int startPosition = slider->value();//scrollbar->sliderPosition();//sliderPositionFromValue(bar->minimum, bar->maximum, scrollBar->value(), maxlen - sliderlen, bar->upsideDown);
                if(startPosition <= slider->maximum() && startPosition >= slider->minimum()){
                    if(animator->value("move_position") != 1.0){
                        if(animator->getExtraProperty("addValue").isValid()){
                            aSliderDebug << "pagestep..." << slider->pageStep() << animator->getExtraProperty("addValue").toBool();
                            int step = (animator->getExtraProperty("addValue").toBool() ? 1 : -1) * slider->pageStep();
                            aSliderDebug << "step............" << step;
                            bool absoluteButton = proxy()->styleHint(QStyle::SH_Slider_AbsoluteSetButtons) == mousePressEvent->button();
                            if(absoluteButton)
                            {
                                QStyleOptionSlider *opt = new QStyleOptionSlider();
                                opt->init(slider);

                                QRect gr = proxy()->subControlRect(QStyle::CC_Slider, opt, QStyle::SC_SliderGroove, slider);
                                QRect sr = proxy()->subControlRect(QStyle::CC_Slider, opt, QStyle::SC_SliderHandle, slider);
                                int sliderMin, sliderMax, sliderLength;
                                QPoint center = sr.center() - sr.topLeft();
                                int pos;

                                if (slider->orientation() == Qt::Horizontal) {
                                    sliderLength = sr.width();
                                    sliderMin = gr.x();
                                    sliderMax = gr.right() - sliderLength + 1;
                                    pos = (mousePressEvent->pos() - center).x();
                                    aSliderDebug << "Horizontal pos....:" << mousePressEvent->pos() << center << pos;
                                } else {
                                    sliderLength = sr.height();
                                    sliderMin = gr.y();
                                    sliderMax = gr.bottom() - sliderLength + 1;
                                    pos = (mousePressEvent->pos() - center).y();
                                    aSliderDebug << "vertical pos....:" << mousePressEvent->pos() << center << pos;
                                }
                                aSliderDebug << "minumum..." << slider->minimum() << slider->maximum() << sliderMin << sliderMax << pos << opt->upsideDown;
                                bool upsideDown = (slider->orientation() == Qt::Horizontal) ?
                                            (slider->invertedAppearance() != (slider->layoutDirection() == Qt::RightToLeft))
                                            : (!slider->invertedAppearance());
                                int sliderValue = sliderValueFromPosition(slider->minimum(), slider->maximum(), pos - sliderMin, sliderMax - sliderMin, upsideDown);//opt->upsideDown);
                                aSliderDebug << "sliderValue....." << sliderValue << slider->value();
                                step = sliderValue - slider->value();
                                aSliderDebug << "step0000...." << step;
                                aSliderDebug << "addValue........." << animator->getExtraProperty("addValue");
                            }
                            animator->startAnimator("move_position");

                            int endPos = (startPosition + step) - (absoluteButton ? 0 : (startPosition + step)%step);
                            aSliderDebug << "endPos0000....." << endPos;
                            if(!animator->getExtraProperty("addValue").toBool())
                                 endPos = (startPosition + step) + (absoluteButton ? 0 : (((startPosition + step)%step > 0) ?(qAbs(step) - (startPosition + step)%step) : 0));
                            aSliderDebug << "endPos1111....." << endPos;

                            if(animator->getExtraProperty("addValue").toBool())
                                endPos = qMin(endPos, slider->maximum());
                            else
                                endPos = qMax(endPos, slider->minimum());
                            aSliderDebug << "endPos....." << endPos;
                            animator->setExtraProperty("end_position", endPos);
                            animator->setExtraProperty("start_position", startPosition);

//                            qApp->sendEvent(slider, e);
                            slider->setFocus();
                            return true;
                        }
                    }
                }
            }
            else{
                slider->setFocus();
                return true;
            }
            return false;
        }

    }

    if(qobject_cast<QScrollBar*>(obj)){
        auto mousePressEvent = static_cast<QMouseEvent*>(e);
        auto *scrollbar = qobject_cast<QScrollBar *>(obj);
        if(scrollbar == nullptr || !scrollbar->isEnabled())
            return false;

        bool midButtonAbsPos = proxy()->styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition, 0, scrollbar);
//        if(mousePressEvent->button() != Qt::LeftButton)
//            return false;
        if ((e->type() != QEvent::MouseButtonPress && e->type() != QEvent::MouseButtonDblClick)
                || (!(mousePressEvent->button() == Qt::LeftButton || (midButtonAbsPos && mousePressEvent->button() == Qt::MidButton))))
            return false;

            if(obj->property("animation").isValid() && !obj->property("animation").toBool())
                return false;

            auto animator = m_scrollbar_animation_helper->animator(scrollbar);
            if(!animator)
                return false;
            aScrollBarDebug << "event..." << e->type() << mousePressEvent->button();
            const bool horizontal = scrollbar->orientation() == Qt::Horizontal;

            QRect handleRect = scrollbarSliderRec(scrollbar);

            if(horizontal){
                if(mousePressEvent->pos().x() <= handleRect.x()){
                    animator->setExtraProperty("addValue", /*scrollbar->layoutDirection() == Qt::RightToLeft ? true :*/ false);
                }
                else if(mousePressEvent->pos().x() >= handleRect.x() + handleRect.width()){
                    animator->setExtraProperty("addValue", /*scrollbar->layoutDirection() == Qt::RightToLeft ? false :*/ true);
                }
                else
                    return false;
            }
            else{
                if(mousePressEvent->pos().y() <= handleRect.y()){
                    animator->setExtraProperty("addValue", false);
                }
                else if(mousePressEvent->pos().y() >= handleRect.y() + handleRect.height()){
                    animator->setExtraProperty("addValue", true);
                }
                else
                    return false;
            }

            if(!animator->isRunning("move_position")){
                int startPosition = scrollbar->value();
                aScrollBarDebug << "startPosition......." << startPosition << scrollbar->value();
                if(startPosition <= scrollbar->maximum() && startPosition >= scrollbar->minimum()){
                    aScrollBarDebug << "CC_ScrollBar............" << animator->isRunning("move_position") << animator->currentAnimatorTime("move_position");
                    if(animator->value("move_position") != 1.0){
                        if(animator->getExtraProperty("addValue").isValid()){
                            aScrollBarDebug << "pagestep..." << scrollbar->pageStep() << animator->getExtraProperty("addValue").toBool();
                            int step = (animator->getExtraProperty("addValue").toBool() ? 1 : -1) * scrollbar->pageStep();
                            aScrollBarDebug << "step............" << step << animator->getExtraProperty("addValue").toBool();

                            QStyleOptionSlider *opt = new QStyleOptionSlider();
                            opt->init(scrollbar);

                            aScrollBarDebug << "11111111" << scrollbar->property("ScrollBarSliderRect").isValid();

                            QRect sr = scrollbar->property("ScrollBarSliderRect").isValid() ?
                                        scrollbar->property("ScrollBarSliderRect").value<QRect>() :
                                        QRect();

                            aScrollBarDebug << "12333333" << scrollbar->property("ScrollBarGrooveRect").isValid();
                            QRect gr = scrollbar->property("ScrollBarGrooveRect").isValid() ?
                                        scrollbar->property("ScrollBarGrooveRect").value<QRect>() :
                                        QRect();

                            bool absoluteButton = (midButtonAbsPos && mousePressEvent->button() == Qt::MidButton)
                                    || (proxy()->styleHint(QStyle::SH_ScrollBar_LeftClickAbsolutePosition, opt, scrollbar)
                                        && mousePressEvent->button() == Qt::LeftButton);
                            if(absoluteButton){
                                int sliderMin, sliderMax, sliderLength, pos;

                                if (horizontal == Qt::Horizontal) {
                                    sliderLength = sr.width();
                                    sliderMin = gr.x();
                                    sliderMax = gr.right() - sliderLength + 1;
                                    aScrollBarDebug << "sliderLength....." << sliderLength << gr << gr.right();
                                    pos = mousePressEvent->pos().x() - sliderLength / 2;
                                    if (scrollbar->layoutDirection() == Qt::RightToLeft)
                                        opt->upsideDown = !opt->upsideDown;
                                } else {
                                    sliderLength = sr.height();
                                    sliderMin = gr.y();
                                    sliderMax = gr.bottom() - sliderLength + 1;
                                    pos = mousePressEvent->pos().y()  - sliderLength / 2;

                                }

                                bool upsideDown = scrollbar->invertedAppearance();
                                aScrollBarDebug << "minimum..." << scrollbar->minimum() << scrollbar->maximum() << pos << sliderMin << sliderMax;
                                int scrollValue = sliderValueFromPosition(scrollbar->minimum(), scrollbar->maximum(), pos - sliderMin,
                                                                        sliderMax - sliderMin, upsideDown);//opt->upsideDown);
                                aScrollBarDebug << "scrollValue....." << scrollValue << scrollbar->value();
                                step = scrollValue - scrollbar->value();
                                aScrollBarDebug << "step0000...." << step;
                                aScrollBarDebug << "addValue........." << animator->getExtraProperty("addValue");

                            }
                            animator->startAnimator("move_position");

                            int endPos = (startPosition + step) - (absoluteButton ? 0 : (startPosition + step)%step);
                            aScrollBarDebug << "endPos000....." << startPosition << step << endPos;

                            if(!animator->getExtraProperty("addValue").toBool())
                                 endPos = (startPosition + step) + (absoluteButton ? 0 : (((startPosition + step)%step > 0) ?(qAbs(step) - (startPosition + step)%step) : 0));
                            aScrollBarDebug << "endPos1111....." << endPos;

                            if(animator->getExtraProperty("addValue").toBool())
                                endPos = qMin(endPos, scrollbar->maximum());
                            else
                                endPos = qMax(endPos, scrollbar->minimum());
                            aScrollBarDebug << "endPos....." << endPos;
                            animator->setExtraProperty("end_position", endPos);
                            animator->setExtraProperty("start_position", startPosition);

//                            qApp->sendEvent(scrollbar, e);
                            return true;
                        }
                    }
                }
            }
            else
                return true;

            return false;

    }


    if (qobject_cast<QMenu *>(obj)) {
        if (e->type() == QEvent::Show) {
            if (QMenu *w = qobject_cast<QMenu *>(obj)) {
                if (!KWindowSystem::compositingActive()) {
                    w->setProperty("useSystemStyleBlur", false);
                }
            }
        }
    }
    return false;
}

/*
 * Note there are some widgets can not be set as transparent one in polish.
 * Because it has been created as a rgb window.
 *
 * To reslove this problem, we have to let attribute be setted more ahead.
 * Some styleHint() methods are called in the early creation of a widget.
 * So we can real set them as alpha widgets.
 */
int LINGMOConfigStyle::styleHint(QStyle::StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    realSetWindowSurfaceFormatAlpha(widget);
    realSetMenuTypeToMenu(widget);

    switch (hint) {
    case SH_ScrollBar_Transient:
        return false;
    case SH_ItemView_ShowDecorationSelected:
        return true;
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return int(true);
    case SH_DialogButtons_DefaultButton:
        return int(true);
    case SH_UnderlineShortcut:
        return true;
    case SH_ComboBox_Popup:
        return true;
    case SH_ComboBox_AllowWheelScrolling:
        return int(false);
    case SH_ComboBox_PopupFrameStyle:
        return QFrame::NoFrame | QFrame::Plain;

    case SH_Button_FocusPolicy:
        return Qt::TabFocus;

    case SH_Header_ArrowAlignment:
        return Qt::AlignRight | Qt::AlignVCenter;

    case SH_Table_GridLineColor:
        return option ? option->palette.color(QPalette::Active, QPalette::Midlight).rgb() : 0;

    case SH_ItemView_ActivateItemOnSingleClick:
        if(m_is_tablet_mode){
            if(widget)
            {
                if(widget->topLevelWidget())
                {
                    if(widget->topLevelWidget()->objectName() == QString::fromUtf8("KyNativeFileDialog"))
                        return false;
                }
            }
        }
        return m_is_tablet_mode;

    case SH_DialogButtonLayout:
        return QDialogButtonBox::MacLayout;

    case SH_Menu_Scrollable: {
        return 1;
    }
    case SH_BlinkCursorWhenTextSelected: {
        return m_blink_cursor;
    }
    case SH_TabBar_Alignment: {
        if (widget && widget->layoutDirection() == Qt::RightToLeft)
            return Qt::AlignRight;
        return Qt::AlignLeft;
    }
    default:
        break;
    }
    return Style::styleHint(hint, option, widget, returnData);
}
void LINGMOConfigStyle::polish(QPalette &palette){
    palette = standardPalette();
    return Style::polish(palette);
}

QPalette LINGMOConfigStyle::standardPalette() const
{
    auto palette = Style::standardPalette();
    sp->setPalette(palette);

    if (auto settings = LINGMOStyleSettings::globalInstance()) {
        QString themeColor = settings->get("themeColor").toString();
        themeColor = settings->get("theme-color").toString();

        QColor color = palette.color(QPalette::Active, QPalette::Highlight);
        if (themeColor == "default") {

        } else if (themeColor == "daybreakBlue") {
            color = QColor(55, 144, 250);
        } else if (themeColor == "jamPurple") {
            color = QColor(120, 115, 245);
        } else if (themeColor == "magenta") {
            color = QColor(235, 48, 150);
        } else if (themeColor == "sunRed") {
            color = QColor(243, 34, 45);
        } else if (themeColor == "sunsetOrange") {
           color = QColor(246, 140, 39);
        } else if (themeColor == "dustGold") {
            color = QColor(249, 197, 61);
        } else if (themeColor == "polarGreen") {
            color = QColor(82, 196, 41);
        } else if (!themeColor.isEmpty() && themeColor.startsWith("(") && themeColor.endsWith(")")) {
            QColor c = color;
            QString colorStr = themeColor;
            colorStr = colorStr.remove(colorStr.length() - 1, 1);
            colorStr = colorStr.remove(0, 1);
            QStringList list = colorStr.split(",");
            if (list.length() == 3 || list.length() == 4) {
                c.setRed(list[0].toInt());
                c.setGreen(list[1].toInt());
                c.setBlue(list[2].toInt());
                if (list.length() == 4) {
                    QStringList alphaList = QString(list[3]).split("*");
                    if (alphaList.length() == 2) {
                        if (alphaList[0].toFloat() == 255.0)
                            c.setAlphaF(alphaList[1].toFloat());
                        else if (alphaList[1].toFloat() == 255.0)
                            c.setAlphaF(alphaList[0].toFloat());
                        else
                            c.setAlphaF(alphaList[0].toFloat() * alphaList[1].toFloat() / 255.0);

                    } else if (alphaList.length() == 1) {
                        if(alphaList[0].toFloat() <= 1.0)
                            c.setAlphaF(alphaList[0].toFloat());
                        else
                            c.setAlpha(alphaList[0].toInt());
                    } else
                        c.setAlphaF(1.0);
                }
            } else if (list.length() == 1 && list[0].length() == 7 && list[0].startsWith("#")) {
                c = list[0];
            }

            color = c;
        }

        setThemeColor(color, palette);
        sp->setLINGMOThemeColor(palette, themeColor);
    }

    return palette;
}

bool LINGMOConfigStyle::isUseDarkPalette() const
{
    bool usedark = (!useDefaultPaletteList().contains(qAppName()) && (m_drak_palette || (m_default_palette && useDarkPaletteList().contains(qAppName()))));

    if (qAppName() == "lingmo-screensaver-dialog") {
        usedark = false;
    }

    return usedark;
}

QColor LINGMOConfigStyle::button_Click(const QStyleOption *option) const
{
    QColor button = option->palette.color(QPalette::Active, QPalette::Button);
    QColor mix    = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (isUseDarkPalette()) {
        return configMixColor(button, mix, 0.05);
    }

    return configMixColor(button, mix, 0.2);
}


QColor LINGMOConfigStyle::button_Hover(const QStyleOption *option) const
{
    QColor button = option->palette.color(QPalette::Active, QPalette::Button);
    QColor mix    = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (isUseDarkPalette()) {
        return configMixColor(button, mix, 0.2);
    }

    return configMixColor(button, mix, 0.05);
}


QColor LINGMOConfigStyle::transparentButton_Click(const QStyleOption *option) const
{
    QColor button = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (isUseDarkPalette()) {
        button.setAlphaF(0.15);
    } else {
        button.setAlphaF(0.28);
    }

    return button;
}

QColor LINGMOConfigStyle::transparentButton_Hover(const QStyleOption *option) const
{
    QColor button = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (isUseDarkPalette()) {
        button.setAlphaF(0.28);
    } else {
        button.setAlphaF(0.15);
    }

    return button;
}


QColor LINGMOConfigStyle::highLight_Click(const QStyleOption *option) const
{
    QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix    = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (isUseDarkPalette()) {
        return configMixColor(highlight, mix, 0.05);
    }

    return configMixColor(highlight, mix, 0.2);
}



QColor LINGMOConfigStyle::highLight_Hover(const QStyleOption *option) const
{
    QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
    QColor mix    = option->palette.color(QPalette::Active, QPalette::BrightText);

    if (isUseDarkPalette()) {
        return configMixColor(highlight, mix, 0.2);
    }

    return configMixColor(highlight, mix, 0.05);
}



void LINGMOConfigStyle::setThemeColor(QColor color, QPalette &palette) const
{
    palette.setColor(QPalette::Active, QPalette::Highlight, color);
    palette.setColor(QPalette::Inactive, QPalette::Highlight, color);
}

void LINGMOConfigStyle::updateTabletModeValue(bool isTabletMode)
{
    m_is_tablet_mode = isTabletMode;
    sp->updateParameters(m_is_tablet_mode);

    qApp->setPalette(qGuiApp->palette());
    QEvent event(QEvent::ApplicationPaletteChange);
    foreach (QWidget *widget, qApp->allWidgets()) {
        qApp->sendEvent(widget, &event);
    }
}

QRect LINGMOConfigStyle::scrollbarSliderRec(QWidget *w)
{
    if(w && m_scrollBarSliderRec.contains(w))
        return m_scrollBarSliderRec.value(w);
    return QRect();
}

void LINGMOConfigStyle::setScrollbarSliderRec(QWidget *w, QRect rect) const
{
    if(w){
        if(m_scrollBarSliderRec.contains(w)){
            QRect r = m_scrollBarSliderRec.value(w);
            if(r != rect)
                m_scrollBarSliderRec.insert(w, rect);
            return;
        }
        m_scrollBarSliderRec.insert(w, rect);
    }
}

QRect LINGMOConfigStyle::sliderHandleRec(QWidget *w)
{
    if(w && m_sliderHandleRec.contains(w))
        return m_sliderHandleRec.value(w);
    return QRect();
}

void LINGMOConfigStyle::setSliderHandleRec(QWidget *w, QRect rect) const
{
    if(w){
        if(m_sliderHandleRec.contains(w)){
            QRect r = m_sliderHandleRec.value(w);
            if(r != rect)
                m_sliderHandleRec.insert(w, rect);
            return;
        }
        m_sliderHandleRec.insert(w, rect);
    }
}


void LINGMOConfigStyle::polish(QWidget *widget)
{
    QLibrary gestureLib(GESTURE_LIB);
    if (widget && gestureLib.load()) {
        typedef void (*RegisterFun) (QWidget*, QObject*);
        auto fun = (RegisterFun) gestureLib.resolve("registerWidget");
        if (fun) {
            fun(widget, widget);
        } else {
            qWarning() << "Can't resolve registerWidget from" << GESTURE_LIB << gestureLib.errorString();
        }
    }

    if(!widget)
        return;

    realSetWindowSurfaceFormatAlpha(widget);

    if (qAppName() != "lingmo-menu" || (qAppName() == "lingmo-menu" && widget && widget->inherits("QMenu"))) {
        if (widget && widget->testAttribute(Qt::WA_TranslucentBackground) && widget->isTopLevel()) {
            //FIXME:
    #if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
            m_blur_helper->registerWidget(widget);
    #endif

            /*
            if (QString(widget->metaObject()->className())=="QMenu" ||
                    widget->inherits("Peony::DirectoryViewMenu") ||
                    widget->inherits("Peony::DesktopMenu")) {
                m_blur_helper->registerWidget(widget);
            }
            */
        }

        if (widget && widget->isWindow()) {
            auto var = widget->property("useStyleWindowManager");

            if (var.isNull()) {
                m_window_manager->registerWidget(widget);
            } else {
                if (var.toBool()) {
                    m_window_manager->registerWidget(widget);
                }
            }
        }
    }

    widget->setProperty("maxRadius", sp->radiusProperty().maxRadius);
    widget->setProperty("normalRadius", sp->radiusProperty().normalRadius);
    widget->setProperty("minRadius", sp->radiusProperty().minRadius);

    Style::polish(widget);

    m_shadow_helper->registerWidget(widget);

    if (qobject_cast<QTabWidget*>(widget)) {
        //FIXME: unpolish, extensiable.
        if (qAppName() != "lingmo-sidebar")
            m_tab_animation_helper->registerWidget(widget);
    }

    if (qobject_cast<QScrollBar*>(widget)) {
        //set scroll must have mouse_hover state
        widget->setAttribute(Qt::WA_Hover);

        //cancel scroll right click menu
        widget->setContextMenuPolicy(Qt::NoContextMenu);
        m_scrollbar_animation_helper->registerWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
        //QCompleter BUG113969
//        v->setAttribute(Qt::WA_InputMethodEnabled);
    }

    if(qobject_cast<QToolButton*>(widget))
    {
//        m_button_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QPushButton*>(widget))
    {
//        m_button_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QRadioButton*>(widget))
    {
        m_radiobutton_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QCheckBox*>(widget))
    {
        m_checkbox_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QSlider*>(widget)){
        m_slider_animation_helper->registerWidget(widget);
    }
    /*!
      \todo QDateTimeEdit widget affected with calendarPopup() draw QComboBox style or QSpinBox style.
       So temporarily haven't added the QDateTimeEdit animation and style.
      */
    if(qobject_cast<QComboBox*>(widget))
    {
        auto combobx = qobject_cast<QComboBox *>(widget);
        m_combobox_animation_helper->registerWidget(widget);
        m_button_animation_helper->registerWidget(widget);
        combobx->view()->viewport()->setAutoFillBackground(false);
    }

    if(qobject_cast<QSpinBox*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    if(qobject_cast<QDoubleSpinBox*>(widget))
    {
        m_button_animation_helper->registerWidget(widget);
    }

    if (widget->inherits("QTipLabel")) {
        auto label = qobject_cast<QLabel *>(widget);
        label->setWordWrap(true);
        label->setScaledContents(true);
        if(sp->m_isTableMode){
            QFontMetrics fm = label->fontMetrics();
            int textHeight = fm.height();
            if(textHeight < sp->ToolTip_Height)
            {
                //PE_PanelTipLabel rec adjusted 8
                label->setMinimumHeight(sp->ToolTip_Height + 8);
            }
        }
    }

    if (qobject_cast<QMessageBox *>(widget)) {
        widget->setAutoFillBackground(true);
        widget->setBackgroundRole(QPalette::Base);
    }

    if (qobject_cast<QTabBar *>(widget)) {
        auto tabBar = qobject_cast<QTabBar *>(widget);
        if(tabBar->elideMode() == Qt::ElideNone)
            tabBar->setElideMode(Qt::ElideRight);
    }

    if (qobject_cast<QLineEdit *>(widget) || qobject_cast<QTabBar *>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
    }

    if (qobject_cast<QLineEdit *>(widget)) {
        //lineedit action use highlight effect
        if (widget->findChild<QAction *>()) {
            QStyleOption subOption;
            subOption.state = State_None;
            subOption.palette = sp->defaultPalette;
            if (widget->isEnabled())
                subOption.state |= State_Enabled;
            for (QAction *act : widget->findChildren<QAction *>()) {
                act->setIcon(QIcon(HighLightEffect::ordinaryGeneratePixmap(act->icon().pixmap(16, 16), &subOption, widget, HighLightEffect::BothDefaultAndHighlit)));
            }
        }
    }

    if(qobject_cast<QTableView*>(widget))
    {
        auto tableView = qobject_cast<QTableView *>(widget);
//        qDebug() << "header height:" << tableView->verticalHeader()->count();
        bool isInitial = true;
        if (qApp->property("isInitial").isValid()) {
            isInitial = qApp->property("isInitial").toBool();
        }
        //模式切换时tablewidget中项高度变化  处理得还是有点问题
        for(int i = 0; i < tableView->verticalHeader()->count(); i++)
        {
//            qDebug() << "i........" << i << tableView->rowHeight(i) << sp->m_isTableMode << sp->m_scaleRatio4_3 << (tableView->rowHeight(i) / (4.0/3.0)) << tableView->rowHeight(i) * (4.0/3.0);
            if(tableView->rowHeight(i) == sp->m_headerDefaultSectionSizeVertical1_1 ||
                    tableView->rowHeight(i) == sp->m_headerDefaultSectionSizeVertical4_3)
                tableView->setRowHeight(i, (isInitial ? tableView->rowHeight(i) : sp->m_headerDefaultSectionSizeVertical));
            //            else //目前没办法用这个方式  因为这个polish可能被同时调用多次 导致高度被进行了多次调整
            //                tableView->setRowHeight(i, (isInitial ? tableView->rowHeight(i) : (sp->m_isTableMode ? tableView->rowHeight(i) * 4.0/3.0 : tableView->rowHeight(i) * 3.0/4.0)));

        }
    }

    if(widget != nullptr && widget->inherits("QListView"))
    {
        auto listview = qobject_cast<QListView *>(widget);
        if(listview != nullptr)
        {
            connect(listview->selectionModel(), &QItemSelectionModel::selectionChanged, [listview](const QItemSelection &selected, const QItemSelection &deselected){
                listview->repaint();
            });
        }
    }

    if(widget != nullptr && widget->inherits("QTreeView")){
        m_tree_animation_helper->registerWidget(widget);
        if(!qobject_cast<QTreeView *>(widget)->isAnimated() &&
                widget->property("animation").isValid() && !widget->property("animation").toBool()){
            qobject_cast<QTreeView *>(widget)->setAnimated(false);
        }
        else{
            widget->setProperty("animation", true);
            qobject_cast<QTreeView *>(widget)->setAnimated(true);
        }

        auto tree = qobject_cast<QTreeView*>(widget);
        connect(tree, &QTreeView::expanded, this, [=](const QModelIndex &index){
            if(m_tree_animation_helper){
                auto treeview = qobject_cast<QTreeView *>(widget);
                auto animator = m_tree_animation_helper->animator(treeview);
                if(!animator)
                    return;

                animator->setExtraProperty("expandItem", index);

                if(animator->isRunning("collaps")){
                    animator->stopAnimator("collaps");
                    animator->setAnimatorCurrentTime("collaps", 0);
                }
                if(!animator->isRunning("expand")){
                    animator->startAnimator("expand");
                }
            }
        }, Qt::UniqueConnection);
        connect(tree, &QTreeView::collapsed, this, [=](const QModelIndex &index){
            if(m_tree_animation_helper){
                auto treeview = qobject_cast<QTreeView *>(widget);
                auto animator = m_tree_animation_helper->animator(treeview);
                if(!animator)
                    return;

                animator->setExtraProperty("collapsItem", index);

                if(animator->isRunning("expand")){
                    animator->stopAnimator("expand");
                    animator->setAnimatorCurrentTime("expand", 0);
                }
                if(!animator->isRunning("collaps")){
                    animator->startAnimator("collaps");
                }
            }
        }, Qt::UniqueConnection);

    }

    //input right to left
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
//        if (auto spinbox = qobject_cast<QSpinBox *>(widget)) {
//            spinbox->setAlignment(Qt::AlignRight);
//        }
//        if (auto lineedit = qobject_cast<QLineEdit *>(widget)) {
//            lineedit->setAlignment(Qt::AlignRight);
//        }
//        if (auto textedit = qobject_cast<QTextEdit *>(widget)) {
//            textedit->setAlignment(Qt::AlignRight);
//        }
//        if (auto label = qobject_cast<QLabel *>(widget)) {
//            label->setAlignment(Qt::AlignRight);
//        }
    }

    widget->installEventFilter(this);
}

void LINGMOConfigStyle::unpolish(QWidget *widget)
{
    QLibrary gestureLib(GESTURE_LIB);
    if (widget && gestureLib.load()) {
        typedef void (*UnRegisterFun) (QWidget*, QObject*);
        auto fun = (UnRegisterFun) gestureLib.resolve("unregisterWidget");
        if (fun) {
            fun(widget, widget);
        } else {
            qWarning() << "Can't resolve unregisterWidget from" << GESTURE_LIB << gestureLib.errorString();
        }
    }
    if (qAppName() != "lingmo-menu" || (qAppName() == "lingmo-menu" && widget && widget->inherits("QMenu"))) {
        if (widget->testAttribute(Qt::WA_TranslucentBackground) && widget->isTopLevel()) {
    #if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
            m_blur_helper->unregisterWidget(widget);
    #endif
        }
        if (widget && widget->isWindow()) {
            auto var = widget->property("useStyleWindowManager");

            if (var.isNull()) {
                m_window_manager->unregisterWidget(widget);
            } else {
                if (var.toBool()) {
                    m_window_manager->unregisterWidget(widget);
                }
            }
        }
    }

    if(!widget)
        return;
    m_shadow_helper->unregisterWidget(widget);

    widget->removeEventFilter(this);

    if (widget && widget->inherits("QTipLabel")) {
        auto label = qobject_cast<QLabel *>(widget);
        label->setWordWrap(false);
    }

    if (qobject_cast<QTabWidget*>(widget)) {
        m_tab_animation_helper->unregisterWidget(widget);
    }

    if (qobject_cast<QScrollBar*>(widget)) {
        m_scrollbar_animation_helper->unregisterWidget(widget);
    }

    if (auto v = qobject_cast<QAbstractItemView *>(widget)) {
        v->viewport()->setAttribute(Qt::WA_Hover);
    }

    if(qobject_cast<QToolButton*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QPushButton*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QComboBox*>(widget))
    {
        m_combobox_animation_helper->unregisterWidget(widget);
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QSpinBox*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QDoubleSpinBox*>(widget))
    {
        m_button_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QRadioButton*>(widget))
    {
        m_radiobutton_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QCheckBox*>(widget))
    {
        m_checkbox_animation_helper->unregisterWidget(widget);
    }

    if(qobject_cast<QSlider*>(widget))
        m_slider_animation_helper->unregisterWidget(widget);

    if (qobject_cast<QLineEdit *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, false);
    }

    if(qobject_cast<QTreeView *>(widget)){
        m_tree_animation_helper->unregisterWidget(widget);
    }

    Style::unpolish(widget);
}

void LINGMOConfigStyle::polish(QApplication *app)
{
//    QtConcurrent::run(this, &LINGMOConfigStyle::getDefaultControlParameters, app);
    getDefaultControlParameters(app);

    Style::polish(app);
}

QIcon LINGMOConfigStyle::standardIcon(StandardPixmap standardIcon, const QStyleOption *option, const QWidget *widget) const
{
    if (QApplication::desktopSettingsAware() && !QIcon::themeName().isEmpty()) {
        switch (standardIcon) {
        case SP_FileDialogListView:
        {
            return QIcon::fromTheme(QLatin1String("view-list-symbolic"));
        }break;

        case SP_FileDialogDetailedView:
        {
            return QIcon::fromTheme(QLatin1String("view-grid-symbolic"));
        }break;

        case SP_ToolBarHorizontalExtensionButton:
        {
            //toolbar horizontal extension button icon
            return QIcon::fromTheme(QLatin1String("lingmo-end-symbolic"));
        }break;

        case SP_LineEditClearButton:
        {
            //lineedit close button icon
            return QIcon::fromTheme(QLatin1String("edit-clear-symbolic"));
        }break;

        default:
            break;
        }
    }
    return Style::standardIcon(standardIcon, option, widget);
}



static inline uint qt_intensity(uint r, uint g, uint b)
{
    // 30% red, 59% green, 11% blue
    return (77 * r + 150 * g + 28 * b) / 255;
}

/*! \reimp */
QPixmap LINGMOConfigStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const
{
    switch (iconMode) {
    case QIcon::Disabled: {
        QStyleOption tmp = *option;
        tmp.state = State_Enabled;
        QPixmap target = HighLightEffect::ordinaryGeneratePixmap(pixmap, &tmp);

//        //Fix me:QT original code
        QImage im = target.toImage().convertToFormat(QImage::Format_ARGB32);

        // Create a colortable based on the background (black -> bg -> white)
        QColor bg = option->palette.color(QPalette::Disabled, QPalette::ButtonText);
        int red = bg.red();
        int green = bg.green();
        int blue = bg.blue();
        uchar reds[256], greens[256], blues[256];
        for (int i=0; i<128; ++i) {
            reds[i]   = uchar((red   * (i<<1)) >> 8);
            greens[i] = uchar((green * (i<<1)) >> 8);
            blues[i]  = uchar((blue  * (i<<1)) >> 8);
        }
        for (int i=0; i<128; ++i) {
            reds[i+128]   = uchar(qMin(red   + (i << 1), 255));
            greens[i+128] = uchar(qMin(green + (i << 1), 255));
            blues[i+128]  = uchar(qMin(blue  + (i << 1), 255));
        }

        int intensity = qt_intensity(red, green, blue);
        const int factor = 191;

        // High intensity colors needs dark shifting in the color table, while
        // low intensity colors needs light shifting. This is to increase the
        // perceived contrast.
        if ((red - factor > green && red - factor > blue)
            || (green - factor > red && green - factor > blue)
            || (blue - factor > red && blue - factor > green))
            intensity = qMin(255, intensity + 91);
//        else if (intensity <= 128)
//            intensity -= 51;
        bool isPureColor= HighLightEffect::isPixmapPureColor(pixmap);

        for (int y=0; y<im.height(); ++y) {
            QRgb *scanLine = (QRgb*)im.scanLine(y);
            for (int x=0; x<im.width(); ++x) {
                QRgb pixel = *scanLine;
                // Calculate color table index, taking intensity adjustment
                // and a magic offset into account.
                uint ci = uint(qGray(pixel)/3 + (130 - intensity / 3));

                int r = int(reds[ci]);
                int g = int(greens[ci]);
                int b = int(blues[ci]);
                int a = qAlpha(pixel)/255.0 > 0.7 ? 0.7 * 255 : qAlpha(pixel);
                if(isPureColor){
                    r = qMax(int(reds[ci]), bg.red());
                    g = qMax(int(greens[ci]), bg.green());
                    b = qMax(int(blues[ci]), bg.blue());
                    a = qAlpha(pixel);
                }
                *scanLine = qRgba(r, g, b, a);
                ++scanLine;
            }
        }

        return QPixmap::fromImage(im);


        //Fix me:set same color to text when set icon mode disable.But it has error in color icons.

//        QColor bg = option->palette.color(QPalette::Disabled, QPalette::WindowText);
//        bg.setAlphaF(0.5);
//        QPainter p(&target);
//        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
//        p.fillRect(target.rect(), bg);
//        p.end();

//        return target;
    }

    default:
        break;
    }

    return Style::generatedIconPixmap(iconMode, pixmap, option);
}


QPixmap LINGMOConfigStyle::testGeneratedSVGPixmap(QPixmap pixmap) const
{
        QImage im = pixmap.toImage();
        for (int y=0; y<im.height(); ++y) {
            QRgb *scanLine = (QRgb*)im.scanLine(y);
            for (int x=0; x<im.width(); ++x) {
                QRgb pixel = *scanLine;
                QColor c(pixel);
                int h, s, l, a;
                c.getHsl(&h, &s, &l, &a);
                s += a > 0 ? 82 : 0;
                l += a > 0 ? 78 : 0;
                c.setHsl(h, s, l, a);

                *scanLine = c.rgba();//qRgba(c.red(), c.green(), c.blue(), c.alpha());
                ++scanLine;
            }
        }

        return QPixmap::fromImage(im);


        //Fix me:set same color to text when set icon mode disable.But it has error in color icons.

//        QColor bg = option->palette.color(QPalette::Disabled, QPalette::WindowText);
//        bg.setAlphaF(0.5);
//        QPainter p(&target);
//        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
//        p.fillRect(target.rect(), bg);
//        p.end();

//        return target;

}

void LINGMOConfigStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case QStyle::PE_PanelMenu:
    case QStyle::PE_FrameMenu:
    {
        sp->initConfigMenuParameters(isUseDarkPalette(), option, widget);
        painter->save();

        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter->setBrush(sp->m_MenuParameters.menuBackgroundBrush);
        painter->setPen(sp->m_MenuParameters.menuBackgroundPen);
        if(widget)
            painter->drawRoundedRect(widget->rect(), sp->m_MenuParameters.frameRadius, sp->m_MenuParameters.frameRadius);
        painter->restore();

        if(sp->Menu_Right_Bottom_Corner){
            QRect drawRect;
            drawRect = option->rect.adjusted(0, 0, 1, 1);
            QPainterPath drawRightBottomPath;
            drawRightBottomPath.moveTo(drawRect.right(), drawRect.bottom() - 28);
            drawRightBottomPath.arcTo(QRect(drawRect.right() - 2 * 20, drawRect.bottom() - 2 * 28, 2 * 20, 2 * 28), 0, -90);
            drawRightBottomPath.lineTo(drawRect.right() - sp->m_MenuParameters.frameRadius, drawRect.bottom());
            drawRightBottomPath.arcTo(QRect(drawRect.right() - 2 * sp->m_MenuParameters.frameRadius, drawRect.bottom() - 2 * sp->m_MenuParameters.frameRadius,
                                            2 * sp->m_MenuParameters.frameRadius, 2 * sp->m_MenuParameters.frameRadius), -90, 90);
            drawRightBottomPath.lineTo(drawRect.right(), drawRect.bottom() - 28);

            //draw right bottom corner
            QColor startColor = option->palette.color(QPalette::Active, QPalette::Base).lighter(300);
            QColor endColor = configMixColor(startColor, QColor(Qt::black), 0.25);
            QLinearGradient linearGradient;

            painter->save();
            linearGradient.setColorAt(0, startColor);
            linearGradient.setColorAt(1, endColor);
            linearGradient.setStart(drawRect.right(), drawRect.bottom() - 28);
            linearGradient.setFinalStop(drawRect.right(), drawRect.bottom());
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->setPen(Qt::transparent);
            painter->setBrush(linearGradient);
            painter->drawPath(drawRightBottomPath);
            painter->restore();
        }
        return;
    }
    case PE_FrameFocusRect: {
        if (qApp->focusWidget() && qApp->focusWidget()->inherits("QTreeView"))
        {
            sp->initConfigTreeParameters(isUseDarkPalette(), option, widget);
            QPen focusPen = sp->m_TreeParameters.treeFocusPen;
            painter->save();
            painter->setBrush(Qt::NoBrush);
            painter->setPen(focusPen);
            int width = focusPen.width();

            if(width == 1 && sp->m_TreeParameters.radius != 0)
                painter->translate(0.5, 0.5);
            painter->setRenderHint(QPainter::Antialiasing, sp->m_TreeParameters.radius != 0);

            painter->drawRoundedRect(option->rect.adjusted(width/2.0, width/2.0, -width/2.0, -width/2.0),
                                     sp->m_TreeParameters.radius, sp->m_TreeParameters.radius);
            painter->restore();
            return;
        }
        else if (qobject_cast<const QAbstractItemView *>(widget))
            return;
        break;
    }

    case PE_Frame:{
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(option->palette.color(QPalette::Active, QPalette::Midlight), 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case PE_PanelButtonCommand:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
//            auto animator = m_button_animation_helper->animator(widget);
            const bool enable = button->state & State_Enabled;
            const bool hover = button->state & State_MouseOver;
            const bool sunken = button->state & State_Sunken;
            const bool on = button->state & State_On;
            const bool focus = button->state & State_HasFocus;
            const bool active = button->state & State_Active;
            const bool inActive = (enable && !active);
            bool roundButton = false;
            int width = 0;
            QRect rect = option->rect;

//            qDebug() << "PE_PanelButtonCommand........" << button->text << "active:" << active << "inActive;" << inActive;
            if (widget && widget->property("isRoundButton").isValid()) {
                roundButton = widget->property("isRoundButton").toBool();
            }

            painter->save();
            if (!enable) {
//                if (animator) {
//                    animator->stopAnimator("SunKen");
//                    animator->stopAnimator("MouseOver");
//                }

                if (on) {
                    width = sp->m_PushButtonParameters.pushButtonCheckDisablePen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonCheckDisablePen);
                    painter->setBrush(sp->m_PushButtonParameters.pushButtonCheckDisableBrush);
                } else if (button->features & QStyleOptionButton::Flat) {
                    width = 0;
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(Qt::NoBrush);
                } else {
                    width = sp->m_PushButtonParameters.pushButtonDisablePen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonDisablePen);
                    painter->setBrush(sp->m_PushButtonParameters.pushButtonDisableBrush);
                }
            } else {
                if (on) {
                    if (sunken) {
                        width = sp->m_PushButtonParameters.pushButtonCheckClickPen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonCheckClickPen);
                        painter->setBrush(sp->m_PushButtonParameters.pushButtonCheckClickBrush);
                    } else if (hover) {
                        width = sp->m_PushButtonParameters.pushButtonCheckHoverPen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonCheckHoverPen);
                        painter->setBrush(sp->m_PushButtonParameters.pushButtonCheckHoverBrush);
                    } else {
                        width = sp->m_PushButtonParameters.pushButtonCheckPen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonCheckPen);
                        painter->setBrush(sp->m_PushButtonParameters.pushButtonCheckBrush);
                    }
                } else {
                    if (sunken) {
                        width = sp->m_PushButtonParameters.pushButtonClickPen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonClickPen);
                        painter->setBrush(sp->m_PushButtonParameters.pushButtonClickBrush);
                    } else if (hover) {
                        width = sp->m_PushButtonParameters.pushButtonHoverPen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonHoverPen);
                        painter->setBrush(sp->m_PushButtonParameters.pushButtonHoverBrush);
                    } else {
                        if (button->features & QStyleOptionButton::Flat) {
                            width = 0;
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(Qt::NoBrush);
                        } else {
                            width = sp->m_PushButtonParameters.pushButtonDefaultPen.width();
                            painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonDefaultPen);
                            painter->setBrush(sp->m_PushButtonParameters.pushButtonDefaultBrush);
                        }
                    }
                }
                //focus
                if (focus) {
                    width = sp->m_PushButtonParameters.pushButtonFocusPen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_PushButtonParameters.pushButtonFocusPen);
                }
            }

            painter->setRenderHint(QPainter::Antialiasing, sp->m_PushButtonParameters.radius != 0);
            if (roundButton) {
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawEllipse(rect.adjusted(width, width, -width, -width));
            } else {
                if(width == 1 && sp->m_PushButtonParameters.radius != 0)
                    painter->translate(0.5, 0.5);
                painter->drawRoundedRect(rect.adjusted(width, width, -width, -width),
                                         sp->m_PushButtonParameters.radius, sp->m_PushButtonParameters.radius);
            }
            painter->restore();
            return;

//            //button animation cancel temporary
//            if (sunken || on || animator->isRunning("SunKen") || animator->value("SunKen") == 1.0) {
//                double opacity = animator->value("SunKen").toDouble();
//                if (sunken || on) {
//                    if (opacity == 0.0) {
//                        animator->setAnimatorDirectionForward("SunKen", true);
//                        animator->startAnimator("SunKen");
//                    }
//                } else {
//                    if (opacity == 1.0) {
//                        animator->setAnimatorDirectionForward("SunKen",false);
//                        animator->startAnimator("SunKen");
//                    }
//                }

//                QColor hoverColor, sunkenColor;
//                painter->save();
//                painter->setPen(Qt::NoPen);
//                if (isWindowColoseButton) {
//                    hoverColor = closeButton_Hover(option);
//                    sunkenColor = closeButton_Click(option);
//                } else if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
//                    hoverColor = transparentButton_Hover(option);
//                    sunkenColor = transparentButton_Click(option);
//                } else {
//                    if (isImportant) {
//                        hoverColor = highLight_Hover(option);
//                        sunkenColor = highLight_Click(option);
//                    } else if (useButtonPalette || isWindowButton) {
//                        hoverColor = button_Hover(option);
//                        sunkenColor = button_Click(option);
//                    } else {
//                        hoverColor = highLight_Hover(option);
//                        sunkenColor = highLight_Click(option);
//                    }
//                }
//                painter->setBrush(mixColor(hoverColor, sunkenColor, opacity));
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                painter->drawRoundedRect(option->rect, sp->radius, sp->radius);
//                painter->restore();
//                return;
//            }

//            if (hover || animator->isRunning("MouseOver")
//                    || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")) {
//                double opacity = animator->value("MouseOver").toDouble();
//                if (hover) {
//                    animator->setAnimatorDirectionForward("MouseOver",true);
//                    if (opacity == 0.0) {
//                        animator->startAnimator("MouseOver");
//                    }
//                } else {
//                    animator->setAnimatorDirectionForward("MouseOver",false);
//                    if (opacity == 1.0) {
//                        animator->startAnimator("MouseOver");
//                    }
//                }

//                painter->save();
//                painter->setPen(Qt::NoPen);
//                painter->setOpacity(opacity);
//                if (isWindowColoseButton) {
//                    painter->setBrush(closeButton_Hover(option));
//                } else if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
//                    QColor color = transparentButton_Hover(option);
//                    painter->setBrush(color);
//                } else {
//                    if (isImportant) {
//                        painter->setBrush(highLight_Hover(option));
//                    } else if (useButtonPalette || isWindowButton) {
//                        painter->setBrush(button_Hover(option));
//                    } else {
//                        painter->setBrush(highLight_Hover(option));
//                    }
//                }
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                painter->drawRoundedRect(option->rect, sp->radius, sp->radius);
//                painter->restore();
//                return;
//            }

//            //button inactive state
//            if ((button->state & QStyle::State_Enabled) && !(button->state & QStyle::State_Active)) {
//                painter->save();
//                painter->setPen(Qt::NoPen);
//                painter->setBrush(Qt::red);
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                painter->drawRoundedRect(option->rect, sp->radius, sp->radius);
//                painter->restore();
//            }
//            return;
        }
        break;
    }

    case PE_PanelTipLabel://LINGMO Tip  style: Open ground glass
    {
        if (widget && widget->isEnabled()) {
            sp->initConfigToolTipParameters(isUseDarkPalette(), option, widget);
            if (!KWindowSystem::compositingActive()) {
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                painter->setPen(sp->m_ToolTipParameters.toolTipBackgroundPen);
                painter->setBrush(sp->m_ToolTipParameters.toolTipBackgroundBrush.color());
                painter->drawRect(option->rect);
                painter->restore();
                return;
            }

            QStyleOption opt = *option;

            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            QPainterPath rectPath;
            rectPath.addRoundedRect(option->rect.adjusted(+4, +4, -4, -4), sp->m_ToolTipParameters.radius, sp->m_ToolTipParameters.radius);

            // Draw a black floor
            QPixmap pixmap(option->rect.size());
            pixmap.fill(Qt::transparent);
            QPainter pixmapPainter(&pixmap);
            pixmapPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            pixmapPainter.setPen(Qt::transparent);
            pixmapPainter.setBrush(option->palette.color(QPalette::Active, QPalette::BrightText));
            pixmapPainter.drawPath(rectPath);
            pixmapPainter.end();

            // Blur the black background
            QImage img = pixmap.toImage();
            qt_blurImage(img, 4, false, false);

            // Dig out the center part
            pixmap = QPixmap::fromImage(img);
            QPainter pixmapPainter2(&pixmap);
            pixmapPainter2.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
            pixmapPainter2.setPen(Qt::transparent);
            pixmapPainter2.setBrush(Qt::transparent);
            pixmapPainter2.drawPath(rectPath);

            // Shadow rendering
            painter->drawPixmap(option->rect, pixmap, pixmap.rect());

            //This is the beginning of drawing the bottom of the prompt box
            auto color = sp->m_ToolTipParameters.toolTipBackgroundBrush.color();
            if (LINGMOStyleSettings::isSchemaInstalled("org.lingmo.style")) {
                auto opacity = LINGMOStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
                color.setAlphaF(opacity);
            }

            if (qApp->property("blurEnable").isValid()) {
                bool blurEnable = qApp->property("blurEnable").toBool();
                if (!blurEnable) {
                    color.setAlphaF(1);
                }
            }

            //if blur effect is not supported, do not use transparent color.
            if (!KWindowEffects::isEffectAvailable(KWindowEffects::BlurBehind) || blackAppListWithBlurHelper().contains(qAppName())) {
                color.setAlphaF(1);
            }

            opt.palette.setColor(QPalette::ToolTipBase, color);
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            QPen pen(opt.palette.toolTipBase().color().darker(150), 1);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(Qt::transparent);
            painter->setBrush(color);

            QPainterPath path;
            auto region = widget->mask();
            if (region.isEmpty()) {
                path.addRoundedRect(opt.rect.adjusted(+4, +4, -4, -4), sp->m_ToolTipParameters.radius, sp->m_ToolTipParameters.radius);
            } else {
                path.addRegion(region);
            }

            painter->drawPath(path);
            painter->restore();
            return;
        }
        break;
    }

    case PE_FrameStatusBar://LINGMO Status style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.color(QPalette::Dark));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case PE_IndicatorButtonDropDown: //LINGMO IndicatorButton  style
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing,true);
        painter->setPen(Qt::NoPen);
        if(widget && widget->isEnabled()){
            if (option->state & State_MouseOver) {
                painter->setBrush(option->palette.color(QPalette::Highlight));
                painter->drawRoundedRect(option->rect.left()-4,option->rect.y(),option->rect.width()+4,option->rect.height(), sp->radius, sp->radius);
            }
        }
        painter->restore();
        return;
    }

    case PE_PanelButtonTool:
    {
        sp->initConfigToolButtonPanelParameters(isUseDarkPalette(), option, widget);
//        auto animator = m_button_animation_helper->animator(widget);

        const bool enable = option->state & State_Enabled;
        const bool raise = option->state & State_AutoRaise;
        const bool sunken = option->state & State_Sunken;
        const bool hover = option->state & State_MouseOver;
        const bool on = option->state & State_On;
        const bool focus = option->state & State_HasFocus;
        bool roundButton = false;
        int width = 0;
        QRect rect = option->rect;

        if (widget && widget->property("isRoundButton").isValid()) {
            roundButton = widget->property("isRoundButton").toBool();
        }

        painter->save();
        if (!enable) {
//                if (animator) {
//                    animator->stopAnimator("SunKen");
//                    animator->stopAnimator("MouseOver");
//                }

            if (on) {
                width = sp->m_ToolButtonParameters.toolButtonCheckDisablePen.width();
                painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonCheckDisablePen);
                painter->setBrush(sp->m_ToolButtonParameters.toolButtonCheckDisableBrush);
            }
            else if (raise) {
                width = 0;
                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::NoBrush);
            }
            else {
                width = sp->m_ToolButtonParameters.toolButtonDisablePen.width();
                painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonDisablePen);
                painter->setBrush(sp->m_ToolButtonParameters.toolButtonDisableBrush);
            }
        } else {
            if (on) {
                if (sunken) {
                    width = sp->m_ToolButtonParameters.toolButtonCheckClickPen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonCheckClickPen);
                    painter->setBrush(sp->m_ToolButtonParameters.toolButtonCheckClickBrush);
                } else if (hover) {
                    width = sp->m_ToolButtonParameters.toolButtonCheckHoverPen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonCheckHoverPen);
                    painter->setBrush(sp->m_ToolButtonParameters.toolButtonCheckHoverBrush);
                } else {
                    width = sp->m_ToolButtonParameters.toolButtonCheckPen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonCheckPen);
                    painter->setBrush(sp->m_ToolButtonParameters.toolButtonCheckBrush);
                }
            } else {
                if (sunken) {
                    width = sp->m_ToolButtonParameters.toolButtonClickPen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonClickPen);
                    painter->setBrush(sp->m_ToolButtonParameters.toolButtonClickBrush);
                } else if (hover) {
                    width = sp->m_ToolButtonParameters.toolButtonHoverPen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonHoverPen);
                    painter->setBrush(sp->m_ToolButtonParameters.toolButtonHoverBrush);
                } else {
                    if (raise) {
                        width = 0;
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(Qt::NoBrush);
                    } else {
                        width = sp->m_ToolButtonParameters.toolButtonDefaultPen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonDefaultPen);
                        painter->setBrush(sp->m_ToolButtonParameters.toolButtonDefaultBrush);
                    }
                }
            }
            //focus
            if (focus) {
                width = sp->m_ToolButtonParameters.toolButtonFocusPen.width();
                painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ToolButtonParameters.toolButtonFocusPen);
            }
        }

        painter->setRenderHint(QPainter::Antialiasing, sp->m_ToolButtonParameters.radius != 0);
        if (roundButton) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawEllipse(rect.adjusted(width, width, -width, -width));
        } else {
            if(width == 1 && sp->m_ToolButtonParameters.radius != 0)
                painter->translate(0.5, 0.5);
            painter->drawRoundedRect(rect.adjusted(width, width, -width, -width), sp->m_ToolButtonParameters.radius, sp->m_ToolButtonParameters.radius);
        }
        painter->restore();
        return;


//        if (sunken || on || animator->isRunning("SunKen")
//                || animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen")) {
//            double opacity = animator->value("SunKen").toDouble();
//            if (sunken || on) {
//                if (opacity == 0.0) {
//                    animator->setAnimatorDirectionForward("SunKen", true);
//                    animator->startAnimator("SunKen");
//                }
//            } else {
//                if (animator->currentAnimatorTime("SunKen") == animator->totalAnimationDuration("SunKen")) {
//                    animator->setAnimatorDirectionForward("SunKen", false);
//                    animator->startAnimator("SunKen");
//                }
//            }
//            QColor hoverColor, sunkenColor;
//            painter->save();
//            painter->setPen(Qt::NoPen);
//            if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
//                hoverColor = transparentButton_Hover(option);
//                sunkenColor = transparentButton_Click(option);
//            } else if (isWindowColoseButton) {
//                hoverColor = closeButton_Hover(option);
//                sunkenColor = closeButton_Click(option);
//            } else if (isWindowButton || useButtonPalette){
//                hoverColor = button_Hover(option);
//                sunkenColor = button_Click(option);
//            } else {
//                hoverColor = highLight_Hover(option);
//                sunkenColor = highLight_Click(option);
//            }
//            painter->setBrush(mixColor(hoverColor, sunkenColor, opacity));
//            painter->setRenderHint(QPainter::Antialiasing, true);
//            painter->drawRoundedRect(option->rect, sp->radius, sp->radius);
//            painter->restore();
//            return;
//        }

//        if (hover || animator->isRunning("MouseOver")
//                || animator->currentAnimatorTime("MouseOver") == animator->totalAnimationDuration("MouseOver")) {
//            double opacity = animator->value("MouseOver").toDouble();
//            if (hover) {
//                animator->setAnimatorDirectionForward("MouseOver", true);
//                if(opacity == 0.0) {
//                    animator->startAnimator("MouseOver");
//                }
//            } else {
//                animator->setAnimatorDirectionForward("MouseOver", false);
//                if (opacity == 1.0) {
//                    animator->startAnimator("MouseOver");
//                }
//            }
//            painter->save();
//            painter->setPen(Qt::NoPen);
//            painter->setOpacity(opacity);
//            if (isWindowButton && useTransparentButtonList().contains(qAppName())) {
//                QColor color = transparentButton_Hover(option);
//                painter->setBrush(color);
//            } else if (isWindowColoseButton) {
//                painter->setBrush(closeButton_Hover(option));
//            } else if (isWindowButton || useButtonPalette){
//                painter->setBrush(button_Hover(option));
//            } else {
//                painter->setBrush(highLight_Hover(option));
//            }
//            painter->setRenderHint(QPainter::Antialiasing, true);
//            painter->drawRoundedRect(option->rect, sp->radius, sp->radius);
//            painter->restore();
//            return;
//        }
//        return;
    }

    case PE_IndicatorTabClose:
    {
        QIcon icon = QIcon::fromTheme("window-close-symbolic");
        QColor color = sp->Indicator_IconDefault;
        if (option->state & (State_On | State_Sunken)) {
            color = sp->Indicator_IconSunken;
        }
        else if (option->state & (State_MouseOver)) {
            color = sp->Indicator_IconHover;
        }
        if (!icon.isNull()) {
            int iconSize = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
            QPixmap pixmap = icon.pixmap(QSize(iconSize, iconSize), QIcon::Normal, QIcon::On);
            pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            painter->save();
            drawColorPixmap(painter, color, pixmap);
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            proxy()->drawItemPixmap(painter, option->rect, Qt::AlignCenter, pixmap);
            painter->restore();
        }
        return;
    }

    case PE_FrameTabBarBase:
    {
        if (const QStyleOptionTabBarBase *tbb = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(sp->m_TabWidgetParameters.tabBarBackgroundBrush);
            painter->drawRect(tbb->tabBarRect);
            painter->restore();
            return;
        }
        break;
    }

    case PE_FrameTabWidget: {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            sp->initConfigTabWidgetParameters(isUseDarkPalette(), option, widget);
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(sp->m_TabWidgetParameters.tabWidgetBackgroundBrush);
            painter->drawRect(twf->rect);
            painter->restore();
            return;
        }
        break;
    }

    case PE_FrameGroupBox: //LINGMO GroupBox style:
    {
        /*
        * Remove the style of the bounding box according to the design
        */
        //painter->save();
        //painter->setRenderHint(QPainter::Antialiasing,true);
        //painter->setPen(option->palette.color(QPalette::Base));
        //painter->setBrush(option->palette.color(QPalette::Base));
        //painter->drawRoundedRect(option->rect,4,4);
        //painter->restore();
        return;
    }

    case PE_PanelLineEdit://LINGMO Text edit style
    {
        if (widget) {
//            if (widget->findChild<QAction *>()) {
//                QStyleOption subOption = *option;
//                for (QAction *act : widget->findChildren<QAction *>()) {
//                    act->setIcon(QIcon(HighLightEffect::ordinaryGeneratePixmap(act->icon().pixmap(16, 16), &subOption, widget, HighLightEffect::BothDefaultAndHighlit)));
//                }
//            }
//            if (QAction *clearAction = widget->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"))) {
//                QStyleOption subOption = *option;
//                QColor color = subOption.palette.color(QPalette::Text);
//                color.setAlphaF(1.0);
//                subOption.palette.setColor(QPalette::Text, color);
//                clearAction->setIcon(QIcon(HighLightEffect::ordinaryGeneratePixmap(clearAction->icon().pixmap(16, 16), &subOption, widget, HighLightEffect::BothDefaultAndHighlit)));
//            }
        }

        // Conflict with qspinbox and so on, The widget text cannot use this style
        if (widget) {
            if (widget->parentWidget())
                if (widget->parentWidget()->inherits("QDoubleSpinBox") || widget->parentWidget()->inherits("QSpinBox")
                        || widget->parentWidget()->inherits("QComboBox") || widget->parentWidget()->inherits("QDateTimeEdit"))
                {
                    return;
                }
        }

        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            sp->initConfigLineEditParameters(isUseDarkPalette(), option, widget);
            const bool enable = f->state & State_Enabled;
            const bool hover = f->state & State_MouseOver;
            const bool focus = f->state & State_HasFocus;
            int width = 0;

            if (!enable) {
                width = sp->m_LineEditParameters.lineEditDisablePen.width();
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, sp->m_LineEditParameters.radius != 0);
                if(width == 1 && sp->m_LineEditParameters.radius != 0)
                    painter->translate(0.5, 0.5);
                painter->setPen(sp->m_LineEditParameters.lineEditDisablePen);
                painter->setBrush(sp->m_LineEditParameters.lineEditDisableBrush);
                painter->drawRoundedRect(option->rect.adjusted(((width%2) == 1 ? (width - 1)/2 : width/2),
                                                               ((width%2) == 1 ? (width - 1)/2 : width/2),
                                                               -1 * ((width%2) == 1 ? (width + 1)/2 : width/2),
                                                               -1 * ((width%2) == 1 ? (width + 1)/2 : width/2)),
                                         sp->m_LineEditParameters.radius, sp->m_LineEditParameters.radius);
                painter->restore();
                return;
            }

//            //read only mode
//            if (f->state & State_ReadOnly) {
//                painter->save();
//                painter->setRenderHint(QPainter::Antialiasing, true);
//                painter->setPen(Qt::NoPen);
//                painter->setBrush(f->palette.brush(QPalette::Active, QPalette::Button));
//                painter->drawRoundedRect(option->rect, sp->radius, sp->radius);
//                painter->restore();
//                return;
//            }

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, sp->m_LineEditParameters.radius != 0);
            if (focus) {
                width = sp->m_LineEditParameters.lineEditFocusPen.width();
                painter->setPen(sp->m_LineEditParameters.lineEditFocusPen);
                painter->setBrush(sp->m_LineEditParameters.lineEditFocusBrush);
            } else if (hover) {
                width = sp->m_LineEditParameters.lineEditHoverPen.width();
                painter->setPen(sp->m_LineEditParameters.lineEditHoverPen);
                painter->setBrush(sp->m_LineEditParameters.lineEditHoverBrush);
            } else {
                width = sp->m_LineEditParameters.lineEditDefaultPen.width();
                painter->setPen(sp->m_LineEditParameters.lineEditDefaultPen);
                painter->setBrush(sp->m_LineEditParameters.lineEditDefaultBrush);
            }
            if(width == 1 && sp->m_LineEditParameters.radius != 0)
                painter->translate(0.5, 0.5);
            painter->drawRoundedRect(option->rect.adjusted(((width%2) == 1 ? (width - 1)/2 : width/2),
                                                           ((width%2) == 1 ? (width - 1)/2 : width/2),
                                                           -1 * ((width%2) == 1 ? (width + 1)/2 : width/2),
                                                           -1 * ((width%2) == 1 ? (width + 1)/2 : width/2)),
                                     sp->m_LineEditParameters.radius, sp->m_LineEditParameters.radius);

            painter->restore();

            return;
        }
        break;
    }

    case PE_IndicatorArrowUp:
    {
        QIcon icon = QIcon::fromTheme("lingmo-up-symbolic");
        if(sp->m_isTableMode){
            if(qobject_cast<const QSpinBox *>(widget))
            {
                icon = QIcon::fromTheme("list-add-symbolic");
            }
            else if(qobject_cast<const QDoubleSpinBox *>(widget))
            {
                icon = QIcon::fromTheme("list-add-symbolic");
            }
        }

        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
            QPixmap target;
            if(widget != nullptr && (widget->inherits("QDoubleSpinBox") || widget->inherits("QSpinBox")) && !sp->m_SpinBoxParameters.spinBoxUpIconHightPixMap)
                target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            else if(!sp->indicatorIconHoverNeedHighLight)
                target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            else
                target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowDown:
    {
        double rotate = 0.0;
        bool startAnimation = false;
        if(auto treeview = qobject_cast<const QTreeView *>(widget)){
            if(!m_tree_animation_helper)
                break;
            auto animator = m_tree_animation_helper->animator(treeview);
            if(animator){
                if(animator && animator->isRunning("expand") &&
                        treeview->indexAt(option->rect.center()) == animator->getExtraProperty("expandItem").toModelIndex()) {

                    rotate = animator->value("expand").toDouble();

                    if(animator->value("expand").toDouble() == 1){
                        animator->setAnimatorCurrentTime("expand", 0);
                        animator->stopAnimator("expand");
                        }
                    startAnimation = true;
//                    qDebug() << "PE_IndicatorArrowDown expand...." << rotate;
                }
            }
        }
        QIcon icon = QIcon::fromTheme("lingmo-down-symbolic");
        if(sp->m_isTableMode){
            if(qobject_cast<const QSpinBox *>(widget))
            {
                icon = QIcon::fromTheme("list-remove-symbolic");
            }
            else if(qobject_cast<const QDoubleSpinBox *>(widget))
            {
                icon = QIcon::fromTheme("list-remove-symbolic");
            }
        }

        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
            QPixmap target;
            if(widget != nullptr && (widget->inherits("QDoubleSpinBox") || widget->inherits("QSpinBox")) && !sp->m_SpinBoxParameters.spinBoxUpIconHightPixMap)
                            target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            else if(!sp->indicatorIconHoverNeedHighLight)
                target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            else if(widget && (widget->inherits("QTreeView") || widget->inherits("QTreeWidget"))){
                if(widget->property("highlightMode").isValid() && widget->property("highlightMode").toBool())
                    target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                else
                    target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            }
            else
                target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);

            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            if(startAnimation){
                QMatrix matrix;
                int i = 90.0 * (rotate - 1.0);
                matrix.rotate(i);

                target = target.scaled(iconsize.width(), iconsize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//                painter->rotate(i);
                auto pix = target.transformed(matrix);
                QRect rect = QRect(drawRect.x() - (pix.rect().width() - drawRect.width()) / 2, drawRect.y() - (pix.rect().height() - drawRect.height()) / 2,
                                   pix.rect().width(), pix.rect().height());
                pix = pix.scaled(rect.width(), rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                painter->drawPixmap(rect, pix);
            }
            else
                painter->drawPixmap(drawRect, target);

            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowRight:
    {
        double rotate = 0.0;
        bool startAnimation = false;
        if(auto treeview = qobject_cast<const QTreeView *>(widget)){
            if(!m_tree_animation_helper)
                break;
            auto animator = m_tree_animation_helper->animator(treeview);
            if (animator) {
                if(animator && animator->isRunning("collaps") &&
                        treeview->indexAt(option->rect.center()) == animator->getExtraProperty("collapsItem").toModelIndex()) {

                        rotate = animator->value("collaps").toDouble();
                        if(animator->value("collaps").toDouble() == 1){
                            animator->setAnimatorCurrentTime("collaps", 0);
                            animator->stopAnimator("collaps");
                            }
                        startAnimation = true;
//                        qDebug() << "PE_IndicatorArrowRight expand...." << rotate;
                }
            }
        }

        QIcon icon = QIcon::fromTheme("lingmo-end-symbolic");
        if(widget && widget->parent() && widget->parentWidget()->inherits("QTabBar") &&
                sp->getTabBarIndicatorLayout() != LINGMOConfigStyleParameters::TabBarIndicatorLayout::TabBarIndicator_Horizontal)
            icon = QIcon::fromTheme("lingmo-down-symbolic");

        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);

            if(widget && (widget->inherits("QDoubleSpinBox") || widget->inherits("QSpinBox")) && !sp->m_SpinBoxParameters.spinBoxUpIconHightPixMap)
                target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            else if(!sp->indicatorIconHoverNeedHighLight && widget && !widget->inherits("QMenu"))
                target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            else if(widget && (widget->inherits("QTreeView") || widget->inherits("QTreeWidget"))){
                if(widget->property("highlightMode").isValid() && widget->property("highlightMode").toBool())
                    target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                else
                    target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            }
            else
                target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);

            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            if(startAnimation){
                QMatrix matrix;
                int i = 90.0 * (1.0 - rotate);
//                qDebug() << "PE_IndicatorArrowRight i........." << i;
                matrix.rotate(i);

                target = target.scaled(iconsize.width(), iconsize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//                painter->rotate(i);
                auto pix = target.transformed(matrix);
                QRect rect = QRect(drawRect.x() - (pix.rect().width() - drawRect.width()) / 2, drawRect.y() - (pix.rect().height() - drawRect.height()) / 2,
                                   pix.rect().width(), pix.rect().height());
                pix = pix.scaled(rect.width(), rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                painter->drawPixmap(rect, pix);
            }
            else
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorArrowLeft:
    {
        QIcon icon = QIcon::fromTheme("lingmo-start-symbolic");
        if(widget && widget->parent() && widget->parentWidget()->inherits("QTabBar") &&
                sp->getTabBarIndicatorLayout() != LINGMOConfigStyleParameters::TabBarIndicatorLayout::TabBarIndicator_Horizontal)
            icon = QIcon::fromTheme("lingmo-up-symbolic");

        if(!icon.isNull()) {
            int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            QSize iconsize(indWidth, indHight);
            QRect drawRect(option->rect.topLeft(), iconsize);
            drawRect.moveCenter(option->rect.center());
            const bool enable(option->state & State_Enabled);
            QIcon::Mode mode =  enable ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = icon.pixmap(iconsize, mode, QIcon::Off);
            pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
            QPixmap target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
            if(!sp->indicatorIconHoverNeedHighLight && widget && !widget->inherits("QMenu"))
                target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            painter->drawPixmap(drawRect, target);
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorRadioButton:
    {
        if (const QStyleOptionButton* radiobutton = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            sp->initConfigRadioButtonParameters(isUseDarkPalette(), option, widget);

            QRectF rect = radiobutton->rect.adjusted(1, 1, -1, -1);
            bool enable = radiobutton->state & State_Enabled;
            bool mouseOver = radiobutton->state & State_MouseOver;
            bool sunKen = radiobutton->state & State_Sunken;
            bool on = radiobutton->state & State_On;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            auto animator = m_radiobutton_animation_helper->animator(widget);

            bool hasAnimation = widget &&(!widget->property("animation").isValid() ||
                                          (widget->property("animation").isValid() && widget->property("animation").toBool()));
            if (enable && hasAnimation &&
                    animator != nullptr && animator->isRunning("SunKenOn")) {
                float value = animator->value("SunKenOn").toFloat();
                QColor clickColor = sp->m_RadioButtonParameters.radioButtonClickBrush.color();
                QColor onDefaultPenColor = mouseOver ? sp->m_RadioButtonParameters.radioButtonOnHoverPen.color() :
                                                       sp->m_RadioButtonParameters.radioButtonOnDefaultPen.color();
                QColor onDefaultColor = mouseOver ? sp->m_RadioButtonParameters.radioButtonOnClickBrush.color() :
                                                    sp->m_RadioButtonParameters.radioButtonOnDefaultBrush.color();
                QColor frameColor = mouseOver ? sp->m_RadioButtonParameters.radioButtonHoverPen.color() :
                                                sp->m_RadioButtonParameters.radioButtonDefaultPen.color();
                QBrush brush(QColor(clickColor.red() + value * (onDefaultColor.red() - clickColor.red()),
                                    clickColor.green() + value * (onDefaultColor.green() - clickColor.green()),
                                    clickColor.blue() + value * (onDefaultColor.blue() - clickColor.blue()),
                                    clickColor.alpha() + value * (onDefaultColor.alpha() - clickColor.alpha())));
                QBrush frameBrush(QColor(frameColor.red() + value * (onDefaultPenColor.red() - frameColor.red()),
                                         frameColor.green() + value * (onDefaultPenColor.green() - frameColor.green()),
                                         frameColor.blue() + value * (onDefaultPenColor.blue() - frameColor.blue()),
                                         frameColor.alpha() + value * (onDefaultPenColor.alpha() - frameColor.alpha())));
                QColor penColor = frameBrush.color();
                penColor.setAlphaF(onDefaultPenColor.alphaF());
                painter->setPen(QPen(QBrush(penColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

                painter->setBrush(brush);
                painter->drawEllipse(rect);
                painter->restore();
                painter->save();

                QRectF childRect(rect.x(), rect.y(), value * 1.0 * ( sp->m_RadioButtonParameters.childrenRadius), value * 1.0 * ( sp->m_RadioButtonParameters.childrenRadius));
                childRect.moveCenter(rect.center());
                painter->setPen(Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setBrush(sp->m_RadioButtonParameters.radioButtonChildrenOnDefaultBrush);
                painter->drawEllipse(childRect);
                painter->restore();
                return;
            }
            else if (enable && hasAnimation &&
                     animator != nullptr && animator->isRunning("Off")) {
                float value = animator->value("Off").toFloat();
                QPen pen = sp->m_RadioButtonParameters.radioButtonDefaultPen;
                pen.color().setAlphaF(sp->m_RadioButtonParameters.radioButtonDefaultPen.color().alphaF() * (1.0 - value));
                painter->setPen(pen);
                QColor defaultColor = sp->m_RadioButtonParameters.radioButtonDefaultBrush.color();
                QColor onDefaultColor = sp->m_RadioButtonParameters.radioButtonOnDefaultBrush.color();
                QBrush brush(QColor(defaultColor.red() + value * (onDefaultColor.red() - defaultColor.red()),
                                    defaultColor.green() + value * (onDefaultColor.green() - defaultColor.green()),
                                    defaultColor.blue() + value * (onDefaultColor.blue() - defaultColor.blue()),
                                    defaultColor.alpha() + value * (onDefaultColor.alpha() - defaultColor.alpha())));

                painter->setBrush(brush);
                painter->drawEllipse(rect);
                QRectF childRect(rect.x(), rect.y(), ( sp->m_RadioButtonParameters.childrenRadius) * (value) * 1.0, (value) * 1.0 * ( sp->m_RadioButtonParameters.childrenRadius));
                childRect.moveCenter(rect.center());
                painter->setPen(Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setBrush(sp->m_RadioButtonParameters.radioButtonChildrenOnDefaultBrush);
                painter->drawEllipse(childRect);
                painter->restore();
                return;
            }

            if (enable) {
                if (on) {
                    if (animator != nullptr) {
                        if (hasAnimation && !animator->isRunning("SunKenOn") ) {
                            if(animator->currentAnimatorTime("SunKenOn") == 0){
                                animator->startAnimator("SunKenOn");
                                animator->setAnimatorCurrentTime("Off", 0);
                                painter->setPen(sp->m_RadioButtonParameters.radioButtonDefaultPen);
                                painter->setBrush(sp->m_RadioButtonParameters.radioButtonDefaultBrush);
                                painter->drawEllipse(rect);
                                painter->restore();
                                return;
                            }
                        }
                    }
                    if (sunKen) {
                        painter->setPen(sp->m_RadioButtonParameters.radioButtonOnClickPen);
                        painter->setBrush(sp->m_RadioButtonParameters.radioButtonOnClickBrush);
                    } else if (mouseOver) {
                        painter->setPen(sp->m_RadioButtonParameters.radioButtonOnHoverPen);
                        painter->setBrush(sp->m_RadioButtonParameters.radioButtonOnHoverBrush);
                    } else {
                        painter->setPen(sp->m_RadioButtonParameters.radioButtonOnDefaultPen);
                        painter->setBrush(sp->m_RadioButtonParameters.radioButtonOnDefaultBrush);
                    }
                    painter->drawEllipse(rect);
                    painter->restore();
                    painter->save();
                    QRectF childRect(rect.x(), rect.y(), sp->m_RadioButtonParameters.childrenRadius, sp->m_RadioButtonParameters.childrenRadius);
                    childRect.moveCenter(rect.center());
                    painter->setPen(Qt::NoPen);
                    painter->setRenderHint(QPainter::Antialiasing);
                    painter->setBrush(sp->m_RadioButtonParameters.radioButtonChildrenOnDefaultBrush);
                    painter->drawEllipse(childRect);

                    painter->restore();
                    return;
                } else {
                    if (hasAnimation && animator != nullptr) {
                        if (!animator->isRunning("Off") && animator->currentAnimatorTime("SunKenOn") > 0 && animator->currentAnimatorTime("Off") == 0) {
                            animator->startAnimator("Off");
                            animator->setAnimatorCurrentTime("SunKenOn", 0);
                            painter->restore();
                            return;
                        }
                    }
                    if (sunKen) {
                        painter->setPen(sp->m_RadioButtonParameters.radioButtonClickPen);
                        painter->setBrush(sp->m_RadioButtonParameters.radioButtonClickBrush);
                    } else if (mouseOver) {
                        painter->setPen(sp->m_RadioButtonParameters.radioButtonHoverPen);
                        painter->setBrush(sp->m_RadioButtonParameters.radioButtonHoverBrush);
                    } else {
                        painter->setPen(sp->m_RadioButtonParameters.radioButtonDefaultPen);
                        painter->setBrush(sp->m_RadioButtonParameters.radioButtonDefaultBrush);
                    }
                    painter->drawEllipse(rect);
                }
            } else {
                painter->setPen(sp->m_RadioButtonParameters.radioButtonDisablePen);
                painter->setBrush(sp->m_RadioButtonParameters.radioButtonDisableBrush);
                painter->drawEllipse(rect);

                if (on) {
                    QRectF childRect(rect.x(), rect.y(), sp->m_RadioButtonParameters.childrenRadius, sp->m_RadioButtonParameters.childrenRadius);
                    childRect.moveCenter(rect.center());
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(sp->m_RadioButtonParameters.radioButtonChildrenOnDisableBrush);
                    painter->drawEllipse(childRect);
                }
            }
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorCheckBox:
    {
        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            sp->initConfigCheckBoxParameters(isUseDarkPalette(), option, widget);
            bool enable = checkbox->state & State_Enabled;
            bool mouseOver = checkbox->state & State_MouseOver;
            bool sunKen = checkbox->state & State_Sunken;
            bool on = checkbox->state & State_On;
            bool noChange = checkbox->state & State_NoChange;

            QRectF rect = checkbox->rect;
            QRectF pathrect = rect.adjusted(1, 1, -1, -1);
            double width = pathrect.width();
            double heigth = pathrect.height();
            int margin = 0;

            QPainterPath path;
            painter->save();
            painter->setClipRect(rect);
            painter->setRenderHint(QPainter::Antialiasing, sp->m_CheckBoxParameters.radius != 0);

            bool hasAnimation = widget &&(!widget->property("animation").isValid() ||
                                          (widget->property("animation").isValid() && widget->property("animation").toBool()));

            auto animator = m_checkbox_animation_helper->animator(widget);
            if (hasAnimation && animator != nullptr) {
                if(animator->isRunning("OnBase"))
                {
                    float value = animator->value("OnBase").toFloat();
                    margin = sp->m_CheckBoxParameters.checkBoxDefaultPen.width();

                    QColor onDefaultColor = mouseOver ? (!sp->m_CheckBoxParameters.onHoverBrushIslinearGradient ?
                                                             sp->m_CheckBoxParameters.checkBoxOnHoverBrush.color() :
                                                             sp->m_CheckBoxParameters.checkBoxOnDefaultBrush.color()) :
                                                        sp->m_CheckBoxParameters.checkBoxOnDefaultBrush.color();
                    QColor clickColor = sp->m_CheckBoxParameters.checkBoxClickBrush.color();
                    QColor frameColor = sp->m_CheckBoxParameters.checkBoxClickPen.color();//checkBoxHoverPen.color();
                    QColor onDefaultPenColor = mouseOver ? sp->m_CheckBoxParameters.checkBoxOnHoverPen.color() : sp->m_CheckBoxParameters.checkBoxOnDefaultPen.color();
                    QBrush brush(QColor(clickColor.red() + value * (onDefaultColor.red() - clickColor.red()),
                                        clickColor.green() + value * (onDefaultColor.green() - clickColor.green()),
                                        clickColor.blue() + value * (onDefaultColor.blue() - clickColor.blue()),
                                        clickColor.alpha() + value * (onDefaultColor.alpha() - clickColor.alpha())));

                    QBrush frameBrush(QColor(frameColor.red() + value * (onDefaultPenColor.red() - frameColor.red()),
                                             frameColor.green() + value * (onDefaultPenColor.green() - frameColor.green()),
                                             frameColor.blue() + value * (onDefaultPenColor.blue() - frameColor.blue()),
                                             frameColor.alpha() + value * (onDefaultPenColor.alpha() - frameColor.alpha())));

                    QColor penColor = frameBrush.color();
                    painter->setPen(QPen(QBrush(penColor), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));//sp->m_CheckBoxParameters.checkBoxDefaultPen);
                    painter->setBrush(brush);

                    if(painter->pen().width() == 1 && sp->m_CheckBoxParameters.radius != 0)
                        painter->translate(0.5, 0.5);
                    painter->drawRoundedRect(rect.adjusted(margin, margin, -margin, -margin), sp->m_CheckBoxParameters.radius, sp->m_CheckBoxParameters.radius);
                    painter->restore();
                    painter->save();
                }
                else if (animator->isRunning("OnScale")) {
                    margin = sp->m_CheckBoxParameters.checkBoxOnDefaultPen.width();
                    painter->setBrush(mouseOver ? sp->m_CheckBoxParameters.checkBoxOnHoverBrush : sp->m_CheckBoxParameters.checkBoxOnDefaultBrush);
                    painter->setPen(mouseOver ? sp->m_CheckBoxParameters.checkBoxOnHoverPen.color() : sp->m_CheckBoxParameters.checkBoxOnDefaultPen.color());
                    if(painter->pen().width() == 1 && sp->m_CheckBoxParameters.radius != 0)
                        painter->translate(0.5, 0.5);
                    painter->drawRoundedRect(rect.adjusted(margin, margin, -margin, -margin), sp->m_CheckBoxParameters.radius, sp->m_CheckBoxParameters.radius);
                    painter->restore();
                    painter->save();
                }
            }


            if (animator != nullptr) {
                if(hasAnimation && animator->isRunning("OnOpacity"))
                {
                    float value = animator->value("OnOpacity").toFloat();

                    if (on) {
                        QBrush brush = sp->m_CheckBoxParameters.checkBoxContentPen.brush();
                        QColor color = brush.color();
                        color.setAlphaF(value);
                        brush.setColor(color);
                        painter->setPen(QPen(brush, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    } else if (noChange) {
                        QBrush brush = sp->m_CheckBoxParameters.checkBoxContentPen.brush();
                        QColor color = brush.color();
                        color.setAlphaF(value);
                        brush.setColor(color);
                        painter->setPen(QPen(brush, 2));
                    }
                }
                else{
                    if (on) {
                        painter->setPen(QPen(sp->m_CheckBoxParameters.checkBoxContentPen.brush(), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    } else if (noChange) {
                        painter->setPen(QPen(sp->m_CheckBoxParameters.checkBoxContentPen.brush(), 2));
                    }
                }
            }

            if (hasAnimation && animator != nullptr && animator->isRunning("OnScale")) {
                float value = animator->value("OnScale").toFloat();
                if (on) {
                    path.moveTo(pathrect.left() + width / 2 - 0.4 *  width/4.0 - value * 0.6 * width /4.0, pathrect.top() + heigth / 2);
                    path.lineTo(pathrect.left() + width * 0.45, pathrect.bottom() - heigth / 3 - (1.0 - value) * heigth /8);
                    path.lineTo(pathrect.right() - width / 4  - (1.0 - value) * 0.6 * width /4.0, pathrect.top() + heigth / 3 + (1.0 - value) * heigth /6);
                    animator->setExtraProperty("nochange", false);
                } else if (noChange) {
                    path.moveTo(pathrect.left() + width / 2 - 0.4 *  width/4.0 - value * 0.6 * width /4.0, pathrect.center().y() - 0.5);
                    path.lineTo(pathrect.right() - width / 4 - (1.0 - value) * 0.6 * width /4.0 , pathrect.center().y() - 0.5);
                    animator->setExtraProperty("nochange", true);
                }
                painter->setBrush(Qt::NoBrush);
                painter->drawPath(path);
                painter->restore();
                return;
            }

            if(path.isEmpty())
            {
                if (on) {
                    path.moveTo(pathrect.left() + width / 4, pathrect.top() + heigth / 2);
                    path.lineTo(pathrect.left() + width * 0.45, pathrect.bottom() - heigth / 3);
                    path.lineTo(pathrect.right() - width / 4, pathrect.top() + heigth / 3);
                    if(animator != nullptr)
                        animator->setExtraProperty("nochange", false);
                } else if (noChange) {
                    path.moveTo(pathrect.left() + width / 4, pathrect.center().y());
                    path.lineTo(pathrect.right() - width / 4, pathrect.center().y());
                    if(animator != nullptr)
                        animator->setExtraProperty("nochange", true);
                }
            }

            if (hasAnimation && animator != nullptr && animator->isRunning("Off")) {
                float value = animator->value("Off").toFloat();

                margin = sp->m_CheckBoxParameters.checkBoxDefaultPen.width();
                QPen pen = mouseOver ? sp->m_CheckBoxParameters.checkBoxHoverPen : sp->m_CheckBoxParameters.checkBoxDefaultPen;
                pen.color().setAlphaF(sp->m_CheckBoxParameters.checkBoxDefaultPen.color().alphaF() * (1.0 - value));
                painter->setPen(pen);
                QColor defaultColor = mouseOver ? (sp->m_CheckBoxParameters.onHoverBrushIslinearGradient ?
                                                   sp->m_CheckBoxParameters.checkBoxClickBrush.color() : sp->m_CheckBoxParameters.checkBoxDefaultBrush.color()):
                                                  sp->m_CheckBoxParameters.checkBoxDefaultBrush.color();
                QColor onDefaultColor = mouseOver ? sp->m_CheckBoxParameters.checkBoxOnClickBrush.color() : sp->m_CheckBoxParameters.checkBoxOnClickBrush.color();
                QBrush brush(QColor(defaultColor.red() + value * (onDefaultColor.red() - defaultColor.red()),
                                    defaultColor.green() + value * (onDefaultColor.green() - defaultColor.green()),
                                    defaultColor.blue() + value * (onDefaultColor.blue() - defaultColor.blue()),
                                    defaultColor.alpha() + value * (onDefaultColor.alpha() - defaultColor.alpha())));
                painter->setBrush(brush);
                if(painter->pen().width() == 1 && sp->m_CheckBoxParameters.radius != 0)
                    painter->translate(0.5, 0.5);
                painter->drawRoundedRect(rect.adjusted(margin, margin, -margin, -margin), sp->m_CheckBoxParameters.radius, sp->m_CheckBoxParameters.radius);
                painter->restore();
                painter->save();
                bool _nochange = animator->getExtraProperty("noChange").toBool();
                if (!_nochange) {
                    path.moveTo(pathrect.left() + width / 2 - 0.4 *  width/4.0 - value * 0.6 * width /4.0, pathrect.top() + heigth / 2);
                    path.lineTo(pathrect.left() + width * 0.45, pathrect.bottom() - heigth / 3 - (1.0 - value) * heigth /8);
                    path.lineTo(pathrect.right() - width / 4  - (1.0 - value) * 0.6 * width /4.0, pathrect.top() + heigth / 3 + (1.0 - value) * heigth /6);

                    QBrush brush = sp->m_CheckBoxParameters.checkBoxContentPen.brush();
                    QColor color = brush.color();
                    color.setAlphaF(value);
                    brush.setColor(color);
                    painter->setPen(QPen(brush, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    painter->setRenderHint(QPainter::Antialiasing, true);
                } else if (_nochange) {

                    path.moveTo(pathrect.left() + width / 2 - 0.4 *  width/4.0 - value * 0.6 * width /4.0, pathrect.center().y());
                    path.lineTo(pathrect.right() - width / 4 - (1.0 - value) * 0.6 * width /4.0 , pathrect.center().y());

                    QBrush brush = sp->m_CheckBoxParameters.checkBoxContentPen.brush();
                    QColor color = brush.color();
                    color.setAlphaF(value);
                    brush.setColor(color);
                    painter->setPen(QPen(brush, 2));
                }

                painter->setBrush(Qt::NoBrush);
                painter->drawPath(path);
                painter->restore();
                return;
            }

            if (enable) {
                if (on | noChange) {
                    if(hasAnimation && animator != nullptr)
                    {
                        if(!animator->isRunning("OnBase") && animator->currentAnimatorTime("OnBase") == 0)
                        {
                            animator->startAnimator("OnBase");
                        }
                        if(!animator->isRunning("OnOpacity") && animator->currentAnimatorTime("OnOpacity") == 0)
                        {
                            animator->startAnimator("OnOpacity");
                        }
                        if(!animator->isRunning("OnScale") && animator->currentAnimatorTime("OnScale") == 0)
                        {
                            animator->startAnimator("OnScale");
                            margin = sp->m_CheckBoxParameters.checkBoxDefaultPen.width();
                            painter->setPen(sp->m_CheckBoxParameters.checkBoxClickPen);
                            painter->setBrush(sp->m_CheckBoxParameters.checkBoxClickBrush);

                            if(painter->pen().width() == 1 && sp->m_CheckBoxParameters.radius != 0)
                                painter->translate(0.5, 0.5);
                            painter->drawRoundedRect(rect.adjusted(margin, margin, -margin, -margin), sp->m_CheckBoxParameters.radius, sp->m_CheckBoxParameters.radius);
                            painter->restore();
                            animator->setAnimatorCurrentTime("Off", 0);
                            animator->setExtraProperty("nochange", false);
                            return;
                        }
                    }
                    if (sunKen) {
                        margin = sp->m_CheckBoxParameters.checkBoxOnClickPen.width();
                        painter->setPen(sp->m_CheckBoxParameters.checkBoxOnClickPen);
                        painter->setBrush(sp->m_CheckBoxParameters.checkBoxOnClickBrush);
                    } else if (mouseOver) {
                        margin = sp->m_CheckBoxParameters.checkBoxOnHoverPen.width();
                        painter->setPen(sp->m_CheckBoxParameters.checkBoxOnHoverPen);
                        painter->setBrush(sp->m_CheckBoxParameters.checkBoxOnHoverBrush);
                    } else {
                        margin = sp->m_CheckBoxParameters.checkBoxOnDefaultPen.width();
                        painter->setPen(sp->m_CheckBoxParameters.checkBoxOnDefaultPen);
                        painter->setBrush(sp->m_CheckBoxParameters.checkBoxOnDefaultBrush);
                    }
                    if(painter->pen().width() == 1 && sp->m_CheckBoxParameters.radius != 0)
                        painter->translate(0.5, 0.5);
                    painter->drawRoundedRect(rect.adjusted(margin, margin, -margin, -margin),
                                             sp->m_CheckBoxParameters.radius, sp->m_CheckBoxParameters.radius);
                    painter->restore();
                    painter->save();
                    //draw path
                    if (on) {
                        painter->setPen(QPen(sp->m_CheckBoxParameters.checkBoxContentPen.color(), 1.5,
                                             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                        painter->setRenderHint(QPainter::Antialiasing, true);
                    } else if (noChange) {
                        painter->setPen(QPen(sp->m_CheckBoxParameters.checkBoxContentPen.color(), 1.5,
                                             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPath(path);
                } else {
                    if (animator != nullptr) {
                        if(hasAnimation && !animator->isRunning("Off") &&
                                animator->currentAnimatorTime("Off") == 0 &&
                                animator->currentAnimatorTime("OnScale") > 0 &&
                                animator->currentAnimatorTime("OnOpacity") > 0 &&
                                animator->currentAnimatorTime("OnBase") > 0) {
                            animator->startAnimator("Off");
                            animator->setAnimatorCurrentTime("OnScale", 0);
                            animator->setAnimatorCurrentTime("OnOpacity", 0);
                            animator->setAnimatorCurrentTime("OnBase", 0);
                            painter->restore();
                            return;
                        }
                    }
                    if (sunKen) {
                        margin = sp->m_CheckBoxParameters.checkBoxClickPen.width();
                        painter->setPen(sp->m_CheckBoxParameters.checkBoxClickPen);
                        painter->setBrush(sp->m_CheckBoxParameters.checkBoxClickBrush);
                    } else if (mouseOver) {
                        margin = sp->m_CheckBoxParameters.checkBoxHoverPen.width();
                        painter->setPen(sp->m_CheckBoxParameters.checkBoxHoverPen);
                        painter->setBrush(sp->m_CheckBoxParameters.checkBoxHoverBrush);
                    } else {
                        margin = sp->m_CheckBoxParameters.checkBoxDefaultPen.width();
                        painter->setPen(sp->m_CheckBoxParameters.checkBoxDefaultPen);
                        painter->setBrush(sp->m_CheckBoxParameters.checkBoxDefaultBrush);
                    }
                    if(painter->pen().width() == 1 && sp->m_CheckBoxParameters.radius != 0)
                        painter->translate(0.5, 0.5);
                    painter->drawRoundedRect(rect.adjusted(margin, margin, -margin, -margin),
                                             sp->m_CheckBoxParameters.radius, sp->m_CheckBoxParameters.radius);
                }
            } else {
                margin = sp->m_CheckBoxParameters.checkBoxDisablePen.width();
                painter->setPen(sp->m_CheckBoxParameters.checkBoxDisablePen);
                painter->setBrush(sp->m_CheckBoxParameters.checkBoxDisableBrush);
                if(painter->pen().width() == 1 && sp->m_CheckBoxParameters.radius != 0)
                    painter->translate(0.5, 0.5);
                painter->drawRoundedRect(rect.adjusted(margin, margin, -margin, -margin),
                                         sp->m_CheckBoxParameters.radius, sp->m_CheckBoxParameters.radius);
                painter->restore();
                painter->save();
                if (on | noChange) {
                    if (on) {
                        painter->setPen(QPen(checkbox->palette.brush(QPalette::Disabled, QPalette::ButtonText), 1.5,
                                             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    } else if (noChange) {
                        painter->setPen(QPen(checkbox->palette.brush(QPalette::Disabled, QPalette::ButtonText), 2));
                    }
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPath(path);
                }
            }
            painter->restore();
            return;
        }
        break;
    }

    case PE_IndicatorTabTearLeft:
        return;

    case PE_IndicatorTabTearRight:
        return;

    case PE_PanelScrollAreaCorner:
    {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(option->palette.brush(QPalette::Active, QPalette::Base));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case PE_IndicatorHeaderArrow:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            if (header->sortIndicator == QStyleOptionHeader::SortDown)
                return proxy()->drawPrimitive(PE_IndicatorArrowDown, header, painter, widget);
            if (header->sortIndicator == QStyleOptionHeader::SortUp)
                return proxy()->drawPrimitive(PE_IndicatorArrowUp, header, painter, widget);
        }
        break;
    }

    case PE_PanelItemViewRow:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            QPalette::ColorGroup cg = (widget ? widget->isEnabled() : (vi->state & QStyle::State_Enabled))
                    ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(vi->state & QStyle::State_Active))
                cg = QPalette::Inactive;

            int Radius = 0;//sp->radius;
            bool isTree = false;

            if (qobject_cast<const QTreeView*>(widget) || qobject_cast<const QTreeWidget*>(widget)) {
                isTree = true;
            }

            if (isTree) {
                if (proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, option, widget)) {
                    if (vi->features & QStyleOptionViewItem::Alternate) {
                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(vi->palette.brush(cg, QPalette::AlternateBase));
                        painter->drawRect(vi->rect);
                        painter->restore();
                    }
                }
            } else if (vi->features & QStyleOptionViewItem::Alternate) {
                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(vi->palette.brush(cg, QPalette::AlternateBase));
                painter->drawRoundedRect(vi->rect, Radius, Radius);
                painter->restore();
            }

            if (dynamic_cast<QPixmap *>(painter->device())) {
                proxy()->drawPrimitive(PE_PanelItemViewItem, option, painter, widget);
            }
            return;
        }
        break;
    }

    case PE_PanelItemViewItem:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            bool isIconMode = false;
            const bool enable = vi->state & State_Enabled;
            const bool select = vi->state & State_Selected;
            const bool hover = vi->state & State_MouseOver;
            bool focus = vi->state & State_HasFocus;

            int focusRadius;
            QBrush defaultBrush;
            QBrush hoverBrush;
            QBrush selectBrush;
            QBrush disableBrush;
            QPainterPath path;
            QPainterPath rectPath;


            QPen hoverPen = Qt::NoPen;
            QPen focusPen = Qt::NoPen;
            QPen selectPen = Qt::NoPen;
            QPainterPath borderPath0;
            QPainterPath borderPath1;
            QPainterPath borderPath2;

            bool isList = false;
            bool isTable = false;
            bool isTree = false;
            if(qobject_cast<const QListView *>(widget) || qobject_cast<const QListWidget *>(widget))
                isList = true;
            if(qobject_cast<const QTableView *>(widget) || qobject_cast<const QTableWidget *>(widget))
                isTable = true;
            if(qobject_cast<const QTreeView *>(widget) || qobject_cast<const QTreeView *>(widget))
                isTree = true;

            if (qobject_cast<const QTreeView*>(widget) || qobject_cast<const QTreeWidget*>(widget)) {
                sp->initConfigTreeParameters(isUseDarkPalette(), option, widget);
//                if(const QStyleOptionViewItem *vimm = qstyleoption_cast<const QStyleOptionViewItem *>(option)){
//                    qDebug() << "item vi..........." << vimm << vimm->index;
//                    if(vimm->widget){
//                        auto tree = qobject_cast<const QTreeView*>(widget);

//                        if(focus){
//                            QRect treeFocusRec = QRect(tree->x(), option->rect.y(), tree->width(), option->rect.height());
//                            painter->save();
//                            painter->setPen(QPen(QColor(Qt::red), 1));
//                            painter->setBrush(Qt::NoBrush);
//                            painter->drawRect(treeFocusRec);
//                            painter->restore();
//                        }
//                        //focus = itemWidget->hasFocus();
//                    }
//                }
                defaultBrush = sp->m_TreeParameters.treeDefaultBrush;
                hoverBrush = sp->m_TreeParameters.treeHoverBrush;
                selectBrush = sp->m_TreeParameters.treeSelectBrush;
                disableBrush = sp->m_TreeParameters.treeDisableBrush;
                hoverPen = sp->m_TreeParameters.treeHoverPen;
                selectPen = sp->m_TreeParameters.treeSelectPen;
                focusPen = sp->m_TreeParameters.treeFocusPen;
                focusRadius = sp->m_TreeParameters.radius;

                int borderWidth = (hoverPen.color().alpha() != 0 && hoverPen.color() != Qt::NoPen) ? hoverPen.width() : 0;

                if (vi->viewItemPosition == QStyleOptionViewItem::Beginning) {
                    if(vi->rect.left() == 0) {
                        QRect rect = vi->rect.adjusted(0, 0, 1, 1);
                        int margin = 4;

                        path.setFillRule(Qt::WindingFill);
                        path.moveTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);
                        path.arcTo(rect.left(), rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 180, -90);
                        path.lineTo(rect.right(), rect.top());
                        path.lineTo(rect.right(), rect.bottom());
                        path.lineTo(rect.left() + sp->m_TreeParameters.radius, rect.bottom());
                        path.arcTo(rect.left(), rect.bottom() - 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, -90, -90);
                        path.lineTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);
                        path.addRect(rect.adjusted(sp->m_TreeParameters.radius, -margin, margin, +margin));

                        borderPath0.moveTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);
                        borderPath0.arcTo(rect.left(), rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 180, -90);
                        borderPath0.lineTo(rect.right() + borderWidth, rect.top());
                        borderPath1.moveTo(rect.right() + borderWidth, rect.bottom() - 1);
                        borderPath1.lineTo(rect.left() + sp->m_TreeParameters.radius, rect.bottom() - 1);
                        borderPath1.arcTo(rect.left(), rect.bottom() - 1 - 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, -90, -90);
                        borderPath1.lineTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);
                    } else {
                        rectPath.addRect(vi->rect);

                        borderPath0.moveTo(vi->rect.left(), vi->rect.top());
                        borderPath0.lineTo(vi->rect.right() + borderWidth, vi->rect.top());
                        borderPath1.moveTo(vi->rect.right() + borderWidth, vi->rect.bottom());
                        borderPath1.lineTo(vi->rect.left(), vi->rect.bottom());
                    }
                } else if (vi->viewItemPosition == QStyleOptionViewItem::Middle) {
                    rectPath.addRect(option->rect);

                    borderPath0.moveTo(option->rect.left(), option->rect.top());
                    borderPath0.lineTo(option->rect.right() + borderWidth, option->rect.top());
                    borderPath1.moveTo(option->rect.left(), option->rect.bottom());
                    borderPath1.lineTo(option->rect.right() + borderWidth, option->rect.bottom());
                } else if (vi->viewItemPosition == QStyleOptionViewItem::End) {
                    QRect rect = vi->rect.adjusted(0, 0, 1, 1);
                    int margin = 4;

                    path.setFillRule(Qt::WindingFill);

                    path.moveTo(rect.left() +  rect.width() / 2.0, rect.top());
                    path.lineTo(rect.right() - sp->m_TreeParameters.radius, rect.top());
                    path.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 90, -90);
                    path.lineTo(rect.right(), rect.bottom() - 2 * sp->m_TreeParameters.radius);
                    path.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.bottom() - 2 * sp->m_TreeParameters.radius,
                               2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 0, -90);
                    path.lineTo(rect.left() +  rect.width() / 2.0, rect.bottom());
                    path.lineTo(rect.left() +  rect.width() / 2.0, rect.top());
                    path.addRect(rect.adjusted(-margin, 0, -sp->m_TreeParameters.radius, margin));

                    borderPath0.moveTo(rect.left(), rect.top());
                    borderPath0.lineTo(rect.right() - sp->m_TreeParameters.radius, rect.top());
                    borderPath0.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 90, -90);
                    borderPath2.moveTo(rect.right(), rect.top() + sp->m_TreeParameters.radius);
                    borderPath2.lineTo(rect.right(), rect.bottom() -1 - 2 * sp->m_TreeParameters.radius);
                    borderPath1.moveTo(rect.right(), rect.bottom() -1 - 2 * sp->m_TreeParameters.radius);
                    borderPath1.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.bottom() - 1 - 2 * sp->m_TreeParameters.radius,
                               2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 0, -90);
                    borderPath1.lineTo(rect.left(), rect.bottom() - 1);
                } else if (vi->viewItemPosition == QStyleOptionViewItem::OnlyOne) {
                    if(vi->rect.left() == 0) {
                        path.addRoundedRect(vi->rect, sp->m_TreeParameters.radius, sp->m_TreeParameters.radius);
                        borderPath0.addRoundedRect(vi->rect, sp->m_TreeParameters.radius, sp->m_TreeParameters.radius);
                    } else {
                        QRect rect = vi->rect.adjusted(0, 0, 1, 1);
                        int margin = 4;

                        path.setFillRule(Qt::WindingFill);

                        path.moveTo(rect.left(), rect.top());
                        path.lineTo(rect.right() - sp->m_TreeParameters.radius, rect.top());
                        path.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 90, -90);
                        path.lineTo(rect.right(), rect.bottom() - 2 * sp->m_TreeParameters.radius);
                        path.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.bottom() - 2 * sp->m_TreeParameters.radius,
                                   2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 0, -90);
                        path.lineTo(rect.left(), rect.bottom());
                        path.lineTo(rect.left(), rect.top());
                        path.addRect(rect.adjusted(-margin, 0, -sp->m_TreeParameters.radius, margin));

                        borderPath0.moveTo(rect.left(), rect.top());
                        borderPath0.lineTo(rect.right() - sp->m_TreeParameters.radius, rect.top());
                        borderPath0.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 90, -90);
                        borderPath2.moveTo(rect.right(), rect.top() + sp->m_TreeParameters.radius);
                        borderPath2.lineTo(rect.right(), rect.bottom() -1 - 2 * sp->m_TreeParameters.radius);
                        borderPath1.moveTo(rect.right(), rect.bottom() -1 - 2 * sp->m_TreeParameters.radius);
                        borderPath1.arcTo(rect.right() - 2 * sp->m_TreeParameters.radius, rect.bottom() - 1 - 2 * sp->m_TreeParameters.radius,
                                   2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 0, -90);
                        borderPath1.lineTo(rect.left(), rect.bottom() - 1);
                    }
                } else {
                    path.addRect(vi->rect);
                }
            } else if(qobject_cast<const QListView *>(widget) || qobject_cast<const QListWidget *>(widget)){
                sp->initConfigListParameters(isUseDarkPalette(), option, widget);

                defaultBrush = sp->m_ListParameters.listDefaultBrush;
                hoverBrush = sp->m_ListParameters.listHoverBrush;
                selectBrush = sp->m_ListParameters.listSelectBrush;
                disableBrush = sp->m_ListParameters.listDisableBrush;
                hoverPen = sp->m_ListParameters.listHoverPen;
                selectPen = sp->m_ListParameters.listSelectPen;
                focusPen = sp->m_ListParameters.listFocusPen;
                focusRadius = sp->m_ListParameters.radius;

                auto *listView = qobject_cast<const QListView *>(widget);
                int focusWidth = focus ? focusPen.width()/2 : 0;

                if(select && listView != nullptr){

                    if(listView == nullptr){
                        path.addRoundedRect(vi->rect, sp->m_ListParameters.radius, sp->m_ListParameters.radius);
                        return;
                    }
                    int currentIndex = vi->index.row();
                    int allNum = listView->model()->rowCount();

                    bool begin = false;
                    bool end = false;
                    bool mid = false;
                    bool onlyOne = false;
                    if(allNum == 1)
                    {
                        onlyOne = true;
                    }
                    else{
                        QList<QModelIndex> selectList = listView->selectionModel()->selectedIndexes();
                        QList<int> selectIndexList;
                        foreach (QModelIndex index, selectList) {
                            int i = index.row();
                            selectIndexList.append(i);
                        }
                        if(selectIndexList.contains(currentIndex - 1) && selectIndexList.contains(currentIndex +1))
                            mid = true;
                        else if(selectIndexList.contains(currentIndex - 1))
                            end = true;
                        else if(selectIndexList.contains(currentIndex +1))
                            begin = true;
                    }

                    if(begin){
                        QRect rect = vi->rect;
                        path.moveTo(rect.left() + focusWidth, rect.bottom() + 1 - focusWidth);
                        path.lineTo(rect.left() + focusWidth, rect.top() + sp->m_ListParameters.radius + focusWidth);
                        path.arcTo(rect.left() + focusWidth, rect.top() + focusWidth, 2 * sp->m_ListParameters.radius, 2 * sp->m_ListParameters.radius, 180, -90);
                        path.lineTo(rect.right() - sp->m_ListParameters.radius - focusWidth, rect.top() + focusWidth);
                        path.arcTo(rect.right() - 2 * sp->m_ListParameters.radius- focusWidth, rect.top() + focusWidth, 2 * sp->m_ListParameters.radius, 2 * sp->m_ListParameters.radius, 90, -90);
                        path.lineTo(rect.right() - focusWidth, rect.bottom() +1 - focusWidth);
                        path.lineTo(rect.left() + focusWidth, rect.bottom() + 1 - focusWidth);
                    }
                    else if(mid){
                        QRect rect = vi->rect;
                        path.moveTo(rect.left() + focusWidth, rect.bottom() + 1 - focusWidth);
                        path.lineTo(rect.left() + focusWidth, rect.top() + focusWidth);
                        path.lineTo(rect.right() - focusWidth, rect.top() + focusWidth);
                        path.lineTo(rect.right() - focusWidth, rect.bottom() + 1 - focusWidth);
                        path.lineTo(rect.left() + focusWidth, rect.bottom() + 1 - focusWidth);
                    }
                    else if(end)
                    {
                        QRect rect = vi->rect;
                        path.moveTo(rect.left() + focusWidth, rect.top() + focusWidth);
                        path.lineTo(rect.right() - focusWidth, rect.top() + focusWidth);
                        path.lineTo(rect.right() - focusWidth, rect.bottom() - sp->m_ListParameters.radius - focusWidth);
                        path.arcTo(rect.right() - 2 * sp->m_ListParameters.radius - focusWidth, rect.bottom() - 2 * sp->m_ListParameters.radius - focusWidth,
                                   2 * sp->m_ListParameters.radius, 2 * sp->m_ListParameters.radius, 0, -90);
                        path.lineTo(rect.left() + sp->m_ListParameters.radius + focusWidth, rect.bottom() - focusWidth);
                        path.arcTo(rect.left() + focusWidth, rect.bottom() - 2 * sp->m_ListParameters.radius - focusWidth, 2 * sp->m_ListParameters.radius, 2 * sp->m_ListParameters.radius, -90, -90);
                        path.lineTo(rect.left() + focusWidth, rect.top() + focusWidth);
                    }
                    else
                    {
                        path.addRoundedRect(vi->rect.adjusted(focusWidth, focusWidth, -1 * focusWidth, -1 * focusWidth), sp->m_ListParameters.radius, sp->m_ListParameters.radius);
                    }
                }
                else
                    path.addRoundedRect(vi->rect.adjusted(focusWidth, focusWidth, -1 * focusWidth, -1 * focusWidth), sp->m_ListParameters.radius, sp->m_ListParameters.radius);
            } else if(qobject_cast<const QTableView *>(widget) || qobject_cast<const QTableWidget *>(widget)){
                sp->initConfigTableParameters(isUseDarkPalette(), option, widget);

                defaultBrush = sp->m_TableParameters.tableDefaultBrush;
                hoverBrush = sp->m_TableParameters.tableHoverBrush;
                selectBrush = sp->m_TableParameters.tableSelectBrush;
                disableBrush = sp->m_TableParameters.tableDisableBrush;
                hoverPen = sp->m_TableParameters.tableHoverPen;
                selectPen = sp->m_TableParameters.tableSelectPen;
                focusPen = sp->m_TableParameters.tableFocusPen;
                focusRadius = sp->m_TableParameters.radius;
                int focusWidth = focus ? focusPen.width()/2 : 0;

                path.addRoundedRect(vi->rect.adjusted(focusWidth, focusWidth, -1 * focusWidth, -1 * focusWidth), sp->m_TableParameters.radius, sp->m_TableParameters.radius);
            }
            else {
                path.addRect(vi->rect);
                focusRadius = 0;
                defaultBrush = QBrush(Qt::transparent);
                hoverBrush = option->palette.brush(QPalette::Disabled, QPalette::Midlight);
                selectBrush = option->palette.brush(QPalette::Active, QPalette::Highlight);
                disableBrush = QBrush(Qt::transparent);
            }

            if ((vi->decorationPosition == QStyleOptionViewItem::Top) || (vi->decorationPosition ==  QStyleOptionViewItem::Bottom))
                isIconMode = true;

            if (isIconMode) {
                QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, option, widget);
                QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, option, widget);
                int iconMode_Radius = sp->radius;

                if(isList)
                    iconMode_Radius = sp->m_ListParameters.radius;
                else if(isTree)
                    iconMode_Radius = sp->m_TreeParameters.radius;
                else if(isTable)
                    iconMode_Radius = sp->m_TableParameters.radius;

                if (!enable)
                    return;

                if (select) {
                    if (iconRect.isValid() && textRect.isValid()) {
                        int Margin_Height = 2;
                        const int Margin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, option, widget);
                        iconRect.setRect(option->rect.x(), option->rect.y(), option->rect.width(), iconRect.height() + Margin + Margin_Height);
                        textRect.setRect(option->rect.x(), iconRect.bottom() + 1, option->rect.width(), option->rect.height() - iconRect.height());
                        QPainterPath iconPath, textPath;
                        iconPath.moveTo(iconRect.x(), iconRect.y() + iconMode_Radius);
                        iconPath.arcTo(iconRect.x(), iconRect.y(), iconMode_Radius * 2, iconMode_Radius * 2, 180, -90);
                        iconPath.lineTo(iconRect.right() + 1 - iconMode_Radius, iconRect.y());
                        iconPath.arcTo(iconRect.right() + 1 - iconMode_Radius * 2, iconRect.y(), iconMode_Radius * 2, iconMode_Radius * 2,
                                       90, -90);
                        iconPath.lineTo(iconRect.right() + 1, iconRect.bottom() + 1);
                        iconPath.lineTo(iconRect.left(), iconRect.bottom() + 1);
                        iconPath.lineTo(iconRect.left(), iconRect.top() + iconMode_Radius);

                        textPath.moveTo(textRect.x(), textRect.y());
                        textPath.lineTo(textRect.right() + 1, textRect.y());
                        textPath.lineTo(textRect.right() + 1, textRect.bottom() + 1 - iconMode_Radius);
                        textPath.arcTo(textRect.right() + 1 - iconMode_Radius * 2, textRect.bottom() + 1 - iconMode_Radius * 2, iconMode_Radius * 2, iconMode_Radius * 2,
                                       0, -90);
                        textPath.lineTo(textRect.left() + iconMode_Radius, textRect.bottom() + 1);
                        textPath.arcTo(textRect.left(), textRect.bottom() + 1 - iconMode_Radius * 2, iconMode_Radius * 2, iconMode_Radius * 2,
                                       270, -90);
                        textPath.lineTo(textRect.left(), textRect.top());

                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setBrush(vi->palette.brush(QPalette::Active, QPalette::Button));
                        painter->drawPath(iconPath);
                        painter->setBrush(selectBrush);
                        painter->drawPath(textPath);
                        painter->restore();
                    } else {
                        painter->save();
                        painter->setPen((selectPen == Qt::NoPen || selectPen.color().alpha() == 0) ? Qt::NoPen : selectPen);
                        painter->setRenderHint(QPainter::Antialiasing, true);
                        painter->setBrush(selectBrush);
                        painter->drawRoundedRect(option->rect, iconMode_Radius, iconMode_Radius);
                        painter->restore();
                    }
                } else {
                    painter->save();
                    painter->setPen(Qt::NoPen);
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    if (hover) {
                        painter->setPen((hoverPen == Qt::NoPen || hoverPen.color().alpha() == 0) ? Qt::NoPen : hoverPen);
                        painter->setBrush(hoverBrush);
                        painter->drawRoundedRect(option->rect, iconMode_Radius, iconMode_Radius);
                    }
                    else if (vi->backgroundBrush.style() != Qt::NoBrush) {
                        QPointF oldBO = painter->brushOrigin();
                        painter->setBrushOrigin(vi->rect.topLeft());
                        painter->setBrush(vi->backgroundBrush);
                        painter->drawRoundedRect(vi->rect, iconMode_Radius, iconMode_Radius);
                        painter->setBrushOrigin(oldBO);
                    }

                    painter->restore();
                }
            } else {
                if (vi->backgroundBrush.style() != Qt::NoBrush) {
                    QPointF oldBO = painter->brushOrigin();
                    painter->setBrushOrigin(vi->rect.topLeft());
                    painter->fillRect(vi->rect, vi->backgroundBrush);
                    painter->setBrushOrigin(oldBO);
                }

                painter->save();

                painter->setPen(Qt::NoPen);
                if (!enable) {
                    painter->setBrush(disableBrush);
                } else if (select) {
                    painter->setBrush(selectBrush);
                } else if (hover) {
                    painter->setBrush(hoverBrush);
                } else {
                    painter->setBrush(defaultBrush);
                }

                if(!path.isEmpty()){
                    painter->setRenderHint(QPainter::Antialiasing, true);
                    painter->drawPath(path);
                }
                painter->setRenderHint(QPainter::Antialiasing, false);
                painter->drawPath(rectPath);

                painter->restore();

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::NoBrush);
                if (enable && select) {
                    painter->setPen((selectPen == Qt::NoPen || selectPen.color().alpha() == 0) ? Qt::NoPen : selectPen);
                    if(isTree){
                        if (painter->pen().width() == 1 &&
                                !(vi->viewItemPosition == QStyleOptionViewItem::OnlyOne &&
                                  vi->rect.left() == 0)) {
                            painter->translate(0, 0.5);
                        }

                        if(!borderPath0.isEmpty()){
                            painter->drawPath(borderPath0);
                        }
                        if(!borderPath1.isEmpty()){
                            painter->drawPath(borderPath1);
                        }
                        if(!borderPath2.isEmpty()){
                            if(painter->pen().width() == 1)
                                painter->translate(-0.5, 0);
                            painter->drawPath(borderPath2);
                        }
                    }
                    else
                        painter->drawPath(path);
                } else if (enable && hover) {
                    painter->setPen((hoverPen == Qt::NoPen || hoverPen.color().alpha() == 0) ? Qt::NoPen : hoverPen);
                    if(isTree){
                        if (painter->pen().width() == 1 &&
                                !(vi->viewItemPosition == QStyleOptionViewItem::OnlyOne &&
                                  vi->rect.left() == 0)) {
                            painter->translate(0, 0.5);
                        }

                        if(!borderPath0.isEmpty()){
                            painter->drawPath(borderPath0);
                        }
                        if(!borderPath1.isEmpty()){
                            painter->drawPath(borderPath1);
                        }
                        if(!borderPath2.isEmpty()){
                            if(painter->pen().width() == 1)
                                painter->translate(-0.5, 0);
                            painter->drawPath(borderPath2);
                        }

                    }
                    else
                        painter->drawPath(path);
                }

//                if(isTree && focus){
//                    auto tree = qobject_cast<const QTreeView*>(widget);

//                    QRect treeFocusRec = QRect(tree->viewport()->x(), option->rect.y(), tree->viewport()->width(), option->rect.height());
//                    qDebug() << "treeFocusRec......" <<treeFocusRec;
//                    painter->save();
//                    painter->setPen(QPen(QColor(Qt::red), 1));
//                    painter->setBrush(Qt::NoBrush);
//                    painter->drawRect(treeFocusRec);
//                    painter->restore();
//                }
                painter->restore();

//                if(focus)
//                    qDebug() << "item focusssss true" << focus;

//                if (focus) {
//                    painter->save();
//                    painter->setPen((focusPen == Qt::NoPen || focusPen.color().alpha() == 0) ? Qt::NoPen : focusPen);
//                    if(isTree){
//                        if (painter->pen().width() == 1 &&
//                                !(vi->viewItemPosition == QStyleOptionViewItem::OnlyOne &&
//                                  vi->rect.left() == 0)) {
//                            painter->translate(0, 0.5);
//                        }

//                        if(!borderPath0.isEmpty()){
//                            painter->drawPath(borderPath0);
//                        }
//                        if(!borderPath1.isEmpty()){
//                            painter->drawPath(borderPath1);
//                        }
//                        if(!borderPath2.isEmpty()){
//                            if(painter->pen().width() == 1)
//                                painter->translate(-0.5, 0);
//                            painter->drawPath(borderPath2);
//                        }
//                    }
//                    else
//                        painter->drawPath(path);
//                    painter->restore();
//                }

            }
            if (!isTree && focus) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen((focusPen == Qt::NoPen || focusPen.color().alpha() == 0) ? Qt::NoPen : focusPen);
                painter->drawPath(path);
                painter->restore();
            }
//            else if(isTree){
//                if(focus){
//                    auto tree = qobject_cast<const QTreeView*>(widget);
//                    QRect treeItemRec = QRect(tree->viewport()->x(), option->rect.y(), tree->width(), option->rect.height());
//                    qDebug() << "treeItemRec......" << treeItemRec;

//                    QStyleOptionFocusRect o;
//                    o.QStyleOption::operator=(*vi);
//                    o.rect = treeItemRec;//proxy()->subElementRect(SE_ItemViewItemFocusRect, vi, widget);
//                    qDebug() << "ooooooo" << o.rect << proxy()->subElementRect(SE_TreeViewDisclosureItem, vi, widget);
//                    painter->save();
//                    painter->setPen(QPen(QColor(Qt::blue), 1));
//                    painter->setBrush(Qt::NoBrush);
//                    painter->drawRect(o.rect);
//                    painter->restore();

//                    drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, tree);
//                    auto t = const_cast<QTreeView *>(tree);
//                    t->setProperty("focusRect", vi->rect);
                    //drawPrimitive(QStyle::PE_IndicatorBranch, option, painter, tree);
//                }
//                auto tree = qobject_cast<const QTreeView*>(widget);
//                auto t = const_cast<QTreeView *>(tree);
//                tree->model()->index()
//                if(const QStyleOptionViewItem *vimm = qstyleoption_cast<const QStyleOptionViewItem *>(option))
//                    qDebug() << "item vi..........." << vimm << vimm->index << vimm->index.row() << vimm->index.column();
//                qDebug() << "children........" << tree->childAt(vi->rect.center());
//                if(vi->state & QStyle::State_HasFocus){
//                    qDebug() << "hasfocus.......";
//                }
                    //                if(vi->state & QStyle::State_HasFocus){
//                    t->setProperty("focusPoint", vi->rect.center());
//                }

//                QRect treeItemRec = QRect(tree->x(), option->rect.y(), tree->width(), option->rect.height());
//                qDebug() << "treeItemRec......" << treeItemRec;
//                if(t->property("focusPoint").isValid() && t->property("focusPoint").canConvert<QPoint>()){
//                    QPoint p = t->property("focusPoint").toPoint();
////                    qDebug() << "focuspoint...." << p;
//                    if(treeItemRec.contains(p))
//                    {
//                        painter->save();
//                        painter->setPen(QPen(QColor(Qt::red), 1));
//                        painter->setBrush(Qt::red);
//                        painter->drawRect(treeItemRec);
//                        painter->restore();
//                    }
//                }
//            }
            return;
        }
        break;
    }

    case PE_IndicatorBranch:
    {
        if (qobject_cast<const QTreeView *>(widget)) {
            bool isHover = option->state & State_MouseOver;
            bool isSelected = option->state & State_Selected;
            bool enable = option->state & State_Enabled;
//            bool focus = false;
//            if(const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)){
//                focus = vi->state & State_HasFocus;
//            }
            QBrush brush;
            QPen hoverPen = Qt::NoPen;
            QPen selectPen = Qt::NoPen;
//            QPen focusPen = Qt::NoPen;
            QPainterPath borderPath0;
            QPainterPath borderPath1;

            sp->initConfigTreeParameters(isUseDarkPalette(), option, widget);
            hoverPen = sp->m_TreeParameters.treeHoverPen;
            selectPen = sp->m_TreeParameters.treeSelectPen;
            //focusPen = sp->m_TreeParameters.treeFocusPen;

            if (!enable) {
                brush = sp->m_TreeParameters.treeBranchDisableBrush;
            } else if (isSelected) {
                brush = sp->m_TreeParameters.treeBranchSelectBrush;
            } else if (isHover) {
                brush = sp->m_TreeParameters.treeBranchHoverBrush;
            } else {
                brush = sp->m_TreeParameters.treeBranchDefaultBrush;
            }

            QPainterPath path;
            QPainterPath rectPath;

            if (option->rect.left() == 0) {
                //raund radius at beginning
                QRect rect = option->rect.adjusted(0, 0, 1, 1);
                path.setFillRule(Qt::WindingFill);
                int margin = 4;

                path.moveTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);
                path.arcTo(rect.left(), rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 180, -90);
                path.lineTo(rect.right() / 2.0, rect.top());
                path.lineTo(rect.right() / 2.0, rect.bottom());
                path.lineTo(rect.left() + sp->m_TreeParameters.radius, rect.bottom());
                path.arcTo(rect.left(), rect.bottom() - 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, -90, -90);
                path.lineTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);
                path.addRect(rect.adjusted(sp->m_TreeParameters.radius, -margin, margin, +margin));

                borderPath0.moveTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);
                borderPath0.arcTo(rect.left(), rect.top(), 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 180, -90);
                borderPath0.lineTo(rect.right(), rect.top());
                borderPath0.moveTo(rect.right(), rect.bottom() - 1);
                borderPath0.lineTo(rect.left() + sp->m_TreeParameters.radius, rect.bottom() - 1);
                borderPath1.moveTo(rect.left() + sp->m_TreeParameters.radius, rect.bottom() - 1);
                borderPath1.arcTo(rect.left(), rect.bottom() - 1 - 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, 2 * sp->m_TreeParameters.radius, -90, -90);
                borderPath1.lineTo(rect.left(), rect.top() + sp->m_TreeParameters.radius);

            } else {
                rectPath.addRect(option->rect);

                QRect rect = option->rect.adjusted(0, 0, 1, 1);

                borderPath0.moveTo(rect.left(), rect.top());
                borderPath0.lineTo(rect.right(), rect.top());
                borderPath1.moveTo(rect.right(), rect.bottom() - 1);
                borderPath1.lineTo(rect.left(), rect.bottom() - 1);
            }
            painter->save();
            painter->setClipRect(option->rect);

            auto tree = qobject_cast<const QTreeView *>(widget);
            QPoint oldBO = painter->brushOrigin();
            if(isHover && tree && tree->verticalScrollMode() == QAbstractItemView::ScrollPerPixel){
                painter->setBrushOrigin(QPoint(0,0));
            }
            painter->setPen(Qt::NoPen);
            painter->setBrush(brush);

            if(!path.isEmpty()){
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->drawPath(path);
            }

            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->drawPath(rectPath);

            painter->setBrushOrigin(oldBO);
            painter->restore();

            painter->save();
            if(sp->m_TreeParameters.radius != 0)
                painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::NoBrush);


            if (enable && isSelected) {
                painter->setPen(selectPen);
            } else if (enable && isHover) {
                painter->setPen(hoverPen);
            }

//            if(focus)
//                qDebug() << "branch focusssss true" << focus;
//            if(option->state & State_HasFocus)
//            {
//                painter->setPen(focusPen);
//            }

            if(painter->pen().width() == 1 && sp->m_TreeParameters.radius != 0)
                painter->translate(0.5, 0.5);

            if(!borderPath0.isEmpty()){
                painter->drawPath(borderPath0);
            }
            if(!borderPath1.isEmpty()){
                painter->drawPath(borderPath1);
            }

            painter->restore();

            //branch arrow
            if (!(option->state & State_Children))
                break;
            if (option->state & State_Open) {
                drawPrimitive(PE_IndicatorArrowDown, option, painter, widget);
            } else {
                const bool reverse = (option->direction == Qt::RightToLeft);
                drawPrimitive(reverse ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight, option, painter, widget);
            }

            return;
        }

//        if (!(option->state & State_Children)) {
//            return;
//        }
//        QStyleOption subOption = *option;
//        if (proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, option, widget))
//            subOption.state = option->state;
//        else
//            subOption.state = option->state & State_Enabled ? State_Enabled : State_None;

//        if (option->state & State_Open) {
//            proxy()->drawPrimitive(PE_IndicatorArrowDown, &subOption, painter, widget);
//        } else {
//            const bool reverse = (option->direction == Qt::RightToLeft);
//            proxy()->drawPrimitive(reverse ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight, &subOption, painter, widget);
//        }
//        return;
    } break;


    case PE_IndicatorViewItemCheck:
        return proxy()->drawPrimitive(PE_IndicatorCheckBox, option, painter, widget);

    case PE_IndicatorItemViewItemDrop:
    {
        QRect rect = option->rect;
        int Radius = 4;
        painter->save();
        painter->setClipRect(rect);
        painter->setRenderHint(QPainter::Antialiasing, Radius != 0);
        if(painter->pen().width() == 1 && Radius != 0)
            painter->translate(0.5, 0.5);
        painter->setPen(option->palette.color(QPalette::Active, QPalette::Highlight));
        painter->setBrush(Qt::NoBrush);
        if (option->rect.height() == 0)
            painter->drawLine(rect.topLeft(), rect.topRight());
        else
            painter->drawRoundedRect(rect, Radius, Radius);
        painter->restore();
        return;
    }

    default:
        break;
    }

    return Style::drawPrimitive(element, option, painter, widget);
}

void LINGMOConfigStyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar: {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            sp->initConfigScrollBarParameters(isUseDarkPalette(), option, widget);
            if (widget && widget->property("drawScrollBarGroove").isValid() && (!widget->property("drawScrollBarGroove").toBool())) {
                /*const bool active = bar->state & State_Active;
                QColor color;
                if (active) {
                    color = sp->m_ScrollBarParameters.scrollBarGrooveDefaultBrush.color();
                    auto animator = m_scrollbar_animation_helper->animator(widget);
                    if (animator) {
                        const bool mouseOver = bar->state & State_MouseOver;
                        if(sp->m_ScrollBarParameters.animation) {
                            //hover color animation
                            animator->setAnimatorDirectionForward("bg_width", mouseOver);

                            if (mouseOver) {
                                if (!animator->isRunning("bg_width") && animator->currentAnimatorTime("bg_width") == 0) {
                                    animator->startAnimator("bg_width");
                                }
                            } else {
                                if (!animator->isRunning("bg_width") && animator->currentAnimatorTime("bg_width") > 0) {
                                    animator->startAnimator("bg_width");
                                }
                            }
                            if(animator->isRunning("bg_width") || mouseOver){
                                qreal ratio = animator->value("bg_width").toReal();
                                color.setAlphaF(sp->m_ScrollBarParameters.scrollBarGrooveDefaultBrush.color().alphaF() * ratio);
                                QRect rect = bar->rect;
                                const bool horizontal = bar->orientation == Qt::Horizontal;
                                if(horizontal){
                                    rect = QRect(bar->rect.x(), (bar->rect.y() + bar->rect.height() * (1 - ratio)), bar->rect.width(), bar->rect.height() * ratio);
                                }
                                else{
                                    rect = QRect((bar->rect.x() + bar->rect.width() * (1 - ratio)), bar->rect.top(), bar->rect.width() * ratio, bar->rect.height());
                                }
                                painter->save();
                                painter->setPen(Qt::NoPen);
                                painter->setBrush(color);
                                painter->drawRect(rect);
                                painter->restore();
                            }
                        }
                    }
                }*/
            } else {
                const bool enable = bar->state & State_Enabled;
                painter->save();
                painter->setPen(Qt::NoPen);
                if (enable) {
                    painter->setBrush(sp->m_ScrollBarParameters.scrollBarGrooveDefaultBrush);
                } else {
                    painter->setBrush(sp->m_ScrollBarParameters.scrollBarGrooveInactiveBrush);
                }

                painter->drawRect(bar->rect);
                painter->restore();
            }

//            if (!(bar->activeSubControls & SC_ScrollBarSlider)) {
//                newScrollbar.state &= ~(State_Sunken | State_MouseOver);
//            }

            QStyleOptionSlider newScrollbar = *bar;
            newScrollbar.rect = proxy()->subControlRect(control, option, SC_ScrollBarSlider, widget);
            proxy()->drawControl(CE_ScrollBarSlider, &newScrollbar, painter, widget);

            if (sp->ScrooBar_ShowLine && widget && ((widget->property("drawScrollBarGroove").isValid() &&
                                                    widget->property("drawScrollBarGroove").toBool()) || !widget->property("drawScrollBarGroove").isValid())) {
                if (bar->subControls & SC_ScrollBarSubLine) {
                    QStyleOptionSlider subline = *bar;
                    subline.rect = proxy()->subControlRect(control, option, SC_ScrollBarSubLine, widget);
                    proxy()->drawControl(CE_ScrollBarSubLine, &subline, painter, widget);
                }
                if (bar->subControls & SC_ScrollBarAddLine) {
                    QStyleOptionSlider addline = *bar;
                    addline.rect = proxy()->subControlRect(control, option, SC_ScrollBarAddLine, widget);
                    proxy()->drawControl(CE_ScrollBarAddLine, &addline, painter, widget);
                }

            }
            proxy()->subControlRect(control, option, SC_ScrollBarGroove, widget);
            return;
        }
        break;
    }

    case CC_Slider:
    {
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            sp->initConfigSliderParameters(isUseDarkPalette(), option, widget);
            QRect groove = proxy()->subControlRect(CC_Slider, option, SC_SliderGroove, widget);
            QRect handle = proxy()->subControlRect(CC_Slider, option, SC_SliderHandle, widget);
            int width = 0;
            const bool horizontal = slider->orientation == Qt::Horizontal;
            const bool enable = slider->state & State_Enabled;
            const bool hover = slider->state & State_MouseOver;
            const bool suken = slider->state & State_Sunken;
            const bool focus = slider->state & State_HasFocus;
            if (slider->subControls & SC_SliderGroove) {
                if (horizontal) {
                    groove.setHeight(sp->Slider_GrooveLength);
                    groove.setWidth(groove.width() - 2 * sp->Slider_Margin);
                } else {
                    groove.setWidth(sp->Slider_GrooveLength);
                    groove.setHeight(groove.height() - 2 * sp->Slider_Margin);
                }
                groove.moveTo((option->rect.width() - groove.width()) / 2, (option->rect.height() - groove.height()) /2);
//                groove.moveCenter(option->rect.center());

                QBrush sBrush, gBrush;
                QRect sRect, gRect;
                if (horizontal) {
                    if (slider->upsideDown) {
                        sRect.setLeft(handle.center().x());
                        sRect.setTop(groove.top());
                        sRect.setRight(groove.right() - 1);
                        sRect.setBottom(groove.bottom());
                        gRect.setLeft(groove.left() + 1);
                        gRect.setTop(groove.top());
                        gRect.setRight(handle.center().x());
                        gRect.setBottom(groove.bottom());
                    } else {
                        gRect.setLeft(handle.center().x());
                        gRect.setTop(groove.top());
                        gRect.setRight(groove.right() - 1);
                        gRect.setBottom(groove.bottom());
                        sRect.setLeft(groove.left() + 1);
                        sRect.setTop(groove.top());
                        sRect.setRight(handle.center().x());
                        sRect.setBottom(groove.bottom());
                    }
                } else {
                    if (slider->upsideDown) {
                        gRect.setLeft(groove.left());
                        gRect.setTop(groove.top() + 1);
                        gRect.setRight(groove.right());
                        gRect.setBottom(handle.center().y());
                        sRect.setLeft(groove.left());
                        sRect.setTop(handle.center().y());
                        sRect.setRight(groove.right());
                        sRect.setBottom(groove.bottom() - 1);
                    } else {
                        sRect.setLeft(groove.left());
                        sRect.setTop(groove.top() + 1);
                        sRect.setRight(groove.right());
                        sRect.setBottom(handle.center().y());
                        gRect.setLeft(groove.left());
                        gRect.setTop(handle.center().y());
                        gRect.setRight(groove.right());
                        gRect.setBottom(groove.bottom() - 1);
                    }
                }

                if (!enable) {
                    sBrush = sp->m_SliderParameters.sliderGrooveValueDisableBrush;
                    gBrush = sp->m_SliderParameters.sliderGrooveUnvalueDisableBrush;
                } else if (hover) {
                    sBrush = sp->m_SliderParameters.sliderGrooveValueHoverBrush;
                    gBrush = sp->m_SliderParameters.sliderGrooveUnvalueHoverBrush;
                } else {
                    sBrush = sp->m_SliderParameters.sliderGrooveValueDefaultBrush;
                    gBrush = sp->m_SliderParameters.sliderGrooveUnvalueDefaultBrush;
                }

                painter->save();
                painter->setPen(sp->m_SliderParameters.sliderGroovePen.width() > 0 ? sp->m_SliderParameters.sliderGroovePen : Qt::NoPen);
                painter->setRenderHint(QPainter::Antialiasing, sp->m_SliderParameters.sliderGrooveRadius != 0);
                if(painter->pen().width() == 1 && sp->m_SliderParameters.sliderGrooveRadius != 0)
                    painter->translate(0.5, 0.5);
                painter->setBrush(sBrush);
                painter->drawRoundedRect(sRect, sp->m_SliderParameters.sliderGrooveRadius, sp->m_SliderParameters.sliderGrooveRadius);
                painter->setBrush(gBrush);
                painter->drawRoundedRect(gRect, sp->m_SliderParameters.sliderGrooveRadius, sp->m_SliderParameters.sliderGrooveRadius);
                painter->restore();
            }

            if (slider->subControls & SC_SliderTickmarks) {
                int tick = 4;
                int interval = slider->tickInterval;
                if (interval < 1)
                    interval = slider->pageStep;
                if (interval >= 1) {
                    int v = slider->minimum;
                    int len = proxy()->pixelMetric(PM_SliderLength, slider, widget);
                    const int thickSpace = 2;
                    painter->save();
                    painter->setPen(option->palette.color(QPalette::Disabled, QPalette::NoRole));
                    painter->setBrush(Qt::NoBrush);
                    while (v <= slider->maximum) {
                        int pos = sliderPositionFromValue(slider->minimum, slider->maximum, v, proxy()->pixelMetric(PM_SliderSpaceAvailable, option, widget),
                                                          slider->upsideDown) + len / 2;
                        pos = pos + sp->Slider_Margin;
                        if (horizontal) {
                            if (slider->tickPosition & QSlider::TicksAbove)
                                painter->drawLine(pos, handle.top() - thickSpace, pos, handle.top() - thickSpace + tick);
                            if (slider->tickPosition & QSlider::TicksBelow)
                                painter->drawLine(pos, handle.bottom() + thickSpace - tick, pos, handle.bottom() + thickSpace);
                        } else {
                            if (slider->tickPosition & QSlider::TicksAbove)
                                painter->drawLine(handle.left() - thickSpace, pos, handle.left() - thickSpace + tick, pos);
                            if (slider->tickPosition & QSlider::TicksBelow)
                                painter->drawLine(handle.right() + thickSpace, pos, handle.right() + thickSpace - tick, pos);
                        }
                        v += interval;
                    }
                    painter->restore();
                }
            }

            if (slider->subControls & SC_SliderHandle) {
                //we need to reinit our brush because it has QGradient without use option rect
                QStyleOptionSlider handleOption = *slider;
                if(horizontal)
                    handle.setY((option->rect.height() - handle.height()) / 2);
                else
                    handle.setX((option->rect.width() - handle.width()) / 2);
                handleOption.rect = handle;
                setSliderHandleRec(const_cast<QWidget*>(widget), handleOption.rect);

                sp->initConfigSliderParameters(isUseDarkPalette(), &handleOption, widget);

                QBrush hBrush;
                QPen hPen;
                if (!enable) {
                    hBrush = sp->m_SliderParameters.sliderHandleDisableBrush;
                    hPen = sp->m_SliderParameters.sliderHandleDisablePen;
                } else if (suken) {
                    hBrush = sp->m_SliderParameters.sliderHandleClickBrush;
                    hPen = sp->m_SliderParameters.sliderHandleClickPen;
                } else if (hover) {
                    hBrush = sp->m_SliderParameters.sliderHandleHoverBrush;
                    hPen = sp->m_SliderParameters.sliderHandleHoverPen;
                } else {
                    hBrush = sp->m_SliderParameters.sliderHandleDefaultBrush;
                    hPen = sp->m_SliderParameters.sliderHandleDefaultPen;
                }

                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(hPen.width() == 0 ? Qt::NoPen : hPen);
                if(painter->pen().width() == 1)
                    painter->translate(0.5, 0.5);
                painter->setBrush(hBrush);
                painter->drawPath(sp->m_SliderParameters.sliderHandlePath);
                painter->restore();
            }

            if (enable && focus) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(sp->m_SliderParameters.focusPen.width() == 0 ? Qt::NoPen : sp->m_SliderParameters.focusPen);
                width = painter->pen().width();
                if(width == 1)
                    painter->translate(0.5, 0.5);
                painter->setBrush(Qt::NoBrush);
                painter->drawRoundedRect(slider->rect.adjusted(width, width, -width, -width),
                                         sp->m_SliderParameters.radius, sp->m_SliderParameters.radius);
                painter->restore();
            }
            return;
        }
        break;
    }

    case CC_ComboBox:
    {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            sp->initConfigComboBoxParameters(isUseDarkPalette(), option, widget);
            const bool enable = comboBox->state & State_Enabled;
            const bool on = comboBox->state & State_On;
            const bool hover = comboBox->state & State_MouseOver;
            const bool focus = comboBox->state & State_HasFocus;

            int width = 0;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, sp->m_ComboBoxParameters.radius != 0);
            if (!enable) {
                width = sp->m_ComboBoxParameters.comboBoxDisablePen.width();
                painter->setPen(sp->m_ComboBoxParameters.comboBoxDisablePen);
                painter->setBrush(sp->m_ComboBoxParameters.comboBoxDisableBrush);
            } else {
                if (comboBox->editable) {
                    if (comboBox->state & (State_HasFocus | State_On)) {
                        width = sp->m_ComboBoxParameters.comboBoxEditPen.width();
                        painter->setPen(sp->m_ComboBoxParameters.comboBoxEditPen);
                        painter->setBrush(sp->m_ComboBoxParameters.comboBoxEditBrush);
                    } else {
                        width = sp->m_ComboBoxParameters.comboBoxDefaultPen.width();
                        painter->setPen(sp->m_ComboBoxParameters.comboBoxDefaultPen);
                        painter->setBrush(sp->m_ComboBoxParameters.comboBoxDefaultBrush);
                    }
                } else {
                    if (on) {
                        width = sp->m_ComboBoxParameters.comboBoxOnPen.width();
                        painter->setPen(sp->m_ComboBoxParameters.comboBoxOnPen);
                        painter->setBrush(sp->m_ComboBoxParameters.comboBoxOnBrush);
                    } else if (hover) {
                        width = sp->m_ComboBoxParameters.comboBoxHoverPen.width();
                        painter->setPen(sp->m_ComboBoxParameters.comboBoxHoverPen);
                        painter->setBrush(sp->m_ComboBoxParameters.comboBoxHoverBrush);
                    } else {
                        width = sp->m_ComboBoxParameters.comboBoxDefaultPen.width();
                        painter->setPen(sp->m_ComboBoxParameters.comboBoxDefaultPen);
                        painter->setBrush(sp->m_ComboBoxParameters.comboBoxDefaultBrush);
                    }
                }

                //focus
                if (focus) {
                    width = sp->m_ComboBoxParameters.comboBoxFocusPen.width();
                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ComboBoxParameters.comboBoxFocusPen);
                }
            }
            if(width == 1 && sp->m_ComboBoxParameters.radius != 0)
                painter->translate(0.5, 0.5);

            painter->drawRoundedRect(option->rect.adjusted(width, width, -width, -width), sp->m_ComboBoxParameters.radius, sp->m_ComboBoxParameters.radius);
            painter->restore();

            if (comboBox->subControls & SC_ComboBoxArrow) {
                QRect arrowRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxArrow, widget);
                QStyleOption arrow = *option;
                arrow.state = option->state & State_Enabled ? State_Enabled : State_None;
                arrow.rect = arrowRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrow, painter, widget);
            }
            return;
        }
        break;
    }

    case CC_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            bool isHorizontalLayout = (sp->getSpinBoxControlLayout() == LINGMOConfigStyleParameters::SpinBoxControlLayout::Horizontal);
            sp->initConfigSpinBoxParameters(isUseDarkPalette(), option, widget, isHorizontalLayout);
            bool enable = sb->state & State_Enabled;
            bool up = sb->activeSubControls == SC_SpinBoxUp;
            bool down = sb->activeSubControls == SC_SpinBoxDown;
            bool hover = sb->state & State_MouseOver;
            bool focus = sb->state & State_HasFocus;
            bool sunken = sb->state & State_Sunken;
            int width = 0;
            QRect upRect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
            QRect downRect = proxy()->subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);

            QStyleOption upOption, downOption;
            upOption.rect = upRect;
            downOption.rect = downRect;
            bool hasBorder = false;

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, sp->m_SpinBoxParameters.radius != 0);

            if(isHorizontalLayout)//xc mode
            {
                if (!enable) {
                    upOption.state = State_None;
                    downOption.state = State_None;
                    painter->setPen(sp->m_SpinBoxParameters.spinBoxDisablePen);
                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxDisableBrush);
                    hasBorder = false;
//                    if(painter->pen().width() == 1)
//                        painter->translate(0.5, 0.5);
                    painter->drawRect(option->rect.adjusted(sp->SpinBox_DefaultHeight, 0, -sp->SpinBox_DefaultHeight, -1));
                    painter->restore();
                }
                else
                {
                    if (focus) {
                        int width = sp->m_SpinBoxParameters.spinBoxFocusPen.width();
                        hasBorder = true;
                        painter->setPen(sp->m_SpinBoxParameters.spinBoxFocusPen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxFocusBrush);
                        if(painter->pen().width() == 1 && sp->m_SpinBoxParameters.radius != 0)
                            painter->translate(0.5, 0.5);
                        painter->drawRoundedRect(option->rect.adjusted(width, width, -width, -width),
                                                 sp->m_SpinBoxParameters.radius, sp->m_SpinBoxParameters.radius);

                        painter->restore();
                    } else if (hover) {
                        painter->setPen(sp->m_SpinBoxParameters.spinBoxHoverPen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxHoverBrush);
//                        if(painter->pen().width() == 1)
//                            painter->translate(0.5, 0.5);
                        painter->drawRect(option->rect.adjusted(sp->SpinBox_DefaultHeight, 0, -sp->SpinBox_DefaultHeight, -1));
                        painter->restore();
                    }
                    else {
                        hasBorder = false;
                        painter->setPen(sp->m_SpinBoxParameters.spinBoxDefaultPen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxDefaultBrush);
//                        if(painter->pen().width() == 1)
//                            painter->translate(0.5, 0.5);
                        painter->drawRect(option->rect.adjusted(sp->SpinBox_DefaultHeight, 0, -sp->SpinBox_DefaultHeight, -1));
                        painter->restore();
                    }
                }

                painter->save();
                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::NoBrush);
                int lineWidth = hasBorder ? 2 : 0;
                int radius= sp->m_SpinBoxParameters.radius;
                if (sb->subControls & (SC_SpinBoxUp | SC_SpinBoxDown)) {
                    QPainterPath upPath;

                    upPath.moveTo(upRect.left() + 2, upRect.top() + lineWidth + (hasBorder ? 1 : 0));
                    upPath.lineTo(upRect.right() + 1 - radius - lineWidth, upRect.top() + lineWidth + (hasBorder ? 1 : 0));
                    upPath.arcTo(upRect.right() + 1 - 2 * radius - lineWidth, upRect.top() + lineWidth + (hasBorder ? 1 : 0), radius * 2, radius * 2, 90, -90);
                    upPath.lineTo(upRect.right() + 1 - lineWidth, upRect.bottom() - lineWidth - radius);
                    upPath.arcTo(upRect.right() + 1 - 2 * radius - lineWidth, upRect.bottom() - lineWidth - 2 * radius, radius * 2, radius * 2, 0, -90);
                    upPath.lineTo(upRect.left() + 2, upRect.bottom() - lineWidth);
                    upPath.lineTo(upRect.left() + 2, upRect.top() + lineWidth + (hasBorder ? 1 : 0));

                    if (sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) {
                        upOption.state |= State_Enabled;
                        if (up && enable) {
                            if (focus) {
                                if(sunken)
                                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpClickBrush);
                                else
                                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpHoverBrush);

                                upOption.state |= State_MouseOver;
                            } else {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpHoverBrush);
                            }

                            if (option->state & State_Sunken) {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpClickBrush);
                                upOption.state |= State_Sunken;
                            }
                            painter->drawPath(upPath);
                        }
                        else {
                            if(enable)
                            {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpDefaultBrush);
                            }
                            else
                            {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxDisableBrush);
                            }
                            painter->drawPath(upPath);
                        }
                    } else {
                        if(enable)
                        {
                            painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpDefaultBrush);
                        }
                        else
                        {
                            painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpDefaultBrush);
                        }
                        painter->drawPath(upPath);

                        upOption.state = State_None;
                    }
                    painter->restore();

                    QPainterPath downPath;
                    painter->save();
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(Qt::NoBrush);

                    downPath.moveTo(downRect.left() + 1 + radius + lineWidth, downRect.top() + lineWidth + (hasBorder ? 1 : 0));
                    downPath.arcTo(downRect.left() + 1 + lineWidth, downRect.top() + lineWidth + (hasBorder ? 1 : 0), radius * 2, radius * 2, 90, 90);
                    downPath.lineTo(downRect.left() + 1 + lineWidth, downRect.bottom() - lineWidth - radius);
                    downPath.arcTo(downRect.left() + 1 + lineWidth, downRect.bottom() - lineWidth - 2 * radius, radius * 2, radius * 2, 180, 90);
                    downPath.lineTo(downRect.right(), downRect.bottom() - lineWidth);
                    downPath.lineTo(downRect.right(), downRect.top() + lineWidth + (hasBorder ? 1 : 0));
                    downPath.lineTo(downRect.left() + 1 + radius, downRect.top() + lineWidth + (hasBorder ? 1 : 0));

                    if (sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) {
                        downOption.state |= State_Enabled;
                        if (down && enable) {
                            if (focus) {
                                if(sunken)
                                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownClickBrush);
                                else
                                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownHoverBrush);
                                downOption.state |= State_MouseOver;
                            } else {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownHoverBrush);
                            }

                            if (option->state & State_Sunken) {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownClickBrush);
                                downOption.state |= State_Sunken;
                            }
                            painter->drawPath(downPath);
                        }
                        else {
                            if(enable)
                            {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownDefaultBrush);
                            }
                            else
                            {
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxDisableBrush);
                            }
                            painter->drawPath(downPath);
                        }
                    }
                    else {
                        if(enable)
                        {
                            painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownDefaultBrush);
                        }
                        else
                        {
                            painter->setBrush(sp->m_SpinBoxParameters.spinBoxDisableBrush);
                        }
                        painter->drawPath(downPath);

                        downOption.state = State_None;
                    }
                    painter->restore();

                    if (!enable) {
                        upOption.state = State_None;
                        downOption.state = State_None;
                    }

                    if (sb->buttonSymbols == QAbstractSpinBox::PlusMinus) {

                    } else if (sb->buttonSymbols == QAbstractSpinBox::UpDownArrows) {
                        proxy()->drawPrimitive(PE_IndicatorArrowUp, &upOption, painter, widget);
                        proxy()->drawPrimitive(PE_IndicatorArrowDown, &downOption, painter, widget);
                    }
                }
            }
            else{//pc mode
                if (!enable) {
                    width = sp->m_SpinBoxParameters.spinBoxDisablePen.width();
                    painter->setPen(sp->m_SpinBoxParameters.spinBoxDisablePen);
                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxDisableBrush);
                } else {
                    if (focus) {
                        width = sp->m_SpinBoxParameters.spinBoxFocusPen.width();
                        painter->setPen(sp->m_SpinBoxParameters.spinBoxFocusPen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxFocusBrush);
                    } else if (hover) {
                        width = sp->m_SpinBoxParameters.spinBoxHoverPen.width();
                        painter->setPen(sp->m_SpinBoxParameters.spinBoxHoverPen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxHoverBrush);
                    } else {
                        width = sp->m_SpinBoxParameters.spinBoxDefaultPen.width();
                        painter->setPen(sp->m_SpinBoxParameters.spinBoxDefaultPen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxDefaultBrush);
                    }
                }

                if(painter->pen().width() == 1 && sp->m_SpinBoxParameters.radius != 0)
                    painter->translate(0.5, 0.5);

                painter->drawRoundedRect(option->rect.adjusted(((width%2) == 1 ? (width - 1)/2 : width/2),
                                                               ((width%2) == 1 ? (width - 1)/2 : width/2),
                                                               -1 * ((width%2) == 1 ? (width + 1)/2 : width/2),
                                                               -1 * ((width%2) == 1 ? (width + 1)/2 : width/2)),
                                         sp->m_SpinBoxParameters.radius, sp->m_SpinBoxParameters.radius);
                painter->restore();


                //up and down button
                if (sb->subControls & (SC_SpinBoxUp | SC_SpinBoxDown)) {

                    //up button
                    if (sb->stepEnabled & QAbstractSpinBox::StepUpEnabled) {
                        QPainterPath upPath;

                        painter->save();
                        painter->setRenderHint(QPainter::Antialiasing, sp->m_SpinBoxParameters.radius != 0);
                        if (!enable) {
                            upOption.state = State_None;
                            int width = sp->m_SpinBoxParameters.spinBoxUpDisablePen.width();
                            painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxUpDisablePen);
                            painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpDisableBrush);
                        } else {
                            upOption.state |= State_Enabled;

                            if (up) {
                                if (option->state & State_Sunken) {
                                    upOption.state |= State_Sunken;
                                    int width = sp->m_SpinBoxParameters.spinBoxUpClickPen.width();
                                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxUpClickPen);
                                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpClickBrush);
                                } else {
                                    if (focus) {
                                        upOption.state |= State_MouseOver;
                                        int width = sp->m_SpinBoxParameters.spinBoxUpFocusHoverPen.width();
                                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxUpFocusHoverPen);
                                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpFocusHoverBrush);
                                    } else {
                                        int width = sp->m_SpinBoxParameters.spinBoxUpHoverPen.width();
                                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxUpHoverPen);
                                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpHoverBrush);
                                    }
                                }
                            } else {
                                int width = sp->m_SpinBoxParameters.spinBoxUpDefaultPen.width();
                                painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxUpDefaultPen);
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpDefaultBrush);
                            }
                        }
                        upRect.adjust(0, 0, 1 - width, 1);
                        upPath.moveTo(upRect.left(), upRect.top() + sp->m_SpinBoxParameters.radius);
                        upPath.arcTo(upRect.left(), upRect.top(), sp->m_SpinBoxParameters.radius * 2, sp->m_SpinBoxParameters.radius * 2, 180, -90);
                        upPath.lineTo(upRect.right() - sp->m_SpinBoxParameters.radius, upRect.top());
                        upPath.arcTo(upRect.right() - 2 * sp->m_SpinBoxParameters.radius, upRect.top(), sp->m_SpinBoxParameters.radius * 2,
                                     sp->m_SpinBoxParameters.radius * 2, 90, -90);
                        upPath.lineTo(upRect.right(), upRect.bottom());
                        upPath.lineTo(upRect.left(), upRect.bottom());
                        upPath.lineTo(upRect.left(), upRect.top() + sp->m_SpinBoxParameters.radius);

                        if(painter->pen().width() == 1 && sp->m_SpinBoxParameters.radius != 0)
                            painter->translate(0.5, 0.5);

                        painter->drawPath(upPath);
//                        painter->drawRect(upRect);
                        painter->restore();
                    } else {
                        upOption.state = State_None;
                        int width = sp->m_SpinBoxParameters.spinBoxUpDisablePen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxUpDisablePen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxUpDisableBrush);
                        QPainterPath upPath;

                        upRect.adjust(0, 0, 1 - width, 1);
                        upPath.moveTo(upRect.left(), upRect.top() + sp->m_SpinBoxParameters.radius);
                        upPath.arcTo(upRect.left(), upRect.top(), sp->m_SpinBoxParameters.radius * 2, sp->m_SpinBoxParameters.radius * 2, 180, -90);
                        upPath.lineTo(upRect.right() - sp->m_SpinBoxParameters.radius, upRect.top());
                        upPath.arcTo(upRect.right() - 2 * sp->m_SpinBoxParameters.radius, upRect.top(), sp->m_SpinBoxParameters.radius * 2,
                                     sp->m_SpinBoxParameters.radius * 2, 90, -90);
                        upPath.lineTo(upRect.right(), upRect.bottom());
                        upPath.lineTo(upRect.left(), upRect.bottom());
                        upPath.lineTo(upRect.left(), upRect.top() + sp->m_SpinBoxParameters.radius);

                        if(painter->pen().width() == 1 && sp->m_SpinBoxParameters.radius != 0)
                            painter->translate(0.5, 0.5);

                        painter->drawPath(upPath);
//                        painter->drawRect(upRect);
                        painter->restore();

                    }

                    //down button
                    if (sb->stepEnabled & QAbstractSpinBox::StepDownEnabled) {
                        QPainterPath downPath;

                        painter->save();
                        painter->setRenderHint(QPainter::Antialiasing, sp->m_SpinBoxParameters.radius != 0);
                        if (!enable) {
                            downOption.state = State_None;
                            int width = sp->m_SpinBoxParameters.spinBoxDownDisablePen.width();
                            painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxDownDisablePen);
                            painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownDisableBrush);
                        } else {
                            downOption.state |= State_Enabled;

                            if (down) {
                                if (option->state & State_Sunken) {
                                    downOption.state |= State_Sunken;
                                    int width = sp->m_SpinBoxParameters.spinBoxDownClickPen.width();
                                    painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxDownClickPen);
                                    painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownClickBrush);
                                } else {
                                    if (focus) {
                                        downOption.state |= State_MouseOver;
                                        int width = sp->m_SpinBoxParameters.spinBoxDownFocusHoverPen.width();
                                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxDownFocusHoverPen);
                                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownFocusHoverBrush);
                                    } else {
                                        int width = sp->m_SpinBoxParameters.spinBoxDownHoverPen.width();
                                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxDownHoverPen);
                                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownHoverBrush);
                                    }
                                }
                            } else {
                                int width = sp->m_SpinBoxParameters.spinBoxDownDefaultPen.width();
                                painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxDownDefaultPen);
                                painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownDefaultBrush);
                            }
                        }
                        downRect.adjust(0, 0, 1 - width, 1 - width);
                        downPath.moveTo(downRect.left(), downRect.bottom() - sp->m_SpinBoxParameters.radius);
                        downPath.arcTo(downRect.left(), downRect.bottom() - sp->m_SpinBoxParameters.radius * 2, sp->m_SpinBoxParameters.radius * 2,
                                       sp->m_SpinBoxParameters.radius * 2, 180, 90);
                        downPath.lineTo(downRect.right() - sp->m_SpinBoxParameters.radius, downRect.bottom());
                        downPath.arcTo(downRect.right() - sp->m_SpinBoxParameters.radius * 2, downRect.bottom() - sp->m_SpinBoxParameters.radius * 2,
                                       sp->m_SpinBoxParameters.radius * 2, sp->m_SpinBoxParameters.radius * 2, 270, 90);
                        downPath.lineTo(downRect.right(), downRect.top());
                        downPath.lineTo(downRect.left(), downRect.top());
                        downPath.lineTo(downRect.left(), downRect.bottom() - sp->m_SpinBoxParameters.radius);

                        if(painter->pen().width() == 1 && sp->m_SpinBoxParameters.radius != 0)
                            painter->translate(0.5, 0.5);

                        painter->drawPath(downPath);
//                        painter->drawRect(downRect);
                        painter->restore();
                    } else {
                        QPainterPath downPath;
                        painter->save();
                        painter->setRenderHint(QPainter::Antialiasing, sp->m_SpinBoxParameters.radius != 0);

                        downOption.state = State_None;
                        int width = sp->m_SpinBoxParameters.spinBoxDownDisablePen.width();
                        painter->setPen(width <= 0 ? Qt::NoPen : sp->m_SpinBoxParameters.spinBoxDownDisablePen);
                        painter->setBrush(sp->m_SpinBoxParameters.spinBoxDownDisableBrush);
                        downRect.adjust(0, 0, 1 - width, 1 - width);
                        downPath.moveTo(downRect.left(), downRect.bottom() - sp->m_SpinBoxParameters.radius);
                        downPath.arcTo(downRect.left(), downRect.bottom() - sp->m_SpinBoxParameters.radius * 2, sp->m_SpinBoxParameters.radius * 2,
                                       sp->m_SpinBoxParameters.radius * 2, 180, 90);
                        downPath.lineTo(downRect.right() - sp->m_SpinBoxParameters.radius, downRect.bottom());
                        downPath.arcTo(downRect.right() - sp->m_SpinBoxParameters.radius * 2, downRect.bottom() - sp->m_SpinBoxParameters.radius * 2,
                                       sp->m_SpinBoxParameters.radius * 2, sp->m_SpinBoxParameters.radius * 2, 270, 90);
                        downPath.lineTo(downRect.right(), downRect.top());
                        downPath.lineTo(downRect.left(), downRect.top());
                        downPath.lineTo(downRect.left(), downRect.bottom() - sp->m_SpinBoxParameters.radius);

                        if(painter->pen().width() == 1 && sp->m_SpinBoxParameters.radius != 0)
                            painter->translate(0.5, 0.5);

                        painter->drawPath(downPath);
//                        painter->drawRect(downRect);
                        painter->restore();
                    }

                    if (!enable) {
                        upOption.state = State_None;
                        downOption.state = State_None;
                    }

                    if (sb->buttonSymbols == QAbstractSpinBox::PlusMinus) {

                    } else if (sb->buttonSymbols == QAbstractSpinBox::UpDownArrows) {
                        proxy()->drawPrimitive(PE_IndicatorArrowUp, &upOption, painter, widget);
                        proxy()->drawPrimitive(PE_IndicatorArrowDown, &downOption, painter, widget);
                    }
                }
            }
//            painter->restore();
            return;
        }
        break;

    }

    case CC_Dial:
    {
        if (const QStyleOptionSlider *dial = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const bool enable = dial->state & State_Enabled;
            int tickOffset = 8;
            const int width = option->rect.width();
            const int height = option->rect.height();
            qreal r = qMin(dial->rect.width(), dial->rect.height()) / 2;
            qreal r_ = r - tickOffset;
            qreal dx = dial->rect.x() + (width - 2 * r_) / 2;
            qreal dy = dial->rect.y() + (height - 2 * r_) / 2;
            QRectF br(dial->rect.x() + dx, dial->rect.y() + dy, r_ * 2, r_ * 2);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(Qt::NoPen);
            painter->setBrush(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::Button));
            painter->drawEllipse(br);
            painter->restore();

            int tickHandle = 4;
            int handleLength = 8;
            QRectF circleRect(br.adjusted(tickHandle + handleLength, tickHandle + handleLength,
                                          -(tickHandle + handleLength), -(tickHandle + handleLength)));

            QColor highLight = enable ? dial->palette.color(QPalette::Active, QPalette::Highlight)
                                      : dial->palette.color(QPalette::Disabled, QPalette::ButtonText);
            QColor groove_color = isUseDarkPalette() ? QColor(38, 38, 38) : QColor(191, 191, 191);
            qreal fist = configCalcRadial(dial, dial->minimum);
            qreal dp = configCalcRadial(dial, dial->sliderPosition);
            qreal end = configCalcRadial(dial, dial->maximum);
            QPen pen(groove_color, handleLength, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->save();
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->drawArc(circleRect, fist * 16 * 180 / M_PI, (end - fist) * 16 * 180 / M_PI);
            pen.setColor(highLight);
            painter->setPen(pen);
            painter->drawArc(circleRect, fist * 16 * 180 / M_PI, (dp - fist) * 16 * 180 / M_PI);
            painter->restore();

            qreal xc = dial->rect.width() / 2;
            qreal yc = dial->rect.height() / 2;
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setPen(QPen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::Base),
                                 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(QPointF(xc + (r_ - 8 - 2) * qCos(dp), yc - (r_ - 8 - 2) * qSin(dp)),
                              QPointF(xc + (r_ - 8 - 2 - 4) * qCos(dp), yc - (r_ - 8 - 2 - 4) * qSin(dp)));
            painter->restore();

            if (dial->subControls & SC_DialTickmarks) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                QPen pen(dial->palette.color(enable ? QPalette::Active : QPalette::Disabled, QPalette::WindowText),
                         1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter->setPen(pen);
                if(painter->pen().width() == 1)
                    painter->translate(0.5, 0.5);
                painter->drawLines(configCalcLines(dial, tickOffset));
                painter->restore();
            }
            return;
        }
        break;
    }

    case CC_ToolButton:
    {
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            proxy()->drawPrimitive(PE_PanelButtonTool, option, painter, widget);
            proxy()->drawControl(CE_ToolButtonLabel, option, painter, widget);

            return;
        }
        break;
    }

    case CC_GroupBox: //LINGMO GroupBox style
    {
        painter->save();
        if (const QStyleOptionGroupBox *groupBox = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            // Draw frame
            QRect textRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
            QRect checkBoxRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
            //新加
            QRect groupContents = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxContents, widget);

            if (groupBox->subControls & QStyle::SC_GroupBoxFrame) {
                QStyleOptionFrame frame;
                frame.QStyleOption::operator=(*groupBox);
                frame.features = groupBox->features;
                frame.lineWidth = groupBox->lineWidth;
                frame.midLineWidth = groupBox->midLineWidth;
                frame.rect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxFrame, widget);
                proxy()->drawPrimitive(PE_FrameGroupBox, &frame, painter, widget);
            }

            // Draw title
            if ((groupBox->subControls & QStyle::SC_GroupBoxLabel) && !groupBox->text.isEmpty()) {
                // groupBox->textColor gets the incorrect palette here
                painter->setPen(QPen(option->palette.windowText(), 1));
                int alignment = int(groupBox->textAlignment);
                if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, option, widget))
                    alignment |= Qt::TextHideMnemonic;

                proxy()->drawItemText(painter, textRect,  Qt::TextShowMnemonic | Qt::AlignLeft | alignment,
                                      groupBox->palette, groupBox->state & State_Enabled, groupBox->text, QPalette::NoRole);

                if (groupBox->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*groupBox);
                    fropt.rect = textRect.adjusted(-2, -1, 2, 1);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing,true);
                painter->setPen(Qt::NoPen);
                painter->setBrush(option->palette.color(QPalette::Base));
                painter->drawRoundedRect(groupContents, sp->radius, sp->radius);
                painter->restore();

            }

            // Draw checkbox
            if (groupBox->subControls & SC_GroupBoxCheckBox) {
                QStyleOptionButton box;
                box.QStyleOption::operator=(*groupBox);
                box.rect = checkBoxRect;
                proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
            }
        }
        painter->restore();
        return;
        break;

    }

    default:        return Style::drawComplexControl(control, option, painter, widget);
    }
}

void LINGMOConfigStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
    } else {
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
    }
    switch (element) {
    case CE_ScrollBarSlider:
    {
        auto animator = m_scrollbar_animation_helper->animator(widget);
        if (!animator) {
            return Style::drawControl(element, option, painter, widget);
        }

        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const bool enable = bar->state & State_Enabled;
            const bool sunKen = bar->state & State_Sunken;
            const bool mouseOver = bar->state & State_MouseOver;
            const bool horizontal = (bar->orientation == Qt::Horizontal);
            QRect rect = bar->rect;
            QRect drawRect;

            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);

            bool hasAnimation = widget &&(!widget->property("animation").isValid() ||
                                          (widget->property("animation").isValid() && widget->property("animation").toBool()));

            if(hasAnimation) {
                qreal len;
                if(sp->m_ScrollBarParameters.grooveWidthAnimation){
                    //width animation
                    animator->setAnimatorDirectionForward("groove_width", mouseOver);
                    if (mouseOver) {
                        if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") == 0) {
                            animator->startAnimator("groove_width");
                        }
                    } else {
                        if (!animator->isRunning("groove_width") && animator->currentAnimatorTime("groove_width") > 0) {
                            animator->startAnimator("groove_width");
                        }
                    }
                    if(widget && widget->isEnabled())
                        len = animator->value("groove_width").toReal() * 4 * sp->m_scaleRatio2_1 + 4;
                    else
                        len = 4;
                }
                else
                    len = sp->ScroolBar_Width;
            if (horizontal) {
                drawRect.setRect(rect.x(), rect.y() + (rect.height() - len) / 2, rect.width(), len);
            } else {
                drawRect.setRect(rect.x() + (rect.width() - len) / 2, rect.y(), len, rect.height());
            }
            }else {
                drawRect = bar->rect;
            }

            setScrollbarSliderRec(const_cast<QWidget*>(widget), QRect(drawRect.x(), drawRect.y(), drawRect.width(), drawRect.height()));

            //hover color animation
//            animator->setAnimatorDirectionForward("slider_opacity", mouseOver);
//            if (mouseOver) {
//                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") == 0) {
//                    animator->startAnimator("slider_opacity");
//                }
//            } else {
//                if (!animator->isRunning("slider_opacity") && animator->currentAnimatorTime("slider_opacity") > 0) {
//                    animator->startAnimator("slider_opacity");
//                }
//            }
//            qreal m_opacity = animator->value("slider_opacity").toReal();

//            //click color animation
//            animator->setAnimatorDirectionForward("additional_opacity", sunKen);
//            if (sunKen) {
//                if (animator->currentAnimatorTime("additional_opacity") == 0) {
//                    animator->startAnimator("additional_opacity");
//                }
//            } else {
//                if (animator->currentAnimatorTime("additional_opacity") > 0) {
//                    animator->startAnimator("additional_opacity");
//                }
//            }
//            qreal s_opacity = animator->value("additional_opacity").toReal();
//            if (isUseDarkPalette()) {
//                painter->setBrush(mixColor(sp->m_ScrollBarParameters.scrollBarSliderDefaultBrush.color(),
//                                           bar->palette.color(QPalette::Active, QPalette::WindowText), m_opacity + (0.1 - s_opacity)));
//            } else {
//                painter->setBrush(mixColor(sp->m_ScrollBarParameters.scrollBarSliderDefaultBrush.color(),
//                                           bar->palette.color(QPalette::Active, QPalette::WindowText), m_opacity + s_opacity));
//            }

            QBrush brush;
            if (!enable){
                brush = sp->m_ScrollBarParameters.scrollBarSliderDisableBrush;
            }
            else if (sunKen && ((bar->activeSubControls & SC_ScrollBarSlider) ||
                                (bar->activeSubControls & SC_ScrollBarAddPage) ||
                                (bar->activeSubControls & SC_ScrollBarSubPage))){

                 brush = sp->m_ScrollBarParameters.scrollBarSliderClickBrush;
            }
            else if(mouseOver && ((bar->activeSubControls & SC_ScrollBarSlider) ||
                                  (bar->activeSubControls & SC_ScrollBarAddPage) ||
                                  (bar->activeSubControls & SC_ScrollBarSubPage))){

                brush = sp->m_ScrollBarParameters.scrollBarSliderHoverBrush;
            }
            /*else if (animator->isRunning("slider_opacity")) {//fashion style hoverbrush is QLinearGradient
                painter->setOpacity(0.5 * m_opacity);
                QColor hColor = sp->m_ScrollBarParameters.scrollBarSliderHoverBrush.color();
                QColor dColor = sp->m_ScrollBarParameters.scrollBarSliderDefaultBrush.color();

                color.setRed(dColor.red() + (hColor.red() - dColor.red()) * m_opacity);
                color.setBlue(dColor.blue() + (hColor.blue() - dColor.blue()) * m_opacity);
                color.setGreen(dColor.green() + (hColor.green() - dColor.green()) * m_opacity);
                color.setAlphaF(dColor.alphaF() + (hColor.alphaF() - dColor.alphaF()) * m_opacity);
            }*/
            else {
                brush = sp->m_ScrollBarParameters.scrollBarSliderDefaultBrush;
            }
            painter->setBrush(brush);
            painter->drawRoundedRect(drawRect, sp->m_ScrollBarParameters.radius, sp->m_ScrollBarParameters.radius);
            painter->restore();
            return;
        }
        break;
    }

    case CE_ScrollBarAddLine: {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const bool active = bar->state & State_Active;
            painter->save();
            painter->setPen(Qt::NoPen);
            if (active) {
                painter->setBrush(sp->m_ScrollBarParameters.scrollBarGrooveDefaultBrush);
            } else {
                painter->setBrush(sp->m_ScrollBarParameters.scrollBarGrooveInactiveBrush);
            }
            painter->drawRect(bar->rect);
            painter->restore();
            painter->save();

            QColor color = sp->Indicator_IconDefault;
            if(bar->sliderValue == bar->maximum || !(bar->state & State_Enabled))
                color = sp->Indicator_IconDisable;
            else if((bar->activeSubControls & SC_ScrollBarAddLine) && option->state & State_Enabled){
                if (option->state & (State_On | State_Sunken)) {
                    color = sp->Indicator_IconSunken;
                }
                else if (option->state & State_MouseOver) {
                    color = sp->Indicator_IconHover;
                }
            }
            QIcon icon = QIcon::fromTheme("lingmo-start-symbolic");

            const bool horizontal = (bar->orientation == Qt::Horizontal);
            if(horizontal)
                icon = QIcon::fromTheme(bar->direction == Qt::LeftToRight ? "lingmo-end-symbolic" : "lingmo-start-symbolic");
            else
                icon = QIcon::fromTheme(bar->direction == Qt::LeftToRight ? "lingmo-down-symbolic" : "lingmo-up-symbolic");

            if (!icon.isNull()) {
                int iconSize = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
                QPixmap pixmap = icon.pixmap(QSize(iconSize, iconSize), QIcon::Normal, QIcon::On);

                if(!sp->indicatorIconHoverNeedHighLight)
                    pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                else
                    pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                painter->save();
                drawColorPixmap(painter, color, pixmap);
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                proxy()->drawItemPixmap(painter, option->rect, Qt::AlignCenter, pixmap);
                painter->restore();
            }
        }
        painter->restore();
        return;
    }

    case CE_ScrollBarSubLine: {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const bool active = bar->state & State_Active;
            painter->save();
            painter->setPen(Qt::NoPen);
            if (active) {
                painter->setBrush(sp->m_ScrollBarParameters.scrollBarGrooveDefaultBrush);
            } else {
                painter->setBrush(sp->m_ScrollBarParameters.scrollBarGrooveInactiveBrush);
            }
            painter->drawRect(bar->rect);
            painter->restore();
            painter->save();
            QColor color = sp->Indicator_IconDefault;
            if(bar->sliderValue == bar->minimum || !(bar->state & State_Enabled))
                color = sp->Indicator_IconDisable;
            else if((bar->activeSubControls & SC_ScrollBarSubLine) && option->state & State_Enabled){
                if (option->state & (State_On | State_Sunken)) {
                    color = sp->Indicator_IconSunken;
                }
                else if (option->state & State_MouseOver) {
                    color = sp->Indicator_IconHover;
                }
            }

            QIcon icon = QIcon::fromTheme("lingmo-start-symbolic");

            const bool horizontal = (bar->orientation == Qt::Horizontal);
            if(horizontal)
                icon = QIcon::fromTheme(bar->direction == Qt::LeftToRight ? "lingmo-start-symbolic": "lingmo-end-symbolic");
            else
                icon = QIcon::fromTheme(bar->direction == Qt::LeftToRight ? "lingmo-up-symbolic" : "lingmo-down-symbolic");

            if (!icon.isNull()) {
                int iconSize = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
                QPixmap pixmap = icon.pixmap(QSize(iconSize, iconSize), QIcon::Normal, QIcon::On);

                if(!sp->indicatorIconHoverNeedHighLight)
                    pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                else
                    pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                painter->save();
                drawColorPixmap(painter, color, pixmap);
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                proxy()->drawItemPixmap(painter, option->rect, Qt::AlignCenter, pixmap);
                painter->restore();
            }
        }
        painter->restore();
        return;
    }

    case CE_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            proxy()->drawControl(CE_PushButtonBevel, option, painter, widget);
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, option, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    case CE_PushButtonBevel:
    {
        sp->initConfigPushButtonBevelParameters(isUseDarkPalette(), option, widget);
        proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);
        return;
    }

    case CE_PushButtonLabel:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            sp->initConfigPushButtonLabelParameters(isUseDarkPalette(), option, widget);
            const bool enable = button->state & State_Enabled;
            const bool text = !button->text.isEmpty();
            const bool icon = !button->icon.isNull();
            const bool inactive = enable && !(button->state & State_Active);

            bool isImportant = false;
            if (widget && widget->property("isImportant").isValid()) {
                isImportant = widget->property("isImportant").toBool();
            }

            QRect drawRect = button->rect;
            int spacing = 8;
            QStyleOption sub = *option;
            if (!sp->m_PushButtonParameters.iconHoverClickHighlight) {
                sub.state = enable ? State_Enabled : State_None;
            } else {
                sub.state = option->state;
                if (isImportant && !(button->features & QStyleOptionButton::Flat)) {
                    sub.state = option->state | State_On;
                }
            }

            if (button->features & QStyleOptionButton::HasMenu) {
                QRect arrowRect;
                int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
                arrowRect.setRect(drawRect.right() - indicator, drawRect.top() + (drawRect.height() - indicator) / 2, indicator, indicator);
                arrowRect = visualRect(option->direction, option->rect, arrowRect);
                if (!text && !icon) {
                    spacing = 0;
                }
                drawRect.setWidth(drawRect.width() - indicator - spacing);
                drawRect = visualRect(button->direction, button->rect, drawRect);
                sub.rect = arrowRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
            }

            int tf = Qt::AlignCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, button, widget)) {
                tf |= Qt::TextShowMnemonic;
            }
            QPixmap pixmap;
            if (icon) {
                QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                if (mode == QIcon::Normal && button->state & State_HasFocus) {
                    mode = QIcon::Active;
                }
                QIcon::State state = QIcon::Off;
                if (button->state & State_On) {
                    state = QIcon::On;
                }
                pixmap = button->icon.pixmap(button->iconSize, mode, state);
                pixmap = proxy()->generatedIconPixmap(mode, pixmap, button);
            }

            QFontMetrics fm = button->fontMetrics;
            int textWidth = fm.boundingRect(option->rect, tf, button->text).width() + 2;
            int iconWidth = icon ? button->iconSize.width() : 0;
            QRect iconRect, textRect;
            if (icon && text) {
                int width = textWidth + spacing + iconWidth;
                if (width > drawRect.width()) {
                    width = drawRect.width();
                    textWidth = width - spacing - iconWidth;
                }
                textRect.setRect(drawRect.x(), drawRect.y(), width, drawRect.height());
                textRect.moveCenter(drawRect.center());
                iconRect.setRect(textRect.left(), textRect.top(), iconWidth, textRect.height());
                textRect.setRect(iconRect.right() + spacing + 1, textRect.y(), textWidth, textRect.height());
                iconRect = visualRect(option->direction, drawRect, iconRect);
                textRect = visualRect(option->direction, drawRect, textRect);
            } else if (icon) {
                iconRect = drawRect;
            } else if (text) {
                textRect = drawRect;
            }

            if (iconRect.isValid()) {
                bool qqcButtonUseSymbolicIcon = option->styleObject && option->styleObject->inherits("QQuickStyleItem1") && button->icon.name().contains("symbolic");
                if(sp->getAdjustColorRules() == LINGMOConfigStyleParameters::AdjustColorRules::HSL &&
                        inactive && useTransparentButtonList().contains(qAppName()) && widget &&
                        widget->property("isWindowButton").isValid() &&
                        (widget->property("isWindowButton").toInt() == 0x01 ||
                         widget->property("isWindowButton").toInt() == 0x02)) {
                    QColor color = QColor(255, 255, 255);
                    color.setAlphaF(0.45);
                    drawColorPixmap(painter, color, pixmap);
                }
                else if ((widget && !widget->property("useIconHighlightEffect").isValid()) || qqcButtonUseSymbolicIcon) {
                    if (!sp->m_PushButtonParameters.iconHoverClickHighlight) {
                        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, &sub, widget);
                    }
                    else{
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    }
                } else {
                    pixmap = HighLightEffect::generatePixmap(pixmap, &sub, widget);
                }
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                /*
                 * Use pushbutton own highlighteffect rule. Do not use lingmo drawItemPixmap
                */
//                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
                QStyle::drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
                painter->restore();
            }

            if (textRect.isValid()) {
//                if (enable) {
//                    if (sp->m_PushButtonParameters.textHoverClickHighlight) {
//                        proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::HighlightedText);
//                    } else {
//                        proxy()->drawItemText(painter, textRect, tf, button->palette, true, button->text, QPalette::ButtonText);
//                    }
//                } else {
//                    proxy()->drawItemText(painter, textRect, tf, button->palette, false, button->text, QPalette::ButtonText);
//                }
                QColor penColor = sp->m_PushButtonParameters.textDefaultColor;
                if(!enable)
                    penColor = sp->m_PushButtonParameters.textDisableColor;
                else if(button->state & QStyle::State_MouseOver || button->state & QStyle::State_Sunken)
                    penColor = sp->m_PushButtonParameters.textHoverColor;

                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(penColor));
                painter->drawText(textRect, button->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));

//                proxy()->drawItemText(painter, textRect, tf, button->palette, false, button->text, sp->m_PushButtonParameters.textColor);

            }
            return;
        }
        break;
    }

    case CE_ToolButtonLabel:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            sp->initConfigToolButtonLabelParameters(isUseDarkPalette(), option, widget);
            const bool text = !tb->text.isEmpty();
            const bool icon = !tb->icon.isNull();
            const bool arrow = tb->features & QStyleOptionToolButton::MenuButtonPopup;
            const bool ha = tb->features & QStyleOptionToolButton::Arrow;
            const bool enable = tb->state & State_Enabled;
            const bool inactive = enable && !(tb->state & State_Active);
            QFontMetrics fm = tb->fontMetrics;
            int Margin_Height = sp->Button_MarginHeight;
            int ToolButton_MarginWidth = sp->ToolButton_MarginWidth;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);

            int iconWidth = (icon || ha) ? tb->iconSize.width() : 0;
            int spacing = 8;
            QRect textRect, iconRect, arrowRect;
            QRect drawRect = tb->rect;

            QStyleOption sub = *option;
            if (!sp->m_ToolButtonParameters.iconHoverClickHighlight) {
                sub.state = enable ? State_Enabled : State_None;
            } else {
                sub.state = option->state;
            }

            if (arrow) {
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
                drawRect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
                arrowRect.setRect(drawRect.right() - mbi + 1, drawRect.y(), mbi, drawRect.height());
                drawRect.adjust(0, 0, - (mbi + spacing), 0);
                arrowRect = visualRect(option->direction, option->rect, arrowRect);
                drawRect = visualRect(option->direction, option->rect, drawRect);
                sub.rect = arrowRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
            } else {
                if (tb->toolButtonStyle == Qt::ToolButtonTextOnly) {
                    drawRect.adjust(Button_MarginWidth, 0, -Button_MarginWidth, 0);
                } else if (tb->toolButtonStyle == Qt::ToolButtonIconOnly){
                    drawRect.adjust(0, Margin_Height, 0, -Margin_Height);
                } else {
                    drawRect.adjust(ToolButton_MarginWidth, Margin_Height, -ToolButton_MarginWidth, -Margin_Height);
                }

            }

            int alignment = Qt::AlignCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextShowMnemonic;
            int textDis = fm.boundingRect(option->rect, alignment, tb->text).width() + 2;
            QPixmap pixmap;
            if (icon) {
                QIcon::State state = tb->state & State_On ? QIcon::On : QIcon::Off;
                QIcon::Mode mode;
                if (!enable)
                    mode = QIcon::Disabled;
                else if ((tb->state & State_MouseOver) && (tb->state & State_AutoRaise))
                    mode = QIcon::Active;
                else
                    mode = QIcon::Normal;
                pixmap = tb->icon.pixmap(tb->iconSize, mode, state);
                pixmap = proxy()->generatedIconPixmap(mode, pixmap, tb);
            }

            if (tb->toolButtonStyle == Qt::ToolButtonTextOnly && text) {
                textRect = drawRect;
            } else if (tb->toolButtonStyle == Qt::ToolButtonIconOnly) {
                if (icon || ha) {
                    iconRect = drawRect;
                } else if (text) {
                    textRect = drawRect;
                }
            } else if (tb->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                if (text) {
                    int width = iconWidth + spacing + textDis;
                    if (width > drawRect.width()) {
                        width = drawRect.width();
                        textDis = drawRect.width() - iconWidth - spacing;
                    }
                    textRect.setRect(drawRect.x(), drawRect.y(), width, drawRect.height());
                    textRect.moveCenter(drawRect.center());
                    iconRect.setRect(textRect.x(), textRect.y(), iconWidth, textRect.height());
                    textRect.setRect(iconRect.right() + spacing + 1, textRect.y(), textDis, textRect.height());
                    iconRect = visualRect(option->direction, drawRect, iconRect);
                    textRect = visualRect(option->direction, drawRect, textRect);
                } else {
                    iconRect = drawRect;
                }
            } else if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                if (text) {
                    textDis = qMax(iconWidth, textDis);
                    int height = fm.size(Qt::TextShowMnemonic, tb->text).height() + spacing + iconWidth;
                    if (textDis > drawRect.width())
                        textDis = drawRect.width();
                    textRect.setRect(drawRect.x(), drawRect.y(), textDis, height);
                    textRect.moveCenter(drawRect.center());
                    iconRect.setRect(textRect.x(), textRect.y(), textDis, iconWidth);
                    textRect.setRect(textRect.x(), iconRect.bottom() + spacing + 1, textDis, fm.size(Qt::TextShowMnemonic, tb->text).height());
                } else {
                    iconRect = drawRect;
                }
            }

            if (textRect.isValid()) {
//                if (enable) {
//                    if (sp->m_ToolButtonParameters.textHoverClickHighlight) {
//                        proxy()->drawItemText(painter, textRect, alignment, tb->palette, true, tb->text, QPalette::HighlightedText);
//                    } else {
//                        proxy()->drawItemText(painter, textRect, alignment, tb->palette, true, tb->text, QPalette::ButtonText);
//                    }
//                } else {
//                    proxy()->drawItemText(painter, textRect, alignment, tb->palette, false, tb->text, QPalette::ButtonText);
//                }
                QColor penColor = sp->m_ToolButtonParameters.textDefaultColor;
                if(!enable)
                    penColor = sp->m_ToolButtonParameters.textDisableColor;
                else if(tb->state & QStyle::State_MouseOver || tb->state & QStyle::State_Sunken)
                    penColor = sp->m_ToolButtonParameters.textHoverColor;

                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(penColor));
                painter->drawText(textRect, tb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
            }

            if (iconRect.isValid()) {
                sub.rect = iconRect;
                switch (tb->arrowType) {
                case Qt::UpArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowUp, &sub, painter, widget);
                    return;
                }
                case Qt::DownArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &sub, painter, widget);
                    return;
                }
                case Qt::LeftArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowLeft, &sub, painter, widget);
                    return;
                }
                case Qt::RightArrow:
                {
                    proxy()->drawPrimitive(PE_IndicatorArrowRight, &sub, painter, widget);
                    return;
                }
                default:
                    break;
                }

                if(sp->getAdjustColorRules() == LINGMOConfigStyleParameters::AdjustColorRules::HSL &&
                        inactive && useTransparentButtonList().contains(qAppName()) &&
                        widget->property("isWindowButton").isValid() &&
                        (widget->property("isWindowButton").toInt() == 0x01 ||
                         widget->property("isWindowButton").toInt() == 0x02)) {
                    QColor color = QColor(255, 255, 255);
                    color.setAlphaF(0.45);
                    drawColorPixmap(painter, color, pixmap);
                }
                else if (widget && !widget->property("useIconHighlightEffect").isValid()) {
                    if (!sp->m_ToolButtonParameters.iconHoverClickHighlight) {
                        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, &sub, widget);
                    } else {
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &sub, widget);
                    }
                } else {
                    pixmap = HighLightEffect::generatePixmap(pixmap, &sub, widget);
                }
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                /*
                 * Use toolbutton own highlighteffect rule. Do not use lingmo drawItemPixmap
                */
//                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
                QStyle::drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
                painter->restore();
            }
            return;
        }
        break;
    }

    case CE_TabBarTab:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            proxy()->drawControl(CE_TabBarTabShape, tab, painter, widget);
            proxy()->drawControl(CE_TabBarTabLabel, tab, painter, widget);
            return;
        }
        break;
    }

    case CE_TabBarTabShape:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            sp->initConfigTabWidgetParameters(isUseDarkPalette(), option, widget);
            bool selected = tab->state & State_Selected;
            bool hover = tab->state & State_MouseOver;
            bool click = tab->state & (State_On | State_Sunken);
            QRect drawRect = option->rect;
            QTransform rotMatrix;
            bool flip = false;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing, true);

            switch (tab->shape) {
            case QTabBar::RoundedNorth:
                break;
            case QTabBar::RoundedSouth:
            {
                rotMatrix.rotate(180);
                rotMatrix.translate(0, -tab->rect.height() + 1);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            }
            case QTabBar::RoundedWest:
            {
                flip = true;
                rotMatrix.rotate(180 + 90);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            }
            case QTabBar::RoundedEast:
            {
                flip = true;
                rotMatrix.rotate(90);
                rotMatrix.translate(0, - tab->rect.width() + 1);
                painter->setTransform(rotMatrix);
                break;
            }
            default:
                painter->restore();
                QCommonStyle::drawControl(element, tab, painter, widget);
                return;
            }

            if (flip) {
                drawRect.adjust(0, 0, 1, 0);
                QRect tmp = drawRect;
                drawRect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
            } else {
                drawRect.adjust(0, 0, 0, 1);
            }

//            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
//                               && (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::RoundedSouth));
//            bool fisttab = ((!rtlHorTabs && tab->position == QStyleOptionTab::Beginning)
//                            || (rtlHorTabs && tab->position == QStyleOptionTab::End));
//            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
//                            || (rtlHorTabs && tab->position == QStyleOptionTab::Beginning));
//            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;

            int tabOverlap = proxy()->pixelMetric(PM_TabBarTabOverlap, option, widget);
            int TabBarTab_Radius = sp->m_TabWidgetParameters.radius;
            int moveWidth = 0;
            if(widget != NULL && qobject_cast<const QTabBar*>(widget)){
                const auto *tabbar = qobject_cast<const QTabBar*>(widget);
                moveWidth = (tabbar->tabAt(QPoint(drawRect.x(),drawRect.y())) == tabbar->count() - 1) ? 1 : 0;
            }
            QPainterPath path;
            QPainterPath penPath;
            if (selected) {
                drawRect.adjust(0, 0, tabOverlap, 0);
                path.moveTo(drawRect.left() + TabBarTab_Radius, drawRect.top());
                path.arcTo(QRect(drawRect.left(), drawRect.top(), TabBarTab_Radius * 2, TabBarTab_Radius * 2), 90, 90);
                path.lineTo(drawRect.left(), drawRect.bottom() - TabBarTab_Radius);
                path.arcTo(QRect(drawRect.left() - TabBarTab_Radius * 2, drawRect.bottom() - TabBarTab_Radius * 2,
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, -90);
                path.lineTo(drawRect.right() + TabBarTab_Radius, drawRect.bottom());
                path.arcTo(QRect(drawRect.right(), drawRect.bottom() - TabBarTab_Radius * 2,
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 270, -90);
                path.lineTo(drawRect.right(), drawRect.top() + TabBarTab_Radius);
                path.arcTo(QRect(drawRect.right() - TabBarTab_Radius * 2, drawRect.top(),
                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, 90);
                path.lineTo(drawRect.left() + TabBarTab_Radius, drawRect.top());

//                penPath.moveTo(drawRect.left() + TabBarTab_Radius, drawRect.top());
//                penPath.arcTo(QRect(drawRect.left(), drawRect.top(), TabBarTab_Radius * 2, TabBarTab_Radius * 2), 90, 90);
//                penPath.lineTo(drawRect.left(), drawRect.bottom() - TabBarTab_Radius);
//                penPath.arcTo(QRect(drawRect.left() - TabBarTab_Radius * 2, drawRect.bottom() - TabBarTab_Radius * 2,
//                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, -90);
//                penPath.moveTo(drawRect.right() - moveWidth + TabBarTab_Radius, drawRect.bottom());
//                penPath.arcTo(QRect(drawRect.right() - moveWidth, drawRect.bottom() - TabBarTab_Radius * 2,
//                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 270, -90);
//                penPath.lineTo(drawRect.right() - moveWidth, drawRect.top() + TabBarTab_Radius);
//                penPath.arcTo(QRect(drawRect.right() - moveWidth - TabBarTab_Radius * 2, drawRect.top(),
//                                 TabBarTab_Radius * 2, TabBarTab_Radius * 2), 0, 90);
//                penPath.lineTo(drawRect.left() + TabBarTab_Radius, drawRect.top());
            } else if (hover){

                path.addRoundedRect(drawRect, TabBarTab_Radius, TabBarTab_Radius);

            } else {

                path.addRect(drawRect);
            }

            int penWidth = (sp->m_TabWidgetParameters.tabBarDefaultPen == Qt::NoPen ||
                            sp->m_TabWidgetParameters.tabBarDefaultPen.color().alpha() == 0) ?
                        0 : sp->m_TabWidgetParameters.tabBarDefaultPen.width();
            int radius = sp->m_TabWidgetParameters.radius;
            QPen pen = Qt::NoPen;
//            drawRect.adjust(0, 0, -1 * penWidth, 0);
//            penPath.moveTo(drawRect.left(), drawRect.bottom() - penWidth - radius);
//            penPath.lineTo(drawRect.left(), drawRect.top());
//            penPath.lineTo(drawRect.right(), drawRect.top());
//            penPath.lineTo(drawRect.right(), drawRect.bottom() - penWidth);
//            penPath.lineTo(drawRect.left(), drawRect.bottom() - penWidth);

            if(click) {
                penWidth = sp->m_TabWidgetParameters.tabBarClickPen.width();
                painter->setBrush(sp->m_TabWidgetParameters.tabBarClickBrush);
                pen = sp->m_TabWidgetParameters.tabBarClickPen;
            }
            else if (selected) {
                painter->setBrush(sp->m_TabWidgetParameters.tabBarSelectBrush);
            } else if (hover) {
                penWidth = sp->m_TabWidgetParameters.tabBarHoverPen.width();
                painter->setBrush(sp->m_TabWidgetParameters.tabBarHoverBrush);
                pen = sp->m_TabWidgetParameters.tabBarHoverPen;
            } else{
                penWidth = sp->m_TabWidgetParameters.tabBarDefaultPen.width();
                painter->setBrush(sp->m_TabWidgetParameters.tabBarDefaultBrush);
                pen = sp->m_TabWidgetParameters.tabBarDefaultPen;
            }
            if((tab->state & State_Enabled) && (tab->state & State_HasFocus)){
                penWidth = sp->m_TabWidgetParameters.tabBarFocusPen.width();
                pen = sp->m_TabWidgetParameters.tabBarFocusPen;
            }
            painter->drawPath(path);
            painter->restore();
            painter->save();
            painter->setPen(penWidth > 0 ? pen : Qt::NoPen);
            painter->setBrush(Qt::NoBrush);
            painter->setRenderHint(QPainter::Antialiasing, true);
//            if(painter->pen().width() == 1)
//                painter->translate(0.5, 0.5);
            painter->drawRoundedRect(drawRect.adjusted(penWidth/2, penWidth/2, -1 * penWidth/2, -1 * penWidth), radius, radius);
            painter->restore();

            return;
        }
        break;
    }

    case CE_TabBarTabLabel:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int iconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
            bool verticalTabs = tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::RoundedWest
                    || tab->shape == QTabBar::TriangularEast || tab->shape == QTabBar::TriangularWest;
            bool useSeparateLine = true;
            uint alignment = Qt::AlignLeft | Qt::AlignVCenter;
            if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextShowMnemonic;
            else
                alignment |= Qt::TextHideMnemonic;

            QRect drawRect = tab->rect;
            QRect iconRect;
            QRect textRect = proxy()->subElementRect(SE_TabBarTabText, option, widget);
            configTabLayout(tab, widget, proxy(), &textRect, &iconRect);

            if (widget && widget->property("useTabbarSeparateLine").isValid() && (!widget->property("useTabbarSeparateLine").toBool())){
                useSeparateLine = false;
            }

            painter->save();
            if (verticalTabs) {
                int newX, newY, newRot;
                if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                    newX = drawRect.width() + drawRect.x();
                    newY = drawRect.y();
                    newRot = 90;
                } else {
                    newX = drawRect.x();
                    newY = drawRect.y() + drawRect.height();
                    newRot = -90;
                }
                QTransform m;
                m.translate(newX, newY);
                m.rotate(newRot);
                painter->setTransform(m, true);
            }

            if (!tab->icon.isNull()) {
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                QIcon::Mode mode = (tab->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled;
                QPixmap pixmap = tab->icon.pixmap(widget ? widget->window()->windowHandle() : 0, tab->iconSize,
                                                  mode, (tab->state & State_Selected) ? QIcon::On : QIcon::Off);
                pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
                QPixmap drawPixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                painter->drawPixmap(iconRect.x(), iconRect.y(), drawPixmap);
                painter->restore();
            }

            proxy()->drawItemText(painter, textRect, alignment, tab->palette, tab->state & State_Enabled, tab->text, QPalette::ButtonText);
            painter->restore();

            //draw separate line
            if (useSeparateLine && sp->TabBar_SplitLine) {
                if (!(tab->state & State_Selected)) {
                    int dis = ((verticalTabs ? drawRect.width() : drawRect.height()) - iconSize) / 2;
                    painter->save();
                    painter->resetTransform();
                    painter->setPen(tab->palette.color(QPalette::Active, QPalette::Midlight));
                    painter->setBrush(Qt::NoBrush);
                    if (verticalTabs) {
                        if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                            painter->drawLine(drawRect.x() + dis, drawRect.bottom(), drawRect.right() - dis, drawRect.bottom());
                        } else {
                            painter->drawLine(drawRect.x() + dis, drawRect.top(), drawRect.right() - dis, drawRect.top());
                        }
                    } else if (tab->direction == Qt::RightToLeft) {
                        painter->drawLine(drawRect.x(), drawRect.top() + dis, drawRect.x(), drawRect.bottom() - dis);
                    } else {
                        painter->drawLine(drawRect.right(), drawRect.top() + dis, drawRect.right(), drawRect.bottom() - dis);
                    }
                    painter->restore();
                }
            }

            return;
        }
        break;
    }

    case CE_ComboBoxLabel:
    {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QRect editRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxEditField, widget);

            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            if (!comboBox->currentIcon.isNull()) {
                QIcon::Mode mode = comboBox->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                QPixmap pixmap = comboBox->currentIcon.pixmap(comboBox->iconSize, mode);
                pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
                QRect iconRect;
                if (comboBox->direction == Qt::RightToLeft) {
                    iconRect.setRect(editRect.right() - comboBox->iconSize.width(), editRect.y(), comboBox->iconSize.width(), editRect.height());
                    editRect.setRect(editRect.x(), editRect.y(), editRect.width() - iconRect.width() - 8, editRect.height());
                } else {
                    iconRect.setRect(editRect.x(), editRect.y(), comboBox->iconSize.width(), editRect.height());
                    editRect.setRect(editRect.x() + iconRect.width() + 8, editRect.y(), editRect.width() - iconRect.width() - 8, editRect.height());
                }
                pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
            }

            if (!comboBox->currentText.isEmpty() && !comboBox->editable) {
                proxy()->drawItemText(painter, editRect, visualAlignment(option->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                      option->palette, option->state & State_Enabled, comboBox->currentText);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_MenuScroller:{
        if(widget == nullptr)
            return;
        else
            return Style::drawControl(element, option, painter, widget);
    }

    case CE_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_RadioButtonIndicator, option, widget);
            proxy()->drawPrimitive(PE_IndicatorRadioButton, &subopt, painter, widget);
            subopt.rect = proxy()->subElementRect(SE_RadioButtonContents, option, widget);
            proxy()->drawControl(CE_RadioButtonLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    case CE_CheckBox:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.rect = proxy()->subElementRect(SE_CheckBoxIndicator, option, widget);
            proxy()->drawPrimitive(PE_IndicatorCheckBox, &subopt, painter, widget);
            subopt.rect = proxy()->subElementRect(SE_CheckBoxContents, option, widget);
            proxy()->drawControl(CE_CheckBoxLabel, &subopt, painter, widget);
            return;
        }
        break;
    }

    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            uint alignment = visualAlignment(button->direction, Qt::AlignLeft | Qt::AlignVCenter);
            const bool enable = button->state & State_Enabled;

            if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pixmap;
            QRect textRect = button->rect;
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            if (!button->icon.isNull()) {
                QIcon::Mode mode = enable ? QIcon::Normal : QIcon::Disabled;
                pixmap = button->icon.pixmap(button->iconSize, mode);
                pixmap = proxy()->generatedIconPixmap(mode, pixmap, button);
                pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, button, widget);
                proxy()->drawItemPixmap(painter, button->rect, alignment, pixmap);
                if (button->direction == Qt::RightToLeft)
                    textRect.setRight(textRect.right() - button->iconSize.width() - sp->IconButton_Distance);
                else
                    textRect.setLeft(textRect.left() + button->iconSize.width() + sp->IconButton_Distance);
            }
            if (!button->text.isEmpty()){
                proxy()->drawItemText(painter, textRect, alignment | Qt::TextShowMnemonic,
                                      button->palette, button->state & State_Enabled, button->text, QPalette::WindowText);
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_SizeGrip:
    {
        /*
             * Style is not required here, as required by design
             */
        return;

    }break;


        //Drawing of single menu item of menu bar
    case CE_MenuBarItem:
    {

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(option))
        {
            QStyleOptionMenuItem item = *mbi;
            item.rect = mbi->rect.adjusted(0, 1, 0, -3);
            painter->fillRect(option->rect, option->palette.window());

            uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip| Qt::TextSingleLine;

            if (!proxy()->styleHint(SH_UnderlineShortcut, mbi, widget))
                alignment |= Qt::TextHideMnemonic;

            QPixmap pix = mbi->icon.pixmap(proxy()->pixelMetric(PM_SmallIconSize, option, widget), QIcon::Normal);
            if (!pix.isNull()) {
                pix = HighLightEffect::ordinaryGeneratePixmap(pix, mbi, widget);
                proxy()->drawItemPixmap(painter,mbi->rect, alignment, pix);
            } else
                proxy()->drawItemText(painter, mbi->rect, alignment, mbi->palette, mbi->state & State_Enabled,
                                      mbi->text, QPalette::ButtonText);

            bool act = (mbi->state & State_Selected) && ((mbi->state & State_Sunken) | (mbi->state & State_HasFocus));
            bool dis = !(mbi->state & State_Enabled);


            QRect r = option->rect;
            //when hover、click and other state, begin to draw style
            if (act) {
                painter->setBrush(option->palette.highlight().color());
                painter->setPen(Qt::NoPen);
                painter->drawRoundedRect(r.adjusted(0, 0, -1, -1),4,4);

                QPalette::ColorRole textRole = dis ? QPalette::Text : QPalette::HighlightedText;
                uint alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!Style::styleHint(SH_UnderlineShortcut, mbi, widget))
                    alignment |= Qt::TextHideMnemonic;
                proxy()->drawItemText(painter, item.rect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
            } else {

            }
        }
        painter->restore();

        return;
        break;

    }



    case CE_MenuItem:
    {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            const bool enable(menuItem->state & State_Enabled);
            const bool focus = menuItem->state & State_HasFocus;
            if (menuItem->menuItemType == QStyleOptionMenuItem::EmptyArea) {
                return;
            }
            sp->initConfigMenuParameters(isUseDarkPalette(), option, widget);

            QRect drawRect = menuItem->rect;
            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                painter->save();
                QColor SepColor = menuItem->palette.color(QPalette::Active, QPalette::BrightText);
                SepColor.setAlphaF(0.1);
                painter->setPen(QPen(SepColor, 1, Qt::SolidLine, Qt::RoundCap));
                painter->setBrush(Qt::NoBrush);
                painter->drawLine(QPointF(drawRect.left() + sp->MenuItemSeparator_MarginWidth, drawRect.center().y()),
                                  QPointF(drawRect.right() - sp->MenuItemSeparator_MarginWidth, drawRect.center().y()));
                painter->restore();
                return;
            }

            const bool sunken(menuItem->state & State_Sunken);
            const bool selected(menuItem->state & State_Selected);
            const bool layout(menuItem->direction == Qt::LeftToRight);
            bool isComboBox = qobject_cast<const QComboBox*>(widget);

            //if it's comobobox popup, set combobox popup size rect
            if (isComboBox) {
                int MenuItem_HMargin = proxy()->pixelMetric(PM_MenuHMargin, option, widget);
                drawRect.adjust(MenuItem_HMargin, sp->ComboBox_VMargin, -MenuItem_HMargin, -1 * sp->ComboBox_VMargin);
            }

            if (enable && (selected | sunken)) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(Qt::NoPen);
                painter->setBrush(sp->m_MenuParameters.menuItemSelectBrush);
                painter->drawRoundedRect(drawRect, sp->m_MenuParameters.itemRadius, sp->m_MenuParameters.itemRadius);
                painter->restore();
            }
            if(enable && focus){
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(sp->m_MenuParameters.menuItemFocusPen);
                painter->setBrush(Qt::transparent);
                painter->drawRoundedRect(drawRect, sp->m_MenuParameters.itemRadius, sp->m_MenuParameters.itemRadius);
                painter->restore();
            }

            int MenuItem_Spacing = sp->MenuItem_Spacing;
            int MenuItem_HMargin = sp->MenuContent_HMargin;
            //去除item边框
            drawRect = drawRect.adjusted(MenuItem_HMargin, sp->MenuItem_MarginHeight, -MenuItem_HMargin, -sp->MenuItem_MarginHeight);

            //Checkable items draw
            if (menuItem->menuHasCheckableItems) {
                const bool checked = menuItem->checked;
                if (menuItem->checkType == QStyleOptionMenuItem::Exclusive) {
                    if (checked) {
//                        QStyleOptionButton radioOption;
//                        radioOption.QStyleOption::operator=(*option);
//                        radioOption.rect = drawRect;
//                        radioOption.state |= State_On;
//                        radioOption.rect = proxy()->subElementRect(SE_RadioButtonIndicator, &radioOption, widget);
//                        proxy()->drawPrimitive(PE_IndicatorRadioButton, &radioOption, painter, widget);
                        int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                        QIcon icon = QIcon::fromTheme("object-select-symbolic");
                        QIcon::Mode mode = enable ?  QIcon::Active : QIcon::Disabled;
                        QPixmap pixmap = icon.pixmap(iconWidth, iconWidth, mode , QIcon::On);
                        QPixmap drawPixmap;
//                        if(!isComboBox && !sp->indicatorIconHoverNeedHighLight)
//                            drawPixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
//                        else
                            drawPixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);

                        QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - iconWidth) / 2, iconWidth, iconWidth);
                        iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(Qt::NoBrush);
                        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                        painter->drawPixmap(iconRect, drawPixmap);
                        painter->restore();
                    }
                } else if (menuItem->checkType == QStyleOptionMenuItem::NonExclusive) {
                    if (checked) {
                        int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                        QIcon icon = QIcon::fromTheme("object-select-symbolic");
                        QIcon::Mode mode = enable ? (selected ? QIcon::Active : QIcon::Normal) : QIcon::Disabled;
                        QPixmap pixmap = icon.pixmap(iconWidth, iconWidth, mode , QIcon::On);
                        QPixmap drawPixmap;
//                        if(!isComboBox && !sp->indicatorIconHoverNeedHighLight)
//                            drawPixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
//                        else
                            drawPixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);

                        QRect iconRect(drawRect.x(), drawRect.y() + (drawRect.height() - iconWidth) / 2, iconWidth, iconWidth);
                        iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                        painter->save();
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(Qt::NoBrush);
                        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                        painter->drawPixmap(iconRect, drawPixmap);
                        painter->restore();
                    }
                }
            }


            bool hasIcon = false;
            if (isComboBox) {
                if(!menuItem->icon.isNull())
                    hasIcon = true;
            } else if(menuItem->maxIconWidth != 0) {
                hasIcon = true;
            }

            if (menuItem->menuHasCheckableItems || hasIcon) {
                int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                int w = (menuItem->menuHasCheckableItems ? iconWidth : 0) +
                        (hasIcon ? iconWidth : 0) +
                        ((menuItem->menuHasCheckableItems && hasIcon) ? MenuItem_Spacing : 0);
                drawRect = visualRect(menuItem->direction, drawRect, drawRect.adjusted(w + MenuItem_Spacing, 0, 0, 0));
            } else {
                drawRect = drawRect.adjusted(4, 0, -4, -0);//去除item边框
            }


            if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
                int indWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
                int indHight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
                QRect indRect(drawRect.right() - indWidth, drawRect.y() + (drawRect.height() - indHight)/2, indWidth, indHight);//箭头绘制是居中的，绘制会有写误差。
                indRect = visualRect(menuItem->direction, drawRect, indRect);
                QStyleOption indOption = *option;
                if (selected)
                    indOption.state |= State_MouseOver;
                indOption.rect = indRect;
                proxy()->drawPrimitive(layout ? PE_IndicatorArrowRight : PE_IndicatorArrowLeft, &indOption, painter, widget);
                drawRect = visualRect(menuItem->direction, drawRect, drawRect.adjusted(0, 0, -(indWidth + MenuItem_Spacing), 0));
            }

            if (hasIcon) {
                int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                QSize iconSize(smallIconSize, smallIconSize);
                QIcon::Mode mode =  enable ? (selected ? QIcon::Active : QIcon::Normal) : QIcon::Disabled;
                QIcon::State state = menuItem->checked ? QIcon::On : QIcon::Off;
                QPixmap pixmap = menuItem->icon.pixmap(iconSize, mode, state);
                pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
                QPixmap target;
//                if(!isComboBox && !sp->indicatorIconHoverNeedHighLight)
//                    target = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
//                else
                    target = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);


                int recX = drawRect.x() - (MenuItem_Spacing + smallIconSize);
                QRect iconRect(recX, drawRect.y() + (drawRect.height() - smallIconSize)/2, smallIconSize, smallIconSize);
                iconRect = visualRect(menuItem->direction, drawRect, iconRect);
                painter->save();
                painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::Text));
                painter->setBrush(Qt::NoBrush);
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                painter->drawPixmap(iconRect, target);
                painter->restore();
            }

            //text
            if (!menuItem->text.isEmpty()) {
                QString text = menuItem->text;
                painter->save();
                if (enable) {
                    if (selected | sunken) {
                        painter->setPen(sp->m_MenuParameters.menuTextHoverPen);
                    } else {
                        painter->setPen(menuItem->palette.color(QPalette::Active, QPalette::Text));
                    }
                } else {
                    painter->setPen(menuItem->palette.color(QPalette::Disabled, QPalette::Text));
                }
                painter->setBrush(Qt::NoBrush);

                int text_flags = Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine;
                if (proxy()->styleHint(SH_UnderlineShortcut, option, widget))
                    text_flags |= Qt::TextShowMnemonic;
                int textToDraw_flags = text_flags;
                int t = menuItem->text.indexOf(QLatin1Char('\t'));
                if (t >=0) {
                    const QString textToDraw = menuItem->text.mid(t + 1);
                    text = menuItem->text.left(t);
                    if (layout)
                        textToDraw_flags |= Qt::AlignRight;
                    else
                        textToDraw_flags |= Qt::AlignLeft | Qt::AlignAbsolute;
                    painter->drawText(drawRect, textToDraw_flags, textToDraw);
                }

                QFont font = menuItem->font;
                font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());
                if (menuItem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                painter->setFont(font);

                if (layout)
                    text_flags |= Qt::AlignLeft;
                else
                    text_flags |= Qt::AlignRight | Qt::AlignAbsolute;

                QFontMetrics metrics(font);
                QString elidedText = metrics.elidedText(text, Qt::ElideRight, drawRect.width());
                painter->drawText(drawRect, text_flags, elidedText);

                painter->restore();
            }
            return;
        }
        break;
    }



        // Draw LINGMO ToolBoxTab Style
    case CE_ToolBoxTab:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            proxy()->drawControl(CE_ToolBoxTabShape, tb, painter, widget);
            proxy()->drawControl(CE_ToolBoxTabLabel, tb, painter, widget);
            return;
        }
        break;

    case CE_ToolBoxTabShape:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            painter->save();
            painter->setPen(Qt::NoPen);
            if(widget && widget->isEnabled()){
                painter->setBrush(tb->palette.button().color());
                if(option->state & State_MouseOver)
                {
                    painter->setBrush(tb->palette.button().color().darker(110));
                    if(option->state & State_Sunken)
                        painter->setBrush(tb->palette.button().color().darker(110));
                }
            }
            else {
                painter->setBrush(tb->palette.color(QPalette::Disabled,QPalette::Button));
            }

            if (tb->direction != Qt::RightToLeft || tb->direction != Qt::RightToLeft) {
                painter->drawRoundedRect(option->rect,4,4);
            }
            painter->restore();
            return;
        } break;

    case CE_ToolBoxTabLabel:
        if (const QStyleOptionToolBox *tb = qstyleoption_cast<const QStyleOptionToolBox *>(option)) {
            bool enabled = tb->state & State_Enabled;
            bool selected = tb->state & State_Selected;
            int iconExtent = proxy()->pixelMetric(QStyle::PM_SmallIconSize, tb, widget);
            QPixmap pm = tb->icon.pixmap(widget ? widget->window()->windowHandle() : 0, QSize(iconExtent, iconExtent),enabled ? QIcon::Normal : QIcon::Disabled);

            QRect cr = subElementRect(QStyle::SE_ToolBoxTabContents, tb, widget);
            QRect tr, ir;
            int ih = 0;
            if (pm.isNull()) {
                tr = cr;
                tr.adjust(4, 0, -8, 0);
            } else {
                int iw = pm.width() / pm.devicePixelRatio() + 4;
                ih = pm.height()/ pm.devicePixelRatio();
                ir = QRect(cr.left() + 4, cr.top(), iw + 2, ih);
                tr = QRect(ir.right(), cr.top(), cr.width() - ir.right() - 4, cr.height());
            }

            if (selected && proxy()->styleHint(QStyle::SH_ToolBox_SelectedPageTitleBold, tb, widget)) {
                QFont f(painter->font());
                f.setBold(true);
                painter->setFont(f);
            }

            QString txt = tb->fontMetrics.elidedText(tb->text, Qt::ElideRight, tr.width());

            if (ih) {
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                painter->drawPixmap(ir.left(), (tb->rect.height() - ih) / 2, pm);
                painter->restore();
            }

            int alignment = Qt::AlignCenter | Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, tb, widget))
                alignment |= Qt::TextHideMnemonic;

            // painter->drawText(option->rect,tb->text, QTextOption(Qt::AlignCenter));
            proxy()->drawItemText(painter, tr, alignment, tb->palette, enabled, txt, QPalette::ButtonText);

            if (!txt.isEmpty() && option->state & State_HasFocus) {
                QStyleOptionFocusRect opt;
                opt.rect = tr;
                opt.palette = tb->palette;
                opt.state = QStyle::State_None;
                proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, painter, widget);
            }
            return;
        } break;

    case CE_ProgressBar:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            QStyleOptionProgressBar subOption = *pb;
            subOption.rect = proxy()->subElementRect(SE_ProgressBarGroove, pb, widget);
            proxy()->drawControl(CE_ProgressBarGroove, &subOption, painter, widget);
            subOption.rect = proxy()->subElementRect(SE_ProgressBarContents, pb, widget);
            proxy()->drawControl(CE_ProgressBarContents, &subOption, painter, widget);
            if (pb->textVisible) {
                subOption.rect = proxy()->subElementRect(SE_ProgressBarLabel, pb, widget);
                proxy()->drawControl(CE_ProgressBarLabel, &subOption, painter, widget);
            }
            return;
        }
        break;
    }

    case CE_ProgressBarGroove:
    {
        sp->initConfigProgressBarParameters(isUseDarkPalette(), option, widget);
        const bool enable = option->state & State_Enabled;
        int width = 0;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, sp->m_ProgressBarParameters.radius != 0);
        if (!enable) {
            width = sp->m_ProgressBarParameters.progressBarGrooveDisablePen.width();
            painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ProgressBarParameters.progressBarGrooveDisablePen);
            painter->setBrush(sp->m_ProgressBarParameters.progressBarGrooveDisableBrush);
        } else {
            width = sp->m_ProgressBarParameters.progressBarGrooveDefaultPen.width();
            painter->setPen(width <= 0 ? Qt::NoPen : sp->m_ProgressBarParameters.progressBarGrooveDefaultPen);
            painter->setBrush(sp->m_ProgressBarParameters.progressBarGrooveDefaultBrush);
        }
        if(width == 1 && sp->m_ProgressBarParameters.radius != 0)
            painter->translate(0.5, 0.5);
        painter->drawRoundedRect(option->rect.adjusted(width, width, -width, -width), sp->m_ProgressBarParameters.radius, sp->m_ProgressBarParameters.radius);
        painter->restore();
        return;
    }

    case CE_ProgressBarContents:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            sp->initConfigProgressBarParameters(isUseDarkPalette(), pb, widget);
            QColor progressBarContentHightColor = sp->m_ProgressBarParameters.progressBarContentHightColor;
            QColor progressBarContentMidLightColor = sp->m_ProgressBarParameters.progressBarContentMidLightColor;

            const bool vertical = pb->orientation == Qt::Vertical;
            const bool inverted = pb->invertedAppearance;

            bool reverse = (!vertical && (pb->direction == Qt::RightToLeft)) || vertical;
            if (inverted)
                reverse = !reverse;

            QLinearGradient linearGradient;
            if(vertical){
                linearGradient.setStart(pb->rect.bottomRight());
                linearGradient.setFinalStop(pb->rect.topLeft());
            }
            else{
                linearGradient.setStart(pb->rect.topLeft());
                linearGradient.setFinalStop(pb->rect.bottomRight());
            }
            linearGradient.setColorAt(0, progressBarContentMidLightColor);
            linearGradient.setColorAt(1, progressBarContentHightColor);
            QBrush brush(linearGradient);

            if (QVariantAnimation *animation = m_animation_helper->animation(option->styleObject)) {
                if(animation->state() == QAbstractAnimation::Running){
                    bool forward = animation->direction() == QAbstractAnimation::Forward;
                    if(!forward)
                        reverse = !reverse;

                    linearGradient.setColorAt(0, progressBarContentMidLightColor);
                    linearGradient.setColorAt(1, progressBarContentHightColor);

                    if (vertical) {
                        if (reverse) {
                            linearGradient.setStart(pb->rect.bottomLeft());
                            linearGradient.setFinalStop(pb->rect.topLeft());
                        } else {
                            linearGradient.setStart(pb->rect.topLeft());
                            linearGradient.setFinalStop(pb->rect.bottomLeft());
                        }
                    } else {
                        if (reverse) {
                            linearGradient.setStart(pb->rect.topRight());
                            linearGradient.setFinalStop(pb->rect.topLeft());
                        } else {
                            linearGradient.setStart(pb->rect.topLeft());
                            linearGradient.setFinalStop(pb->rect.topRight());
                        }
                    }

                    brush = QBrush(linearGradient);
                }
            }


            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, sp->m_ProgressBarParameters.radius != 0);
            QColor penColor = sp->m_ProgressBarParameters.progressBarContentPen.color();
            int width = (penColor.alpha() == 0 || penColor == Qt::NoPen) ? 0 : sp->m_ProgressBarParameters.progressBarContentPen.width();
            painter->setPen(width > 0 ? sp->m_ProgressBarParameters.progressBarContentPen : Qt::NoPen);
    //            if(width == 1 && sp->m_ProgressBarParameters.radius != 0)
    //                painter->translate(0.5, 0.5);
            painter->setBrush(brush);

            width = (sp->m_ProgressBarParameters.progressBarGrooveDefaultPen.color().alpha() == 0 ||  sp->m_ProgressBarParameters.progressBarGrooveDefaultPen.color() == Qt::NoPen) ?
                        0 : sp->m_ProgressBarParameters.progressBarGrooveDefaultPen.width();
            QRect rect = width > 0 ? pb->rect.adjusted(width + 1, width + 1, -1 * width, -1 * width) : pb->rect;

            if(pb->rect.width() == 0)
                rect.setWidth(0);
            if(pb->rect.height() == 0)
                rect.setHeight(0);
            painter->drawRoundedRect(rect, sp->m_ProgressBarParameters.radius, sp->m_ProgressBarParameters.radius);
            painter->restore();
            return;
        }
        break;
    }

    case CE_ProgressBarLabel:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            if (pb->textVisible) {
                const auto progress = qMax(pb->progress, pb->minimum);
                const bool vertical = pb->orientation == Qt::Vertical;
                const bool inverted = pb->invertedAppearance;
                const bool indeterminate = (pb->minimum == 0 && pb->maximum == 0);

                int maxWidth = vertical ? pb->rect.height() : pb->rect.width();
                const auto totalSteps = qMax(Q_INT64_C(1), qint64(pb->maximum) - pb->minimum);
                const auto progressSteps = qint64(progress) - pb->minimum;
                const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
                int len = indeterminate ? maxWidth : progressBarWidth;

                bool reverse = (!vertical && (pb->direction == Qt::RightToLeft)) || vertical;
                if (inverted)
                    reverse = !reverse;

                painter->save();
                painter->setBrush(Qt::NoBrush);
                QRect rect = pb->rect;
                if (pb->orientation == Qt::Vertical) {
                    rect.setRect(rect.y(), rect.x(), rect.height(), rect.width());
                    QTransform m;
                    m.rotate(90);
                    m.translate(0, -rect.height());
                    painter->setTransform(m, true);
                }
                QRect textRect(rect.x(), rect.y(), pb->fontMetrics.horizontalAdvance(pb->text), rect.height());
                textRect.moveCenter(rect.center());
                if (len <= textRect.left()) {
                    painter->setPen(pb->palette.color(QPalette::Active, QPalette::WindowText));
                    painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                } else if (len >= textRect.right()) {
                    painter->setPen(pb->palette.color(QPalette::Active, QPalette::HighlightedText));
                    painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                } else {
                    QRect leftRect(textRect.x(), textRect.y(), len - textRect.left(), textRect.height());
                    QRect rightRect(leftRect.right() + 1, textRect.y(), textRect.right() + 1 - len, textRect.height());
                    if (reverse) {
                        leftRect.setRect(textRect.left(), textRect.top(), maxWidth - len - textRect.left(), textRect.height());
                        rightRect.setRect(leftRect.right() + 1, textRect.top(), textRect.width() - leftRect.width(), textRect.height());
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::HighlightedText));
                        painter->setClipRect(rightRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::WindowText));
                        painter->setClipRect(leftRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                    } else {
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::WindowText));
                        painter->setClipRect(rightRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                        painter->setPen(pb->palette.color(QPalette::Active, QPalette::HighlightedText));
                        painter->setClipRect(leftRect);
                        painter->drawText(textRect, pb->text, QTextOption(Qt::AlignAbsolute | Qt::AlignHCenter | Qt::AlignVCenter));
                    }
                }
                painter->resetTransform();
                painter->restore();
            }
            return;
        }
        break;
    }

    case CE_Header:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QRegion clipRegion = painter->clipRegion();
            painter->setClipRect(option->rect);
            proxy()->drawControl(CE_HeaderSection, option, painter, widget);
            QStyleOptionHeader subopt = *header;
            subopt.rect = proxy()->subElementRect(SE_HeaderLabel, header, widget);
            if (subopt.rect.isValid())
                proxy()->drawControl(CE_HeaderLabel, &subopt, painter, widget);
            if (header->sortIndicator != QStyleOptionHeader::None) {
                subopt.rect = proxy()->subElementRect(SE_HeaderArrow, option, widget);
                proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
            }
            painter->setClipRegion(clipRegion);

            return;
        }
        break;
    }

    case CE_HeaderSection:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            const bool enable = header->state & State_Enabled;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(header->palette.brush(enable ? QPalette::Active : QPalette::Disabled, QPalette::Base));
            painter->drawRect(header->rect);
            painter->restore();
            painter->save();
            QColor color = header->palette.color(QPalette::Active, isUseDarkPalette() ? QPalette::Midlight : QPalette::Shadow);
            painter->setPen(color);
            painter->setBrush(Qt::NoBrush);
            if (header->orientation == Qt::Horizontal) {
//                int iconSize = proxy()->pixelMetric(PM_SmallIconSize);
//                int dis = (header->rect.height() - iconSize) / 2 - 1;
                int dis = (header->rect.height() / 2) / 2;
                if (header->position != QStyleOptionHeader::End && header->position != QStyleOptionHeader::OnlyOneSection) {
                    if (header->direction == Qt::LeftToRight) {
                        painter->drawLine(header->rect.right(), header->rect.top() + dis, header->rect.right(), header->rect.bottom() - dis);
                    } else {
                        painter->drawLine(header->rect.left(), header->rect.top() + dis, header->rect.left(), header->rect.bottom() - dis);
                    }
                }
                painter->drawLine(header->rect.left(), header->rect.bottom(), header->rect.right(), header->rect.bottom());
            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_HeaderLabel:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QRect rect = header->rect;
            QRect textRect = header->rect;
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
            textRect.adjust(margin, 0, -margin, -0);

            int iconSize = proxy()->pixelMetric(PM_SmallIconSize);
            if (!header->icon.isNull()) {
                QIcon::Mode mode =  header->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                QPixmap pixmap = header->icon.pixmap(iconSize, iconSize, mode);
                pixmap = proxy()->generatedIconPixmap(mode, pixmap, option);
                pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                QRect iconRect(textRect.x(), textRect.y() + (textRect.height() - iconSize) / 2, iconSize, iconSize);
                textRect.setRect(iconRect.right() + 1 + 8, textRect.y(), textRect.width() - iconRect.width() - 8, textRect.height());
                iconRect = visualRect(header->direction, rect, iconRect);
                textRect = visualRect(header->direction, rect, textRect);
                painter->save();
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                painter->drawPixmap(iconRect, pixmap);
                painter->restore();
            }

            if (header->state & QStyle::State_On || header->state & QStyle::State_Sunken) {
                QFont font = painter->font();
                font.setBold(true);
                painter->setFont(font);
            }
            proxy()->drawItemText(painter, textRect, header->textAlignment | Qt::AlignVCenter, header->palette,
                                  (header->state & State_Enabled), header->text, QPalette::ButtonText);
            return;
        }
        break;
    }

    case CE_HeaderEmptyArea:
    {
        const bool enable = option->state & State_Enabled;
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::NoBrush);//option->palette.brush(enable ? QPalette::Active : QPalette::Disabled, QPalette::Base));
        painter->drawRect(option->rect);
        painter->restore();
        return;
    }

    case CE_ItemViewItem:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            painter->save();
            if (painter->clipPath().isEmpty())
                painter->setClipRect(option->rect);

            QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, option, widget);
            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, option, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, option, widget);

            proxy()->drawPrimitive(PE_PanelItemViewItem, option, painter, widget);


            if (vi->features & QStyleOptionViewItem::HasCheckIndicator) {
                QStyleOptionButton option;
                option.rect = checkRect;
                option.state = vi->state & ~QStyle::State_HasFocus;
                option.palette = vi->palette;

                switch (vi->checkState) {
                case Qt::Unchecked:
                    option.state |= QStyle::State_Off;
                    break;
                case Qt::PartiallyChecked:
                    option.state |= QStyle::State_NoChange;
                    break;
                case Qt::Checked:
                    option.state |= QStyle::State_On;
                    break;
                }
                proxy()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, painter, widget);
            }

            if (!vi->icon.isNull()) {
                QIcon::Mode mode = QIcon::Normal;
                if (!(vi->state & QStyle::State_Enabled))
                    mode = QIcon::Disabled;
//                else if (vi->state & QStyle::State_Selected)
//                    mode = QIcon::Selected;
                QIcon::State state = vi->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
                QPixmap pixmap = vi->icon.pixmap(vi->decorationSize, mode, state);

                if(!sp->indicatorIconHoverNeedHighLight)
                    pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                else if(widget && (widget->inherits("QTreeView") || widget->inherits("QTreeWidget"))){
                    if(widget->property("highlightMode").isValid() && widget->property("highlightMode").toBool())
                        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);
                    else
                        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, option, widget);
                }
                else
                    pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, option, widget);

                //pixmap = proxy()->generatedIconPixmap(mode, pixmap, vi);
                painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                proxy()->drawItemPixmap(painter, iconRect, vi->decorationAlignment, HighLightEffect::ordinaryGeneratePixmap(pixmap, vi, widget));
            }



            QPen ViewItemHover;
            QPen ViewItemSelect;
            QPen ViewItemDisable;

            if (qobject_cast<const QTreeView*>(widget) || qobject_cast<const QTreeWidget*>(widget)) {
                ViewItemHover = sp->m_TreeParameters.treeTextHoverPen;
                ViewItemSelect = sp->m_TreeParameters.treeTextSelectPen;
                ViewItemDisable = sp->m_TreeParameters.treeTextDisablePen;
            } else if(qobject_cast<const QListView *>(widget) || qobject_cast<const QListWidget *>(widget)){
                ViewItemHover = sp->m_ListParameters.listTextHoverPen;
                ViewItemSelect = sp->m_ListParameters.listTextSelectPen;
                ViewItemDisable = sp->m_ListParameters.listTextDisablePen;
            } else if(qobject_cast<const QTableView *>(widget) || qobject_cast<const QTableWidget *>(widget)){
                ViewItemHover = sp->m_TableParameters.tableTextHoverPen;
                ViewItemSelect = sp->m_TableParameters.tableTextSelectPen;
                ViewItemDisable = sp->m_TableParameters.tableTextDisablePen;
            }
            else {

            }

            if (!vi->text.isEmpty()) {
                QPalette::ColorGroup cg = vi->state & QStyle::State_Enabled
                        ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vi->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                if(!vi->state & QStyle::State_Enabled)
                    painter->setPen(ViewItemDisable);
                else if (((vi->decorationPosition == QStyleOptionViewItem::Top) || (vi->decorationPosition ==  QStyleOptionViewItem::Bottom))
                        && !(vi->state & State_Selected) && (vi->state & QStyle::State_Enabled)) {
                    painter->setPen(vi->palette.color(cg, QPalette::Text));
                }
                else if ((vi->state & (QStyle::State_Selected)) && (vi->state & QStyle::State_Enabled)) {
                    painter->setPen(ViewItemSelect);
                }
                else if ((vi->state & (QStyle::State_MouseOver)) && (vi->state & QStyle::State_Enabled))
                {
                    painter->setPen(ViewItemHover);
                }
                else {
                    painter->setPen(vi->palette.color(cg, QPalette::Text));
                }

                if (vi->state & QStyle::State_Editing) {
                    painter->setPen(vi->palette.color(cg, QPalette::Text));
                    painter->drawRect(textRect.adjusted(0, 0, -1, -1));
                }
                viewItemDrawText(painter, vi, textRect);

            }

//            if(vi->state & QStyle::State_HasFocus){
//                QStyleOptionFocusRect o;
//                o.QStyleOption::operator=(*vi);
//                o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vi, widget);
//                qDebug() << "ooooooo" << o.rect << proxy()->subElementRect(SE_TreeViewDisclosureItem, vi, widget);
//                painter->save();
//                painter->setPen(QPen(QColor(Qt::blue), 1));
//                painter->setBrush(Qt::NoBrush);
//                painter->drawRect(o.rect);
//                painter->restore();
//            }
            painter->restore();
            return;
        }
        break;
    }

    case CE_ShapedFrame:
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            painter->save();
            int frameShape  = f->frameShape;
            int frameShadow = QFrame::Plain;
            if (f->state & QStyle::State_Sunken) {
                frameShadow = QFrame::Sunken;
            } else if (f->state & QStyle::State_Raised) {
                frameShadow = QFrame::Raised;
            }

            int lw = f->lineWidth;
            int mlw = f->midLineWidth;

            switch (frameShape) {
            case QFrame::HLine:
            case QFrame::VLine: {
                QPoint p1, p2;
                if (frameShape == QFrame::HLine) {
                    p1 = QPoint(option->rect.x(), option->rect.y() + option->rect.height() / 2);
                    p2 = QPoint(option->rect.x() + option->rect.width(), p1.y());
                } else {
                    p1 = QPoint(option->rect.x() + option->rect.width() / 2, option->rect.y());
                    p2 = QPoint(p1.x(), p1.y() + option->rect.height());
                }
                if (frameShadow == QFrame::Plain) {
                    QPen oldPen = painter->pen();
                    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                    painter->setPen(QPen(QBrush(f->palette.color(QPalette::Active, QPalette::NoRole)), lw));
                    painter->drawLine(p1, p2);
                    painter->setPen(oldPen);
                } else {
                    qDrawShadeLine(painter, p1, p2, f->palette, frameShadow == QFrame::Sunken, lw, mlw);
                }
                painter->restore();
                break;
            }
            case QFrame::WinPanel:
            case QFrame::Panel:
            case QFrame::StyledPanel:
            case QFrame::Box:
                Style::drawControl(element, option, painter, widget);
            }
        }
        painter->restore();
        break;
    default:
        return Style::drawControl(element, option, painter, widget);
    }
}

int LINGMOConfigStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_ScrollBarExtent:
        return sp->ScroolBar_Width;
    case PM_ScrollBarSliderMin:
        return sp->ScroolBar_Height;
    case PM_MaximumDragDistance:
        return -1;

    case PM_MenuPanelWidth:
        return sp->Menu_MarginPanelWidth;
    case PM_MenuHMargin:
        if (qobject_cast<const QComboBox*>(widget)) {
            return sp->Menu_Combobox_Popup_MarginWidth;
        }
        return sp->Menu_MarginWidth;
    case PM_MenuVMargin:
    {
        if (qobject_cast<const QComboBox*>(widget)) {
            return sp->Menu_Combobox_Popup_MarginHeight;
        }
        return sp->Menu_MarginHeight;
    }
    case PM_SubMenuOverlap:
        return 8;

    case PM_SliderThickness:
        return sp->Slider_Thickness;
    case PM_SliderControlThickness:
    case PM_SliderLength:
        return sp->Slider_Length;
    case PM_SliderTickmarkOffset:
        return 5;
    case PM_SliderSpaceAvailable:
    {
        if (const QStyleOptionSlider *sl = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            if (sl->orientation == Qt::Horizontal)
                return sl->rect.width() - proxy()->pixelMetric(PM_SliderLength, option, widget) - 2 * sp->Slider_Margin;
            else
                return sl->rect.height() - proxy()->pixelMetric(PM_SliderLength, option, widget)- 2 * sp->Slider_Margin;
        } else {
            return 0;
        }
        break;
    }

    case PM_SmallIconSize:
        return sp->SmallIcon_Size;

    case PM_MenuScrollerHeight:
        if(widget == nullptr)
            return 0;
        else
            return Style::pixelMetric(metric, option, widget);

    case PM_DefaultFrameWidth://qframe margin 边距
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            return 4;
        }
        return 2;

    case PM_MenuBarItemSpacing:return 16;
    case PM_MenuBarVMargin:return 4;
    case PM_ToolTipLabelFrameWidth:
        return sp->ToolTip_DefaultMargin;

    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
        return 8;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return 8;
    case PM_ToolBarFrameWidth:
    case PM_ToolBarItemMargin:
        return 0;
    case PM_ToolBarItemSpacing:
        return 4;

    case PM_MessageBoxIconSize:
        return 24;

    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
        return 20;

    case PM_TabBarTabHSpace:
        return 8 * 2;
    case PM_TabBarTabVSpace:
        return 4 * 2;

    case PM_TabBarTabOverlap:
        return 1;

    case PM_TabBarScrollButtonWidth:
        return sp->TabBar_ScrollButtonWidth;

    case PM_TabBar_ScrollButtonOverlap:
        return sp->TabBar_ScrollButtonOverlap;

    case PM_TabBarIconSize:
        return 16;

    case PM_TabBarTabShiftVertical:
        return 0;
    case PM_TabBarTabShiftHorizontal:
        return 0;

    case PM_TabBarBaseHeight:
        return 2;

    case PM_TabBarBaseOverlap:
        return 0;

    case PM_ExclusiveIndicatorWidth:
        return sp->ExclusiveIndicator_Width;
    case PM_ExclusiveIndicatorHeight:
        return sp->ExclusiveIndicator_Height;
    case PM_RadioButtonLabelSpacing:
        return sp->RadioButtonLabel_Spacing;
    case PM_IndicatorWidth:
        return sp->Indicator_Width;
    case PM_IndicatorHeight:
        return sp->Indicator_Height;

    case PM_ButtonIconSize:
        return sp->Button_IconSize;
    case PM_ButtonMargin:
        return sp->Button_MarginWidth;
    case PM_MenuButtonIndicator:
        return sp->Button_IndicatorSize;
    case PM_ButtonDefaultIndicator:
        return sp->Button_DefaultIndicatorSize;

    case PM_ComboBoxFrameWidth:
        return sp->ComboBox_FrameWidth;

    case PM_SpinBoxFrameWidth:
        return sp->SpinBox_FrameWidth;

    case PM_ProgressBarChunkWidth:
        return 9;

    case PM_HeaderMargin:
        return 2;
    case PM_HeaderMarkSize:
        return 16;
    case PM_ScrollView_ScrollBarOverlap:
        return 0;

    case PM_ListViewIconSize:
        return 16;
    case PM_IconViewIconSize:
        return 32;
    case PM_FocusFrameHMargin:
        return 4;

    case PM_TreeViewIndentation:
        return 20;
    case PM_HeaderDefaultSectionSizeVertical:{
        return sp->m_headerDefaultSectionSizeVertical;
    }
    default:
        break;
    }
    return Style::pixelMetric(metric, option, widget);
}

QRect LINGMOConfigStyle::subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option, QStyle::SubControl subControl, const QWidget *widget) const
{
    switch (control) {
    case CC_ScrollBar:
    {
        if (const QStyleOptionSlider *bar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const QRect rect = bar->rect;
            int lineLength = 0;
            int distance = 4;
            if (sp->ScrooBar_ShowLine && widget && ((widget->property("drawScrollBarGroove").isValid() &&
                           widget->property("drawScrollBarGroove").toBool()) || !widget->property("drawScrollBarGroove").isValid())) {
                lineLength = sp->ScroolBar_LineLength;
                distance = 0;
            }
            const bool horizontal = bar->orientation == Qt::Horizontal;
            int maxlen = (horizontal ? rect.width() : rect.height()) - distance - 2 * lineLength;
            int sliderlen = 0;
            if (bar->maximum != bar->minimum) {
                uint range = bar->maximum - bar->minimum;
                sliderlen = (qint64(bar->pageStep) * maxlen) / (range + bar->pageStep);

                int slidermin = proxy()->pixelMetric(PM_ScrollBarSliderMin, bar, widget);
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
            } else {
                sliderlen = maxlen;
            }

            auto animator = m_scrollbar_animation_helper->animator(widget);
            bool hasAnimation = widget &&(!widget->property("animation").isValid() ||
                                          (widget->property("animation").isValid() && widget->property("animation").toBool()));
            int sliderstart = lineLength + sliderPositionFromValue(bar->minimum, bar->maximum,
                                           bar->sliderPosition,maxlen - sliderlen, bar->upsideDown) + distance / 2;

            aScrollBarDebug << "sliderstart0000......" << sliderstart << lineLength << bar->sliderPosition;
            if (animator && hasAnimation) {
                aScrollBarDebug << "bar value..." << bar->sliderValue << bar->sliderPosition;
                if(animator->isRunning("move_position") ||
                        (animator->getExtraProperty("start_position").toInt() !=
                         animator->getExtraProperty("end_position").toInt() &&
                         bar->sliderValue != animator->getExtraProperty("end_position").toInt())){
                    ///|| (animator->value("move_position").toReal() > animator->animatorStartValue("move_position").toReal() &&
                    ///                                        animator->value("move_position").toReal() <= animator->animatorEndValue("move_position").toReal())){
                    if(const QScrollBar* scrollbar = qobject_cast<const QScrollBar *>(widget)){
                        QScrollBar *sbar = const_cast<QScrollBar *>(scrollbar);
                        if(sbar){
                            int startValue = animator->getExtraProperty("start_position").toInt();
                            int endValue = animator->getExtraProperty("end_position").toInt();
                            int v = startValue + (endValue - startValue) * 1.0 * animator->value("move_position").toReal();
                            aScrollBarDebug << "v...................." << v << startValue << endValue;
                            if (v >= qMin(startValue, endValue) && v <= qMax(startValue, endValue)) {
                                sbar->setValue(v);
                                auto startPos = sliderPositionFromValue(bar->minimum, bar->maximum, startValue,
                                                                        maxlen - sliderlen, bar->upsideDown) + distance / 2;
                                auto endPos = sliderPositionFromValue(bar->minimum, bar->maximum,
                                                                      endValue, maxlen - sliderlen, bar->upsideDown) + distance / 2;

                                aScrollBarDebug << "startpos....:" << startPos << "endpos...:" << endPos;
                                        sliderstart = lineLength + startPos + (endPos - startPos) * 1.0 * animator->value("move_position").toReal();
                            aScrollBarDebug << "sliderstart11111......" << sliderstart;

                            }
                        }
                    }
                }
            }

            switch (subControl) {
            case SC_ScrollBarSubLine:
            {
                if(!sp->ScrooBar_ShowLine)
                    return QRect();
                if (horizontal)
                    return QRect(0, 0, lineLength, rect.height());
                else
                    return QRect(0, 0, rect.width(), lineLength);
            }
            case SC_ScrollBarAddLine:
            {
                if(!sp->ScrooBar_ShowLine)
                    return QRect();
                if (horizontal)
                    return QRect(rect.right() - lineLength, 0, lineLength, rect.height());
                else
                    return QRect(0, rect.bottom() - lineLength, rect.width(), lineLength);
            }
            case SC_ScrollBarFirst:
            case SC_ScrollBarLast:
                return QRect();

            case SC_ScrollBarSubPage:
            {
                if (horizontal)
                    return QRect(lineLength, 0, sliderstart - lineLength, rect.height());
                else
                    return QRect(0, lineLength, rect.width(), sliderstart - lineLength);
            }

            case SC_ScrollBarAddPage:
            {
                if (horizontal)
                    return QRect(sliderstart + sliderlen, 0, rect.width() - sliderstart - sliderlen - lineLength, rect.height());
                else
                    return QRect(0, sliderstart + sliderlen, rect.width(), rect.height() - sliderstart - sliderlen - lineLength);
            }


            case SC_ScrollBarSlider:
            {
                if(animator && !animator->isRunning("move_position") && animator->currentAnimatorTime("move_position") != 0){
                    aScrollBarDebug << "stop animator time.........";
                    animator->setAnimatorCurrentTime("move_position", 0);
                    animator->setExtraProperty("end_position", 0);
                    animator->setExtraProperty("start_position", 0);
                }
                QRect r;
                if (horizontal)
                    r = QRect(sliderstart, 0, sliderlen, rect.height());
                else
                    r = QRect(0, sliderstart, rect.width(), sliderlen);
                QWidget *w = const_cast<QWidget *>(widget);
                aScrollBarDebug << "SC_ScrollBarSlider......" << w << r;
                if (w)
                    w->setProperty("ScrollBarSliderRect", r);
                return r;
            }

            case SC_ScrollBarGroove:
            {
                QRect r;
                if (horizontal)
                    r = QRect(rect.x() + lineLength, rect.y(), rect.width() - 2 * lineLength, rect.height());
                else
                    r = QRect(rect.x(), rect.y() + lineLength, rect.width(), rect.height() - 2 * lineLength);
                QWidget *w = const_cast<QWidget *>(widget);
                aScrollBarDebug << "SC_ScrollBarGroove......" << w << r;
                if (w)
                    w->setProperty("ScrollBarGrooveRect", r);
                return r;
            }

            default:
                break;
            }
        }
        break;
    }

    case CC_Slider:
    {
        if (const QStyleOptionSlider* slider = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            QRect rect = option->rect;
            const bool horizontal(slider->orientation == Qt::Horizontal);
            const int thickSpace = 2;
            if (horizontal) {
                if (slider->tickPosition & QSlider::TicksAbove)
                    rect.adjust(0, thickSpace, 0, 0);
                if (slider->tickPosition & QSlider::TicksBelow)
                    rect.adjust(0, 0, 0, -thickSpace);
            } else {
                if (slider->tickPosition & QSlider::TicksAbove)
                    rect.adjust(thickSpace, 0, 0, 0);
                if (slider->tickPosition & QSlider::TicksBelow)
                    rect.adjust(0, 0, -thickSpace, 0);
            }

            switch (subControl) {
            case SC_SliderHandle:
            {
                auto animator = m_slider_animation_helper->animator(widget);

                int sliderPos = sliderPositionFromValue(slider->minimum, slider->maximum, slider->sliderValue,
                                                        proxy()->pixelMetric(PM_SliderSpaceAvailable, option, widget), slider->upsideDown);
                aScrollBarDebug << "sliderPos000........" << sliderPos;
                bool hasAnimation = widget && ((widget->property("animation").isValid() && widget->property("animation").toBool()) ||
                                               (!widget->property("animation").isValid()));
                if (slider && animator && hasAnimation){
                    if(animator->isRunning("move_position") ||
                            (animator->getExtraProperty("start_position").toInt() !=
                             animator->getExtraProperty("end_position").toInt() &&
                             slider->sliderValue != animator->getExtraProperty("end_position").toInt())) {
                        auto sliderBar = qobject_cast<const QAbstractSlider *>(widget);
                        aScrollBarDebug << "value......." << subControl << slider->sliderValue << animator->getExtraProperty("start_position").toInt();
                        aScrollBarDebug << "isrunning............." << animator->isRunning("move_position");
                        aScrollBarDebug << "CC_Slider.....:" << animator->isRunning("move_position") << animator->value("move_position").toReal();
                        aScrollBarDebug << "value:.........." << animator->value("move_position").toReal();

                        QAbstractSlider *sbar = const_cast<QAbstractSlider *>(sliderBar);
                        if (sbar) {
                            int startValue = animator->getExtraProperty("start_position").toInt();
                            int endValue = animator->getExtraProperty("end_position").toInt();
                            aScrollBarDebug << "startvalue:" << startValue << "endvalue:" << endValue
                                     << animator->getExtraProperty("start_position").toInt()
                                     << animator->getExtraProperty("end_position").toInt();
                            int v = startValue + (endValue - startValue) * 1.0 * animator->value("move_position").toReal();
                            aScrollBarDebug << "vvvvvvvvvvv" << v;
                            if(v >= qMin(startValue, endValue) && v <= qMax(startValue, endValue)){
                                sbar->setValue(startValue + (endValue - startValue) * 1.0 * animator->value("move_position").toReal());

                                auto startPos = sliderPositionFromValue(slider->minimum, slider->maximum, startValue,
                                                  proxy()->pixelMetric(PM_SliderSpaceAvailable, option, widget), slider->upsideDown);
                                auto endPos = sliderPositionFromValue(slider->minimum, slider->maximum, endValue,
                                                  proxy()->pixelMetric(PM_SliderSpaceAvailable, option, widget), slider->upsideDown);
                                sliderPos = startPos + (endPos - startPos) * 1.0 * animator->value("move_position").toReal();
                            aScrollBarDebug << "sliderPos1111........" << sliderPos;
                            }
                                aScrollBarDebug << "sbar value:" << sbar->value();
                        }
                    }
                }


                if (animator && hasAnimation) {
                    if(!animator->isRunning("move_position") && animator->currentAnimatorTime("move_position") != 0){
                        animator->setAnimatorCurrentTime("move_position", 0);
                        animator->setExtraProperty("end_position", 0);
                        animator->setExtraProperty("start_position", 0);
                        aScrollBarDebug << "setAnimatorCurrentTime0000000000000000" << animator->getExtraProperty("start_position").toInt() << animator->getExtraProperty("end_position").toInt();
                    }
                }

                QRect handleRect = option->rect;
                int handleThickness = proxy()->pixelMetric(PM_SliderThickness, option, widget);
                int handleLength = proxy()->pixelMetric(PM_SliderLength, option, widget);
                if(horizontal)
                    handleRect.setSize(QSize(handleLength, handleThickness));
                else
                    handleRect.setSize(QSize(handleThickness, handleLength));
                handleRect.moveCenter(rect.center());

                if (horizontal) {
                    handleRect.moveLeft(sliderPos + sp->Slider_Margin);
                } else {
                    handleRect.moveTop((sliderPos + sp->Slider_Margin));
                }
                return visualRect(slider->direction, slider->rect, handleRect);
            }

            case SC_SliderGroove:
            {
                QRect grooveRect = rect;
                return grooveRect;
            }

            case SC_SliderTickmarks:
                break;

            default:
                break;
            }
        }
        break;
    }

    case CC_ToolButton:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, tb, widget);
            QRect rect = tb->rect;
            switch (subControl) {
            case SC_ToolButton:
            {
                if ((tb->features & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                    rect.adjust(0, 0, - (mbi + sp->ToolButton_MarginWidth), 0);
                break;
            }
            case SC_ToolButtonMenu:
            {
                if ((tb->features & (QStyleOptionToolButton::MenuButtonPopup | QStyleOptionToolButton::PopupDelay))
                        == QStyleOptionToolButton::MenuButtonPopup)
                    rect.adjust(rect.width() - (mbi + sp->ToolButton_MarginWidth), 0, 0, 0);
                break;
            }
            default:
                break;
            }
            rect = visualRect(tb->direction, tb->rect, rect);
            return rect;
        }
        break;
    }

    case CC_ComboBox:
    {
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            int comboBox_Margin = proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget);
            int comboBox_MarginWidth = 8;
            int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
            QRect rect = option->rect.adjusted(comboBox_MarginWidth, comboBox_Margin, -comboBox_MarginWidth, -comboBox_Margin);

            switch (subControl) {
            case SC_ComboBoxArrow:
            {
                QRect arrowRect(rect.right() - indicator, rect.top(), indicator, rect.height());
                return visualRect(option->direction, rect, arrowRect);
            }

            case SC_ComboBoxEditField:
            {
                QRect textRect = option->rect;
                if (cb->editable) {
                    textRect.setRect(rect.left() - 2, rect.top(), rect.width() - indicator - 4, rect.height());

                } else {
                    textRect.setRect(rect.left(), rect.top(), rect.width() - indicator - 8, rect.height());
                }

                return visualRect(option->direction, option->rect, textRect);
            }

            case SC_ComboBoxListBoxPopup:
            {
                return option->rect.adjusted(0, 0, 0, 4);
            }

            default:
                break;
            }
        }
        break;
    }

    case CC_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            int center = sb->rect.height() / 2;
            const int fw = sb->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, sb, widget) : 0;
            int buttonWidth = 32;
            bool isHorizonLayout = (sp->getSpinBoxControlLayout() == LINGMOConfigStyleParameters::SpinBoxControlLayout::Horizontal);
            if(isHorizonLayout)
            {
                center = sb->rect.height();
                buttonWidth = sp->SpinBox_DefaultHeight;
            }
            QRect rect = option->rect;
            switch (subControl) {
            case SC_SpinBoxUp:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();
                if(isHorizonLayout)
                    rect = QRect(rect.right() - buttonWidth, rect.top(), buttonWidth, center);
                else{
                    rect = QRect(rect.right() - buttonWidth + 1, rect.top(), buttonWidth, center);
                }

                break;
            }
            case SC_SpinBoxDown:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();
                if(sp->getSpinBoxControlLayout() == LINGMOConfigStyleParameters::SpinBoxControlLayout::Vertical)
                    rect = QRect(rect.right() - buttonWidth + 1, rect.top() + center, buttonWidth, center);
                else
                    rect = QRect(rect.left(), rect.top(), buttonWidth, center);
                break;
            }
            case SC_SpinBoxEditField:
            {
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons) {
                    rect = rect.adjusted(fw, 0, -fw, 0);
                } else {
                    if(sp->getSpinBoxControlLayout() == LINGMOConfigStyleParameters::SpinBoxControlLayout::Vertical)
                        rect = rect.adjusted(fw, 0, -(fw + buttonWidth), 0);
                    else
                        rect = rect.adjusted(buttonWidth, 0, -(fw + buttonWidth), 0);
                }
                break;
            }
            case SC_SpinBoxFrame:
                if(!isHorizonLayout)
                    rect = sb->rect;
                else
                    rect = sb->rect.adjusted(buttonWidth, 0, -(fw + buttonWidth), 0);

            default:
                break;
            }

            return visualRect(sb->direction, sb->rect, rect);
        }
    }

    default:
        break;
    }

    return Style::subControlRect(control, option, subControl, widget);
}

QRect LINGMOConfigStyle::subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case SE_TabBarScrollLeftButton:
    {
        const bool verticalTabs = option->rect.width() < option->rect.height();
        const int buttonWidth = qMax(proxy()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, widget), QApplication::globalStrut().width());
        const int lap = proxy()->pixelMetric(QStyle::PM_TabBar_ScrollButtonOverlap, 0, widget);
        if(sp->getTabBarIndicatorLayout() != LINGMOConfigStyleParameters::TabBarIndicatorLayout::TabBarIndicator_Horizontal){
            QRect rect = QStyle::visualRect(option->direction, option->rect,
                                            QRect(option->rect.right() - buttonWidth, option->rect.top() + 2,
                                                  buttonWidth, (option->rect.height() - 4) / 2));
            return rect;
        }

        QRect rect = verticalTabs ? QRect(-lap, option->rect.height() - (buttonWidth * 2), option->rect.width() + 2 * lap, buttonWidth)
                                  : QStyle::visualRect(option->direction, option->rect,
                                                       QRect(option->rect.left() + lap, option->rect.top() + lap,
                                                             buttonWidth, option->rect.height() - 2 * lap));
        return rect;
    }

    case SE_TabBarScrollRightButton:
    {
        const bool verticalTabs = option->rect.width() < option->rect.height();
        const int lap = proxy()->pixelMetric(QStyle::PM_TabBar_ScrollButtonOverlap, 0, widget);
        const int buttonWidth = qMax(proxy()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, 0, widget), QApplication::globalStrut().width());

        if(sp->getTabBarIndicatorLayout() != LINGMOConfigStyleParameters::TabBarIndicatorLayout::TabBarIndicator_Horizontal){
            QRect rect = QStyle::visualRect(option->direction, option->rect,
                                            QRect(option->rect.right() - buttonWidth, option->rect.top() + (option->rect.height()) / 2,
                                                  buttonWidth, (option->rect.height() - 4) / 2));
            return rect;
        }

        QRect rect = verticalTabs ? QRect(-lap, option->rect.height() - buttonWidth, option->rect.width() + 2 * lap, buttonWidth)
                                  : QStyle::visualRect(option->direction, option->rect,
                                                       QRect(option->rect.right() - buttonWidth - lap, option->rect.top() + lap,
                                                             buttonWidth, option->rect.height() - 2 * lap));
        return rect;
    }

    case SE_TabBarTabLeftButton:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, option, widget) / 2;
            QRect buttonRect(QPoint(0, 0), tab->leftButtonSize);
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth:
            {
                buttonRect.moveTop((tab->rect.height() - buttonRect.height()) / 2);
                buttonRect.moveLeft(tab->rect.left() + hpadding);
                buttonRect = visualRect(tab->direction, tab->rect, buttonRect);
                break;
            }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveBottom(tab->rect.bottom() - hpadding);
                break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveTop(tab->rect.top() + hpadding);
                break;
            }
            default:
                break;
            }
            return buttonRect;
        }
        break;
    }

    case SE_TabBarTabRightButton:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, option, widget) / 2;
            QRect buttonRect(QPoint(0, 0), tab->rightButtonSize);
            switch (tab->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularSouth:
            {
                buttonRect.moveTop((tab->rect.height() - buttonRect.height()) / 2);
                buttonRect.moveRight(tab->rect.right() - hpadding);
                buttonRect = visualRect(tab->direction, tab->rect, buttonRect);
                break;
            }
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveTop(tab->rect.top() + hpadding);
                break;
            }
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                buttonRect.moveLeft((tab->rect.width() - buttonRect.width()) / 2);
                buttonRect.moveBottom(tab->rect.bottom() - hpadding);
                break;
            }
            default:
                break;
            }
            return buttonRect;
        }
        break;
    }

    case SE_TabBarTabText:
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            QRect rect = tab->rect;
            QRect iconRect = tab->rect;
            configTabLayout(tab, widget, proxy(), &rect, &iconRect);
            return rect;
        }
        break;
    }

    case SE_TabWidgetTabPane:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            int overlap = proxy()->pixelMetric(PM_TabBarBaseOverlap, option, widget);
            if (twf->lineWidth == 0)
                overlap = 0;
            const QSize tabBarSize(twf->tabBarSize - QSize(overlap, overlap));
            QRect rect(twf->rect);
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                rect.adjust(0, tabBarSize.height(), 0, 0);
                break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                rect.adjust(0, 0, 0, -tabBarSize.height());
                break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                rect.adjust(0, 0, -tabBarSize.width(), 0);
                break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                rect.adjust(tabBarSize.width(), 0, 0, 0);
                break;

            default:
                break;
            }
            return rect;
        }
        break;
    }

    case SE_TabWidgetTabContents:
    {
        if (qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            const QRect rect = proxy()->subElementRect(SE_TabWidgetTabPane, option, widget);
            int TabWidget_Margin = 0;
            return rect.adjusted(TabWidget_Margin, TabWidget_Margin, -TabWidget_Margin, -TabWidget_Margin);
        }
        break;
    }

    case SE_TabWidgetTabBar:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QRect rect = QRect(QPoint(0, 0), twf->tabBarSize);
            const uint alingMask = Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
            {
                rect.setWidth(qMin(rect.width(), twf->rect.width() - twf->leftCornerWidgetSize.width() - twf->rightCornerWidgetSize.width()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveLeft(twf->leftCornerWidgetSize.width());
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveLeft((twf->rect.size() - twf->leftCornerWidgetSize - twf->rightCornerWidgetSize - twf->tabBarSize).width() / 2);
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveLeft(twf->rect.width() - twf->tabBarSize.width() - twf->rightCornerWidgetSize.width());
                    break;
                }
                default:
                    break;
                }
                rect = visualRect(twf->direction, twf->rect, rect);
                switch (twf->shape) {
                case QTabBar::RoundedSouth:
                case QTabBar::TriangularSouth:
                {
                    rect.moveTop(twf->rect.height() - twf->tabBarSize.height());
                }
                default:
                    break;
                }
                return rect;
            }

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
            {
                rect.setHeight(qMin(rect.height(), twf->rect.height()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), 0));
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), (twf->rect.height() - twf->tabBarSize.height()) / 2));
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveTopLeft(QPoint(twf->rect.width() - twf->tabBarSize.width(), twf->rect.height() - twf->tabBarSize.height()));
                    break;
                }
                default:
                    break;
                }
                return rect;
            }

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
            {
                rect.setHeight(qMin(rect.height(), twf->rect.height()));
                switch (proxy()->styleHint(SH_TabBar_Alignment, twf, widget) & alingMask) {
                case Qt::AlignLeft:
                {
                    rect.moveTop(0);
                    break;
                }
                case Qt::AlignHCenter:
                {
                    rect.moveTop((twf->rect.height() - twf->tabBarSize.height()) / 2);
                    break;
                }
                case Qt::AlignRight:
                {
                    rect.moveTop(twf->rect.height() - twf->tabBarSize.height());
                    break;
                }
                default:
                    break;
                }
                return rect;
            }
            default:
                break;
            }
        }
        break;
    }

    case SE_TabWidgetLeftCorner:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
            QRect paneRect = proxy()->subElementRect(SE_TabWidgetTabPane, twf, widget);
            QRect rect;
            switch (twf->shape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
                rect = QRect(QPoint(paneRect.x(), paneRect.y() - twf->leftCornerWidgetSize.height()), twf->leftCornerWidgetSize);
                break;
            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
                rect = QRect(QPoint(paneRect.x(), paneRect.height()), twf->leftCornerWidgetSize);
                break;
            default:
                break;
            }
            rect = visualRect(twf->direction, twf->rect, rect);
            return rect;
        }
        break;
    }

    case SE_TabWidgetRightCorner:
    {
        if (const QStyleOptionTabWidgetFrame *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(option)) {
           QRect paneRect = proxy()->subElementRect(SE_TabWidgetTabPane, twf, widget);
           QRect rect;
           switch (twf->shape) {
           case QTabBar::RoundedNorth:
           case QTabBar::TriangularNorth:
               rect = QRect(QPoint(paneRect.width() - twf->rightCornerWidgetSize.width(),
                                paneRect.y() - twf->rightCornerWidgetSize.height()), twf->rightCornerWidgetSize);
               break;
           case QTabBar::RoundedSouth:
           case QTabBar::TriangularSouth:
               rect = QRect(QPoint(paneRect.width() - twf->rightCornerWidgetSize.width(),
                                paneRect.height()), twf->rightCornerWidgetSize);
               break;
           default:
               break;
           }
           rect = visualRect(twf->direction, twf->rect, rect);
           return rect;
        }
        break;
    }

    case SE_RadioButtonIndicator:
    {
        QRect rect;
        int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
        rect.setRect(option->rect.x(), option->rect.y() + ((option->rect.height() - h) / 2),
                  proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget), h);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    }

    case SE_RadioButtonContents:
    {
        int radioWidth = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget);
        int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
        return visualRect(option->direction, option->rect, option->rect.adjusted(radioWidth + spacing, 0, 0, 0));
    }

    case SE_CheckBoxIndicator:
    {
        QRect rect;
        int h = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
        rect.setRect(option->rect.x() + 1, option->rect.y() + ((option->rect.height() - h) / 2),
                     proxy()->pixelMetric(PM_IndicatorWidth, option, widget), h);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    }

    case SE_CheckBoxContents:
    {
        int radioWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
        int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
        return visualRect(option->direction, option->rect, option->rect.adjusted(radioWidth + spacing, 0, 0, 0));
    }

    case SE_PushButtonContents:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isEmpty();
            QRect rect = option->rect;
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                rect.adjust(Button_MarginWidth, 0, -Button_MarginWidth, 0);
            } else if (!text && icon && !(button->features & QStyleOptionButton::HasMenu)) {

            } else {
                rect.adjust(sp->ToolButton_MarginWidth, sp->Button_MarginHeight, -sp->ToolButton_MarginWidth, -sp->Button_MarginHeight);
            }
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                rect.adjust(dbw, dbw, -dbw, -dbw);
            }
            return rect;
        }
        break;
    }

    case SE_LineEditContents:
    {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            bool clear = false;
            if (widget) {
                if (widget->parent() && qobject_cast<QComboBox *>(widget->parent()))
                {
                    return option->rect;
                }
                if (widget->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction")))
                    clear = true;
            }

            QRect rect = f->rect;
            if (clear) {
                rect.adjust(f->lineWidth + 4, f->lineWidth, 0, -f->lineWidth);
                rect = visualRect(option->direction, option->rect, rect);
            } else {
                rect.adjust(f->lineWidth + 4, f->lineWidth, -(f->lineWidth + 4), -f->lineWidth);
            }

            return rect;
        }
        break;
    }

    case SE_ProgressBarGroove:
    case SE_ProgressBarLabel:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            return pb->rect;
        }
        break;
    }

    case SE_ProgressBarContents:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            const auto progress = qMax(pb->progress, pb->minimum); // workaround for bug in QProgressBar
            const bool vertical = pb->orientation == Qt::Vertical;
            const bool inverted = pb->invertedAppearance;
            const bool indeterminate = (pb->minimum == 0 && pb->maximum == 0);

            int maxWidth = vertical ? pb->rect.height() : pb->rect.width();
            const auto totalSteps = qMax(Q_INT64_C(1), qint64(pb->maximum) - pb->minimum);
            const auto progressSteps = qint64(progress) - pb->minimum;
            const auto progressBarWidth = progressSteps * maxWidth / totalSteps;
            int len = indeterminate ? maxWidth : progressBarWidth;

            bool reverse = (!vertical && (pb->direction == Qt::RightToLeft)) || vertical;
            if (inverted)
                reverse = !reverse;

            int diff = 0;
            if (indeterminate) {
                len = 56;
                double currentValue = 0;
                if (QVariantAnimation *animation = m_animation_helper->animation(option->styleObject)) {
                    currentValue = animation->currentValue().toDouble();
                    int barLength = pb->rect.width();
                    if(vertical)
                        barLength = pb->rect.height();
                    double speed = 0.5;
                    if (animation->currentTime() == 0) {
                        animation->setDirection(QAbstractAnimation::Forward);
                        animation->setDuration(500 + barLength * speed);
                        animation->start();
                    } else if (animation->currentTime() == animation->totalDuration()) {
                        animation->setDirection(QAbstractAnimation::Backward);
                        animation->setDuration(500 + barLength * speed);
                        animation->start();
                    }
                } else {
                    m_animation_helper->startAnimation(new ConfigProgressBarAnimation(option->styleObject));
                }
                diff = currentValue * (maxWidth - len);
            } else {
                m_animation_helper->stopAnimation(option->styleObject);
            }

            QRect rect = proxy()->subElementRect(SE_ProgressBarGroove, pb, widget);
            QRect progressRect;
            if (vertical) {
                if (reverse) {
                    progressRect.setRect(rect.left(), rect.bottom() + 1 - len - diff, rect.width(), len);
                } else {
                    progressRect.setRect(rect.x(), rect.top() + diff, rect.width(), len);
                }
            } else {
                if (reverse) {
                    progressRect.setRect(rect.right() + 1 - len - diff, rect.top(), len, rect.height());
                } else {
                    progressRect.setRect(rect.x() + diff, rect.y(), len, rect.height());
                }
            }

            return progressRect;
        }
        break;
    }

    case SE_HeaderLabel:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            QRect rect = header->rect;
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
            int Header_MarginWidth = 8;
            rect.adjust(Header_MarginWidth - margin, margin, -Header_MarginWidth + margin, -margin);
            if (header->sortIndicator != QStyleOptionHeader::None && header->state & State_Horizontal) {
                int arrowSize = proxy()->pixelMetric(QStyle::PM_HeaderMarkSize, option, widget);
                rect.adjust(0, 0, -arrowSize - Header_MarginWidth, 0);
            }
            return visualRect(option->direction, header->rect, rect);
        }
        break;
    }

    case SE_HeaderArrow:
    {
        if (qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
            int Header_MarginWidth = 8;
            int arrowSize = proxy()->pixelMetric(QStyle::PM_HeaderMarkSize, option, widget);
            QRect rect = option->rect.adjusted(Header_MarginWidth, margin, -Header_MarginWidth, -margin);
            QRect arrowRect(rect.right() + 1 - arrowSize, rect.y() + (rect.height() - arrowSize) / 2, arrowSize, arrowSize);
            return visualRect(option->direction, rect, arrowRect);
        }
        break;
    }

    case SE_ItemViewItemCheckIndicator:
        if (!qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            return proxy()->subElementRect(SE_CheckBoxIndicator, option, widget);
        }
        Q_FALLTHROUGH();

    case SE_ItemViewItemDecoration:
    case SE_ItemViewItemText:
    case SE_ItemViewItemFocusRect:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            QRect checkRect, decorationRect, displayRect;
            viewItemLayout(vi, &checkRect, &decorationRect, &displayRect, false);
            if (element == SE_ViewItemCheckIndicator)
                return checkRect;
            else if (element == SE_ItemViewItemDecoration)
                return decorationRect;
            else if (element == SE_ItemViewItemText || element == SE_ItemViewItemFocusRect){
//                displayRect.setHeight(sp->ViewItem_DefaultHeight * m_scaleRatio4_3);
                return displayRect;
            }
        }
        break;
    }

    case SE_TreeViewDisclosureItem:
        return option->rect;

    default:
        break;
    }

    return Style::subElementRect(element,option,widget);
}

QSize LINGMOConfigStyle::sizeFromContents(ContentsType ct, const QStyleOption *option,
                                     const QSize &size, const QWidget *widget) const
{
    QSize  newSize = size;
    switch (ct) {
    case CT_MenuItem: {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            bool isComboBox = qobject_cast<const QComboBox*>(widget);
            int w = newSize.width();
            int MenuItem_Spacing = 8;
            if (menuItem->text.contains(QLatin1Char('\t'))) {
                w += 20;
            }

            switch (menuItem->menuItemType) {
            case QStyleOptionMenuItem::SubMenu:
            case QStyleOptionMenuItem::Normal:
            case QStyleOptionMenuItem::DefaultItem:
            {
                //Combobox popup menu item
                if (isComboBox) {
                    newSize.setHeight(menuItem->fontMetrics.height());
                }

            bool hasIcon = false;
            if(isComboBox)
            {
                if(!menuItem->icon.isNull())
                    hasIcon = true;
//                auto *comboBox = qobject_cast<const QComboBox*>(widget);
//                int column = comboBox->count();
//                for(int i = 0; i < column; i++)
//                {
//                    if(!comboBox->itemIcon(i).isNull()){
//                        hasIcon = true;
//                        break;
//                    }
//                }
            }
            else if(menuItem->maxIconWidth != 0)
            {
                hasIcon = true;
            }

            if (menuItem->menuHasCheckableItems || hasIcon) {
                int iconWidth = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
                w += ((hasIcon && menuItem->menuHasCheckableItems) ? 2 : 1) * (iconWidth + MenuItem_Spacing);
                newSize.setHeight(qMax(iconWidth, newSize.height()));
            } else {
                w += 8;
            }

            if(!isComboBox)
                w += proxy()->pixelMetric(PM_IndicatorWidth, option, widget) + MenuItem_Spacing;
            newSize.setHeight(qMax(newSize.height(), proxy()->pixelMetric(PM_IndicatorHeight, option, widget)));

            int MenuItem_HMargin = 12 + 4;
            //                int MenuItem_VMargin = 3;
            w +=  MenuItem_HMargin;
            newSize.setWidth(qMax(w, 152));


            newSize.setWidth(qMax(w + sp->MenuItem_MarginWidth, sp->MenuItem_DefaultWidght));
            newSize.setHeight(qMax(newSize.height() + sp->MenuItem_MarginHeight * 2,
                                   sp->MenuItem_DefaultHeight + (isComboBox ? (2 * sp->ComboBox_VMargin) : 0)));


            if (widget) {
                if(newSize.width() > widget->maximumWidth()) {
                    //Fix me:size no more than max size.At least set size 1.BUG 133590 ,support application to set fix width
                    newSize.setWidth((widget->maximumWidth() - 20 > 1)?widget->maximumWidth() - 20 : 1);
                }
            }

            return newSize;
            }

            case QStyleOptionMenuItem::Separator:
            {
                newSize.setHeight(sp->MenuItemSeparator_MarginHeight * 2 + 1);
                return newSize;
            }

            default:
                break;
            }
            return newSize;
        }
        break;
    }

    case CT_LineEdit:
    {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(option)) {
            newSize += QSize(f->lineWidth * 2 + 8, f->lineWidth * 2);
            newSize.setWidth(qMax(newSize.width(), sp->LineEdit_DefaultWidth));
            newSize.setHeight(qMax(newSize.height(), sp->LineEdit_DefaultHeight));
            return newSize;
        }
        break;
    }

    case CT_TabBarTab:
    {
        int padding = 0;
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            if (!tab->icon.isNull())
                padding += 4;

            if (!tab->leftButtonSize.isEmpty() || !tab->rightButtonSize.isEmpty())
                padding += 4;

            if (tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::RoundedEast
                    || tab->shape == QTabBar::TriangularWest || tab->shape == QTabBar::TriangularEast) {
                newSize.setWidth(qMax(newSize.width(), sp->TabBar_DefaultHeight));

                newSize.setHeight(qMax(newSize.height() + padding, sp->TabBar_DefaultMinWidth));
                newSize.setHeight(qMin(newSize.height(), sp->TabBar_DefaultMaxWidth));
            } else {
                newSize.setHeight(qMax(newSize.height(), sp->TabBar_DefaultHeight));

                newSize.setWidth(qMax(newSize.width() + padding, sp->TabBar_DefaultMinWidth));
                newSize.setWidth(qMin(newSize.width(), sp->TabBar_DefaultMaxWidth));
            }

            if (widget && qobject_cast<const QTabBar*>(widget)) {
                //set height between min and max
                newSize.setHeight(qMax(newSize.height(), widget->minimumHeight()));
                newSize.setHeight(qMin(newSize.height(), widget->maximumHeight()));
            }
            return newSize;
        }
        break;
    }

    case CT_RadioButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = proxy()->pixelMetric(PM_ExclusiveIndicatorWidth, option, widget);
            int h = proxy()->pixelMetric(PM_ExclusiveIndicatorHeight, option, widget);
            int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
            if (!button->icon.isNull())
                spacing += 4;
            newSize.setWidth(newSize.width() + w + spacing);
            newSize.setHeight(qMax(qMax(newSize.height(), h), sp->RadioButton_DefaultHeight));
            return newSize;
        }
        break;
    }

    case CT_CheckBox:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            int w = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int h = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            int spacing = proxy()->pixelMetric(PM_RadioButtonLabelSpacing, option, widget);
            if (!button->icon.isNull())
                spacing += 4;
            newSize.setWidth(newSize.width() + w + spacing);
            newSize.setHeight(qMax(qMax(newSize.height(), h), sp->CheckBox_DefaultHeight));
            return newSize;
        }
        break;
    }

    case CT_ToolButton:
    {
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            const bool icon = !tb->icon.isNull();
            const bool text = !tb->text.isEmpty();
            int w = size.width();
            int h = size.height();
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (tb->toolButtonStyle == Qt::ToolButtonTextOnly && !(tb->features & QStyleOptionToolButton::MenuButtonPopup)) {
                w += Button_MarginWidth * 2;
            } else {
                w += sp->ToolButton_MarginWidth * 2;
            }
            h += sp->Button_MarginHeight * 2;

            if (tb->toolButtonStyle != Qt::ToolButtonIconOnly) {
                QFontMetrics fm = tb->fontMetrics;
                w -= fm.horizontalAdvance(QLatin1Char(' ')) * 2;
                if (tb->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                    if (text && icon)
                        w += 4;
                    else
                        w -= 4;
                } else if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                    if (text && icon)
                        h += 4;
                    else
                        h -= 4;
                }
            }
            if (tb->features & QStyleOptionToolButton::MenuButtonPopup) {
                w += 8;
                newSize.setWidth(qMax(w, sp->ToolButton_DefaultWidth));
            } else {
                newSize.setWidth(qMax(w, sp->IconButton_DefaultWidth));
            }
            newSize.setHeight(h > sp->Button_DefaultHeight ? h : sp->Button_DefaultHeight);
            return newSize;
        }
        break;
    }

    case CT_PushButton:
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            const bool icon = !button->icon.isNull();
            const bool text = !button->text.isEmpty();
            int w = size.width();
            int h = size.height();
            int Button_MarginWidth = proxy()->pixelMetric(PM_ButtonMargin, option, widget);
            if (text && !icon && !(button->features & QStyleOptionButton::HasMenu)) {
                w += Button_MarginWidth * 2;
            } else {
                w += sp->ToolButton_MarginWidth * 2;
            }
            h += sp->Button_MarginHeight * 2;

            int spacing = 0;
            if (text && icon)
                spacing += 4;
            if (!text && icon)
                spacing -= 4;
            if (button->features & QStyleOptionButton::HasMenu) {
                if (icon || text)
                    spacing += 8;
            }
            w += spacing;
            if (button->features & (QStyleOptionButton::AutoDefaultButton | QStyleOptionButton::DefaultButton)) {
                int dbw = proxy()->pixelMetric(PM_ButtonDefaultIndicator, option, widget) * 2;
                w += dbw;
                h += dbw;
            }

            newSize.setWidth(w > sp->Button_DefaultWidth ? w : sp->Button_DefaultWidth);
            newSize.setHeight(h > sp->Button_DefaultHeight ? h : sp->Button_DefaultHeight);
            return newSize;
        }
        break;
    }

    case CT_ComboBox:
    {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            QSize newSize = size;
            int indicator = proxy()->pixelMetric(PM_MenuButtonIndicator, option, widget);
            int comboBox_Margin = proxy()->pixelMetric(PM_ComboBoxFrameWidth, option, widget);
            int comboBox_MarginWidth = 8 + 8 + 8 + 8;
            bool hasIcon = false;
            if(qobject_cast<const QComboBox*>(widget))
            {
                auto *box = qobject_cast<const QComboBox*>(widget);
                for(int i = 0; i < box->count(); i++)
                {
                    if(!box->itemIcon(i).isNull())
                    {
                        hasIcon = true;
                        break;
                    }
                }
            }
            if(hasIcon)
            {
                int iconWidth = proxy()->pixelMetric(PM_SmallIconSize, option, widget);
                comboBox_MarginWidth += 8 + iconWidth;
            }

            newSize.setWidth(qMax(newSize.width() + indicator + comboBox_MarginWidth, sp->ComboBox_DefaultWidth));
            newSize.setHeight(qMax(newSize.height() + comboBox_Margin * 2, sp->ComboBox_DefaultHeight));

            return newSize;
        }
        break;
    }

    case CT_SpinBox:
    {
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            const int buttonWidth = (sb->subControls & (QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown)) != 0 ? 32 : 0;
            const int fw = sb->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, sb, widget) : 0;
            newSize += QSize(buttonWidth + 2 * fw, 0);
            newSize.setWidth(qMax(newSize.width(), sp->SpinBox_DefaultWidth));
            newSize.setHeight(qMax(newSize.height(), sp->SpinBox_DefaultHeight));
            return newSize;
        }
        break;
    }

    case CT_ProgressBar:
    {
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            if (pb->orientation == Qt::Vertical) {
                newSize.setWidth(qMax(newSize.width() - 8, sp->ProgressBar_DefaultThick));
                newSize.setHeight(qMax(newSize.height(), sp->ProgressBar_DefaultLength));
            } else {
                newSize.setWidth(qMax(newSize.width(), sp->ProgressBar_DefaultLength));
                newSize.setHeight(qMax(newSize.height() - 8, sp->ProgressBar_DefaultThick));
            }
            return newSize;
        }
        break;
    }

    case CT_Slider:
    {
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            const bool horizontal(slider->orientation == Qt::Horizontal);

            if (horizontal) {
                newSize.setHeight(qMax(newSize.height() + 2 * sp->Slider_Margin, sp->Slider_DefaultWidth));
                newSize.setWidth(qMax(newSize.width(), sp->Slider_DefaultLength));
            } else {
                newSize.setWidth(qMax(newSize.width() + 2 * sp->Slider_Margin, sp->Slider_DefaultWidth));
                newSize.setHeight(qMax(newSize.height(), sp->Slider_DefaultLength));
            }
            return newSize;
        }
        break;
    }

    case CT_HeaderSection:
    {
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            const bool horizontal(header->orientation == Qt::Horizontal);
            const bool text(!header->text.isEmpty());
            const bool icon(!header->icon.isNull());
            int w = header->fontMetrics.size(Qt::TextShowMnemonic, header->text).width();
            int h = header->fontMetrics.size(Qt::TextShowMnemonic, header->text).height();
            int Header_MarginWidth = 8;
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);

            if (icon) {
                int iconSize = proxy()->pixelMetric(QStyle::PM_SmallIconSize, option, widget);
                w += iconSize;
                h = qMax(iconSize, h);
                if (text)
                    w += 8;
            }
            if (/*horizontal && */header->sortIndicator != QStyleOptionHeader::None) {
                int arrowSize = proxy()->pixelMetric(QStyle::PM_HeaderMarkSize, option, widget);
                w += arrowSize;
                h = qMax(arrowSize, h);
                if (text || icon)
                    w += 8;
            }
            h += margin * 2;
            w += Header_MarginWidth * 2;
            newSize.setWidth(w);
            newSize.setHeight(qMax(h, sp->Table_HeaderHeight));
            return newSize;
        }
        break;
    }

    case CT_ItemViewItem:
    {
        if (const QStyleOptionViewItem *vi = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
            QRect decorationRect, displayRect, checkRect;
            viewItemLayout(vi, &checkRect, &decorationRect, &displayRect, true);
            newSize = (decorationRect | displayRect | checkRect).size();

            int Margin_Width = 2;
            int Margin_Height = 0;
            newSize.setWidth(newSize.width() + Margin_Width * 2);
            newSize.setHeight(newSize.height() + Margin_Height * 2);
            newSize.setHeight(qMax(newSize.height(), sp->ViewItem_DefaultHeight));

            return newSize;
        }
        break;
    }

    default:
        break;
    }

    return QFusionStyle::sizeFromContents(ct, option, size, widget);
}

QPixmap LINGMOConfigStyle::drawColorPixmap(QPainter *painter, QColor color, QPixmap &pixmap) const
{
    QPainter p(&pixmap);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(pixmap.rect(), color);
    p.end();

    return pixmap;
}

void LINGMOConfigStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    qreal scale = pixmap.devicePixelRatio();
    QRect aligned = alignedRect(QApplication::layoutDirection(), QFlag(alignment), pixmap.size() / scale, rect);
    QRect inter = aligned.intersected(rect);

    QPixmap target = pixmap;

    auto device = painter->device();
    auto widget = dynamic_cast<QWidget *>(device);
    if (widget) {
        if (HighLightEffect::isWidgetIconUseHighlightEffect(widget)) {
            QStyleOption opt;
            opt.initFrom(widget);
            target = HighLightEffect::generatePixmap(pixmap, &opt, widget);
        }
    }

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter->drawPixmap(inter.x(), inter.y(), target, inter.x() - aligned.x(), inter.y() - aligned.y(), inter.width() * scale, inter.height() *scale);
}


void LINGMOConfigStyle::realSetWindowSurfaceFormatAlpha(const QWidget *widget) const
{
    if (!widget)
        return;

    if (widget->testAttribute(Qt::WA_WState_Created))
        return;

    if (qobject_cast<const QMenu *>(widget)) {
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
    }

    if (shouldBeTransparent(widget))
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
}

void LINGMOConfigStyle::realSetMenuTypeToMenu(const QWidget *widget) const
{
    if (auto menu = qobject_cast<const QMenu *>(widget)) {
        if (menu->testAttribute(Qt::WA_X11NetWmWindowTypeMenu)
                || !menu->windowHandle())
            return;

        int wmWindowType = 0;
        if (menu->testAttribute(Qt::WA_X11NetWmWindowTypeDropDownMenu))
            wmWindowType |= QXcbWindowFunctions::DropDownMenu;
        if (menu->testAttribute(Qt::WA_X11NetWmWindowTypePopupMenu))
            wmWindowType |= QXcbWindowFunctions::PopupMenu;
        if (wmWindowType == 0) return;
        QXcbWindowFunctions::setWmWindowType(menu->windowHandle(),
                                             static_cast<QXcbWindowFunctions::WmWindowType>(wmWindowType));
    }
}



QRect  LINGMOConfigStyle::centerRect(const QRect &rect, int width, int height) const
{ return QRect(rect.left() + (rect.width() - width)/2, rect.top() + (rect.height() - height)/2, width, height); }

void LINGMOConfigStyle::getDefaultControlParameters(QApplication *app)
{
    //pushbutton
    QSharedPointer<ConfigPushButtonParameters> pb(new ConfigPushButtonParameters);
    sp->getDefaultConfigPushButtonBevelParameters(pb.data(), isUseDarkPalette());
    app->setProperty("PushButton_DefaultParameters_Radius"           , pb.data()->radius);
    app->setProperty("PushButton_DefaultParameters_DefaultBrush"     , pb.data()->pushButtonDefaultBrush     );
    app->setProperty("PushButton_DefaultParameters_ClickBrush"       , pb.data()->pushButtonClickBrush       );
    app->setProperty("PushButton_DefaultParameters_HoverBrush"       , pb.data()->pushButtonHoverBrush       );
    app->setProperty("PushButton_DefaultParameters_DisableBrush"     , pb.data()->pushButtonDisableBrush     );
    app->setProperty("PushButton_DefaultParameters_CheckBrush"       , pb.data()->pushButtonCheckBrush       );
    app->setProperty("PushButton_DefaultParameters_CheckHoverBrush"  , pb.data()->pushButtonCheckHoverBrush  );
    app->setProperty("PushButton_DefaultParameters_CheckClickBrush"  , pb.data()->pushButtonCheckClickBrush  );
    app->setProperty("PushButton_DefaultParameters_CheckDisableBrush", pb.data()->pushButtonCheckDisableBrush);
    app->setProperty("PushButton_DefaultParameters_DefaultPen"       , pb.data()->pushButtonDefaultPen       );
    app->setProperty("PushButton_DefaultParameters_ClickPen"         , pb.data()->pushButtonClickPen         );
    app->setProperty("PushButton_DefaultParameters_HoverPen"         , pb.data()->pushButtonHoverPen         );
    app->setProperty("PushButton_DefaultParameters_DisablePen"       , pb.data()->pushButtonDisablePen       );
    app->setProperty("PushButton_DefaultParameters_CheckPen"         , pb.data()->pushButtonCheckPen         );
    app->setProperty("PushButton_DefaultParameters_CheckHoverPen"    , pb.data()->pushButtonCheckHoverPen    );
    app->setProperty("PushButton_DefaultParameters_CheckClickPen"    , pb.data()->pushButtonCheckClickPen    );
    app->setProperty("PushButton_DefaultParameters_CheckDisablePen"  , pb.data()->pushButtonCheckDisablePen  );
    app->setProperty("PushButton_DefaultParameters_FocusPen"         , pb.data()->pushButtonFocusPen         );

    sp->getDefaultConfigPushButtonLabelParameters(pb.data(), isUseDarkPalette());

    app->setProperty("PushButton_DefaultParameters_TextDefaultColor"  , pb.data()->textDefaultColor       );
    app->setProperty("PushButton_DefaultParameters_TextHoverColor"    , pb.data()->textHoverColor         );

    //ToolButton
    QSharedPointer<ConfigToolButtonParameters> tb(new ConfigToolButtonParameters);
    sp->getDefaultConfigToolButtonPanelParameters(tb.data(), isUseDarkPalette());
    app->setProperty("ToolButton_DefaultParameters_Radius"           , tb.data()->radius);
    app->setProperty("ToolButton_DefaultParameters_DefaultBrush"     , tb.data()->toolButtonDefaultBrush     );
    app->setProperty("ToolButton_DefaultParameters_ClickBrush"       , tb.data()->toolButtonClickBrush       );
    app->setProperty("ToolButton_DefaultParameters_HoverBrush"       , tb.data()->toolButtonHoverBrush       );
    app->setProperty("ToolButton_DefaultParameters_DisableBrush"     , tb.data()->toolButtonDisableBrush     );
    app->setProperty("ToolButton_DefaultParameters_CheckBrush"       , tb.data()->toolButtonCheckBrush       );
    app->setProperty("ToolButton_DefaultParameters_CheckHoverBrush"  , tb.data()->toolButtonCheckHoverBrush  );
    app->setProperty("ToolButton_DefaultParameters_CheckClickBrush"  , tb.data()->toolButtonCheckClickBrush  );
    app->setProperty("ToolButton_DefaultParameters_CheckDisableBrush", tb.data()->toolButtonCheckDisableBrush);
    app->setProperty("ToolButton_DefaultParameters_DefaultPen"       , tb.data()->toolButtonDefaultPen       );
    app->setProperty("ToolButton_DefaultParameters_ClickPen"         , tb.data()->toolButtonClickPen         );
    app->setProperty("ToolButton_DefaultParameters_HoverPen"         , tb.data()->toolButtonHoverPen         );
    app->setProperty("ToolButton_DefaultParameters_DisablePen"       , tb.data()->toolButtonDisablePen       );
    app->setProperty("ToolButton_DefaultParameters_CheckPen"         , tb.data()->toolButtonCheckPen         );
    app->setProperty("ToolButton_DefaultParameters_CheckHoverPen"    , tb.data()->toolButtonCheckHoverPen    );
    app->setProperty("ToolButton_DefaultParameters_CheckClickPen"    , tb.data()->toolButtonCheckClickPen    );
    app->setProperty("ToolButton_DefaultParameters_CheckDisablePen"  , tb.data()->toolButtonCheckDisablePen  );
    app->setProperty("ToolButton_DefaultParameters_FocusPen"         , tb.data()->toolButtonFocusPen         );

    sp->getDefaultConfigToolButtonLabelParameters(tb.data(), isUseDarkPalette());
    app->setProperty("ToolButton_DefaultParameters_TextDefaultColor"  , tb.data()->textDefaultColor       );
    app->setProperty("ToolButton_DefaultParameters_TextHoverColor"    , tb.data()->textHoverColor         );

    //lineedit
    QSharedPointer<ConfigLineEditParameters> lineedit(new ConfigLineEditParameters);
    sp->getDefaultConfigLineEditParameters(lineedit.data(), isUseDarkPalette());
    app->setProperty("Lineedit_DefaultParameters_Radius",       lineedit.data()->radius              );
    app->setProperty("Lineedit_DefaultParameters_DefaultBrush", lineedit.data()->lineEditDefaultBrush);
    app->setProperty("Lineedit_DefaultParameters_HoverBrush",   lineedit.data()->lineEditHoverBrush  );
    app->setProperty("Lineedit_DefaultParameters_FocusBrush",   lineedit.data()->lineEditFocusBrush  );
    app->setProperty("Lineedit_DefaultParameters_DisableBrush", lineedit.data()->lineEditDisableBrush);
    app->setProperty("Lineedit_DefaultParameters_DefaultPen",   lineedit.data()->lineEditDefaultPen  );
    app->setProperty("Lineedit_DefaultParameters_HoverPen",     lineedit.data()->lineEditHoverPen    );
    app->setProperty("Lineedit_DefaultParameters_FocusPen",     lineedit.data()->lineEditFocusPen    );
    app->setProperty("Lineedit_DefaultParameters_DisablePen",   lineedit.data()->lineEditDisablePen  );

    //spinbox
    QSharedPointer<ConfigSpinBoxParameters> spinbox(new ConfigSpinBoxParameters);
    sp->getDefaultConfigSpinBoxParameters(spinbox.data(), isUseDarkPalette());
    app->setProperty("SpinBox_DefaultParameters_Radius"               ,spinbox.data()->radius                    );
    app->setProperty("SpinBox_DefaultParameters_DefaultBrush"         ,spinbox.data()->spinBoxDefaultBrush       );
    app->setProperty("SpinBox_DefaultParameters_HoverBrush"           ,spinbox.data()->spinBoxHoverBrush         );
    app->setProperty("SpinBox_DefaultParameters_FocusBrush"           ,spinbox.data()->spinBoxFocusBrush         );
    app->setProperty("SpinBox_DefaultParameters_DisableBrush"         ,spinbox.data()->spinBoxDisableBrush       );
    app->setProperty("SpinBox_DefaultParameters_DefaultPen"           ,spinbox.data()->spinBoxDefaultPen         );
    app->setProperty("SpinBox_DefaultParameters_HoverPen"             ,spinbox.data()->spinBoxHoverPen           );
    app->setProperty("SpinBox_DefaultParameters_FocusPen"             ,spinbox.data()->spinBoxFocusPen           );
    app->setProperty("SpinBox_DefaultParameters_DisablePen"           ,spinbox.data()->spinBoxDisablePen         );
    app->setProperty("SpinBox_DefaultParameters_UpDefaultBrush"       ,spinbox.data()->spinBoxUpDefaultBrush     );
    app->setProperty("SpinBox_DefaultParameters_UpHoverBrush"         ,spinbox.data()->spinBoxUpHoverBrush       );
    app->setProperty("SpinBox_DefaultParameters_UpFocusHoverBrush"    ,spinbox.data()->spinBoxUpFocusHoverBrush  );
    app->setProperty("SpinBox_DefaultParameters_UpClickBrush"         ,spinbox.data()->spinBoxUpClickBrush       );
    app->setProperty("SpinBox_DefaultParameters_DownHoverBrush"       ,spinbox.data()->spinBoxDownHoverBrush     );
    app->setProperty("SpinBox_DefaultParameters_DownFocusHoverBrush"  ,spinbox.data()->spinBoxDownFocusHoverBrush);
    app->setProperty("SpinBox_DefaultParameters_DownClickBrush"       ,spinbox.data()->spinBoxDownClickBrush     );
    app->setProperty("SpinBox_DefaultParameters_DownDefaultBrush"     ,spinbox.data()->spinBoxDownDefaultBrush   );
    app->setProperty("SpinBox_DefaultParameters_UpDefaultPen"         ,spinbox.data()->spinBoxUpDefaultPen       );
    app->setProperty("SpinBox_DefaultParameters_UpHoverPen"           ,spinbox.data()->spinBoxUpHoverPen         );
    app->setProperty("SpinBox_DefaultParameters_UpFocusHoverPen"      ,spinbox.data()->spinBoxUpFocusHoverPen    );
    app->setProperty("SpinBox_DefaultParameters_UpClickPen"           ,spinbox.data()->spinBoxUpClickPen         );
    app->setProperty("SpinBox_DefaultParameters_UpDisablePen"         ,spinbox.data()->spinBoxUpDisablePen       );
    app->setProperty("SpinBox_DefaultParameters_DownDefaultPen"       ,spinbox.data()->spinBoxDownDefaultPen     );
    app->setProperty("SpinBox_DefaultParameters_DownHoverPen"         ,spinbox.data()->spinBoxDownHoverPen       );
    app->setProperty("SpinBox_DefaultParameters_DownFocusHoverPen"    ,spinbox.data()->spinBoxDownFocusHoverPen  );
    app->setProperty("SpinBox_DefaultParameters_DownClickPen "        ,spinbox.data()->spinBoxDownClickPen       );
    app->setProperty("SpinBox_DefaultParameters_DownDisablePen"       ,spinbox.data()->spinBoxDownDisablePen     );
    app->setProperty("SpinBox_DefaultParameters_UpIconHightPixMap"    ,spinbox.data()->spinBoxUpIconHightPixMap  );
    app->setProperty("SpinBox_DefaultParameters_DownIconHightPixMap"  ,spinbox.data()->spinBoxDownIconHightPixMap);

    //combobox
    QSharedPointer<ConfigComboBoxParameters> comboBox(new ConfigComboBoxParameters);
    sp->getDefaultConfigComboBoxParameters(comboBox.data(), isUseDarkPalette());
    app->setProperty("ComboBox_DefaultParameters_Radius"      , comboBox.data()->radius              );
    app->setProperty("ComboBox_DefaultParameters_DefaultBrush", comboBox.data()->comboBoxDefaultBrush);
    app->setProperty("ComboBox_DefaultParameters_HoverBrush"  , comboBox.data()->comboBoxHoverBrush  );
    app->setProperty("ComboBox_DefaultParameters_OnBrush"     , comboBox.data()->comboBoxOnBrush     );
    app->setProperty("ComboBox_DefaultParameters_EditBrush"   , comboBox.data()->comboBoxEditBrush   );
    app->setProperty("ComboBox_DefaultParameters_DisableBrush", comboBox.data()->comboBoxDisableBrush);
    app->setProperty("ComboBox_DefaultParameters_DefaultPen"  , comboBox.data()->comboBoxDefaultPen  );
    app->setProperty("ComboBox_DefaultParameters_HoverPen"    , comboBox.data()->comboBoxHoverPen    );
    app->setProperty("ComboBox_DefaultParameters_OnPen"       , comboBox.data()->comboBoxOnPen       );
    app->setProperty("ComboBox_DefaultParameters_EditPen"     , comboBox.data()->comboBoxEditPen     );
    app->setProperty("ComboBox_DefaultParameters_DisablePen"  , comboBox.data()->comboBoxDisablePen  );
    app->setProperty("ComboBox_DefaultParameters_FocusPen"    , comboBox.data()->comboBoxFocusPen    );

    //list
    QSharedPointer<ConfigListParameters> list(new ConfigListParameters);
    sp->getDefaultConfigListParameters(list.data(), isUseDarkPalette());
    app->setProperty("List_DefaultParameters_Radius"         , list.data()->radius            );
    app->setProperty("List_DefaultParameters_DefaultBrush"   , list.data()->listDefaultBrush  );
    app->setProperty("List_DefaultParameters_HoverBrush"     , list.data()->listHoverBrush    );
    app->setProperty("List_DefaultParameters_SelectBrush"    , list.data()->listSelectBrush   );
    app->setProperty("List_DefaultParameters_DisableBrush"   , list.data()->listDisableBrush  );
    app->setProperty("List_DefaultParameters_TextHoverPen"   , list.data()->listTextHoverPen  );
    app->setProperty("List_DefaultParameters_TextSelectPen"  , list.data()->listTextSelectPen );
    app->setProperty("List_DefaultParameters_TextDisablePen" , list.data()->listTextDisablePen);
    app->setProperty("List_DefaultParameters_HoverPen"       , list.data()->listHoverPen      );
    app->setProperty("List_DefaultParameters_SelectPen"      , list.data()->listSelectPen     );
    app->setProperty("List_DefaultParameters_FocusPen"       , list.data()->listFocusPen      );

    sp->getHighlightModeConfigListParameters(list.data(), isUseDarkPalette());
    app->setProperty("List_HighlightMode_Radius"        , list.data()->radius            );
    app->setProperty("List_HighlightMode_DefaultBrush"  , list.data()->listDefaultBrush  );
    app->setProperty("List_HighlightMode_HoverBrush"    , list.data()->listHoverBrush    );
    app->setProperty("List_HighlightMode_SelectBrush"   , list.data()->listSelectBrush   );
    app->setProperty("List_HighlightMode_DisableBrush"  , list.data()->listDisableBrush  );
    app->setProperty("List_HighlightMode_TextHoverPen"  , list.data()->listTextHoverPen  );
    app->setProperty("List_HighlightMode_TextSelectPen" , list.data()->listTextSelectPen );
    app->setProperty("List_HighlightMode_TextDisablePen", list.data()->listTextDisablePen);
    app->setProperty("List_HighlightMode_HoverPen"      , list.data()->listHoverPen      );
    app->setProperty("List_HighlightMode_SelectPen"     , list.data()->listSelectPen     );
    app->setProperty("List_HighlightMode_FocusPen"      , list.data()->listFocusPen      );

    //tree
    QSharedPointer<ConfigTreeParameters> tree(new ConfigTreeParameters);
    sp->getDefaultConfigTreeParameters(tree.data(), isUseDarkPalette());
    app->setProperty("Tree_DefaultParameters_Radius"            , tree.data()->radius                );
    app->setProperty("Tree_DefaultParameters_DefaultBrush"      , tree.data()->treeDefaultBrush      );
    app->setProperty("Tree_DefaultParameters_HoverBrush"        , tree.data()->treeHoverBrush        );
    app->setProperty("Tree_DefaultParameters_SelectBrush"       , tree.data()->treeSelectBrush       );
    app->setProperty("Tree_DefaultParameters_DisableBrush"      , tree.data()->treeDisableBrush      );
    app->setProperty("Tree_DefaultParameters_BranchDefaultBrush", tree.data()->treeBranchDefaultBrush);
    app->setProperty("Tree_DefaultParameters_BranchHoverBrush"  , tree.data()->treeBranchHoverBrush  );
    app->setProperty("Tree_DefaultParameters_BranchSelectBrush" , tree.data()->treeBranchSelectBrush );
    app->setProperty("Tree_DefaultParameters_BranchDisableBrush", tree.data()->treeBranchDisableBrush);
    app->setProperty("Tree_DefaultParameters_HoverPen"          , tree.data()->treeHoverPen          );
    app->setProperty("Tree_DefaultParameters_SelectPen"         , tree.data()->treeSelectPen         );
    app->setProperty("Tree_DefaultParameters_FocusPen"          , tree.data()->treeFocusPen          );
    app->setProperty("Tree_DefaultParameters_TextHoverPen"      , tree.data()->treeTextHoverPen      );
    app->setProperty("Tree_DefaultParameters_TextSelectPen"     , tree.data()->treeTextSelectPen     );
    app->setProperty("Tree_DefaultParameters_TextDisablePen"    , tree.data()->treeTextDisablePen    );

    sp->getHighlightModeConfigTreeParameters(tree.data(), isUseDarkPalette());
    app->setProperty("Tree_HighlightMode_Radius"            , tree.data()->radius                );
    app->setProperty("Tree_HighlightMode_DefaultBrush"      , tree.data()->treeDefaultBrush      );
    app->setProperty("Tree_HighlightMode_HoverBrush"        , tree.data()->treeHoverBrush        );
    app->setProperty("Tree_HighlightMode_SelectBrush"       , tree.data()->treeSelectBrush       );
    app->setProperty("Tree_HighlightMode_DisableBrush"      , tree.data()->treeDisableBrush      );
    app->setProperty("Tree_HighlightMode_BranchDefaultBrush", tree.data()->treeBranchDefaultBrush);
    app->setProperty("Tree_HighlightMode_BranchHoverBrush"  , tree.data()->treeBranchHoverBrush  );
    app->setProperty("Tree_HighlightMode_BranchSelectBrush" , tree.data()->treeBranchSelectBrush );
    app->setProperty("Tree_HighlightMode_BranchDisableBrush", tree.data()->treeBranchDisableBrush);
    app->setProperty("Tree_HighlightMode_HoverPen"          , tree.data()->treeHoverPen          );
    app->setProperty("Tree_HighlightMode_SelectPen"         , tree.data()->treeSelectPen         );
    app->setProperty("Tree_HighlightMode_FocusPen"          , tree.data()->treeFocusPen          );
    app->setProperty("Tree_HighlightMode_TextHoverPen"      , tree.data()->treeTextHoverPen      );
    app->setProperty("Tree_HighlightMode_TextSelectPen"     , tree.data()->treeTextSelectPen     );
    app->setProperty("Tree_HighlightMode_TextDisablePen"    , tree.data()->treeTextDisablePen    );

    //table
    QSharedPointer<ConfigTableParameters> table(new ConfigTableParameters);
    sp->getDefaultConfigTableParameters(table.data(), isUseDarkPalette());
    app->setProperty("Table_DefaultParameters_Radius"        , table.data()->radius             );
    app->setProperty("Table_DefaultParameters_DefaultBrush"  , table.data()->tableDefaultBrush  );
    app->setProperty("Table_DefaultParameters_HoverBrush"    , table.data()->tableHoverBrush    );
    app->setProperty("Table_DefaultParameters_SelectBrush"   , table.data()->tableSelectBrush   );
    app->setProperty("Table_DefaultParameters_DisableBrush"  , table.data()->tableDisableBrush  );
    app->setProperty("Table_DefaultParameters_HoverPen"      , table.data()->tableHoverPen      );
    app->setProperty("Table_DefaultParameters_SelectPen"     , table.data()->tableSelectPen     );
    app->setProperty("Table_DefaultParameters_FocusPen"      , table.data()->tableFocusPen      );
    app->setProperty("Table_DefaultParameters_TextHoverPen"  , table.data()->tableTextHoverPen  );
    app->setProperty("Table_DefaultParameters_TextSelectPen" , table.data()->tableTextSelectPen );
    app->setProperty("Table_DefaultParameters_TextDisablePen", table.data()->tableTextDisablePen);

    sp->getHighlightModeConfigTableParameters(table.data(), isUseDarkPalette());
    app->setProperty("Table_HighlightMode_Radius"        , table.data()->radius             );
    app->setProperty("Table_HighlightMode_DefaultBrush"  , table.data()->tableDefaultBrush  );
    app->setProperty("Table_HighlightMode_HoverBrush"    , table.data()->tableHoverBrush    );
    app->setProperty("Table_HighlightMode_SelectBrush"   , table.data()->tableSelectBrush   );
    app->setProperty("Table_HighlightMode_DisableBrush"  , table.data()->tableDisableBrush  );
    app->setProperty("Table_HighlightMode_HoverPen"      , table.data()->tableHoverPen      );
    app->setProperty("Table_HighlightMode_SelectPen"     , table.data()->tableSelectPen     );
    app->setProperty("Table_HighlightMode_FocusPen"      , table.data()->tableFocusPen      );
    app->setProperty("Table_HighlightMode_TextHoverPen"  , table.data()->tableTextHoverPen  );
    app->setProperty("Table_HighlightMode_TextSelectPen" , table.data()->tableTextSelectPen );
    app->setProperty("Table_HighlightMode_TextDisablePen", table.data()->tableTextDisablePen);


    //checkbox
    QSharedPointer<ConfigCheckBoxParameters> checkBox(new ConfigCheckBoxParameters);
    sp->getDefaultConfigCheckBoxParameters(checkBox.data(), isUseDarkPalette());
    app->setProperty("CheckBox_DefaultParameters_Radius"          , checkBox.data()->radius                  );
    app->setProperty("CheckBox_DefaultParameters_DefaultPen"      , checkBox.data()->checkBoxDefaultPen      );
    app->setProperty("CheckBox_DefaultParameters_HoverPen"        , checkBox.data()->checkBoxHoverPen        );
    app->setProperty("CheckBox_DefaultParameters_ClickPen"        , checkBox.data()->checkBoxClickPen        );
    app->setProperty("CheckBox_DefaultParameters_DisablePen"      , checkBox.data()->checkBoxDisablePen      );
    app->setProperty("CheckBox_DefaultParameters_OnDefaultPen"    , checkBox.data()->checkBoxOnDefaultPen    );
    app->setProperty("CheckBox_DefaultParameters_OnHoverPen"      , checkBox.data()->checkBoxOnHoverPen      );
    app->setProperty("CheckBox_DefaultParameters_OnClickPen"      , checkBox.data()->checkBoxOnClickPen      );
    app->setProperty("CheckBox_DefaultParameters_ContentPen"      , checkBox.data()->checkBoxContentPen      );
    app->setProperty("CheckBox_DefaultParameters_DefaultBrush"    , checkBox.data()->checkBoxDefaultBrush    );
    app->setProperty("CheckBox_DefaultParameters_HoverBrush"      , checkBox.data()->checkBoxHoverBrush      );
    app->setProperty("CheckBox_DefaultParameters_ClickBrush"      , checkBox.data()->checkBoxClickBrush      );
    app->setProperty("CheckBox_DefaultParameters_DisableBrush"    , checkBox.data()->checkBoxDisableBrush    );
    app->setProperty("CheckBox_DefaultParameters_OnDefaultBrush"  , checkBox.data()->checkBoxOnDefaultBrush  );
    app->setProperty("CheckBox_DefaultParameters_OnHoverBrush"    , checkBox.data()->checkBoxOnHoverBrush    );
    app->setProperty("CheckBox_DefaultParameters_OnClickBrush"    , checkBox.data()->checkBoxOnClickBrush    );
    app->setProperty("CheckBox_DefaultParameters_PathBrush"       , checkBox.data()->checkBoxPathBrush       );
    app->setProperty("CheckBox_DefaultParameters_PathDisableBrush", checkBox.data()->checkBoxPathDisableBrush);

    //radiobutton
    QSharedPointer<ConfigRadioButtonParameters> radiobutton(new ConfigRadioButtonParameters);
    sp->getDefaultConfigRadioButtonParameters(radiobutton.data(), isUseDarkPalette());
    app->setProperty("RadioButton_DefaultParameters_Radius"                , radiobutton.data()->radius                  );
    app->setProperty("RadioButton_DefaultParameters_DefaultPen"            , radiobutton.data()->radioButtonDefaultPen            );
    app->setProperty("RadioButton_DefaultParameters_HoverPen"              , radiobutton.data()->radioButtonHoverPen              );
    app->setProperty("RadioButton_DefaultParameters_ClickPen"              , radiobutton.data()->radioButtonClickPen              );
    app->setProperty("RadioButton_DefaultParameters_DisablePen"            , radiobutton.data()->radioButtonDisablePen            );
    app->setProperty("RadioButton_DefaultParameters_OnDefaultPen"          , radiobutton.data()->radioButtonOnDefaultPen          );
    app->setProperty("RadioButton_DefaultParameters_OnHoverPen"            , radiobutton.data()->radioButtonOnHoverPen            );
    app->setProperty("RadioButton_DefaultParameters_OnClickPen"            , radiobutton.data()->radioButtonOnClickPen            );
    app->setProperty("RadioButton_DefaultParameters_DefaultBrush"          , radiobutton.data()->radioButtonDefaultBrush          );
    app->setProperty("RadioButton_DefaultParameters_HoverBrush"            , radiobutton.data()->radioButtonHoverBrush            );
    app->setProperty("RadioButton_DefaultParameters_ClickBrush"            , radiobutton.data()->radioButtonClickBrush            );
    app->setProperty("RadioButton_DefaultParameters_DisableBrush"          , radiobutton.data()->radioButtonDisableBrush          );
    app->setProperty("RadioButton_DefaultParameters_OnDefaultBrush"        , radiobutton.data()->radioButtonOnDefaultBrush        );
    app->setProperty("RadioButton_DefaultParameters_OnHoverBrush"          , radiobutton.data()->radioButtonOnHoverBrush          );
    app->setProperty("RadioButton_DefaultParameters_OnClickBrush"          , radiobutton.data()->radioButtonOnClickBrush          );
    app->setProperty("RadioButton_DefaultParameters_ChildrenOnDefaultBrush", radiobutton.data()->radioButtonChildrenOnDefaultBrush);
    app->setProperty("RadioButton_DefaultParameters_ChildrenOnHoverBrush"  , radiobutton.data()->radioButtonChildrenOnHoverBrush  );
    app->setProperty("RadioButton_DefaultParameters_ChildrenOnClickBrush  ", radiobutton.data()->radioButtonChildrenOnClickBrush  );
    app->setProperty("RadioButton_DefaultParameters_ChildrenOnDisableBrush", radiobutton.data()->radioButtonChildrenOnDisableBrush);

    //slider
    QSharedPointer<ConfigSliderParameters> slider(new ConfigSliderParameters);
    sp->getDefaultSliderParameters(slider.data(), isUseDarkPalette());
    app->setProperty("Slider_DefaultParameters_Radius"                   , slider.data()->radius                         );
    app->setProperty("Slider_DefaultParameters_GrooveRadius"             , slider.data()->sliderGrooveRadius             );
    app->setProperty("Slider_DefaultParameters_HandleDefaultBrush"       , slider.data()->sliderHandleDefaultBrush       );
    app->setProperty("Slider_DefaultParameters_HandleHoverBrush"         , slider.data()->sliderHandleHoverBrush         );
    app->setProperty("Slider_DefaultParameters_HandleClickBrush"         , slider.data()->sliderHandleClickBrush         );
    app->setProperty("Slider_DefaultParameters_HandleDisableBrush"       , slider.data()->sliderHandleDisableBrush       );
    app->setProperty("Slider_DefaultParameters_GrooveValueDefaultBrush"  , slider.data()->sliderGrooveValueDefaultBrush  );
    app->setProperty("Slider_DefaultParameters_GrooveValueHoverBrush"    , slider.data()->sliderGrooveValueHoverBrush    );
    app->setProperty("Slider_DefaultParameters_GrooveValueDisableBrush"  , slider.data()->sliderGrooveValueDisableBrush  );
    app->setProperty("Slider_DefaultParameters_GrooveUnvalueDefaultBrush", slider.data()->sliderGrooveUnvalueDefaultBrush);
    app->setProperty("Slider_DefaultParameters_GrooveUnvalueHoverBrush"  , slider.data()->sliderGrooveUnvalueHoverBrush  );
    app->setProperty("Slider_DefaultParameters_GrooveUnvalueDisableBrush", slider.data()->sliderGrooveUnvalueDisableBrush);
    app->setProperty("Slider_DefaultParameters_HandleDefaultPen"         , slider.data()->sliderHandleDefaultPen         );
    app->setProperty("Slider_DefaultParameters_HandleHoverPen"           , slider.data()->sliderHandleHoverPen           );
    app->setProperty("Slider_DefaultParameters_HandleClickPen"           , slider.data()->sliderHandleClickPen           );
    app->setProperty("Slider_DefaultParameters_HandleDisablePen"         , slider.data()->sliderHandleDisablePen         );
    app->setProperty("Slider_DefaultParameters_GroovePen"                , slider.data()->sliderGroovePen                );
    app->setProperty("Slider_DefaultParameters_focusPen"                 , slider.data()->focusPen                       );

    //progressabr
    QSharedPointer<ConfigProgressBarParameters> progressbar(new ConfigProgressBarParameters);
    sp->getDefaultProgressBarParameters(progressbar.data(), isUseDarkPalette());
    app->setProperty("ProgressBar_DefaultParameters_Radius"              , progressbar.data()->radius                         );
    app->setProperty("ProgressBar_DefaultParameters_ContentPen"          , progressbar.data()->progressBarContentPen          );
    app->setProperty("ProgressBar_DefaultParameters_GrooveDefaultPen"    , progressbar.data()->progressBarGrooveDefaultPen    );
    app->setProperty("ProgressBar_DefaultParameters_GrooveDisablePen"    , progressbar.data()->progressBarGrooveDisablePen    );
    app->setProperty("ProgressBar_DefaultParameters_ContentBrush"        , progressbar.data()->progressBarContentBrush        );
    app->setProperty("ProgressBar_DefaultParameters_GrooveDefaultBrush"  , progressbar.data()->progressBarGrooveDefaultBrush  );
    app->setProperty("ProgressBar_DefaultParameters_GrooveDisableBrush"  , progressbar.data()->progressBarGrooveDisableBrush  );
    app->setProperty("ProgressBar_DefaultParameters_ContentHightColor"   , progressbar.data()->progressBarContentHightColor   );
    app->setProperty("ProgressBar_DefaultParameters_ContentMidLightColor", progressbar.data()->progressBarContentMidLightColor);

    //scrollbar
    QSharedPointer<ConfigScrollBarParameters> scrollbar(new ConfigScrollBarParameters);
    sp->getDefaultScrollBarParameters(scrollbar.data(), isUseDarkPalette());
    app->setProperty("ScrollBar_DefaultParameters_Radius"             , scrollbar.data()->radius                      );
    app->setProperty("ScrollBar_DefaultParameters_GrooveDefaultBrush" , scrollbar.data()->scrollBarGrooveDefaultBrush );
    app->setProperty("ScrollBar_DefaultParameters_GrooveInactiveBrush", scrollbar.data()->scrollBarGrooveInactiveBrush);
    app->setProperty("ScrollBar_DefaultParameters_SliderDefaultBrush" , scrollbar.data()->scrollBarSliderDefaultBrush );
    app->setProperty("ScrollBar_DefaultParameters_SliderHoverBrush"   , scrollbar.data()->scrollBarSliderHoverBrush   );
    app->setProperty("ScrollBar_DefaultParameters_SliderClickBrush"   , scrollbar.data()->scrollBarSliderClickBrush   );
    app->setProperty("ScrollBar_DefaultParameters_SliderDisableBrush" , scrollbar.data()->scrollBarSliderDisableBrush );

    //tooltip
    QSharedPointer<ConfigToolTipParameters> tooltip(new ConfigToolTipParameters);
    sp->getDefaultToolTipParameters(tooltip.data(), isUseDarkPalette());
    app->setProperty("ToolTip_DefaultParameters_Radius"         , tooltip.data()->radius                );
    app->setProperty("ToolTip_DefaultParameters_BackgroundBrush", tooltip.data()->toolTipBackgroundBrush);
    app->setProperty("ToolTip_DefaultParameters_BackgroundPen"  , tooltip.data()->toolTipBackgroundPen  );

    //tabwidget
    QSharedPointer<ConfigTabWidgetParameters> tabwidget(new ConfigTabWidgetParameters);
    sp->getDefaultTabWidgetParameters(tabwidget.data(), isUseDarkPalette());
    app->setProperty("TabWidget_DefaultParameters_Radius"                  , tabwidget.data()->radius                  );
    app->setProperty("TabWidget_DefaultParameters_tabWidgetBackgroundBrush", tabwidget.data()->tabWidgetBackgroundBrush);
    app->setProperty("TabWidget_DefaultParameters_tabBarBackgroundBrush"   , tabwidget.data()->tabBarBackgroundBrush   );
    app->setProperty("TabWidget_DefaultParameters_tabBarDefaultBrush"      , tabwidget.data()->tabBarDefaultBrush      );
    app->setProperty("TabWidget_DefaultParameters_tabBarHoverBrush"        , tabwidget.data()->tabBarHoverBrush        );
    app->setProperty("TabWidget_DefaultParameters_tabBarClickBrush"        , tabwidget.data()->tabBarClickBrush        );
    app->setProperty("TabWidget_DefaultParameters_tabBarSelectBrush"       , tabwidget.data()->tabBarSelectBrush       );
    app->setProperty("TabWidget_DefaultParameters_tabBarDefaultPen"        , tabwidget.data()->tabBarDefaultPen        );
    app->setProperty("TabWidget_DefaultParameters_tabBarHoverPen"          , tabwidget.data()->tabBarHoverPen          );
    app->setProperty("TabWidget_DefaultParameters_tabBarClickPen"          , tabwidget.data()->tabBarClickPen          );
    app->setProperty("TabWidget_DefaultParameters_tabBarFocusPen"          , tabwidget.data()->tabBarFocusPen          );

    //menu
    QSharedPointer<ConfigMenuParameters> menu(new ConfigMenuParameters);
    sp->getDefaultMenuParameters(menu.data(), isUseDarkPalette());
    app->setProperty("Menu_DefaultParameters_frameRadius"        , menu.data()->frameRadius        );
    app->setProperty("Menu_DefaultParameters_itemRadius"         , menu.data()->itemRadius         );
    app->setProperty("Menu_DefaultParameters_menuBackgroundBrush", menu.data()->menuBackgroundBrush);
    app->setProperty("Menu_DefaultParameters_menuBackgroundPen"  , menu.data()->menuBackgroundPen  );
    app->setProperty("Menu_DefaultParameters_menuItemSelectBrush", menu.data()->menuItemSelectBrush);
    app->setProperty("Menu_DefaultParameters_menuItemFocusPen"   , menu.data()->menuItemFocusPen   );
    app->setProperty("Menu_DefaultParameters_menuTextHoverPen"   , menu.data()->menuTextHoverPen   );
}
