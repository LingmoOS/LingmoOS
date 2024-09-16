/*
 * SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "chameleonstyle.h"
#include "common.h"

#include <DNativeSettings>
#include <DStyleOption>
#include <DApplication>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
#include <DPlatformTheme>
#include <DSlider>
#include <DTabBar>
#include <DSearchEdit>
#include <DButtonBox>
#include <DDciIcon>
#include <DDciIconPalette>
#include <DSizeMode>

#include <QLabel>
#include <QCalendarWidget>
#include <QLayout>
#include <QVariant>
#include <QDebug>
#include <QApplication>
#include <QPushButton>
#include <QComboBox>
#include <QScrollBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QToolButton>
#include <QLineEdit>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QPaintEngine>
#include <QAbstractItemView>
#include <QBitmap>
#include <QTableView>
#include <QStyledItemDelegate>
#include <DSpinBox>
#include <DTreeView>
#include <DIconButton>
#include <DTabBar>
#include <DDateTimeEdit>
#include <DListView>
#include <DToolTip>
#include <DPaletteHelper>
#include <QtMath>
#include <QtGlobal>
#include <private/qcombobox_p.h>
#include <private/qcommonstyle_p.h>
#include <private/qevent_p.h>

#include <qdrawutil.h>
#include <qpa/qplatformwindow.h>

#include "dstyleanimation.h"

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace chameleon {

// 判断是TabBar否为竖直方向
inline static bool verticalTabs(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest
           || shape == QTabBar::RoundedEast
           || shape == QTabBar::TriangularWest
           || shape == QTabBar::TriangularEast;
}

inline static int menuItemShortcutWidth(const QStyleOptionMenuItem *menuItem)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return menuItem->reservedShortcutWidth;
#else
    return menuItem->tabWidth;
#endif
}

static QWidget *getSbarParentWidget(QScrollBar *sbar)
{
    if (!sbar)
        return nullptr;
    QWidget *pw = sbar->parentWidget();
    if (!pw)
        return nullptr;

    bool isContainer = !pw->objectName().compare(QLatin1String("qt_scrollarea_vcontainer")) ||
            !pw->objectName().compare(QLatin1String("qt_scrollarea_hcontainer")) ;

    return isContainer ? pw->parentWidget() : pw;
}

static inline bool hoveredOrPressed(const QStyleOption *opt)
{
    return opt->state.testFlag(QStyle::State_MouseOver) ||
            opt->state.testFlag(QStyle::State_Sunken);
}

// Calculating indicator's size for spinbox.
static QRect spinboxIndicatorRect(const QRect &r)
{
    int size = qMin(r.width(), r.height());
    int xOffset = r.x() + (r.width() - size) / 2;
    int yOffset = r.y() + (r.height() - size) / 2;
    return QRect(xOffset, yOffset, size, size);
}

// the object is the class's instance.
template<class T>
inline static bool isTheClassObject(QObject *object)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return object->metaObject()->className() == T::staticMetaObject.className();
#else
    return object->metaObject()->metaType() == T::staticMetaObject.metaType();
#endif
}

ChameleonStyle::ChameleonStyle()
    : DStyle()
{

}

static QColor getThemTypeColor(QColor lightColor, QColor darkColor)
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        return lightColor;
    else
        return darkColor;
}

static DDciIconPalette makeIconPalette(const QPalette &pal)
{
    DDciIconPalette iconPalette;
    iconPalette.setForeground(pal.color(QPalette::WindowText));
    iconPalette.setBackground(pal.color(QPalette::Window));
    iconPalette.setHighlight(pal.color(QPalette::Highlight));
    iconPalette.setHighlightForeground(pal.color(QPalette::HighlightedText));
    return iconPalette;
}

void ChameleonStyle::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt,
                                   QPainter *p, const QWidget *w) const
{
    switch (static_cast<int>(pe)) {
    case PE_PanelButtonCommand: {
//        qDebug() << "### pushbutton state " << (int)opt->state;
        const QMargins &margins = frameExtentMargins();

        // checked
        if (opt->state & State_On) {
            p->setBrush(getColor(opt, QPalette::Highlight));
        } else {
            drawShadow(p, opt->rect - margins, getColor(opt, QPalette::Shadow));
            // 初始化button的渐变背景色
            QLinearGradient lg(QPointF(0, opt->rect.top()),
                               QPointF(0, opt->rect.bottom()));
            lg.setColorAt(0, getColor(opt, QPalette::Light));
            lg.setColorAt(1, getColor(opt, QPalette::Dark));

            p->setBrush(lg);
        }

        p->setPen(Qt::NoPen);
        p->setRenderHint(QPainter::Antialiasing);
        int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
        p->drawRoundedRect(opt->rect - margins, frame_radius, frame_radius);

        // draw border，border应该是完全叠加到按钮的背景上
        p->setPen(QPen(getColor(opt, DPalette::FrameBorder, w), Metrics::Painter_PenWidth));
        p->setBrush(Qt::NoBrush);
        const QMarginsF border_margins(Metrics::Painter_PenWidth, Metrics::Painter_PenWidth, Metrics::Painter_PenWidth, Metrics::Painter_PenWidth);
        p->drawRoundedRect(QRectF(opt->rect) - margins - border_margins / 2.0, frame_radius, frame_radius);

        return;
    }
    case PE_FrameFocusRect: {
        //设计要求QTreeView选中整行，这里不绘制focus rect
        if (qobject_cast<const QTreeView *>(w)) {
            return;
        }

        //QListView不需要绘制focus rect
        if (qobject_cast<const QListView*>(w)) {
            return;
        }

        if (w && w->property("_d_dtk_noFocusRect").toBool())
            return;

        //设计要求DDateTimeEdit focus只绘制边缘
        if (qobject_cast<const QLineEdit*>(w) && qobject_cast<const DDateTimeEdit *>(w->parentWidget())) {
            return;
        }

        drawBorder(p, opt, w);
        return;
    }
    case PE_PanelItemViewItem: {
        if (w && w->objectName() == "qt_calendar_calendarview") {
            if (opt->state & QStyle::State_Selected) {
                QRect textRect = opt->rect;
                textRect.setWidth(textRect.height());
                p->setBrush(opt->palette.brush(QPalette::Highlight));
                textRect.moveCenter(opt->rect.center());
                p->setPen(Qt::NoPen);
                p->setRenderHint(QPainter::Antialiasing);
                p->drawEllipse(textRect.adjusted(1, 1, -1, -1));
            }
            return;
        }

        //QTreeView的绘制复制了QCommonStyle的代码，添加了圆角的处理,hover的处理
        if (qobject_cast<const QTreeView *>(w)) {
            const auto delegate = qobject_cast<const QTreeView *>(w)->itemDelegate();
            //如果QTreeView使用的不是默认代理 QStyledItemDelegate,则采取DStyle的默认绘制(备注:这里的QtCreator不会有hover效果和圆角)
            if (!isTheClassObject<QStyledItemDelegate>(delegate)) {
                break;
            }

            if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                QPalette::ColorGroup cg = (w ? w->isEnabled() : (vopt->state & QStyle::State_Enabled))
                                          ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

                if (vopt->showDecorationSelected && (vopt->state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
                    p->setRenderHint(QPainter::Antialiasing, true);
                    p->setPen(Qt::NoPen);
                    p->setBrush(vopt->palette.brush(cg, (vopt->state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Midlight));

                    if ((vopt->state & QStyle::State_Selected) && (vopt->state & QStyle::State_MouseOver)) {
                        p->setBrush(p->brush().color().lighter(120));
                    }

                    //只对最后一列的item绘制圆角
                    if (vopt->viewItemPosition == QStyleOptionViewItem::End || vopt->viewItemPosition == QStyleOptionViewItem::OnlyOne) {
                        p->drawRoundedRect(vopt->rect.adjusted(-frame_radius, 0, 0, 0), frame_radius, frame_radius);
                    } else if (vopt->viewItemPosition != QStyleOptionViewItem::Invalid) {
                        p->drawRoundedRect(vopt->rect.adjusted(-frame_radius, 0, frame_radius, 0), frame_radius, frame_radius);
                    }
                } else {
                    if (vopt->backgroundBrush.style() != Qt::NoBrush) {
                        QPointF oldBO = p->brushOrigin();
                        p->setBrushOrigin(vopt->rect.topLeft());
                        p->fillRect(vopt->rect, vopt->backgroundBrush);
                        p->setBrushOrigin(oldBO);
                    }

                    if (vopt->state & QStyle::State_Selected) {
                        QRect textRect = subElementRect(QStyle::SE_ItemViewItemText,  opt, w);
                        p->fillRect(textRect, vopt->palette.brush(cg, QPalette::Highlight));
                    }
                }
            }
            return;
        }

        if (drawTableViewItem(pe, opt, p, w))
            return;

        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

            if (vopt->state & QStyle::State_Selected) {
               QRect select_rect = opt->rect;

               if (!vopt->showDecorationSelected) {
                   select_rect = proxy()->subElementRect(QStyle::SE_ItemViewItemText,  opt, w);
               }

               p->setPen(Qt::NoPen);
               p->setBrush(getColor(opt, QPalette::Highlight));
               p->setRenderHint(QPainter::Antialiasing);
               p->drawRoundedRect(select_rect, frame_radius, frame_radius);
               return;
            } else {
                if (vopt->backgroundBrush.style() != Qt::NoBrush) {
                    p->save();
                    p->setPen(Qt::NoPen);
                    p->setBrush(vopt->backgroundBrush);
                    p->setRenderHint(QPainter::Antialiasing);
                    p->drawRoundedRect(opt->rect, frame_radius, frame_radius);
                    p->restore();
                    return;
                }
            }
        }
        break;
    }
    case PE_PanelLineEdit: {
        if (w && w->parentWidget()) {
            //lineEdit作为子控件时不进行绘制
            if (qobject_cast<const QComboBox *>(w->parent()))
                return;

            if (auto edit = qobject_cast<const QDateTimeEdit *>(w->parentWidget())) {
                if (edit->calendarPopup())
                    return;
            }
        }

        if (auto fopt = qstyleoption_cast<const QStyleOptionFrame*>(opt)) {
            // Flat时不绘制输入框的背景
            if (fopt->features == QStyleOptionFrame::Flat) {
                if (opt->state.testFlag(QStyle::State_HasFocus)) {
                    proxy()->drawPrimitive(PE_FrameFocusRect, opt, p, w);
                }
                return;
            }

            if (fopt->lineWidth > 0) {
                proxy()->drawPrimitive(PE_FrameLineEdit, fopt, p, w);
            }
        }

        // 此处设置painter的行为不应该影响外面的绘制，会导致lineedit光标不对 Bug-20967。
        p->save();

        bool isTransBg = false;
        if (w) {
            auto prop = w->property("_d_dtk_lineedit_opacity");

            if (prop.isValid()) {
                isTransBg = prop.toBool();
            } else {
                const auto windowFlags = w->window()->windowFlags();
                const bool hasTransAttr = w->window()->testAttribute(Qt::WA_TranslucentBackground);
                const bool isDialog = windowFlags.testFlag(Qt::Dialog);
                const bool isPopup = windowFlags.testFlag(Qt::Popup);
                // if we set palette to lineedit (e.g. DLineEdit::setAlert )
                const bool isResolved = opt->palette.isBrushSet(QPalette::Current, QPalette::Button);
                isTransBg = hasTransAttr && isDialog && !isPopup && !isResolved;
            }
        }

        if (isTransBg)
            p->setBrush(getThemTypeColor(QColor(0, 0, 0, 255* 0.08),
                                        QColor(255, 255, 255, 255 * 0.15)));
        else
            p->setBrush(opt->palette.button());

        p->setPen(Qt::NoPen);
        p->setRenderHints(QPainter::Antialiasing);
        int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

        if ((w && qobject_cast<QAbstractSpinBox*>(w->parentWidget()))) {
            if (w->parentWidget()->property("_d_dtk_spinBox").toBool()) {
                DDrawUtils::drawRoundedRect(p, opt->rect, frame_radius, frame_radius,
                                        DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
            } else {
                p->drawRoundedRect(opt->rect - frameExtentMargins(), frame_radius, frame_radius);
            }
        } else {
            p->drawRoundedRect(opt->rect, frame_radius, frame_radius);
        }

        if (opt->state.testFlag(QStyle::State_HasFocus)) {
            proxy()->drawPrimitive(PE_FrameFocusRect, opt, p, w);
        }

        p->restore();
        return;
    }
    case PE_FrameLineEdit: {
        // lineedit no frame
        return;
    }
    case PE_IndicatorRadioButton: {
        QRect standard = opt->rect;

        p->setRenderHint(QPainter::Antialiasing, true);

        if (opt->state & State_On) {  //Qt::Checked
            int padding = qCeil(standard.width() / 2.0 / 2.0);
            QPainterPath path;

            path.addEllipse(standard);
            path.addEllipse(standard.adjusted(padding, padding, -padding, -padding));

            p->fillPath(path, getColor(opt, DPalette::Highlight));

            // 内圈填充
            QPainterPath innerCirclePath;
            innerCirclePath.addEllipse(standard.adjusted(padding, padding, -padding, -padding));
            p->fillPath(innerCirclePath, getThemTypeColor(Qt::white, Qt::black));
        } else if (opt->state & State_Off) {
            p->setPen(QPen(getColor(opt, DPalette::WindowText), 1));
            p->drawEllipse(standard.adjusted(1, 1, -1, -1));

            // 内圈填充
            QPainterPath innerCirclePath;
            innerCirclePath.addEllipse(standard.adjusted(1, 1, -1, -1));
            p->fillPath(innerCirclePath, getThemTypeColor(Qt::transparent, QColor(0, 0, 0, qCeil(255 * 0.5))));
        }

        return;
    }
    case PE_IndicatorCheckBox: {
        QRectF standard = opt->rect;

        if (opt->state & State_NoChange) {  //Qt::PartiallyChecked
            DDrawUtils::drawBorder(p, standard, getColor(opt, DPalette::WindowText), 1, 2);

            // 内部矩形填充
            p->setBrush(getThemTypeColor(Qt::transparent, QColor(0, 0, 0, qCeil(255 * 0.5))));
            p->drawRoundedRect(standard.adjusted(1, 1, -1, -1), 2, 2);

            QRectF lineRect(0, 0, standard.width() / 2.0, 2);
            lineRect.moveCenter(standard.center());
            p->fillRect(lineRect, getColor(opt, DPalette::TextTitle, w));
        } else if (opt->state & State_On) {  //Qt::Checked
            // 填充整个矩形
            p->setPen(Qt::NoPen);
            p->setBrush(getThemTypeColor(Qt::transparent, Qt::black));
            p->drawRoundedRect(standard.adjusted(1, 1, -1, -1), 2, 2);

            p->setPen(getColor(opt, DPalette::Highlight));
            p->setBrush(Qt::NoBrush);

            QIcon icon = QIcon::fromTheme("checked");
            icon.paint(p, opt->rect.adjusted(-1, -1, 1, 1));
        } else {
            DDrawUtils::drawBorder(p, standard, getColor(opt, DPalette::WindowText), 1, 2);

            // 内部矩形填充
            p->setBrush(getThemTypeColor(Qt::transparent, getThemTypeColor(Qt::transparent, QColor(0, 0, 0, qCeil(255 * 0.5)))));
            p->drawRoundedRect(standard.adjusted(1, 1, -1, -1), 2, 2);
        }

        return;
    }
    case PE_IndicatorTabClose: {
        QIcon icon = DStyle::standardIcon(SP_CloseButton, opt, w);
        if (DGuiApplicationHelper::isTabletEnvironment()) {
            // 在平板中点击区域为36px 显示区域22px
            QRect iconRect(0, 0, TabBar_TabButtonSize, TabBar_TabButtonSize);
            iconRect.moveCenter(opt->rect.center());
            icon.paint(p, iconRect);
            return;
        }
        icon.paint(p, opt->rect);
        return;
    }
    case PE_FrameTabWidget: {
        p->setPen(QPen(getColor(opt, QPalette::Dark), proxy()->pixelMetric(PM_DefaultFrameWidth, opt, w)));
        p->setBrush(getColor(opt, QPalette::Window));
        p->drawRect(opt->rect);
        return;
    }
    case PE_IndicatorItemViewItemCheck: {
        QRectF standard = opt->rect;
        p->setRenderHint(QPainter::Antialiasing, true);
        QIcon::Mode mode = opt->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;

        if (opt->state & State_NoChange) {  //Qt::PartiallyChecked
            DStyle::standardIcon(SP_IndicatorChecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
        } else if (opt->state & State_On) {  //Qt::Checked
            p->setPen(getColor(opt, DPalette::Highlight));
            DStyle::standardIcon(SP_IndicatorChecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
        } else if (opt->state & State_Off) {  //Qt::Unchecked
            if (w && w->property("_d_dtk_UncheckedItemIndicator").toBool()) {
                DStyle::standardIcon(SP_IndicatorUnchecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
            }
            if (w && !w->property("_d_dtk_UncheckedItemIndicator").isValid() && !qobject_cast<const Dtk::Widget::DListView *>(w)) {
                DStyle::standardIcon(SP_IndicatorUnchecked, opt, w).paint(p, standard.toRect(), Qt::AlignCenter, mode);
            }
        }
        return;
    }
    case PE_PanelMenu: {
        if (opt->palette.window().color().isValid()
                && DWindowManagerHelper::instance()->hasBlurWindow()) {
            QColor color = opt->palette.window().color();
            color.setAlphaF(0.3);
            p->fillRect(opt->rect, color);
        } else {
            p->fillRect(opt->rect, opt->palette.window());
        }

        break;
    }
    case PE_Frame: {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            const bool hasProp = w && w->property("_d_dtk_frameRadius").isValid();
            if (f->features & QStyleOptionFrame::Rounded || hasProp) {
                p->setRenderHint(QPainter::Antialiasing);
                p->setBrush(p->background());

                if (f->lineWidth <= 0) {
                    p->setPen(Qt::NoPen);
                }

                int radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
                QRectF rect(opt->rect);
                rect.adjust(f->lineWidth / 2.0, f->lineWidth / 2.0, -f->lineWidth / 2.0, -f->lineWidth / 2.0);
                p->drawRoundedRect(rect, radius, radius);
                return;
            }
        }
        break;
    }
    case PE_PanelTipLabel: {
        return;
    }
    case PE_FrameGroupBox: {
        if (auto group_opt = qstyleoption_cast<const QStyleOptionFrame*>(opt)) {
            if (group_opt->features & QStyleOptionFrame::Flat)
                return;
        }
        DStyleOptionBackgroundGroup option;
        option.init(w);
        option.position = DStyleOptionBackgroundGroup::OnlyOne;
        static_cast<QStyleOption*>(&option)->operator =(*opt);
        DStyle::drawPrimitive(PE_ItemBackground, &option, p, w);
        return;
    }
    case PE_IndicatorArrowUp: {
        QIcon icon = DStyle::standardIcon(SP_ArrowUp, opt, w);
        icon.paint(p, spinboxIndicatorRect(opt->rect));
        return;
    }
    case PE_IndicatorArrowDown: {
        QIcon icon = DStyle::standardIcon(SP_ArrowDown, opt, w);
        icon.paint(p, spinboxIndicatorRect(opt->rect));
        return;
    }
    case PE_IndicatorArrowRight: {
        QIcon icon = DStyle::standardIcon(SP_ArrowRight, opt, w);
        icon.paint(p, opt->rect);
        return;
    }
    case PE_IndicatorArrowLeft: {
        QIcon icon = DStyle::standardIcon(SP_ArrowLeft, opt, w);
        icon.paint(p, opt->rect);
        return;
    }
    case PE_FrameTabBarBase: return;
    case PE_IndicatorBranch: {
        // tree
        QRect rect = DStyle::subElementRect(SE_HeaderArrow, opt, w);
        int rect_width = rect.width();
        int rect_height = rect.height();
        rect.setWidth(rect_width > rect_height ? rect_width : rect_height);
        rect.setHeight(rect_width > rect_height ? rect_width: rect_height);
        rect.moveCenter(opt->rect.center());

        if (opt->state & State_Children) {
            if (!(opt->state & State_Open)) {
                p->save();

                //在选中时进行反白处理
                if (opt->state & State_Selected) {
                    p->setPen(QColor(Qt::white));
                }

                DStyle::standardIcon(SP_ArrowRight, opt, w).paint(p, rect);
                p->restore();
                return;
            }

            p->save();

            //在选中时进行反白处理
            if (opt->state & State_Selected) {
                p->setPen(QColor(Qt::white));
            }

            DStyle::standardIcon(SP_ArrowDown, opt, w).paint(p, rect);
            p->restore();
        }
        return;
    }
    case PE_PanelItemViewRow: {
        // 不绘制选中的item背景，已经绘制了一个圆了
        if (w && qobject_cast<QCalendarWidget *>(w->parentWidget())) {
            return;
        }
        //这里QTreeView的绘制复制了QCommonStyle的代码，添加了圆角的处理,hover的处理
        if (qobject_cast<const QTreeView *>(w)) {
            const auto delegate = qobject_cast<const QTreeView *>(w)->itemDelegate();
            //如果QTreeView使用的不是默认代理 QStyledItemDelegate,则采取DStyle的默认绘制(备注:这里的QtCreator不会有hover效果和圆角)
            if (!isTheClassObject<QStyledItemDelegate>(delegate)) {
                break;
            }

            if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                QPalette::ColorGroup cg = (w ? w->isEnabled() : (vopt->state & QStyle::State_Enabled))
                        ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);

                if ((vopt->state & (QStyle::State_Selected | QStyle::State_MouseOver)) &&  proxy()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, opt, w)) {
                    p->setRenderHint(QPainter::Antialiasing, true);
                    p->setPen(Qt::NoPen);
                    p->setBrush(vopt->palette.brush(cg, (vopt->state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Midlight));

                    if ((vopt->state & QStyle::State_Selected) && (vopt->state & QStyle::State_MouseOver)) {
                        p->setBrush(p->brush().color().lighter(120));
                    }

                    if (vopt->viewItemPosition != QStyleOptionViewItem::End) {
                        // TODO: determine the positon(left and right) by a flag, draw rounded rect respectively.
                        if (vopt->rect.x() == 0) { // left component
                            DDrawUtils::drawRoundedRect(p, vopt->rect, frame_radius, frame_radius,
                                                        DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
                        } else { // right component
                            DDrawUtils::drawRoundedRect(p, vopt->rect, frame_radius, frame_radius,
                                                        DDrawUtils::TopRightCorner | DDrawUtils::BottomRightCorner);
                        }
                    } else if(vopt->viewItemPosition != QStyleOptionViewItem::Invalid){
                        p->drawRoundedRect(vopt->rect, frame_radius, frame_radius);
                    }
                } else if (vopt->features & QStyleOptionViewItem::Alternate) {
                    p->fillRect(vopt->rect, vopt->palette.brush(cg, QPalette::AlternateBase));
                }
            }
            return;
        }

        break;
    }
    case PE_FrameStatusBarItem: {
        return;
    } case PE_PanelStatusBar: {
        QColor bgcolor;
        QColor lineColor;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            bgcolor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Base), 0, 0, -10, 0, 0, 0, 95);
            lineColor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Button), 0, 0, 0, 0, 0, 0, 5);
        } else {
            bgcolor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Base), 0, 0, 0, 0, 0, 0, 70);
            lineColor = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Button), 0, 0, 60, 0, 0, 0, 0);
        }
        p->setPen(lineColor);
        p->setBrush(bgcolor);

        p->drawLine(opt->rect.topLeft(),  opt->rect.topRight());
        p->drawRect(opt->rect);

        return;
    }
    default:
        break;
    }

    DStyle::drawPrimitive(pe, opt, p, w);
}

// 按动画效果渐变隐藏滚动条，返回值为`true`表示隐藏滚动条，`false`表示继续绘制滚动条(改变了透明度)
bool ChameleonStyle::hideScrollBarByAnimation(const QStyleOptionSlider *scrollBar, QPainter *p) const
{
    QScrollBar *sbar = qobject_cast<QScrollBar *>(scrollBar->styleObject);
    if (!sbar)
        return false;

    // QScrollBar 可以通过设置属性 _d_slider_always_show 为 true 的方式，使得 Slider 一直显示
    // scrollBarObj 获取的即应用界面上 QScrollBar 控件本身的指针值
    if (sbar->property("_d_dtk_slider_always_show").toBool())
        return false;

    // ScrollBarAlwaysOn 也可以控制一直显示
    QAbstractScrollArea *sa = qobject_cast<QAbstractScrollArea *>(getSbarParentWidget(sbar));
    if (sa) {
        const QScrollBar *hsb = sa->horizontalScrollBar();
        const bool hsbAlwaysOn = sa->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOn;
        if (hsb == sbar && hsbAlwaysOn)
            return false;
        const QScrollBar *vsb = sa->verticalScrollBar();
        const bool vsbAlwaysOn = sa->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOn;
        if (vsb == sbar && vsbAlwaysOn)
            return false;
    }

    // 如果应用没有自行控制则查看是否 dcc 有设置相关 xsettings
    DPlatformTheme *theme = DGuiApplicationHelper::instance()->systemTheme();
    if (theme && theme->isValid()) {
        int sb = theme->scrollBarPolicy();
        switch (sb) {
        case Qt::ScrollBarAlwaysOff:
            return true;
        case Qt::ScrollBarAlwaysOn:
            return false;
        default:
            break;
        }
    }

    auto styleAnimation = qobject_cast<dstyle::DScrollbarStyleAnimation*>(this->animation(sbar));
    if (!styleAnimation) {
        // styleAnimation -> updateTarget --sendEvent--> StyleAnimationUpdate -> repaint
        styleAnimation = new dstyle::DScrollbarStyleAnimation(dstyle::DScrollbarStyleAnimation::Deactivating, sbar);
        styleAnimation->setDeletePolicy(QAbstractAnimation::KeepWhenStopped);

        connect(styleAnimation, &dstyle::DStyleAnimation::destroyed,
                this, &ChameleonStyle::_q_removeAnimation, Qt::UniqueConnection);

        animations.insert(styleAnimation->target(), styleAnimation);

        // 滚动和滚动条大小变化时，重启动画改变显示和隐藏
        QObject::connect(sbar, &QAbstractSlider::valueChanged, styleAnimation, &dstyle::DScrollbarStyleAnimation::restart);
        QObject::connect(sbar, &QAbstractSlider::rangeChanged, styleAnimation, &dstyle::DScrollbarStyleAnimation::restart);
    }

    if (!styleAnimation)
        return false;

    QAbstractAnimation::State st = styleAnimation->state();
    // underMouse ==> State_MouseOver sometimes not work well ?
    // underMouse true but State_MouseOver false...
    bool isHoveredOrPressed = hoveredOrPressed(scrollBar) || sbar->underMouse();
    if (isHoveredOrPressed) {
        // 标记一下，鼠标移开时需要重新开启隐藏动画
        sbar->setProperty("_d_dtk_scrollbar_visible", true);
        return false;
    }

    if (sbar->property("_d_dtk_scrollbar_visible").toBool()) {
        sbar->setProperty("_d_dtk_scrollbar_visible", false);
        styleAnimation->restart(true);
        return false;
    }

    if (st == QAbstractAnimation::Running) {
        p->setOpacity(styleAnimation->currentValue());
    }

    // 动画停止时不再绘制滚动条
    return st == QAbstractAnimation::Stopped;
}

void ChameleonStyle::drawControl(QStyle::ControlElement element, const QStyleOption *opt,
                                 QPainter *p, const QWidget *w) const
{
    switch (element) {
    case CE_RadioButton:
    case CE_CheckBox:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            bool isRadio = (element == CE_RadioButton);
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect(isRadio ? SE_RadioButtonIndicator : SE_CheckBoxIndicator, btn, w);
            proxy()->drawPrimitive(isRadio ? PE_IndicatorRadioButton : PE_IndicatorCheckBox, &subopt, p, w);

            subopt.rect = subElementRect(SE_CheckBoxContents, btn, w);
            proxy()->drawControl(CE_CheckBoxLabel, &subopt, p, w);

            if (btn->state & State_HasFocus) {
                QRect rect(subElementRect(isRadio ? SE_RadioButtonFocusRect : SE_CheckBoxFocusRect, btn, w));

                if (isRadio) {
                    p->setPen(QPen(getColor(opt, DPalette::Highlight), DStyle::pixelMetric(PM_FocusBorderWidth)));
                    p->drawEllipse(rect.adjusted(1, 1, -1, -1));
                } else {
                    DDrawUtils::drawBorder(p, rect, getColor(opt, DPalette::Highlight),
                                           DStyle::pixelMetric(PM_FocusBorderWidth), DStyle::pixelMetric(PM_FocusBorderSpacing) + 2);
                }
            }

        }
        return;
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            int alignment = static_cast<int>(visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter));

            if (!proxy()->styleHint(SH_UnderlineShortcut, btn, w))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix;
            QRect textRect = btn->rect;
            if (!btn->icon.isNull()) {
                auto icon_mode_state = toIconModeState(opt); // 与PushButton一致，转换成icon的mode和state.
            #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                pix = btn->icon.pixmap(btn->iconSize, w ? w->devicePixelRatio() : qApp->devicePixelRatio(), icon_mode_state.first, icon_mode_state.second);
            #else
                pix = btn->icon.pixmap(w ? w->window()->windowHandle() : nullptr, btn->iconSize, icon_mode_state.first, icon_mode_state.second);
            #endif
                proxy()->drawItemPixmap(p, btn->rect, alignment, pix);
                if (btn->direction == Qt::RightToLeft)
                    textRect.setRight(textRect.right() - btn->iconSize.width() - 4);
                else
                    textRect.setLeft(textRect.left() + btn->iconSize.width() + 4);
            }
            if (!btn->text.isEmpty()) {
                proxy()->drawItemText(p, textRect, alignment | Qt::TextShowMnemonic,
                    btn->palette, btn->state & State_Enabled, btn->text, QPalette::WindowText);
            }
        }
        return;
    case CE_ScrollBarSlider: {
        if (const QStyleOptionSlider* scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            // 非特效不需要动画，只有显示和隐藏
            if (!DGuiApplicationHelper::isSpecialEffectsEnvironment()) {
                if (!hoveredOrPressed(scrollBar))
                    return;
            }

            bool hidden = hideScrollBarByAnimation(scrollBar, p);
            if (hidden)
                return;

            p->save();
            p->setBrush(getColor(opt, QPalette::Highlight));
            p->setPen(Qt::NoPen);
            p->setRenderHint(QPainter::Antialiasing);
            QRectF rect = opt->rect;
            int realRadius = 0;
            QPoint scrollBarRectCenter;
            int spacing = DStyle::pixelMetric(DStyle::PM_FocusBorderSpacing);

            //用于判断滚动条是否被圆角区域遮住或特殊要求（滚动条上下不到顶、末端），使用方法是 在对应的控件的滚动条， 设置添加间隔，其中左、右和上、下每一个都是分开的，且没有遮挡长度的限制;
            //若是超出滚动条的总长度，则表现无变化（不作处理）
            //eg： scrollbar->setProperty("_d_slider_leftOrRight_spacing", 100);
            bool okLeft = false;
            bool okRight = false;
            bool okUp = false;
            bool okDown = false;
            int spacLeft = 0;
            int spacRight = 0;
            int spacUp = 0;
            int spacDown = 0;

            if (opt->styleObject->property("_d_slider_spaceLeft").isValid())
                spacLeft = opt->styleObject->property("_d_slider_spaceLeft").toInt(&okLeft);
            if (opt->styleObject->property("_d_slider_spaceRight").isValid())
                spacRight = opt->styleObject->property("_d_slider_spaceRight").toInt(&okRight);
            if (opt->styleObject->property("_d_slider_spaceUp").isValid())
                spacUp = opt->styleObject->property("_d_slider_spaceUp").toInt(&okUp);
            if (opt->styleObject->property("_d_slider_spaceDown").isValid())
                spacDown = opt->styleObject->property("_d_slider_spaceDown").toInt(&okDown);

            if (opt->state & QStyle::State_Horizontal) {
                rect.setHeight(rect.height() / 2);
                if ((okLeft && spacLeft > 0) || (okRight && spacRight > 0)) {
                    if ((2 * spacing + spacLeft + spacRight) < rect.width()) {
                        rect = rect.adjusted(spacing + spacLeft, 0, -spacing - spacRight, 0);
                    }
                } else {
                    rect = rect.adjusted(spacing, 0, -spacing, 0);
                }

                if (!(opt->state & QStyle::State_MouseOver))
                    rect.setHeight(rect.height() - 2);

                realRadius = rect.height() / 2.0;

                scrollBarRectCenter.setX(scrollBar->rect.x() + scrollBar->rect.width() / 2);
                scrollBarRectCenter.setY((scrollBar->rect.y() + scrollBar->rect.height()) / 2);
                rect.moveCenter(scrollBarRectCenter);
                rect.moveBottom(scrollBar->rect.bottom() - 2);
            } else {
                rect.setWidth(rect.width() / 2);
                if ((okUp && spacUp > 0) || (okDown && spacDown > 0)) {
                    if ((2 * spacing + spacUp + spacDown) < rect.height()) {
                        rect = rect.adjusted(0, spacing + spacUp, 0, -spacing - spacDown);
                    }
                } else {
                    rect = rect.adjusted(0, spacing, 0, -spacing);
                }

                if (!(opt->state & QStyle::State_MouseOver))
                    rect.setWidth(rect.width() - 2);

                realRadius = rect.width() / 2.0;

                scrollBarRectCenter.setX((scrollBar->rect.x() + scrollBar->rect.width()) / 2);
                scrollBarRectCenter.setY(scrollBar->rect.y() + scrollBar->rect.height() / 2);
                rect.moveCenter(scrollBarRectCenter);
                rect.moveRight(scrollBar->rect.right() - 1);
            }

            QColor lineColor(opt->palette.color(QPalette::Base));
            if (DGuiApplicationHelper::toColorType(lineColor) == DGuiApplicationHelper::LightType) {
                // 外侧拓展一个像素的outside border
                p->setPen(QPen(QColor(255, 255, 255, 0.1 * 255), Metrics::Painter_PenWidth));
                p->setBrush(Qt::NoBrush);
                p->drawRoundedRect(rect.adjusted(-1, -1, 1, 1),
                                   realRadius, realRadius);
                // 内侧绘制一个像素的inside border
                p->setPen(QPen(QColor(0, 0, 0, 0.1 * 255), Metrics::Painter_PenWidth));
                p->drawRoundedRect(rect.adjusted(1, 1, -1, -1),
                                   realRadius, realRadius);
                // normal状态
                p->setBrush(QColor(0, 0, 0, 0.5 * 255));

                if (scrollBar->state & QStyle::State_MouseOver)
                    // hover 状态
                    p->setBrush(QColor(0, 0, 0, 0.7 * 255));
                if (scrollBar->state & QStyle::State_Sunken)
                    // active状态
                    p->setBrush(QColor(0, 0, 0, 0.6 * 255));
            } else {
                // 外侧拓展一个像素的outside border
                p->setPen(QPen(QColor(0, 0, 0, 0.1 * 255), Metrics::Painter_PenWidth));
                p->setBrush(Qt::NoBrush);
                p->drawRoundedRect(rect.adjusted(-1, -1, 1, 1),
                                   realRadius, realRadius);
                // 内侧绘制一个像素的inside border
                p->setPen(QPen(QColor(255, 255, 255, 0.1 * 255), Metrics::Painter_PenWidth));
                p->drawRoundedRect(rect.adjusted(1, 1, -1, -1),
                                   realRadius, realRadius);
                // normal状态
                p->setBrush(QColor(255, 255, 255, 0.2 * 255));

                if (scrollBar->state & QStyle::State_MouseOver)
                    // hover 状态
                    p->setBrush(QColor(255, 255, 255, 0.5 * 255));
                if (scrollBar->state & QStyle::State_Sunken)
                    // active状态
                    p->setBrush(QColor(255, 255, 255, 0.4 * 255));
            }
            p->setPen(Qt::NoPen);
            p->drawRoundedRect(rect, realRadius, realRadius);
            p->restore();
        }
        break;
    }
    case CE_MenuBarItem: {
        if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            p->save();
            QRect rect = mbi->rect;
            drawMenuBarItem(mbi, rect, p, w);
            p->restore();
            return;
        }
    }
    break;
    case CE_MenuBarEmptyArea: {
        p->save();
        QRect menubarRect = opt->rect;
        p->setPen(Qt::NoPen);
        p->setBrush(getColor(opt, QPalette::Window));
        p->drawRect(menubarRect);
        p->restore();
        return;
    }
    case CE_MenuItem: {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            p->save();
            drawMenuItem(menuItem, p, w);
            p->restore();
            return;
        }
        break;
    }
    case CE_MenuEmptyArea: {
        drawMenuItemBackground(opt, p, QStyleOptionMenuItem::EmptyArea);
        return;
    }
    case CE_MenuScroller: {
        QStyleOption arrowOpt = *opt;
        arrowOpt.state |= State_Enabled;
        int minSize = qMin(arrowOpt.rect.width(), arrowOpt.rect.height());
        arrowOpt.rect.setWidth(minSize);
        arrowOpt.rect.setHeight(minSize);
        arrowOpt.rect.moveCenter(opt->rect.center());
        proxy()->drawPrimitive(((opt->state & State_DownArrow) ? PE_IndicatorArrowDown : PE_IndicatorArrowUp),
                               &arrowOpt, p, w);
        return;
    }
    case CE_PushButton: {
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            proxy()->drawControl(CE_PushButtonBevel, btn, p, w);
            QStyleOptionButton subopt = *btn;
            subopt.rect -= frameExtentMargins();
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, &subopt, w);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, p, w);

            if (btn->state & State_HasFocus) {
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*btn);
                fropt.rect = proxy()->subElementRect(SE_PushButtonFocusRect, btn, w);
                proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, p, w);
            }

            return;
        }
        break;
    }
    case CE_PushButtonBevel: {
            if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
                QRect br = btn->rect;
                int dbi = proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, w);
                if (btn->features & QStyleOptionButton::DefaultButton)
                    proxy()->drawPrimitive(PE_FrameDefaultButton, opt, p, w);
                if (btn->features & QStyleOptionButton::AutoDefaultButton)
                    br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);
                if (!(btn->features & (QStyleOptionButton::Flat | QStyleOptionButton::CommandLinkButton))
                    || btn->state & (State_Sunken | State_On)
                    || (btn->features & QStyleOptionButton::CommandLinkButton && btn->state & State_MouseOver)) {
                    QStyleOptionButton tmpBtn = *btn;
                    tmpBtn.rect = br;
                    proxy()->drawPrimitive(PE_PanelButtonCommand, &tmpBtn, p, w);
                }
            }
            return;
        }
    case CE_TabBarTabShape: {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (drawTabBar(p, tab, w))
                return;
        }
    }
    break;
    case CE_TabBarTabLabel: {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (drawTabBarLabel(p, tab, w))
                return;
        }
    }
    break;
    case CE_TabBarTab: {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab*>(opt)) {
            QStyleOptionButton btn;
            btn.rect = tab->rect;
            bool type_check = false;
            if (w)
               type_check = w->property("_d_dtk_tabbartab_type").toBool();

            int leftMarge = 0;
            int rightMarge = 0;
            if (!type_check) {
                leftMarge = TabBar_TabMargin / 2;
                rightMarge = TabBar_TabMargin / 2;
            }
            if (verticalTabs(tab->shape)) {
                btn.rect.adjust(0, leftMarge, 0, -(rightMarge));
            } else {
                btn.rect.adjust(leftMarge, 0, -(rightMarge), 0);
            }

            btn.state = tab->state;

            if (tab->state & QStyle::State_Selected) {
                btn.state |= QStyle::State_On;
            }

            if (type_check) {
                QColor inactive;
                if (btn.state & State_On) {
                    inactive = getColor(opt, QPalette::ToolTipBase);

                    if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::DarkType) {
                        inactive = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::ToolTipBase), 0, 0, 0, -10, -10, -10, 0);
                    }

                    p->setBrush(inactive);
                } else {
                    // 初始化 tabbar 的背景色
                    if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::LightType
                            || (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
                                && DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType)) {
                        inactive = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Light), 0, 0, 0, +20, +20, +20, 0);
                    } else if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::DarkType
                               || (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType
                                   && DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType)) {
                        inactive = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Light), 0, 0, 0, -57, -57, -57, 0);
                    } else {
                        inactive = DGuiApplicationHelper::adjustColor(getColor(opt, QPalette::Light), 0, 0, 0, +20, +20, +20, 0);
                    }

                    p->setBrush(inactive);
                    // Bug:33899 此处添加update出现重复触发绘图事件 导致cpu占用过高 目前注释未发现问题
                    // const_cast<QWidget *>(w)->update();
                }

                p->setPen(Qt::NoPen);
                p->setRenderHint(QPainter::Antialiasing);
                p->drawRect(opt->rect);

                // 绘制边框线
                if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::LightType
                        || (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType
                            && DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)) {
                    p->setPen(QPen(getColor(opt, DPalette::FrameBorder, w), Metrics::Painter_PenWidth));
                } else if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::DarkType
                           || (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::UnknownType
                               && DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)){
                    p->setPen(QPen(QColor(0, 0, 0, static_cast<int>(0.05 * 255)), Metrics::Painter_PenWidth));
                } else {
                    p->setPen(QPen(getColor(opt, DPalette::FrameBorder, w), Metrics::Painter_PenWidth));
                }


                p->setBrush(Qt::NoBrush);
                p->drawRect(opt->rect);
                    //对中间的tabbar尾后加一根明显的线
                if (QStyleOptionTab::End != tab->position && QStyleOptionTab::OnlyOneTab != tab->position) {
                    const QRect &lintRect = opt->rect;
                    if (verticalTabs(tab->shape)) {
                        p->drawLine(lintRect.bottomLeft(), lintRect.bottomRight());
                    } else {
                        p->drawLine(lintRect.topRight(), lintRect.bottomRight());
                    }
                }
            } else {
                DStyle::drawControl(CE_PushButtonBevel, &btn, p, w);
            }

            QStyleOptionTab* newTab = const_cast<QStyleOptionTab *>(tab);
            newTab->rect = btn.rect;
            proxy()->drawControl(CE_TabBarTabLabel, newTab, p, w);
            return;
        }
        break;
    }
    case CE_RubberBand: {
        if (qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
            p->save();
            QColor color = opt->palette.highlight().color();
            color.setAlphaF(0.1);

            // draw rectangle
            p->setRenderHint(QPainter::Antialiasing, false);
            p->fillRect(opt->rect, color);

            // draw inner border
            // 保证border绘制在矩形内部，且不超越了矩形范围
            color.setAlphaF(0.2);
            p->setClipRegion(QRegion(opt->rect) - opt->rect.adjusted(1, 1, -1, -1));
            p->fillRect(opt->rect, color);
            p->restore();
            return;
        }
        break;
    }
    case CE_Header:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            QRegion clipRegion = p->clipRegion();
            p->setClipRect(opt->rect);
            proxy()->drawControl(CE_HeaderSection, header, p, w);
            QStyleOptionHeader subopt = *header;
            subopt.rect = subElementRect(SE_HeaderLabel, header, w);
            if (subopt.rect.isValid())
                proxy()->drawControl(CE_HeaderLabel, &subopt, p, w);
            if (header->sortIndicator != QStyleOptionHeader::None) {
                subopt.rect = subElementRect(SE_HeaderArrow, opt, w);
                proxy()->drawPrimitive(PE_IndicatorHeaderArrow, &subopt, p, w);
            }
            p->setClipRegion(clipRegion);
        }
        return;
    case CE_ShapedFrame: {
        if (const QStyleOptionFrame *f = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            int frameShape  = f->frameShape;
            int frameShadow = QFrame::Plain;
            if (f->state & QStyle::State_Sunken) {
                frameShadow = QFrame::Sunken;
            } else if (f->state & QStyle::State_Raised) {
                frameShadow = QFrame::Raised;
            }

            int lw = f->lineWidth;
            int mlw = f->midLineWidth;
            QPalette::ColorRole foregroundRole = QPalette::WindowText;
            if (w)
                foregroundRole = w->foregroundRole();

            switch (frameShape) {
            case QFrame::Box:
                if (qobject_cast<QComboBoxPrivateContainer *>(const_cast<QWidget *>(w)) && DGuiApplicationHelper::isTabletEnvironment())
                    break;

                if (frameShadow == QFrame::Plain) {
                    qDrawPlainRect(p, f->rect, f->palette.color(foregroundRole), lw);
                } else {
                    qDrawShadeRect(p, f->rect, f->palette, frameShadow == QFrame::Sunken, lw, mlw);
                }
                break;
            case QFrame::StyledPanel:
                //keep the compatibility with Qt 4.4 if there is a proxy style.
                //be sure to call drawPrimitive(QStyle::PE_Frame) on the proxy style
                if (w) {
                    w->style()->drawPrimitive(QStyle::PE_Frame, opt, p, w);
                } else {
                    proxy()->drawPrimitive(QStyle::PE_Frame, opt, p, w);
                }
                break;
            case QFrame::Panel:
                if (frameShadow == QFrame::Plain) {
                    qDrawPlainRect(p, f->rect, f->palette.color(foregroundRole), lw);
                } else {
                    qDrawShadePanel(p, f->rect, f->palette, frameShadow == QFrame::Sunken, lw);
                }
                break;
            case QFrame::WinPanel:
                if (frameShadow == QFrame::Plain) {
                    qDrawPlainRect(p, f->rect, f->palette.color(foregroundRole), lw);
                } else {
                    qDrawWinPanel(p, f->rect, f->palette, frameShadow == QFrame::Sunken);
                }
                break;
            case QFrame::HLine:
            case QFrame::VLine: {
                QPoint p1, p2;
                if (frameShape == QFrame::HLine) {
                    p1 = QPoint(opt->rect.x(), opt->rect.y() + opt->rect.height() / 2);
                    p2 = QPoint(opt->rect.x() + opt->rect.width(), p1.y());
                } else {
                    p1 = QPoint(opt->rect.x() + opt->rect.width() / 2, opt->rect.y());
                    p2 = QPoint(p1.x(), p1.y() + opt->rect.height());
                }
                if (frameShadow == QFrame::Plain) {
                    QPen oldPen = p->pen();
                    QColor color = opt->palette.color(foregroundRole);
                    color.setAlphaF(0.1);
                    p->setPen(QPen(color, lw));
                    p->drawLine(p1, p2);
                    p->setPen(oldPen);
                } else {
                    qDrawShadeLine(p, p1, p2, f->palette, frameShadow == QFrame::Sunken, lw, mlw);
                }
                break;
            }
            }
            return;
        }
        break;
    }
    case CE_ComboBoxLabel: {
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (drawComboBoxLabel(p, cb, w))
                return;
        }
        break;
    }
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QRect textRect = button->rect;
            uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (!proxy()->styleHint(SH_UnderlineShortcut, button, w))
                tf |= Qt::TextHideMnemonic;

            const QPalette::ColorRole &text_color_role = opt->state & State_On ? QPalette::HighlightedText : QPalette::ButtonText;

            QPalette pa = button->palette;

            if (button->features & DStyleOptionButton::WarningButton) {
                pa.setBrush(QPalette::ButtonText, getColor(opt, DPalette::TextWarning, w));
            } else if (button->features & DStyleOptionButton::SuggestButton) {
                pa.setBrush(QPalette::ButtonText, getColor(opt, QPalette::HighlightedText));
            } else {
                pa.setBrush(QPalette::ButtonText, getColor(opt, text_color_role));
            }

            // 设置文字和图标的绘制颜色
            p->setPen(QPen(pa.buttonText(), 1));

            const DStyleOptionButton  *dciButton = qstyleoption_cast<const DStyleOptionButton *>(opt);
            bool hasDciIcon = (dciButton->features & DStyleOptionButton::HasDciIcon);
            QRect iconRect;
            if (hasDciIcon || !button->icon.isNull()) {
                int pixmapWidth = button->iconSize.width();
                int pixmapHeight = button->iconSize.height();
                int labelWidth = pixmapWidth;
                int labelHeight = pixmapHeight;
                int iconSpacing = 4;//### 4 is currently hardcoded in QPushButton::sizeHint()
                int textWidth = button->fontMetrics.boundingRect(opt->rect, tf, button->text).width();
                if (!button->text.isEmpty())
                    labelWidth += (textWidth + iconSpacing);

                iconRect = QRect(textRect.x() + (textRect.width() - labelWidth) / 2,
                                 textRect.y() + (textRect.height() - labelHeight) / 2,
                                 pixmapWidth, pixmapHeight);

                iconRect = visualRect(button->direction, textRect, iconRect);

                tf |= Qt::AlignLeft; //left align, we adjust the text-rect instead

                if (button->direction == Qt::RightToLeft)
                    textRect.setRight(iconRect.left() - iconSpacing);
                else
                    textRect.setLeft(iconRect.left() + iconRect.width() + iconSpacing);

                if (button->state & (State_On | State_Sunken))
                    iconRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, w),
                                       proxy()->pixelMetric(PM_ButtonShiftVertical, opt, w));
            }
            if (hasDciIcon) {
                auto dciTheme = (DGuiApplicationHelper::toColorType(opt->palette)
                                        == DGuiApplicationHelper::LightType) ? DDciIcon::Light : DDciIcon::Dark;
                auto icon_mode_state = toDciIconMode(opt);
                const DDciIconPalette &iconPalette = makeIconPalette(opt->palette);
                dciButton->dciIcon.paint(p, iconRect, p->device() ? p->device()->devicePixelRatioF()
                                                               : qApp->devicePixelRatio(), dciTheme,
                                            icon_mode_state, Qt::AlignCenter, iconPalette);
            } else if (!button->icon.isNull()) {
                //Center both icon and text
                auto icon_mode_state = toIconModeState(opt);
                button->icon.paint(p, iconRect, Qt::AlignCenter, icon_mode_state.first, icon_mode_state.second);
            } else {
                tf |= Qt::AlignHCenter;
            }
            if (button->state & (State_On | State_Sunken))
                textRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, w),
                                   proxy()->pixelMetric(PM_ButtonShiftVertical, opt, w));

            if (button->features & QStyleOptionButton::HasMenu) {
                QRect rectArrowAndLine = drawButtonDownArrow(opt, nullptr, w);
                int frameRadius = DStyle::pixelMetric(PM_FrameRadius);
                drawButtonDownArrow(button, p, w);

                if (button->direction == Qt::LeftToRight) {
                    textRect.setRight(rectArrowAndLine.left() - frameRadius);
                } else {
                    textRect.setLeft(rectArrowAndLine.right() + frameRadius);
                }
            }

            proxy()->drawItemText(p, textRect, tf, pa, (button->state & State_Enabled),
                                  button->text, QPalette::ButtonText);
            return;
        }
        break;
    case CE_ProgressBar: {  //显示进度区域
        if (const QStyleOptionProgressBar *progBar =  qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(Qt::NoPen);
            p->drawRect(opt->rect);

            QStyleOptionProgressBar progGroove = *progBar;
            proxy()->drawControl(CE_ProgressBarGroove, &progGroove, p, w);

            QRect rect = progBar->rect;   //滑块区域矩形
            int min = progBar->minimum;
            int max = progBar->maximum;
            int val = progBar->progress;
            int drawWidth = 0;
            bool horizontal = progBar->state & State_Horizontal;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            horizontal |= progBar->orientation == Qt::Horizontal;
#endif
            if (horizontal) {
                drawWidth = (val * 1.0 / (max - min)) * rect.width();
                rect = QRect(rect.left(), rect.top(), drawWidth, rect.height());
            } else {
                drawWidth = (val * 1.0 / (max - min)) * rect.height();
                rect = QRect(rect.left(), rect.bottom() - drawWidth, rect.width(), drawWidth);
            }

            p->setPen(Qt::NoPen);
            QStyleOptionProgressBar subopt = *progBar;
            proxy()->drawControl(CE_ProgressBarContents, &subopt, p, w);

            if (progBar->textVisible && horizontal) {
                subopt.rect = proxy()->subElementRect(SE_ProgressBarLabel, progBar, w);
                proxy()->drawControl(CE_ProgressBarLabel, &subopt, p, w);
            }
        }
        return;
    }
    case CE_ProgressBarGroove: {  //滑槽显示
        if (const QStyleOptionProgressBar *progBar = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            int frameRadius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
            bool horizontal = progBar->state & State_Horizontal;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            horizontal |= progBar->orientation == Qt::Horizontal;
#endif
            int height = horizontal ? opt->rect.height() : opt->rect.width();
            if (frameRadius * 2 >= height) {
                frameRadius = qMin(height / 2, 4);
            }
            p->setBrush(getColor(opt, DPalette::ObviousBackground, w));
            p->drawRoundedRect(opt->rect, frameRadius, frameRadius);
        }
        return;
    }
    case CE_ProgressBarContents: { //进度滑块显示
        if (const QStyleOptionProgressBar *progBar =  qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            QStyleOptionProgressBar* progBarTemp = const_cast<QStyleOptionProgressBar *>(progBar);
            progBarTemp->state &= (~State_MouseOver);
            progBarTemp = nullptr;
            QRect rect = progBar->rect;   //滑块区域矩形
            int min = progBar->minimum;
            int max = progBar->maximum;
            int val = progBar->progress;
            int drawWidth = 0;
            int frameRadius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
            bool horizontal = progBar->state & State_Horizontal;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            horizontal |= progBar->orientation == Qt::Horizontal;
#endif
            int height = horizontal ? rect.height() : rect.width();
            if (frameRadius * 2 >= height) {
                frameRadius = qMin(height / 2, 4);
            }

            if (horizontal) {
                drawWidth = (val * 1.0 / (max - min)) * rect.width();
                rect = QRect(rect.left(), rect.top(), drawWidth, rect.height());
            } else {
                drawWidth = (val * 1.0 / (max - min)) * rect.height();
                rect = QRect(rect.left(), rect.bottom() - drawWidth, rect.width(), drawWidth);
            }

            QPointF pointStart(rect.left(), rect.center().y());
            QPointF pointEnd(rect.right(), rect.center().y());
            QLinearGradient linear(pointStart, pointEnd);
            QColor startColor = getColor(opt, DPalette::Highlight);
            QColor endColor = DGuiApplicationHelper::adjustColor(startColor, 0, 0, +30, 0, 0, 0, 0);
            linear.setColorAt(0, startColor);
            linear.setColorAt(1, endColor);
            linear.setSpread(QGradient::PadSpread);
            p->setBrush(QBrush(linear));

            if (progBar->textVisible) {
                QPainterPath pathRect;
                pathRect.addRect(rect);
                QPainterPath pathRoundRect;
                pathRoundRect.addRoundedRect(opt->rect, frameRadius, frameRadius);
                QPainterPath inter = pathRoundRect.intersected(pathRect);
                p->drawPath(inter);
            } else {
                //进度条高度 <= 8px && 进度条宽度 <= 8px && value有效
                if (rect.height() <= ProgressBar_MinimumStyleHeight &&
                        rect.width() <= ProgressBar_MinimumStyleHeight && progBar->progress > 0) {
                    QPainterPath path;
                    QRect startRect = rect;
                    startRect.setWidth(rect.height());
                    startRect.setHeight(rect.height());
                    path.moveTo(rect.x() + startRect.width() / 2.0, rect.y());
                    //绘制进度条最小样式前半圆
                    path.arcTo(startRect, 90, 180);
                    p->drawPath(path);

                    //绘制进度条最小样式后半圆
                    if (rect.width() > startRect.width() / 2) {
                        QRect endRect = startRect;
                        int width = rect.width() - startRect.width() / 2;
                        endRect.setX(startRect.x() + startRect.width() / 2 - width);
                        endRect.setWidth(width * 2);

                        QPainterPath path2;
                        path2.moveTo(endRect.x() + endRect.width() / 2.0, rect.y());
                        path2.arcTo(endRect, 90, -180);
                        p->drawPath(path2);
                    }
                } else
                    p->drawRoundedRect(rect, frameRadius, frameRadius);
            }
        }
        return;
    }
    case CE_ProgressBarLabel: {
        if (const QStyleOptionProgressBar *progBar =  qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            double val = progBar->progress * 1.0 / (progBar->maximum - progBar->minimum);
            int radius = DStyle::pixelMetric(PM_FrameRadius);
            int drawWidth = val * (opt->rect.width() + 2 * radius);

            QRect rect = progBar->fontMetrics.boundingRect(progBar->rect, progBar->textAlignment, progBar->text);

            if (rect.left() <= drawWidth && drawWidth <= rect.right()) {
                double division = (drawWidth - rect.left()) / (rect.width() * 1.0);
                QPointF pointStart(rect.left(), rect.center().y());
                QPointF pointEnd(rect.right(), rect.center().y());
                QLinearGradient linear(pointStart, pointEnd);
                linear.setColorAt(0, getColor(opt, DPalette::HighlightedText));
                linear.setColorAt(division, getColor(opt, DPalette::HighlightedText));
                linear.setColorAt(division + 0.01, getColor(opt, DPalette::ButtonText));
                linear.setColorAt(1, getColor(opt, DPalette::ButtonText));
                linear.setSpread(QGradient::PadSpread);

                p->setPen(QPen(QBrush(linear), 1));      //设置画笔渐变色
            } else if (drawWidth < rect.left()) {
                p->setPen(getColor(opt, DPalette::ButtonText));
            } else {
                p->setPen(getColor(opt, DPalette::HighlightedText));
            }

            p->drawText(progBar->rect, progBar->textAlignment, progBar->text);
        }
        return;
    }
    case CE_ToolButtonLabel: {
        if (const QStyleOptionToolButton *toolbutton = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QRect rect = toolbutton->rect;
            int toolButtonAlign = Qt::AlignLeft;
            if (w)
                toolButtonAlign = w->property("_d_dtk_toolButtonAlign").toInt(); // 设置tool button的对齐方式
            int radius = DStyle::pixelMetric(PM_FrameRadius, opt, w); //在绘画icon和text之前,先绘画一层表示靠近或按下状态
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(Qt::NoPen);
            p->setBrush(Qt::NoBrush);
            int menuButtonIndicatorMargin = 4;

            if (toolbutton->state & (State_MouseOver | State_Sunken))  //hover状态 、press状态
                p->setBrush(getBrush(toolbutton, DPalette::Button));

            // 繪制背景
            if (toolbutton->state & State_Enabled) {
                if ((toolbutton->features & QStyleOptionToolButton::MenuButtonPopup) &&
                    (toolbutton->state & (QStyle::State_MouseOver | QStyle::State_Sunken))) {

                    // 绘制外层背景色

                    auto btn = *toolbutton;
                    if (btn.state & (QStyle::State_MouseOver))
                        btn.state &= ~ QStyle::State_MouseOver;
                    if (btn.state & (QStyle::State_Sunken))
                        btn.state &= ~ QStyle::State_Sunken;
                    p->setPen(getColor(&btn, DPalette::Button));
                    p->setBrush(getBrush(&btn, DPalette::Button));
                    QRect tmp = rect;
                    tmp.adjust(0, 0, proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w) + 2 * menuButtonIndicatorMargin, 0);
                    p->drawRoundedRect(tmp, radius, radius);

                    // 绘制子控件背景色
                    p->setPen(getColor(toolbutton, DPalette::Button));
                    p->setBrush(getBrush(toolbutton, DPalette::Button));
                    if (toolbutton->activeSubControls & QStyle::SC_ToolButton) {
                        DDrawUtils::drawRoundedRect(p, rect, radius, radius,
                                                    DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
                    } else if (toolbutton->activeSubControls & QStyle::SC_ToolButtonMenu) {
                        QRect r = rect;
                        r.adjust(r.width(), 0, proxy()->pixelMetric(PM_MenuButtonIndicator , toolbutton, w) + 2  * menuButtonIndicatorMargin, 0);
                        DDrawUtils::drawRoundedRect(p, r, radius, radius,
                                                    DDrawUtils::TopRightCorner | DDrawUtils::BottomRightCorner);
                    }
                } else {
                    p->drawRoundedRect(rect, radius, radius);
                }
            }

            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if (((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty())
                || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {               //只显示文字的情景
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, opt, w))
                    alignment |= Qt::TextHideMnemonic;
                p->setFont(toolbutton->font);
                if (toolbutton->state & State_On) {
                    p->setPen(getColor(toolbutton, DPalette::Highlight));
                } else {
                    p->setPen(getColor(toolbutton, DPalette::ButtonText));
                }

                // 只显示文字且为PopupDelay模式且有菜单则文字和箭头整体居中显示（日历中的选择月份和年份的toolButton也遵循这个规则）
                if ((toolbutton->features & QStyleOptionToolButton::HasMenu && toolbutton->features & QStyleOptionToolButton::PopupDelay && toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) 
                    || w->property("_d_calendarToolBtn").toBool()) {
                    QFontMetrics metrics(toolbutton->font);
                    int fontWidth = metrics.horizontalAdvance(toolbutton->text);
                    int indicatorWidth = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w);
                    int subRectWidth = fontWidth  + indicatorWidth + menuButtonIndicatorMargin;
                    QRect subRect = QRect(rect.left() + (rect.width() - subRectWidth) / 2, rect.top(), subRectWidth, rect.height());
                    QRect textRect = QRect(subRect.topLeft(), QSize(fontWidth, rect.height()));
                    p->drawText(textRect, alignment, toolbutton->text);
                } else {
                    p->drawText(rect, alignment, toolbutton->text);
                }
            } else { //只显示文字的情景 的 补集
                QIcon icon;
                QSize pmSize = toolbutton->iconSize;

                switch (toolbutton->arrowType) {
                case Qt::UpArrow:
                    icon = DStyle::standardIcon(SP_ArrowUp);
                    break;
                case Qt::DownArrow:
                    icon = DStyle::standardIcon(SP_ArrowDown);
                    break;
                case Qt::LeftArrow:
                    icon = DStyle::standardIcon(SP_ArrowLeft);
                    break;
                case Qt::RightArrow:
                    icon = DStyle::standardIcon(SP_ArrowRight);
                    break;
                default:
                    icon = toolbutton->icon;
                    break;
                }

                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(Qt::NoPen);
                p->setBrush(Qt::NoBrush);

                if (toolbutton->state & (State_MouseOver | State_Sunken))   //hover状态 、press状态
                    p->setBrush(getBrush(toolbutton, DPalette::Button));

                if (toolbutton->state & State_HasFocus)
                    p->setPen(getColor(toolbutton, DPalette::Highlight));

                //强制绘制　日历　左右翻页背景
                if (w && (w->objectName() == "qt_calendar_prevmonth" || w->objectName() == "qt_calendar_nextmonth")) {
                    p->setBrush(getColor(toolbutton, DPalette::Button));
                }

                // pr为图标的大小
                QRect pr = rect;
                // tr为文字的大小
                QRect tr = rect;
                pr.setHeight(pmSize.height());
                pr.setWidth(pmSize.width());

                if (toolbutton->state & State_On)      //active状态
                    p->setPen(getColor(toolbutton, DPalette::Highlight));
                else
                    p->setPen(getColor(toolbutton, DPalette::ButtonText));

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) { //只显示icon 的补集情况
                    p->setFont(toolbutton->font);

                    int alignment = Qt::TextShowMnemonic;
                    if (!proxy()->styleHint(SH_UnderlineShortcut, opt, w))
                        alignment |= Qt::TextHideMnemonic;

                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {  //文字在图标下面
                        pr.moveCenter(QPoint(rect.center().x(), rect.center().y() / 2));
                        tr.adjust(0, pr.height(), 0, 0);

                        drawIcon(toolbutton, p, pr, icon);
                        alignment |= Qt::AlignCenter;

                    } else if (toolbutton->toolButtonStyle == Qt::ToolButtonTextBesideIcon) {
                        if (toolButtonAlign == Qt::AlignCenter) {   //toolButton居中对齐
                            //计算文字宽度
                            int textWidget = w->fontMetrics().horizontalAdvance(toolbutton->text);
                            //图标 spacing 文字的矩形
                            QRect textIcon = QRect(0, 0, pr.width() + ToolButton_MarginWidth + textWidget, rect.height());
                            textIcon.moveCenter(rect.center());
                            pr.moveCenter(rect.center());
                            //图标padding
                            pr.moveLeft(textIcon.x() > ToolButton_ItemSpacing ?
                                        textIcon.x() : ToolButton_ItemSpacing );
                            //调整text距离
                            tr.adjust(pr.width() + ToolButton_AlignCenterPadding, 0, 0, 0);
                            drawIcon(toolbutton, p, pr, icon);
                            alignment |= Qt::AlignCenter;
                        } else if (toolButtonAlign == Qt::AlignRight) { //toolButton右对齐
                            int textWidget = w->fontMetrics().horizontalAdvance(toolbutton->text);
                            pr.moveCenter(rect.center());
                            pr.moveRight(tr.width() - textWidget - ToolButton_AlignLeftPadding - ToolButton_ItemSpacing);
                            tr.adjust(-ToolButton_AlignLeftPadding - pr.width() - ToolButton_MarginWidth, 0,
                                      -ToolButton_AlignLeftMargin, 0);
                            drawIcon(toolbutton, p, pr, icon);
                            alignment |= Qt::AlignVCenter | Qt::AlignRight;
                        } else {   //toolButton左对齐
                            pr.moveCenter(rect.center());
                            pr.moveLeft(ToolButton_AlignRightPadding);
                            tr.adjust(ToolButton_AlignRightPadding + pr.width() + ToolButton_MarginWidth, 0,
                                      -ToolButton_ItemSpacing, 0);
                            drawIcon(toolbutton, p, pr, icon);
                            alignment |= Qt::AlignVCenter | Qt::AlignLeft;
                        }
                    } else {    //其他几种（文字和icon布局）方式
                        int radius = DStyle::pixelMetric(PM_FrameRadius);
                        pr.moveCenter(QPoint(rect.left(), rect.center().y()));
                        pr.moveLeft(radius);
                        tr.adjust(pr.width() + radius, 0, 0, 0);

                        drawIcon(toolbutton, p, pr, icon);
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }

                    p->drawText(tr, alignment, toolbutton->text);
                } else {   //只显示icon情况
                    if (toolbutton->features & QStyleOptionToolButton::HasMenu &&
                        !(toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)) {
                        rect.adjust(0, 0, - (proxy()->pixelMetric(PM_MenuButtonIndicator , toolbutton, w) + 4), 0);
                    }
                    pr.moveCenter(rect.center());
                    drawIcon(toolbutton, p, pr, icon);
                }
            }
        }
        return;
    }
    case CE_HeaderSection: {
        const auto headerOption( qstyleoption_cast<const QStyleOptionHeader*>( opt ) );
        if (!headerOption) return;
        const bool horizontal( headerOption->orientation == Qt::Horizontal );
        const bool isLast( headerOption->position == QStyleOptionHeader::End );

        // fill background
        QColor color(opt->palette.color(QPalette::Base));
        QColor lineColor(opt->palette.color(QPalette::Button));           // 挑选一个比较接近的基准色 Button，基于此做微调

        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = DGuiApplicationHelper::adjustColor(color, 0, 0, 0, 0, 0, 0, 60);
            lineColor = DGuiApplicationHelper::adjustColor(lineColor, 0, 0, 0, 0, 0, 0, 10);
        } else {
            color = DGuiApplicationHelper::adjustColor(color, 0, 0, -20, 0, 0, 0, 80);
            lineColor = DGuiApplicationHelper::adjustColor(lineColor, 0, 0, -25, 0, 0, 0, 0);
        }

        p->fillRect(opt->rect, QBrush(color));
        p->setPen(lineColor);
        if (horizontal) {
            if (!isLast) {
                QPoint unit(0, opt->rect.height() / 5);
                p->drawLine(opt->rect.topRight() + unit, opt->rect.bottomRight() - unit);
            }
            p->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
        } else {
            if (!isLast) {
                p->drawLine(opt->rect.bottomLeft(), opt->rect.bottomRight());
            }
            p->drawLine(opt->rect.topRight(), opt->rect.bottomRight());
        }
        return;
    }
    case CE_SizeGrip: {
        p->save();
        int x, y, w, h;
        opt->rect.getRect(&x, &y, &w, &h);

        int sw = qMin(h, w);
        if (h > w)
            p->translate(0, h - w);
        else
            p->translate(w - h, 0);

        Qt::Corner corner;
        if (const QStyleOptionSizeGrip *sgOpt = qstyleoption_cast<const QStyleOptionSizeGrip *>(opt))
            corner = sgOpt->corner;
        else if (opt->direction == Qt::RightToLeft)
            corner = Qt::BottomLeftCorner;
        else
            corner = Qt::BottomRightCorner;

        bool ok = false;
        int radius = DStyle::pixelMetric(PM_FrameRadius, opt) / 2;
        int _d_radius = 0;

        if (opt->styleObject->property("_d_radius").isValid())
            _d_radius = opt->styleObject->property("_d_radius").toInt(&ok);

        if (ok && _d_radius >= 0 && _d_radius != radius)
            radius = _d_radius;

        p->setRenderHint(QPainter::Antialiasing, true);
        DGuiApplicationHelper *guiAppHelp = DGuiApplicationHelper::instance();
        if (guiAppHelp->themeType() == DGuiApplicationHelper::ColorType::DarkType) {  //暗色主题
            p->setPen(QPen(QColor(255, 255, 255, 0.2 * 255), 1));
        } else {
            p->setPen(QPen(QColor(0, 0, 0, 0.2 * 255), 1));
        }

        QRectF rectInner(0, 0, 1.4 * sw, 1.4 * sw);  // 内侧弧线的外切正方形
        QRectF rectExternal(0, 0, 2 * sw, 2 * sw); // 外侧弧线的外切正方形

        if (corner == Qt::BottomLeftCorner) {
            rectExternal.moveBottomLeft(QPointF(opt->rect.bottomLeft().x() + radius, opt->rect.bottomLeft().y() - radius));
            rectInner.moveCenter(rectExternal.center());
            p->drawArc(rectInner, 205 * 16, 40 * 16);
            p->drawArc(rectExternal, 200 * 16, 50 * 16);
        } else if (corner == Qt::BottomRightCorner) {
            rectExternal.moveBottomRight(QPointF(opt->rect.bottomRight().x() - radius, opt->rect.bottomRight().y() - radius));
            rectInner.moveCenter(rectExternal.center());
            p->drawArc(rectInner, 295 * 16, 40 * 16);
            p->drawArc(rectExternal, 290 * 16, 50 * 16);
        } else if (corner == Qt::TopRightCorner) {
            rectExternal.moveTopRight(QPointF(opt->rect.topRight().x() - radius, opt->rect.topRight().y() + radius));
            rectInner.moveCenter(rectExternal.center());
            p->drawArc(rectInner, 25 * 16, 40 * 16);
            p->drawArc(rectExternal, 25 * 16, 50 * 16);
        } else if (corner == Qt::TopLeftCorner) {
            rectExternal.moveTopLeft(QPointF(opt->rect.topLeft().x() + radius, opt->rect.topLeft().y() + radius));
            rectInner.moveCenter(rectExternal.center());
            p->drawArc(rectInner, 115 * 16, 40 * 16);
            p->drawArc(rectExternal, 110 * 16, 50 * 16);
        }
        p->restore();
        return;
    }
    case CE_ItemViewItem: {
        if (w && w->objectName() == "qt_calendar_calendarview") {
            if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
                p->save();
                p->setClipRect(opt->rect);

                //绘制禁用项
                if (!(vopt->state & QStyle::State_Enabled)) {
                    p->save();
                    p->setPen(Qt::NoPen);
                    p->setBrush(getColor(vopt, DPalette::Window));
                    p->drawRect(vopt->rect);
                    p->restore();
                }

                // 绘制当前选中项
                proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, w);

                // draw the text
                if (!vopt->text.isEmpty()) {
                    QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
                                          ? QPalette::Normal : QPalette::Disabled;
                    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                        cg = QPalette::Inactive;

                    if (vopt->state & QStyle::State_Selected) {
                        p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
                    } else {
                        p->setPen(vopt->palette.color(cg, QPalette::Text));
                    }

                    QCommonStylePrivate *d = reinterpret_cast<QCommonStylePrivate *>(qGetPtrHelper(d_ptr));
                    d->viewItemDrawText(p, vopt, opt->rect);
                }

                //绘制日历分割线
                if (vopt->index.row() == 0) {
                    p->save();
                    QColor color = getColor(vopt, DPalette::FrameBorder, w);
                    color.setAlpha(static_cast<int>(255 * 0.05));
                    QPen pen(color, 2);
                    p->setPen(pen);
                    const QTableView *view = static_cast<const QTableView *>(w);

                    int margins = DStyle::pixelMetric(proxy(), DStyle::PM_ContentsMargins);

                    if (vopt->index.column() == 0)
                        p->drawLine(vopt->rect.bottomLeft() + QPoint(margins, 0), vopt->rect.bottomRight());
                    else if (vopt->index.column() == view->model()->columnCount() - 1) {
                        p->drawLine(vopt->rect.bottomLeft(), vopt->rect.bottomRight() - QPoint(margins, 0));
                    } else {
                        p->drawLine(vopt->rect.bottomLeft(), vopt->rect.bottomRight());
                    }

                    p->restore();
                }

                // draw the focus rect
                 if (vopt->state & QStyle::State_HasFocus) {
                    QStyleOptionFocusRect o;
                    o.QStyleOption::operator=(*vopt);
                    o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vopt, w);
                    o.state |= QStyle::State_KeyboardFocusChange;
                    o.state |= QStyle::State_Item;
                    QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
                                  ? QPalette::Normal : QPalette::Disabled;
                    o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
                                                 ? QPalette::Highlight : QPalette::Window);
                    proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, w);
                }

                 p->restore();
            }
            return;
        }
        break;
    }
    default:
        break;
    }

    DStyle::drawControl(element, opt, p, w);
}

bool ChameleonStyle::drawTabBar(QPainter *painter,  const QStyleOptionTab *tab, const QWidget *widget) const
{
    const QTabBar *m_tabbar = qobject_cast<const QTabBar *>(widget);

    if (!m_tabbar)
        return false;

    painter->fillRect(tab->rect, getColor(tab, QPalette::Window));
    painter->save();

    bool isTriangularMode = false;
    bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                       && (tab->shape == QTabBar::RoundedNorth
                           || tab->shape == QTabBar::RoundedSouth));
    bool selected = tab->state & State_Selected && tab->state & State_Enabled;
    bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                    || (rtlHorTabs
                        && tab->position == QStyleOptionTab::Beginning));
    bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
    int tabOverlap = proxy()->pixelMetric(PM_TabBarTabOverlap, tab, widget);
    QRect rect = tab->rect.adjusted(0, 0, (onlyOne || lastTab) ? 0 : tabOverlap, 0);

    QRect r2(rect);
    int x1 = r2.left();
    int x2 = r2.right();
    int y1 = r2.top();
    int y2 = r2.bottom();

    QTransform rotMatrix;
    bool flip = false;
    painter->setPen(getColor(tab, DPalette::Shadow));

    switch (tab->shape) {
    case QTabBar::TriangularNorth:
        rect.adjust(0, 0, 0, -tabOverlap);
        isTriangularMode = true;
        break;
    case QTabBar::TriangularSouth:
        rect.adjust(0, tabOverlap, 0, 0);
        isTriangularMode = true;
        break;
    case QTabBar::TriangularEast:
        rect.adjust(tabOverlap, 0, 0, 0);
        isTriangularMode = true;
        break;
    case QTabBar::TriangularWest:
        rect.adjust(0, 0, -tabOverlap, 0);
        isTriangularMode = true;
        break;
    case QTabBar::RoundedNorth:
        break;
    case QTabBar::RoundedSouth:
        rotMatrix.rotate(180);
        rotMatrix.translate(0, -rect.height() + 1);
        rotMatrix.scale(-1, 1);
        painter->setTransform(rotMatrix, true);
        break;
    case QTabBar::RoundedWest:
        rotMatrix.rotate(180 + 90);
        rotMatrix.scale(-1, 1);
        flip = true;
        painter->setTransform(rotMatrix, true);
        break;
    case QTabBar::RoundedEast:
        rotMatrix.rotate(90);
        rotMatrix.translate(0, - rect.width() + 1);
        flip = true;
        painter->setTransform(rotMatrix, true);
        break;
    }

    if (flip) {
        QRect tmp = rect;
        rect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
        int temp = x1;
        x1 = y1;
        y1 = temp;
        temp = x2;
        x2 = y2;
        y2 = temp;
    }

    QColor lineColor = !isTriangularMode || selected ? Qt::transparent : getColor(tab, QPalette::Light);
    QColor tabFrameColor = selected ? getColor(tab, QPalette::Window) : getColor(tab, QPalette::Button);

    if (!(tab->features & QStyleOptionTab::HasFrame))
        tabFrameColor = getColor(tab, QPalette::Shadow);

    if (!isTriangularMode)
        tabFrameColor = selected ? getColor(tab, QPalette::Highlight) : getColor(tab, QPalette::Button);

    QPen outlinePen(lineColor, proxy()->pixelMetric(PM_DefaultFrameWidth, tab, widget));
    QRect drawRect = rect;
    painter->setPen(outlinePen);
    painter->setBrush(tabFrameColor);
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (!isTriangularMode) {
        int buttonRadius = DStyle::pixelMetric(PM_FrameRadius, tab, widget);
        int buttonBorder = DStyle::pixelMetric(PM_FrameMargins, tab, widget);

        painter->drawRoundedRect(drawRect.adjusted(buttonBorder, buttonBorder, -buttonBorder, -buttonBorder), buttonRadius, buttonRadius);
    } else {
        painter->drawRect(drawRect);
    }

    painter->restore();
    return true;
}

bool ChameleonStyle::drawTabBarLabel(QPainter *painter, const QStyleOptionTab *tab, const QWidget *widget) const
{
    if (!widget)
        return false;

    bool type_check = false;
    bool selected = tab->state & State_Selected && tab->state & State_Enabled;

    if (widget)
        type_check = widget->property("_d_dtk_tabbartab_type").toBool();

    bool visible_close_button = selected;

    if (visible_close_button) {
        if (const DTabBar *tb = qobject_cast<const DTabBar*>(widget)) {
            visible_close_button = tb->tabsClosable();
        } else {
            visible_close_button = false;
        }
    }

    QStyleOptionTab newTab = *tab;

    if (selected) {
        QPalette::ColorRole role = type_check ? QPalette::Highlight : QPalette::HighlightedText;
        newTab.palette.setBrush(QPalette::WindowText, adjustColor(getColor(tab, role), 0, 0, 0, 0, 0, 0, 50));

        // 拖拽的tab不需要绘制渐变到透明，因为没有关闭按钮。拖拽标签时是生成图片 QPaintDevice 是 QPixMap
        bool is_moving_tab = painter->device()->devType() != QInternal::Widget;
        if (visible_close_button && !is_moving_tab) {
            QRect tr = proxy()->subElementRect(SE_TabBarTabText, tab, widget);
            QRect text_rect;
            if (const DTabBar *tabbar = qobject_cast<const DTabBar*>(widget)) {
                int alignment = tabbar->property("_d_dtk_tabbar_alignment").toInt();
                text_rect = tab->fontMetrics.boundingRect(tr, alignment | Qt::TextShowMnemonic, tab->text);
            } else {
                text_rect = tab->fontMetrics.boundingRect(tr, Qt::AlignCenter | Qt::TextShowMnemonic, tab->text);
            }
            int close_button_width = proxy()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, tab, widget);

            // 防止在平板中错误的对文字渐变
            if (DGuiApplicationHelper::isTabletEnvironment())
                close_button_width = TabBar_TabButtonSize;

            QBrush brush;

            //选中状态下文字颜色
            if (type_check) {
                brush = getColor(tab, QPalette::Highlight);
            } else {
                brush = newTab.palette.windowText();
            }
            // 小心除以 0
            qreal stop = text_rect.width() > 0 ? qreal(tr.right() - close_button_width - text_rect.x() - 5) / text_rect.width() : 1.0;
            if (stop < 1 && stop > 0 &&  tr.right() - close_button_width < text_rect.right()) {
                QLinearGradient lg(0, 0, 1, 0);
                QGradientStops stops;
                qreal offset = 5.0 / text_rect.width();

                // 接近关闭按钮部分的颜色渐变到透明
                stops << QGradientStop{0, brush.color()};
                stops << QGradientStop{qMax(0.0, stop - offset), brush.color()};
                stops << QGradientStop{stop, QColor(brush.color().red(), brush.color().green(), brush.color().blue(), 100)};

                // 保证文字超出最大可显示区域的部分为透明
                if (text_rect.right() > tr.right())
                    stops << QGradientStop{1 - (text_rect.right() - tr.right()) / qreal(text_rect.width()), Qt::transparent};

                stops << QGradientStop{1, Qt::transparent};

                lg.setCoordinateMode(QLinearGradient::ObjectBoundingMode);
                lg.setStops(stops);
                newTab.palette.setBrush(QPalette::WindowText, lg);
            } else {
                newTab.palette.setBrush(QPalette::WindowText, brush);
            }
        }

        // 禁止QCommonStyle中绘制默认的焦点颜色
        newTab.state &= ~QStyle::State_HasFocus;

        if (tab->state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect fropt;
            fropt.QStyleOption::operator=(*tab);
            proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
        }
    } else {
        if (type_check) {
            newTab.palette.setBrush(QPalette::WindowText, QColor("#798190"));
        }

        //靠近边缘的文字渐变
        if (const DTabBar *tab = qobject_cast<const DTabBar*>(widget)) {
            if (!tab->expanding()) {
                QRect tr = proxy()->subElementRect(SE_TabBarTabText, &newTab, widget);
                int alignment = tab->property("_d_dtk_tabbar_alignment").toInt();
                QRect text_rect = newTab.fontMetrics.boundingRect(tr, alignment | Qt::TextShowMnemonic, newTab.text);
                QRect tabbar_rect = widget->findChild<QTabBar *>()->rect();

                bool vertTabs = verticalTabs(newTab.shape);
                int stopx = tabbar_rect.x() + (vertTabs ? tabbar_rect.height() : tabbar_rect.width());
                int tabX = (vertTabs ? newTab.rect.y() : text_rect.x()) + (vertTabs ? tabbar_rect.y() : tabbar_rect.x());
                int tabWidth = tabX + text_rect.width();

                if (text_rect.width() > 0 && tabX < stopx && stopx < tabWidth) {
                    const QBrush &brush = newTab.palette.windowText();
                    QLinearGradient lg(0, 0, 1, 0);
                    QGradientStops stops;
                    qreal stop = static_cast<qreal>(tabWidth - stopx) / text_rect.width();

                    stops << QGradientStop{0, brush.color()};
                    stops << QGradientStop{qMax(0.0, 1 - stop - 0.2), brush.color()};
                    stops << QGradientStop{qMax(0.0, 1 - stop), Qt::transparent};

                    lg.setCoordinateMode(QLinearGradient::ObjectBoundingMode);
                    lg.setStops(stops);
                    newTab.palette.setBrush(QPalette::WindowText, lg);
                }
            }
        }
    }

    if (const DTabBar *tb = qobject_cast<const DTabBar*>(widget)) {
        //Qt源码
        QRect tr = tab->rect;
        bool verticalTabs = tab->shape == QTabBar::RoundedEast
                            || tab->shape == QTabBar::RoundedWest
                            || tab->shape == QTabBar::TriangularEast
                            || tab->shape == QTabBar::TriangularWest;

        int alignment = tb->property("_d_dtk_tabbar_alignment").toInt() | Qt::TextShowMnemonic;
        if (!proxy()->styleHint(SH_UnderlineShortcut, &newTab, widget))
            alignment |= Qt::TextHideMnemonic;

        if (verticalTabs) {
            painter->save();
            int newX, newY, newRot;
            if (tab->shape == QTabBar::RoundedEast || tab->shape == QTabBar::TriangularEast) {
                newX = tr.width() + tr.x();
                newY = tr.y();
                newRot = 90;
            } else {
                newX = tr.x();
                newY = tr.y() + tr.height();
                newRot = -90;
            }
            QTransform m = QTransform::fromTranslate(newX, newY);
            m.rotate(newRot);
            painter->setTransform(m, true);
        }
        QRect iconRect;

        tabLayout(tab, widget, &tr, &iconRect);

        tr = proxy()->subElementRect(SE_TabBarTabText, &newTab, widget);

        if (!tab->icon.isNull()) {
 #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QPixmap tabIcon = tab->icon.pixmap(
                tab->iconSize,
                widget ? widget->devicePixelRatio()
                       : qApp->devicePixelRatio(),
                (tab->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                (tab->state & State_Selected) ? QIcon::On : QIcon::Off);
#else
            QPixmap tabIcon = tab->icon.pixmap(widget ? widget->window()->windowHandle() : 0, tab->iconSize,
                                               (tab->state & State_Enabled) ? QIcon::Normal
                                                                            : QIcon::Disabled,
                                               (tab->state & State_Selected) ? QIcon::On
                                                                             : QIcon::Off);
#endif
            painter->drawPixmap(iconRect, tabIcon);
        }

        proxy()->drawItemText(painter, tr.adjusted(1, 0, 0, 0), alignment, newTab.palette, newTab.state & State_Enabled, newTab.text, QPalette::WindowText);
        if (verticalTabs)
            painter->restore();
    } else {
        QCommonStyle::drawControl(CE_TabBarTabLabel, &newTab, painter, widget);
    }

    return true;
}

void ChameleonStyle::tabLayout(const QStyleOptionTab *opt, const QWidget *widget, QRect *textRect, QRect *iconRect) const
{
    QRect tr = opt->rect;
    bool verticalTabs = opt->shape == QTabBar::RoundedEast
                        || opt->shape == QTabBar::RoundedWest
                        || opt->shape == QTabBar::TriangularEast
                        || opt->shape == QTabBar::TriangularWest;
    if (verticalTabs)
        tr.setRect(0, 0, tr.height(), tr.width()); // 0, 0 as we will have a translate transform

    int verticalShift = proxy()->pixelMetric(QStyle::PM_TabBarTabShiftVertical, opt, widget);
    int horizontalShift = proxy()->pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, opt, widget);
    int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, opt, widget) / 2;
    int vpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabVSpace, opt, widget) / 2;
    if (opt->shape == QTabBar::RoundedSouth || opt->shape == QTabBar::TriangularSouth)
        verticalShift = -verticalShift;
    tr.adjust(hpadding, verticalShift - vpadding, horizontalShift - hpadding, vpadding);
    bool selected = opt->state & QStyle::State_Selected;
    if (selected) {
        tr.setTop(tr.top() - verticalShift);
        tr.setRight(tr.right() - horizontalShift);
    }

    // left widget
    if (!opt->leftButtonSize.isEmpty()) {
        tr.setLeft(tr.left() + 4 +
            (verticalTabs ? opt->leftButtonSize.height() : opt->leftButtonSize.width()));
    }
    // right widget
    if (!opt->rightButtonSize.isEmpty()) {
        tr.setRight(tr.right() - 4 -
            (verticalTabs ? opt->rightButtonSize.height() : opt->rightButtonSize.width()));
    }

    // icon
    if (!opt->icon.isNull()) {
        QSize iconSize = opt->iconSize;
        if (!iconSize.isValid()) {
            int iconExtent = proxy()->pixelMetric(QStyle::PM_SmallIconSize);
            iconSize = QSize(iconExtent, iconExtent);
        }
        QSize tabIconSize = opt->icon.actualSize(iconSize,
                        (opt->state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                        (opt->state & QStyle::State_Selected) ? QIcon::On : QIcon::Off);
        // High-dpi icons do not need adjustment; make sure tabIconSize is not larger than iconSize
        tabIconSize = QSize(qMin(tabIconSize.width(), iconSize.width()), qMin(tabIconSize.height(), iconSize.height()));

        // 由于Qt的历史原因 center 返回的坐标总是比矩形的真是中心坐标少一个像素 导致绘制图标时竖直方向无法对齐
        *iconRect = QRect(tr.left(), tr.center().y() - tabIconSize.height() / 2 + 1,
                    tabIconSize.width(), tabIconSize.height());
        if (!verticalTabs)
            *iconRect = proxy()->visualRect(opt->direction, opt->rect, *iconRect);
        tr.setLeft(tr.left() + tabIconSize.width() + 4);
    }

    if (!verticalTabs)
        tr = proxy()->visualRect(opt->direction, opt->rect, tr);

    *textRect = tr;
}

bool ChameleonStyle::drawTableViewItem(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    if(!qobject_cast<const QTableView *>(w))
        return false;

    const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
    if (!vopt)
        return false;

    int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
    QRect select_rect = opt->rect;

    // 设置item的背景颜色
    p->setPen(Qt::NoPen);
    if (vopt->state & QStyle::State_Selected) {
        if (!vopt->showDecorationSelected) {
            select_rect = proxy()->subElementRect(QStyle::SE_ItemViewItemText,  opt, w);
        } else {
            select_rect -= frameExtentMargins();
        }
        p->setBrush(getColor(opt, QPalette::Highlight));
    } else {
        p->setBrush(vopt->backgroundBrush);
    }

    // 绘制背景，选中的item圆角由属性来控制
    if (w->property("_d_dtk_enable_tableviewitem_radius").toBool()) {
        p->setRenderHint(QPainter::Antialiasing);
        p->drawRoundedRect(select_rect, frame_radius, frame_radius);
    } else {
        // 所有的item都是非圆角
        const_cast<QStyleOptionViewItem *>(vopt)->palette.setBrush(vopt->palette.currentColorGroup(),
                                                                   QPalette::Highlight,
                                                                   getColor(opt, QPalette::Highlight));
        DStyle::drawPrimitive(pe, vopt, p, w);
    }

    return true;
}

bool ChameleonStyle::drawTabBarCloseButton(QPainter *painter, const QStyleOption *tab, const QWidget *widget) const
{
    if (!widget)
        return false;
    const QTabBar *tb = qobject_cast<QTabBar *>(widget->parent());

    if (!tb) {
        if (const QWidget *w = dynamic_cast<const QWidget *>(painter->device()))
            widget = w;

        tb = qobject_cast<QTabBar *>(widget->parent());
    }

    if (Q_UNLIKELY(!tb))
        return false;

    int index = -1;

    for (int i = 0; i < tb->count(); ++i) {

        if (Q_LIKELY(tb->tabButton(i, QTabBar::LeftSide) != widget
                     && tb->tabButton(i, QTabBar::RightSide) != widget)) {
            continue;
        }

        index = i;
        break;
    }

    if (Q_UNLIKELY(index < 0))
        return true;

    QStyleOptionButton buttonOpt;
    buttonOpt.rect = tab->rect;
    buttonOpt.state = tab->state;
    QColor iconColor = getColor(&buttonOpt, QPalette::ButtonText);
    painter->setPen(QPen(iconColor, 2));
    painter->setBrush(getColor(&buttonOpt, QPalette::Button));
    DDrawUtils::drawCloseButton(painter, buttonOpt.rect);

    return true;
}

bool ChameleonStyle::drawTabBarScrollButton(QPainter *painter, const QStyleOption *opt, const QWidget *buttonWidget) const
{
    const QToolButton *tabButton = qobject_cast<const QToolButton *>(buttonWidget);

    if (!tabButton)
        return false;

    if (tabButton->arrowType() == Qt::NoArrow || !tabButton->icon().isNull())
        return false;

    bool isTriangularMode = false;
    if (QTabBar *tabBar = qobject_cast<QTabBar *>(buttonWidget->parent())) {
        switch (tabBar->shape()) {
        case QTabBar::TriangularNorth:
        case QTabBar::TriangularSouth:
        case QTabBar::TriangularEast:
        case QTabBar::TriangularWest:
            isTriangularMode = true;
            break;
        default:
            break;
        }
    }



    QStyleOptionToolButton toolButton(*qstyleoption_cast<const QStyleOptionToolButton *>(opt));
    int frameMargin = DStyle::pixelMetric(PM_FrameMargins);
    toolButton.rect -= QMargins(frameMargin, frameMargin, frameMargin, frameMargin);

    QPoint center = toolButton.rect.center();
    qreal sizeRatio = isTriangularMode ? (4.0 / 7.0) : 1.0;
    int minBoundWidth = qMin(toolButton.rect.width(), toolButton.rect.height());
    toolButton.rect.setWidth(qRound(minBoundWidth * sizeRatio));
    toolButton.rect.setHeight(qRound(minBoundWidth * sizeRatio));
    toolButton.rect.moveCenter(center);
    DDrawUtils::Corners corner = static_cast<DDrawUtils::Corners>(0xff);

    int radius = isTriangularMode ? toolButton.rect.width() / 2 :  DStyle::pixelMetric(PM_FrameRadius);
    QLinearGradient lg(QPointF(0, opt->rect.top()),
                       QPointF(0, opt->rect.bottom()));
    lg.setColorAt(0, getColor(opt, QPalette::Light));
    lg.setColorAt(1, getColor(opt, QPalette::Dark));

    painter->setPen(QPen(getColor(opt, DPalette::FrameBorder, buttonWidget), Metrics::Painter_PenWidth));
    painter->setBrush(lg);
    painter->setRenderHint(QPainter::Antialiasing);
    drawShadow(painter, toolButton.rect + frameExtentMargins() * 1.5, getColor(&toolButton, QPalette::Shadow));
    DDrawUtils::drawRoundedRect(painter, toolButton.rect, radius, radius, corner);

    QPoint originCenter = toolButton.rect.center();
    qreal buttonSizeRatio = isTriangularMode ?  3.0 / 4.0 : 1.0 / 2.0;
    toolButton.rect.setWidth(qRound(toolButton.rect.width() * buttonSizeRatio));
    toolButton.rect.setHeight(qRound(toolButton.rect.height() * buttonSizeRatio));
    toolButton.rect.moveCenter(originCenter);
    proxy()->drawControl(CE_ToolButtonLabel, &toolButton, painter, buttonWidget);

    return true;
}

bool ChameleonStyle::drawComboBox(QPainter *painter, const QStyleOptionComboBox *comboBox, const QWidget *widget) const
{
    QRect rect(comboBox->rect);
    QStyleOptionComboBox comboBoxCopy = *comboBox;
    QRect downArrowRect = proxy()->subControlRect(CC_ComboBox, &comboBoxCopy, SC_ComboBoxArrow, widget);

    if (comboBox->frame && comboBox->subControls & SC_ComboBoxFrame) {
        int frameRadius = DStyle::pixelMetric(PM_FrameRadius);
        painter->setPen(Qt::NoPen);
        painter->setRenderHint(QPainter::Antialiasing);

        if (comboBox->editable) {
            QBrush brush = getThemTypeColor(QColor(0, 0, 0, 255 * 0.08),
                                            QColor(255, 255, 255, 255 * 0.15));
            if (widget->testAttribute(Qt::WA_SetPalette)) {
                brush = comboBox->palette.button();
            } else if (const QComboBox *combobox = qobject_cast<const QComboBox *>(widget)) {
                if (auto lineEdit = combobox->lineEdit()) {
                    if (lineEdit->testAttribute(Qt::WA_SetPalette)) {
                        brush = lineEdit->palette().button();
                    } else {
                        brush = getBrush(comboBox, QPalette::Button);
                    }
                }
            }
            painter->setBrush(brush);
        } else {
            painter->setBrush(Qt::transparent);
        }

        DDrawUtils::drawRoundedRect(painter, comboBoxCopy.rect, frameRadius, frameRadius,
                                    DDrawUtils::Corner::TopLeftCorner
                                    | DDrawUtils::Corner::TopRightCorner
                                    | DDrawUtils::Corner::BottomLeftCorner
                                    | DDrawUtils::Corner::BottomRightCorner);
    }


    QStyleOptionButton buttonOption;
    buttonOption.QStyleOption::operator=(*comboBox);
    if (comboBox->editable) {
        buttonOption.rect = rect;
        buttonOption.state = (comboBox->state & (State_Enabled | State_MouseOver | State_HasFocus))
                             | State_KeyboardFocusChange; // Always show hig

        if (comboBox->state & State_Sunken) {
            buttonOption.state |= State_Sunken;
            buttonOption.state &= ~State_MouseOver;
        }

        if (comboBox->direction == Qt::LeftToRight)
            buttonOption.rect.setLeft(downArrowRect.left());
        else
            buttonOption.rect.setRight(downArrowRect.right());

        downArrowRect.moveCenter(buttonOption.rect.center());
        proxy()->drawPrimitive(PE_PanelButtonCommand, &buttonOption, painter, widget);
    } else {
        buttonOption.rect = rect;
        buttonOption.state = comboBox->state & (State_Enabled | State_MouseOver | State_HasFocus | State_KeyboardFocusChange);

        if (comboBox->state & State_Sunken) {
            buttonOption.state |= State_Sunken;
            buttonOption.state &= ~State_MouseOver;
        }

        downArrowRect.moveCenter(buttonOption.rect.center());

        if (comboBox->direction == Qt::LeftToRight) {
            downArrowRect.moveRight(buttonOption.rect.right());
        } else {
            downArrowRect.moveLeft(buttonOption.rect.left());
        }

        proxy()->drawPrimitive(PE_PanelButtonCommand, &buttonOption, painter, widget);
    }

    if (comboBox->subControls & SC_ComboBoxArrow) {
        QStyleOption arrowOpt = *comboBox;
        arrowOpt.rect =  downArrowRect - frameExtentMargins();

        if (comboBox->editable) {
            arrowOpt.rect.setSize(QSize(qRound(buttonOption.rect.width() / 3.0), qRound(buttonOption.rect.height() / 3.0)));
            arrowOpt.rect.moveCenter(buttonOption.rect.center());
        } else {
            QPoint center = arrowOpt.rect.center();
            arrowOpt.rect.setSize(QSize(qRound(arrowOpt.rect.height() / 2.4), qRound(arrowOpt.rect.height() / 2.4)));
            arrowOpt.rect.moveCenter(center);
            int radius = DStyle::pixelMetric(PM_FrameRadius);
            if (comboBox->direction == Qt::RightToLeft) {
                arrowOpt.rect = arrowOpt.rect.adjusted(radius, 0, radius, 0);
            } else {
                arrowOpt.rect = arrowOpt.rect.adjusted(-radius , 0, -radius, 0);
            }
        }

        painter->setPen(getColor(comboBox, DPalette::ButtonText));
        proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, painter, widget);
    }

    if (comboBox->state.testFlag(QStyle::State_HasFocus)) {
        proxy()->drawPrimitive(PE_FrameFocusRect, comboBox, painter, widget);
    }

    return true;
}

bool ChameleonStyle::drawComboBoxLabel(QPainter *painter, const QStyleOptionComboBox *cb, const QWidget *widget) const
{
    const bool hasText(!cb->currentText.isEmpty());
    const bool hasIcon(!cb->currentIcon.isNull());
//    const bool enabled(cb->state & QStyle::State_Enabled);
    const bool sunken(cb->state & (QStyle::State_On | QStyle::State_Sunken));
//    const bool mouseOver(cb->state & QStyle::State_MouseOver);
//    const bool hasFocus(cb->state & QStyle::State_HasFocus);
    const bool flat(!cb->frame);
    const bool editable(cb->editable);

    QRect contentsRect(cb->rect);
    if (sunken && !flat) contentsRect.translate(1, 1);
    contentsRect.adjust(Metrics::Layout_ChildMarginWidth, 0, -Metrics::Layout_ChildMarginWidth - DStyle::pixelMetric(PM_FrameRadius) * 2, 0);
    QSize iconSize;
    if (hasIcon) {
        iconSize = cb->iconSize;
        if (!iconSize.isValid() && widget) {
            const int metric(widget->style()->pixelMetric(QStyle::PM_SmallIconSize, cb, widget));
            iconSize = QSize(metric, metric);
        }
    }

    int textFlags(Qt::AlignVCenter | Qt::AlignLeft);
    const QSize textSize(cb->fontMetrics.size(textFlags, cb->currentText));

    if (styleHint(SH_UnderlineShortcut, cb, widget))
        textFlags |= Qt::TextShowMnemonic;
    else
        textFlags |= Qt::TextHideMnemonic;

    QRect iconRect;
    QRect textRect;
    QRect downArrowRect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxArrow, widget);

    if (hasText && !hasIcon) {
        textRect = contentsRect;
        int leftMargin = Metrics::ComboBox_ContentLeftMargin;
        textRect.adjust(leftMargin, 0, 0, 0);
        textRect.setWidth(textRect.width() - downArrowRect.width());
    }
    else {
        const int contentsWidth(iconSize.width() + textSize.width() + Metrics::Button_ItemSpacing);
        const int contentLeftPadding = flat ? (contentsRect.width() - contentsWidth) / 2 : Metrics::ComboBox_ContentLeftMargin;
        iconRect = QRect(QPoint(contentsRect.left() + contentLeftPadding,
                                contentsRect.top() + (contentsRect.height() - iconSize.height()) / 2), iconSize);
        const int availableTextWidth = contentsRect.width() - contentLeftPadding - iconSize.width() - Metrics::Icon_Margins - downArrowRect.width();
        textRect = QRect(QPoint(iconRect.right() + Metrics::Icon_Margins + 1,
                                contentsRect.top() + (contentsRect.height() - textSize.height()) / 2), QSize(availableTextWidth, textSize.height()));
    }

    // handle right to left
    if (iconRect.isValid()) iconRect = visualRect(cb->direction, contentsRect, iconRect);
    if (textRect.isValid()) textRect = visualRect(cb->direction, contentsRect, textRect);

    // render icon
    if (hasIcon && iconRect.isValid()) {
        // icon state and mode
        cb->currentIcon.paint(painter, iconRect, Qt::AlignLeft);
    }

    // render text
    if (hasText && textRect.isValid() && !editable) {
        painter->setPen(getColor(cb, QPalette::ButtonText));
        QString text = painter->fontMetrics().elidedText(cb->currentText, Qt::ElideRight, textRect.width());
        textRect.setWidth(textRect.width() + downArrowRect.width());
        painter->drawText(textRect, textFlags, text);
    }

    return true;
}

void ChameleonStyle::drawSliderHandle(const QStyleOptionComplex *opt, QRectF& rectHandle, QPainter *p, const QWidget *w) const
{
    if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
        const DSlider *dslider = qobject_cast<const DSlider *>(w);
        QSlider::TickPosition tickPosition = slider->tickPosition;

        if (dslider)
            tickPosition = dslider->tickPosition();

        if (tickPosition == QSlider::NoTicks) {
            p->drawRoundedRect(rectHandle, DStyle::pixelMetric(DStyle::PM_FrameRadius), DStyle::pixelMetric(DStyle::PM_FrameRadius));
        } else {
            qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);
            QRectF rectRoundedPart(0, 0, 0, 0);

            if (slider->orientation == Qt::Horizontal) {
                if (tickPosition == QSlider::TicksAbove) {  //尖角朝上
                    rectRoundedPart = QRectF(rectHandle.left(), rectHandle.bottom() - 2 * radius, rectHandle.width(), 2 * radius);
                    QPointF polygon[5] = { QPointF(rectHandle.left(), rectHandle.bottom() - radius)
                                           , QPointF(rectHandle.left(), rectHandle.top() + radius)
                                           , QPointF(rectHandle.center().x(), rectHandle.top())
                                           , QPointF(rectHandle.right(), rectHandle.top() + radius)
                                           , QPointF(rectHandle.right(), rectHandle.bottom() - radius)};
                    p->drawPolygon(polygon, 5);
                } else { //尖角朝下
                    rectRoundedPart = QRectF(rectHandle.left(), rectHandle.top(), rectHandle.width(), 2 * radius);
                    QPointF polygon[5] = {   QPointF(rectHandle.left(), rectHandle.top() + radius)
                                           , QPointF(rectHandle.left(), rectHandle.bottom() - radius)
                                           , QPointF(rectHandle.center().x(), rectHandle.bottom())
                                           , QPointF(rectHandle.right(), rectHandle.bottom() - radius)
                                           , QPointF(rectHandle.right(), rectHandle.top() + radius)};
                    p->drawPolygon(polygon, 5);
                }
            } else {
                if (tickPosition == QSlider::TicksLeft) {  //尖角朝左
                    rectRoundedPart = QRectF(rectHandle.right() - 2 * radius, rectHandle.top(), 2 * radius, rectHandle.height());
                    QPointF polygon[5] = {   QPointF(rectHandle.right() - radius, rectHandle.top())
                                           , QPointF(rectHandle.left() + radius, rectHandle.top())
                                           , QPointF(rectHandle.left(), rectHandle.center().y())
                                           , QPointF(rectHandle.left() + radius, rectHandle.bottom())
                                           , QPointF(rectHandle.right() - radius, rectHandle.bottom())};
                    p->drawPolygon(polygon, 5);
                } else { //尖角朝右
                    rectRoundedPart = QRectF(rectHandle.left(), rectHandle.top(), 2 * radius, rectHandle.height());
                    QPointF polygon[5] = {   QPointF(rectHandle.left() + radius, rectHandle.top())
                                           , QPointF(rectHandle.right() - radius, rectHandle.top())
                                           , QPointF(rectHandle.right(), rectHandle.center().y())
                                           , QPointF(rectHandle.right() - radius, rectHandle.bottom())
                                           , QPointF(rectHandle.left() + radius, rectHandle.bottom())};
                    p->drawPolygon(polygon, 5);
                }
            }
            p->drawRoundedRect(rectRoundedPart, DStyle::pixelMetric(DStyle::PM_FrameRadius), DStyle::pixelMetric(DStyle::PM_FrameRadius));
        }
    }
}

void ChameleonStyle::drawSliderHandleFocus(const QStyleOptionComplex *opt, QRectF &rectHandle, QPainter *p, const QWidget *w) const
{
    if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
        const DSlider *dslider = qobject_cast<const DSlider *>(w);
        QSlider::TickPosition tickPosition = slider->tickPosition;

        int lineOffset = DStyle::pixelMetric(PM_FocusBorderWidth) / 2;
        int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
        int marginRect = DStyle::pixelMetric(PM_FocusBorderSpacing) + lineOffset;

        if (dslider)
            tickPosition = dslider->tickPosition();

        if (tickPosition == QSlider::NoTicks) {
            p->drawRoundedRect(rectHandle.adjusted(-marginRect, -marginRect, marginRect, marginRect),
                               DStyle::pixelMetric(DStyle::PM_FrameRadius) + marginRect,
                               DStyle::pixelMetric(DStyle::PM_FrameRadius) + marginRect);
        } else {
            qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);
            QPainterPath focusPath;

            if (slider->orientation == Qt::Horizontal) {
                if (tickPosition == QSlider::TicksAbove) {  //尖角朝上
                    focusPath.moveTo(QPointF(rectHandle.left() - marginRect, rectHandle.bottom() - radius));
                    focusPath.lineTo(QPointF(rectHandle.left() - marginRect, rectHandle.top() + radius - lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.center().x(), rectHandle.top() - margin));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.top() + radius - lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.bottom() - radius));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius -radius - marginRect, rectHandle.bottom() - radius - radius -marginRect, 2 *(radius + marginRect), 2 * (radius + marginRect)),
                                    -0, -90);
                    focusPath.lineTo(QPointF(rectHandle.left() + radius, rectHandle.bottom() + marginRect));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect, rectHandle.bottom() - radius -radius -marginRect, 2 * (radius + marginRect), 2 * (radius + marginRect)),
                                    -90, -90);
                } else { //尖角朝下
                    focusPath.moveTo(QPointF(rectHandle.left() - marginRect, rectHandle.top() + radius));
                    focusPath.lineTo(QPointF(rectHandle.left() - marginRect, rectHandle.bottom() - radius + lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.center().x(), rectHandle.bottom() + margin));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.bottom() - radius + lineOffset));
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.top() + radius));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius -radius - marginRect, rectHandle.top() - marginRect, 2 *(radius + marginRect), 2 * (radius + marginRect)),
                                    0, 90);
                    focusPath.lineTo(QPointF(rectHandle.left() + radius, rectHandle.top() - marginRect));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect, rectHandle.top() - marginRect, 2 * (radius + marginRect), 2 * (radius + marginRect)),
                                    90, 90);
                }
            } else {
                if (tickPosition == QSlider::TicksLeft) {  //尖角朝左
                    focusPath.moveTo(QPointF(rectHandle.right() - radius, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.left() + radius - lineOffset, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.left() - margin, rectHandle.center().y()));
                    focusPath.lineTo(QPointF(rectHandle.left() + radius - lineOffset, rectHandle.bottom() + marginRect));
                    focusPath.lineTo(QPointF(rectHandle.right() - radius, rectHandle.bottom() + marginRect));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius - radius - marginRect, rectHandle.bottom() - radius - radius - marginRect, 2 *(radius + marginRect), 2 * (radius + marginRect)),
                                    -90, 90);
                    focusPath.lineTo(QPointF(rectHandle.right() + marginRect, rectHandle.top() + radius));
                    focusPath.arcTo(QRectF(rectHandle.right() - radius - radius - marginRect, rectHandle.top() - marginRect, 2 * (radius + marginRect), 2 * (radius + marginRect)),
                                    0, 90);
                } else { //尖角朝右
                    focusPath.moveTo(QPointF(rectHandle.left() + radius, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.right() - radius + lineOffset, rectHandle.top() - marginRect));
                    focusPath.lineTo(QPointF(rectHandle.right() + margin, rectHandle.center().y()));
                    focusPath.lineTo(QPointF(rectHandle.right() - radius + lineOffset, rectHandle.bottom() + marginRect));
                    focusPath.lineTo(QPointF(rectHandle.left() + radius, rectHandle.bottom() + marginRect));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect, rectHandle.bottom() - radius - radius - marginRect, 2 *(radius + marginRect), 2 * (radius + marginRect)),
                                    -90, -90);
                    focusPath.lineTo(QPointF(rectHandle.left() - marginRect, rectHandle.top() + radius));
                    focusPath.arcTo(QRectF(rectHandle.left() - marginRect, rectHandle.top() - marginRect, 2 * (radius + marginRect), 2 * (radius + marginRect)),
                                    180, -90);
                }
            }
            p->drawPath(focusPath);
        }
    }
}

void ChameleonStyle::drawIcon(const QStyleOption *opt, QPainter *p, QRect& rect, const QIcon& icon, bool checked) const
{
    bool enabled = opt->state & State_Enabled;
    bool selected = opt->state & State_Selected && enabled;
    QIcon::Mode mode = !enabled ? QIcon::Disabled : (selected ? QIcon::Selected : QIcon::Normal);

    if (mode == QIcon::Normal && opt->state & State_On)
        mode = QIcon::Selected;
    icon.paint(p, rect, Qt::AlignCenter, mode, checked ? QIcon::On : QIcon::Off);
}

#ifndef QT_NO_ANIMATION
dstyle::DStyleAnimation *ChameleonStyle::animation(const QObject *target) const
{
    return animations.value(target, nullptr);
}

void ChameleonStyle::startAnimation(dstyle::DStyleAnimation *animation, int delay) const
{
    connect(animation, &dstyle::DStyleAnimation::destroyed,
            this, &ChameleonStyle::_q_removeAnimation, Qt::UniqueConnection);

    animations.insert(animation->target(), animation);

    if (delay > 0) {
        QTimer::singleShot(delay, animation, SLOT(start()));
    } else {
        animation->start();
    }
}

void ChameleonStyle::_q_removeAnimation()
{
    QObject* animation = sender();
    if (animation)
        animations.remove(animation->parent());
}
#endif

bool ChameleonStyle::drawMenuBarItem(const QStyleOptionMenuItem *option, QRect &rect, QPainter *painter, const QWidget *widget) const
{
    bool enabled(option->state & QStyle::State_Enabled);
    bool mouseOver((option->state & QStyle::State_MouseOver) && enabled);
    bool sunken((option->state & QStyle::State_Sunken) && enabled);

    if (mouseOver || sunken) {
        QBrush background(getColor(option, QPalette::Highlight));
        qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(background);
        painter->drawRoundedRect(rect, radius, radius);
    }

    if (option) {
        int alignment = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;

        if (!proxy()->styleHint(SH_UnderlineShortcut, option, widget))
            alignment |= Qt::TextHideMnemonic;

        int iconExtent = proxy()->pixelMetric(PM_SmallIconSize);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPixmap pix = option->icon.pixmap(
            QSize(iconExtent, iconExtent),
            widget ? widget->devicePixelRatio()
                   : qApp->devicePixelRatio(),
            (enabled) ? (mouseOver ? QIcon::Active : QIcon::Normal)
                      : QIcon::Disabled);
#else
        QPixmap pix = option->icon.pixmap(
            widget ? widget->window()->windowHandle() : nullptr,
            QSize(iconExtent, iconExtent),
            (enabled) ? (mouseOver ? QIcon::Active : QIcon::Normal)
                      : QIcon::Disabled);
#endif
        if (!pix.isNull()) {
            proxy()->drawItemPixmap(painter, option->rect, alignment, pix);
        } else {
            QStyleOptionMenuItem itemOption = *option;

            if (mouseOver || sunken)
                itemOption.palette.setBrush(QPalette::ButtonText, itemOption.palette.highlightedText());

            proxy()->drawItemText(painter, itemOption.rect, alignment, itemOption.palette, enabled,
                                  itemOption.text, QPalette::ButtonText);
        }
    }

    return true;
}

void ChameleonStyle::drawMenuItemBackground(const QStyleOption *option, QPainter *painter, QStyleOptionMenuItem::MenuItemType type) const
{
    QBrush color;
    bool selected = (option->state & QStyle::State_Enabled) && option->state & QStyle::State_Selected;
    if (selected && (DGuiApplicationHelper::isTabletEnvironment() ||
                     !DGuiApplicationHelper::isSpecialEffectsEnvironment())) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(getColor(option, QPalette::Highlight));
        painter->drawRect(option->rect);
        return;
     }

    // 清理旧的阴影
    if (option->styleObject) {
        const QRect shadow = option->styleObject->property("_d_menu_shadow_rect").toRect();
        const QRect shadow_base = option->styleObject->property("_d_menu_shadow_base_rect").toRect();

        // 如果当前菜单项时已选中的，并且shadow_base不等于当前区域，此时应当清理阴影区域
        // 如果当前要绘制的item是触发阴影绘制的那一项，那么，此时应当清空阴影区域
        if ((selected && shadow_base != option->rect)
            || (!selected && shadow_base == option->rect)
            || (!selected && shadow_base.width() != option->rect.width())) {
            // 清空阴影区域
            option->styleObject->setProperty("_d_menu_shadow_rect", QVariant());
            option->styleObject->setProperty("_d_menu_shadow_base_rect", QVariant());

            // 确保阴影区域能重绘
            if (QWidget *w = qobject_cast<QWidget*>(option->styleObject)) {
                w->update(shadow);
            }
        }
    }

    if (selected) {
        color = option->palette.highlight();

        // draw shadow
        if (type == QStyleOptionMenuItem::Normal) {
            if (option->styleObject) {
                QRect shadow(0, 0, option->rect.width(), 7);
                shadow.moveTop(option->rect.bottom() + 1);
                option->styleObject->setProperty("_d_menu_shadow_rect", shadow);
                option->styleObject->setProperty("_d_menu_shadow_base_rect", option->rect);

                // 确保阴影区域能重绘
                if (QWidget *w = qobject_cast<QWidget*>(option->styleObject)) {
                    w->update(shadow);
                }
            }
        }

        painter->fillRect(option->rect, color);
    } else {
        color = option->palette.window().color();

        if (color.color().isValid() && color.color().alpha() != 0) {
            QColor c = color.color();

            // 未开启窗口混成时不应该设置背景色的alpha通道，应当显示纯色背景(设置StyleSheet时，不加载此设置，防止alpha通道等对其造成影响)
            if (DWindowManagerHelper::instance()->hasComposite()
                    && qobject_cast<QWidget *>(option->styleObject)
                    && !qobject_cast<QWidget *>(option->styleObject)->testAttribute(Qt::WA_StyleSheet)) {
                if (DGuiApplicationHelper::toColorType(c) == DGuiApplicationHelper::LightType) {
                    c = Qt::white;
                    c.setAlphaF(0.4);
                } else {
                    //在深色背景下,为了处理QComboBox的ListView的背景色和menuItem的背景色不一致的问题,加一个判断
                    if (qobject_cast<QMenu*>(option->styleObject))
                        c = DGuiApplicationHelper::adjustColor(c, 0, 0, -10, 0, 0, 0, 0);
                    c.setAlphaF(0.8);
                }
            }

            color = c;
        }

        painter->fillRect(option->rect, color);

        if (type == QStyleOptionMenuItem::Separator) {
            QColor colorSeparator;
            DGuiApplicationHelper *guiAppHelp = DGuiApplicationHelper::instance();
            if (guiAppHelp->themeType() == DGuiApplicationHelper::ColorType::DarkType)
                colorSeparator = QColor(255, 255, 255, 255 * 0.05);
            else
                colorSeparator = QColor(0, 0, 0, 255 * 0.1);
            painter->fillRect(option->rect, colorSeparator);
        }

        if (!option->styleObject)
            return;

        // 为上一个item绘制阴影
        const QRect shadow = option->styleObject->property("_d_menu_shadow_rect").toRect();

        // 判断阴影rect是否在自己的区域
        if (!option->rect.contains(shadow.center()))
            return;

        static QColor shadow_color;
        static QPixmap shadow_pixmap;

        if (shadow_color != option->palette.color(QPalette::Active, QPalette::Highlight)) {
            shadow_color = option->palette.color(QPalette::Active, QPalette::Highlight);
            QImage image(":/chameleonstyle/menu_shadow.svg");
            QPainter pa(&image);
            pa.setCompositionMode(QPainter::CompositionMode_SourceIn);
            pa.fillRect(image.rect(), shadow_color);
            shadow_pixmap = QPixmap::fromImage(image);
        }

        if (!shadow_pixmap.isNull()) {
            if (QMenu *menu = qobject_cast<QMenu *>(option->styleObject)) {
                if (!menu->geometry().contains(QCursor::pos()))
                    return;
            }
            painter->drawPixmap(shadow, shadow_pixmap);
        }
    }
}

void ChameleonStyle::drawMenuItemRedPoint(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    if (!(option->state & QStyle::State_Enabled))
        return;

    QAction *action = nullptr;
    if (const QMenu *menu = qobject_cast<const QMenu *>(widget)) {
        action = menu->actionAt(option->rect.center());
    }
    if (!action)
        return;

    QObject *obj = action;
    if (option->menuItemType == QStyleOptionMenuItem::SubMenu) {
        obj = action->menu();
    }

    if (!obj->property("_d_menu_item_redpoint").toBool())
        return;

    int h = 6, w = 6; // red point size 6x6
#ifdef ENABLE_RED_POINT_TEXT
    QString text = obj->property("_d_menu_item_info").toString();
    QFont f = option->font;
    f.setPointSize(8);
    if (!text.isEmpty()) {
        QFontMetrics fontMetrics(f);
        h = fontMetrics.height();
        w = fontMetrics.horizontalAdvance(text) + Menu_ItemVTextMargin * 2; // margin left/right 4
    }
#endif
    QPainterPath path;
    QRectF rcf(option->rect);

    rcf.setWidth(w);
    rcf.setHeight(h);
    rcf.moveTop(option->rect.top() + (option->rect.height() - h) / 2.0); // vcenter
    rcf.moveLeft(option->rect.right() - w - 24); // 离右侧24像素

#ifdef ENABLE_RED_POINT_TEXT
    if (!text.isEmpty()) {
        path.addRoundRect(rcf, 50, 99); // 0 is angled corners, 99 is maximum roundedness.
    } else
#endif
    {
        path.addEllipse(rcf);
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillPath(path, QColor("#FF3B30")); // color from ui.ds
    QPen oldpen = painter->pen();
    painter->setPen(QColor("#14000000")); // 外描边，边边颜色为8%的黑 hex(255*8/100)
    painter->drawEllipse(rcf);
    painter->setPen(oldpen);

#ifdef ENABLE_RED_POINT_TEXT
    if (!text.isEmpty()) {
        painter->setPen(Qt::white); // 文字白色
        painter->setFont(f);
        painter->drawText(QPointF(rcf.left() + 4, rcf.bottom() - 4), text.left(3));
    }
#endif
    painter->restore();
}

bool ChameleonStyle::drawMenuItem(const QStyleOptionMenuItem *option, QPainter *painter, const QWidget *widget) const
{
    if (const QStyleOptionMenuItem *menuItem = option) {
        //绘制背景
        QRect menuRect = menuItem->rect;
        bool enabled = menuItem->state & State_Enabled;
        bool selected = menuItem->state & State_Selected && enabled;
        bool checkable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
        bool checked = menuItem->checked;
        bool sunken = menuItem->state & State_Sunken;

        //绘制背景
        drawMenuItemBackground(option, painter, menuItem->menuItemType);

        //绘制分段
        if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
            if (!menuItem->text.isEmpty()) {
                painter->setFont(menuItem->font);
                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::NoBrush);
                proxy()->drawItemText(painter, menuRect
                                      , menuItem->direction == Qt::LeftToRight ? (Qt::AlignLeft | Qt::AlignVCenter) : (Qt::AlignRight | Qt::AlignVCenter)
                                      , menuItem->palette, menuItem->state & State_Enabled, menuItem->text
                                      , QPalette::WindowText);
            }

            return true;
        }

        //绘制选择框
        bool ignoreCheckMark = false;

        int frameRadius = DStyle::pixelMetric(PM_FrameRadius);  //打钩矩形的左侧距离item的左边缘； 也是 打钩矩形的右侧距离 图文内容的左边缘
        int smallIconSize = proxy()->pixelMetric(PM_ButtonIconSize, option, widget);//打钩的宽度
        int realMargins = smallIconSize + 2 * frameRadius;  //左侧固定预留的margin，无论是否能够打钩都要预留

        if (!ignoreCheckMark) {
            /*checkRect = visualRect(menuItem->direction, menuItem->rect, checkRect);*/
            QRect checkRect(menuItem->rect);

            if (checkable) {
                checkRect.setLeft(frameRadius);
                checkRect.setWidth(smallIconSize - 2);
                checkRect.setHeight(smallIconSize - 4);
                checkRect.moveCenter(QPoint(checkRect.left() + smallIconSize / 2, menuItem->rect.center().y()));
                painter->setRenderHint(QPainter::Antialiasing);

                if (selected)
                    painter->setPen(getColor(option, QPalette::HighlightedText));
                else
                    painter->setPen(getColor(option, QPalette::BrightText));

                if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) { //单选框
                    if (checked || sunken) {
                        QIcon markIcon = DStyle::standardIcon(SP_MarkElement, option, widget);
                        markIcon.paint(painter, checkRect);
                    }
                } else if (checked) { //复选框
                    QIcon markIcon = DStyle::standardIcon(SP_MarkElement, option, widget);
                    markIcon.paint(painter, checkRect);
                } else {
                }
            }
        } else { //ignore checkmark //用于combobox

        }

        if (selected) {
            painter->setPen(getColor(option, QPalette::HighlightedText));
        } else {
            if ((option->state & QStyle::State_Enabled)) {
                painter->setPen(getColor(option, QPalette::BrightText));
            } else {
                QColor color = option->palette.color(QPalette::Active, QPalette::BrightText);
                color = DStyle::adjustColor(color, 0, 0, 0, 0, 0, 0, -60);
                painter->setPen(color);
            }
        }

        QSize iconSize(0, 0);
        // 绘制图标
        if (!menuItem->icon.isNull()) {
            /*= visualRect(opt->direction, menuItem->rect,QRect(menuItem->rect.x() + checkColHOffset, menuItem->rect.y(),checkcol, menuitem->rect.height()));*/
            iconSize.setWidth(smallIconSize);
            iconSize.setHeight(smallIconSize);

#if QT_CONFIG(combobox)
            if (const QComboBox *combo = qobject_cast<const QComboBox *>(widget))
                iconSize = combo->iconSize();
#endif
            //计算icon的绘制区域(icon需要居中显示)
            QRect pmr(menuRect.x() + realMargins, menuRect.center().y() - iconSize.height() / 2, iconSize.width(), iconSize.height());
            drawIcon(option, painter, pmr, option->icon, checked);

        }

        // 绘制文本
        int x, y, w, h;
        menuRect.getRect(&x, &y, &w, &h);
        int tab = menuItemShortcutWidth(menuItem);
        int xpos = menuRect.x(); //1.只有文本  2.只有图片加文本  ，xpos为文本的起始坐标

        if (iconSize.width() > 0) {
            xpos += realMargins + frameRadius + iconSize.width();
        } else {
            xpos += realMargins;
        }

        QRect textRect(xpos, y + Menu_ItemHTextMargin, w - xpos - tab, h - 2 * Menu_ItemVTextMargin);
        QRect vTextRect = textRect /*visualRect(option->direction, menuRect, textRect)*/; // 区分左右方向
        QStringView textRef(menuItem->text);

        painter->setBrush(Qt::NoBrush);

        if (!textRef.isEmpty()) {
            int tabIndex = textRef.indexOf(QLatin1Char('\t'));
            int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;

            if (!styleHint(SH_UnderlineShortcut, menuItem, widget))
                text_flags |= Qt::TextHideMnemonic;

            text_flags |= Qt::AlignLeft;

            if (tabIndex >= 0) {
                QPoint vShortcutStartPoint = textRect.topRight();  //快捷键设置显示
                vShortcutStartPoint.setX(vShortcutStartPoint.x() - Menu_PanelRightPadding - realMargins);
                QRect vShortcutRect = QRect(vShortcutStartPoint, QPoint(menuRect.right(), textRect.bottom()));
                /* = visualRect(option->direction,menuRect,QRect(vShortcutStartPoint, QPoint(menuRect.right(), textRect.bottom())))*/;
                const QString textToDraw = textRef.mid(tabIndex + 1).toString();

                painter->drawText(vShortcutRect, text_flags, textToDraw);
                textRef = textRef.left(tabIndex);
            }

            QFont font = menuItem->font;
            font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());
            painter->setFont(font);
            const QString textToDraw = textRef.left(tabIndex).toString();

            painter->setBrush(Qt::NoBrush);
            painter->drawText(vTextRect, text_flags, textToDraw);

            drawMenuItemRedPoint(option, painter, widget);
        }

        // 绘制箭头
        if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
            int dim = (menuRect.height() - 4) / 2;
            int xpos = menuRect.left() + menuRect.width() - 3 - dim;
            QStyleOptionMenuItem newMI = *menuItem;
            xpos += realMargins + iconSize.width() + frameRadius;
            QPoint topLeft(menuItem->rect.right() - frameRadius - smallIconSize / 2, menuItem->rect.center().y() - smallIconSize / 3);  //箭头rect: Size(smallIconSize, smallIconSize*2/3)
            QPoint RightButtom(topLeft.x() + smallIconSize / 2, menuItem->rect.center().y() + smallIconSize / 3 );
            QRect rectArrow(topLeft, RightButtom);
            newMI.rect = rectArrow;

            newMI.state = !enabled ? State_None : State_Enabled;
            if (selected)
                newMI.palette.setColor(QPalette::WindowText,
                                       newMI.palette.highlightedText().color());

            QIcon markIcon = DStyle::standardIcon(SP_ArrowEnter, &newMI, widget);
            markIcon.paint(painter, newMI.rect);
        }
    }

    return true;
}

QRect ChameleonStyle::subElementRect(QStyle::SubElement r, const QStyleOption *opt,
                                     const QWidget *widget) const
{
    switch (r) {
    case SE_HeaderArrow:{
        QRect rect;
        int h = opt->rect.height();
        int w = opt->rect.width();
        int x = opt->rect.x();
        int y = opt->rect.y();
        int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, opt, widget);
        if (opt->state & State_Horizontal) {
            // designer: whatever how big the QHeaderView it is, the arrow size is fixed.
            // size the same as the arrow in combobox. PM_MenuButtonIndicator
            int horiz_size = 12;
            int vert_size = 12;
            rect.setRect(x + w - margin * 2 - horiz_size, y + (h - vert_size) / 2,
                      horiz_size, vert_size);
        } else {
            int horiz_size = 12;
            int vert_size = 12;
            rect.setRect(x + (w - horiz_size) / 2, y + h - margin * 2 - vert_size,
                      horiz_size, vert_size);
        }
        rect = visualRect(opt->direction, opt->rect, rect);
        return rect;
    }
    case SE_PushButtonFocusRect:
    case SE_ItemViewItemFocusRect:
        return opt->rect;
    case SE_ItemViewItemCheckIndicator: Q_FALLTHROUGH();
    case SE_ItemViewItemDecoration: Q_FALLTHROUGH();
    case SE_ItemViewItemText:
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, widget);
            QStyleOptionViewItem option(*vopt);
            option.rect = opt->rect.adjusted(frame_radius, 0, -frame_radius, 0);

            QStyleOptionViewItem::ViewItemFeatures old_features = option.features;

            // 默认把checkbox放置在右边，因此使用QCommonStyle的Item布局时先移除HasCheckIndicator标志
            option.features &= ~QStyleOptionViewItem::HasCheckIndicator;

            if (r == SE_ItemViewItemDecoration) {
                return DStyle::subElementRect(r, &option, widget);
            }

            QRect text_rect = DStyle::subElementRect(SE_ItemViewItemText, &option, widget);
            int indicator_width = proxy()->pixelMetric(PM_IndicatorWidth, &option, widget);
            int indicator_height = proxy()->pixelMetric(PM_IndicatorHeight, &option, widget);

            const QRect indicator_rect = alignedRect(opt->direction, Qt::AlignRight | Qt::AlignVCenter,
                                                     QSize(indicator_width, indicator_height), text_rect);

            if (old_features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
                int margin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, opt, widget);
                text_rect.setRight(qMin(text_rect.right(), indicator_rect.left() - margin));
            }

            return r == SE_ItemViewItemText ? text_rect : indicator_rect;
        }
        break;
    case SE_LineEditContents: {
        int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
        int left_margins = DStyle::pixelMetric(PM_ContentsMargins, opt, widget);
        if (widget && qobject_cast<DSearchEdit *>(widget->parentWidget())) {
            return opt->rect.adjusted(frame_margins / 2, 0, -left_margins / 2, 0);
        }

        return opt->rect.adjusted(frame_margins + left_margins, 0, -(frame_margins + left_margins), 0);
    }
    case SE_RadioButtonFocusRect:
    case SE_CheckBoxFocusRect: {
        QRect re;
        re = subElementRect(SE_CheckBoxIndicator, opt, widget);
        int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
        re.adjust(-margin, -margin, margin, margin);
        return re;
    }
    case SE_RadioButtonClickRect:
    case SE_CheckBoxClickRect: {
        QRect re = DStyle::subElementRect(SE_CheckBoxIndicator, opt, widget);
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            int spacing = proxy()->pixelMetric(PM_CheckBoxLabelSpacing, opt, widget);
            re.setWidth(re.width() + widget->fontMetrics().horizontalAdvance(btn->text) + spacing * 2);
        }
        return re;
    }
    case SE_RadioButtonIndicator:
    case SE_RadioButtonContents:
    case SE_CheckBoxContents:
    case SE_CheckBoxIndicator:
        if (const QStyleOptionButton *vopt = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QStyleOptionButton option(*vopt);
            int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing); //来自SE_CheckBoxFocusRect状态时
            option.rect.translate(margin, 0);   //需往右偏margin数值，FocusRect框显示正常；故对应其size的width也增加margin
            return DStyle::subElementRect(r, &option, widget);
        }
        break;
    case SE_PushButtonContents:
        if (const QStyleOptionButton *vopt = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QRect buttonContentRect = vopt->rect;
            int buttonIconMargin = proxy()->pixelMetric(QStyle::PM_ButtonMargin, opt, widget) ;
            buttonContentRect.adjust(buttonIconMargin, 0, -buttonIconMargin, 0);

            return buttonContentRect;
        }
        break;
    case SE_TabBarScrollLeftButton: {
        const bool vertical = opt->rect.width() < opt->rect.height();
        const int buttonWidth = proxy()->pixelMetric(PM_TabBarScrollButtonWidth, opt, widget);
        QRect buttonRect = vertical ? QRect(0, 0, opt->rect.width(), buttonWidth)
                           : QRect(0, 0, buttonWidth, opt->rect.height());

        return buttonRect;
    }
    case SE_TabBarScrollRightButton: {
        const bool vertical = opt->rect.width() < opt->rect.height();
        const int buttonWidth = proxy()->pixelMetric(PM_TabBarScrollButtonWidth, opt, widget);
        QRect buttonRect = vertical ? QRect(0, opt->rect.height() - buttonWidth, opt->rect.width(), buttonWidth)
                           : QRect(opt->rect.width() - buttonWidth, 0, buttonWidth, opt->rect.height());

        return buttonRect;
    }
    case SE_ProgressBarLabel: {
        int radius = DStyle::pixelMetric(PM_FrameRadius);
        return opt->rect.marginsRemoved(QMargins(radius, radius, radius, radius));
    }
    case SE_TabBarTearIndicatorLeft:
    case SE_TabBarTearIndicatorRight:
        // DTabBar有自己的scroll按钮
        if (widget && qobject_cast<DTabBar*>(widget->parent()))
            return QRect(0, 0, 0, 0);
        break;
    default:
        break;
    }

    return DStyle::subElementRect(r, opt, widget);
}

void ChameleonStyle::drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                        QPainter *p, const QWidget *w) const
{
    switch (cc) {
    case CC_SpinBox: {
        if (const QStyleOptionSpinBox *option = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            if (drawSpinBox(option, p, w))
                return;
        }
        break;
    }
    case CC_ToolButton: {
        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QRect button, menuarea;
            button = proxy()->subControlRect(cc, toolbutton, SC_ToolButton, w);
            menuarea = proxy()->subControlRect(cc, toolbutton, SC_ToolButtonMenu, w);

            State bflags = toolbutton->state & ~State_Sunken;

            if (bflags & State_AutoRaise) {
                if (!(bflags & State_MouseOver) || !(bflags & State_Enabled)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags;
            if (toolbutton->state & State_Sunken) {
                if (toolbutton->activeSubControls & SC_ToolButton)
                    bflags |= State_Sunken;
                mflags |= State_Sunken;
            }


            int menuButtonIndicatorMargin = 4; // 菜单按钮右边距
            QStyleOption tool = *toolbutton;
            if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup) {
                if (w && !w->property("_d_calendarToolBtn").toBool()) {
                    button.adjust(0, 0, -2 * menuButtonIndicatorMargin, 0);
                    menuarea.adjust(-2 * menuButtonIndicatorMargin, 0, 0, 0);
                }
                if (bflags & (State_Sunken | State_On | State_Raised)) {
                    tool.rect = button;
                    tool.state = bflags;
                    //proxy()->drawPrimitive(PE_PanelButtonTool, &tool, p, w);
                }
            }

            QStyleOptionToolButton label = *toolbutton;
            label.state = bflags;
            int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, opt, w);
            label.rect = button.adjusted(fw, fw, -fw, -fw);

            if (w && w->property("_d_calendarToolBtn").toBool()) {
                label.palette = DGuiApplicationHelper::instance()->applicationPalette();
            }
            proxy()->drawControl(CE_ToolButtonLabel, &label, p, w);

            if (toolbutton->state & State_HasFocus) {
                QStyleOptionFocusRect fr;
                fr.QStyleOption::operator=(*toolbutton);
                //fr.rect.adjust(3, 3, -3, -3);
                if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
                    fr.rect.adjust(0, 0, -proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator,
                                                      toolbutton , w) - 2 * menuButtonIndicatorMargin, 0);
                proxy()->drawPrimitive(PE_FrameFocusRect, &fr, p, w);
            }

            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                tool.rect.adjust(menuButtonIndicatorMargin, 0, -menuButtonIndicatorMargin, 0);
                if ((toolbutton->state & (QStyle::State_Sunken)) && (toolbutton->activeSubControls & QStyle::SC_ToolButton)) {
                    p->setPen(Qt::NoPen);
                }
                if((toolbutton->state & (QStyle::State_Sunken)) && toolbutton->activeSubControls & QStyle::SC_ToolButtonMenu) {
                    p->setPen(getColor(toolbutton, DPalette::Highlight));
                }
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &tool, p, w);
            } else if (toolbutton->features & QStyleOptionToolButton::HasMenu) {
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w);
                QRect ir = toolbutton->rect;
                QStyleOptionToolButton newBtn = *toolbutton;
                newBtn.rect = QRect(ir.right() - mbi - menuButtonIndicatorMargin, (ir.height() - mbi) / 2, mbi, mbi);
                newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);

                //仅文字，DelayedPopup 模式，文字和箭头整体居中
                if (toolbutton->features & QStyleOptionToolButton::PopupDelay && toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                    QFontMetrics metrics(toolbutton->font);
                    int fontWidth = metrics.horizontalAdvance(toolbutton->text);
                    int subRectWidth = fontWidth  + mbi + menuButtonIndicatorMargin;
                    QRect subRect = QRect(tool.rect.left() + (tool.rect.width() - subRectWidth) / 2, tool.rect.top(), subRectWidth, tool.rect.height());
                    QRect indicatorRect = QRect(subRect.topLeft() + QPoint(fontWidth  + menuButtonIndicatorMargin, (tool.rect.height() - mbi) / 2), QSize(mbi, mbi));
                    newBtn.rect = indicatorRect;
                }

                proxy()->drawPrimitive(PE_IndicatorArrowDown, &newBtn, p, w);
            }

            //日历　年按钮 特制 文字和箭头整体居中
            if (w && w->objectName() == "qt_calendar_yearbutton") {
                QStyleOptionToolButton newBtn = *toolbutton;
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, w);
                QFontMetrics metrics(toolbutton->font);
                int fontWidth = metrics.horizontalAdvance(toolbutton->text);
                int subRectWidth = fontWidth  + mbi + menuButtonIndicatorMargin;
                QRect subRect = QRect(tool.rect.left() + (tool.rect.width() - subRectWidth) / 2, tool.rect.top(), subRectWidth, tool.rect.height());
                QRect indicatorRect = QRect(subRect.topLeft() + QPoint(fontWidth  + menuButtonIndicatorMargin, (tool.rect.height() - mbi) / 2), QSize(mbi, mbi));
                newBtn.rect = indicatorRect;
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &newBtn, p, w);
            }
        }
        return;
    }
    case CC_Slider : {
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            const DSlider *dslider = qobject_cast<const DSlider *>(w);
            //各个使用的矩形大小和位置
            QRectF rect = opt->rect;                                                                            //Slider控件最大的矩形(包含如下三个)
            QRectF rectHandle = proxy()->subControlRect(CC_Slider, opt, SC_SliderHandle, w);                    //滑块矩形
            QRectF rectSliderTickmarks = proxy()->subControlRect(CC_Slider, opt, SC_SliderTickmarks, w);        //刻度的矩形
            QRect rectGroove = proxy()->subControlRect(CC_Slider, opt, SC_SliderGroove, w);                     //滑槽的矩形
            int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);

//            //测试(保留不删)
//            p->fillRect(rect, Qt::gray);
//            p->fillRect(rectSliderTickmarks, Qt::blue);
//            p->fillRect(rectGroove, Qt::red);
//            p->fillRect(rectHandle, Qt::green);
//            qDebug()<<"---rect:"<<rect<<"  rectHandle:"<<rectHandle<<"   rectSliderTickmarks:"<<rectSliderTickmarks<<"   rectGroove:"<<rectGroove;

            QPen pen;
            //绘画 滑槽(线)
            if (opt->subControls & SC_SliderGroove) {
                pen.setStyle(Qt::CustomDashLine);
                pen.setWidth(4);
                // 默认高亮色滑槽颜色
                pen.setBrush((opt->activeSubControls & SC_SliderHandle) ? getColor(opt, QPalette::Highlight) : opt->palette.highlight());
                pen.setDashOffset(0);
                pen.setDashPattern(QVector<qreal>()<< 0.5 << 0.25);
                pen.setCapStyle(Qt::FlatCap);
                p->setPen(pen);
                p->setRenderHint(QPainter::Antialiasing);

                QColor color = getColor(opt, DPalette::ObviousBackground, w); //绘画的右侧/上侧的滑槽颜色一定是灰

                // 属性启用灰色滑槽
                QVariant prop = dslider ? const_cast<DSlider *>(dslider)->slider()->property("_d_dtk_sldier_across") : QVariant();
                bool hasProperty = prop.isValid();
                // 0. dslider 默认没有设置此属性(设置在d->slider上了...)
                // 1. 设置了属性 true 则灰色滑槽
                // 2. 如果设置了 false 高亮
                // 3. 没有设置属性时，没有刻度的（圆角） slider 默认高亮色，有刻度(尖角) slider 默认灰色
                if (prop.toBool() || (!hasProperty && !isNoticks(slider, p, w))) {
                    pen.setColor(color);
                    p->setPen(pen);
                }

                if (slider->orientation == Qt::Horizontal) {
                    // 绘制最左边到滑块的位置的滑槽
                    qreal rectWidth = rectHandle.width() / 2.0;
                    p->drawLine(QPointF(rectGroove.left() + rectWidth, rectHandle.center().y()), QPointF(rectHandle.center().x(), rectHandle.center().y()));
                    // 绘制滑块到最右的位置的滑槽
                    pen.setColor(color);
                    p->setPen(pen);
                    p->drawLine(QPointF(rectGroove.right() - rectWidth, rectHandle.center().y()), QPointF(rectHandle.center().x(), rectHandle.center().y()));

                } else {
                    // 绘制最上边到滑块的位置的滑槽
                    qreal rectWidth = rectHandle.height() / 2.0;
                    p->drawLine(QPointF(rectGroove.center().x(), rectGroove.bottom() - rectWidth), QPointF(rectGroove.center().x(), rectHandle.center().y()));
                    // 绘制滑块到最下的位置的滑槽
                    pen.setColor(color);
                    p->setPen(pen);
                    p->drawLine(QPointF(rectGroove.center().x(), rectGroove.top() + rectWidth), QPointF(rectGroove.center().x(), rectHandle.center().y()));
                }
            }

            //绘画 滑块
            if (opt->subControls & SC_SliderHandle) {
                pen.setStyle(Qt::SolidLine);
                p->setPen(Qt::NoPen);
                p->setBrush((opt->activeSubControls & SC_SliderHandle) ? getColor(opt, QPalette::Highlight) : opt->palette.highlight());
                drawSliderHandle(opt, rectHandle, p, w);

                // 绘画 滑块焦点
                if (slider->state & State_HasFocus) {
                    pen.setColor(getColor(opt, DPalette::Highlight));
                    pen.setWidth(DStyle::pixelMetric(PM_FocusBorderWidth));
                    p->setPen(pen);
                    p->setBrush(Qt::NoBrush);
                    drawSliderHandleFocus(opt, rectHandle, p, w);
                }
            }

            //绘画 刻度,绘画方式了参考qfusionstyle.cpp
            if ((opt->subControls & SC_SliderTickmarks) && slider->tickInterval) {                                   //需要绘画刻度
                p->setPen(opt->palette.windowText().color());
                int available = proxy()->pixelMetric(PM_SliderSpaceAvailable, slider, w);  //可用空间
                int interval = slider->tickInterval;                                       //标记间隔
//                int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, w);      //标记偏移
//                int ticks = slider->tickPosition;                                          //标记位置

                int v = slider->minimum;
                int len = proxy()->pixelMetric(PM_SliderLength, slider, w);
                while (v <= slider->maximum + 1) {                                          //此处不添加+1的话, 会少绘画一根线
                    const int v_ = qMin(v, slider->maximum);
                    int pos = margin + sliderPositionFromValue(slider->minimum, slider->maximum, v_, available - (2 * margin)) + len / 2;

                    if (slider->orientation == Qt::Horizontal) {
                        if (slider->tickPosition == QSlider::TicksBothSides) {              //两侧都会绘画, 总的矩形-中心滑槽滑块最小公共矩形
                            p->drawLine(pos, rect.top(), pos, rectHandle.top());
                            p->drawLine(pos, rect.bottom(), pos, rectHandle.bottom());
                        } else {
                            p->drawLine(pos, rectSliderTickmarks.top(), pos, rectSliderTickmarks.bottom());
                        }
                    } else {
                        if (slider->tickPosition == QSlider::TicksBothSides) {
                            p->drawLine(rect.left(), pos, rectHandle.left(), pos);
                            p->drawLine(rect.right(), pos, rectHandle.right(), pos);
                        } else {
                            p->drawLine(rectSliderTickmarks.left(), pos, rectSliderTickmarks.right(), pos);
                        }
                    }
                    // in the case where maximum is max int
                    int nextInterval = v + interval;
                    if (nextInterval < v)
                        break;
                    v = nextInterval;
                }
            }

        }
        break;
    }
    case CC_ComboBox: {
        if (const QStyleOptionComboBox *comboBox = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (drawComboBox(p, comboBox, w))
                return;
        }
        break;
    }
    default:
        break;
    }

    DStyle::drawComplexControl(cc, opt, p, w);
}

bool ChameleonStyle::drawSpinBox(const QStyleOptionSpinBox *opt,
                                 QPainter *painter, const QWidget *widget) const
{
    qreal borderRadius = DStyle::pixelMetric(DStyle::PM_FrameRadius);

    if (opt->frame && (opt->subControls & SC_SpinBoxFrame)) {
        QRect frameRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxFrame, widget);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(opt->palette.button());
        painter->drawRoundedRect(frameRect, borderRadius, borderRadius);
    }

    if (opt->subControls & SC_SpinBoxUp) {
        bool upIsActive = opt->activeSubControls == SC_SpinBoxUp;
        bool upIsEnabled = opt->stepEnabled & QAbstractSpinBox::StepUpEnabled && opt->state.testFlag(State_Enabled);
        QRect subRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, widget);
        QStyleOptionButton buttonOpt;
        buttonOpt.rect = subRect;
        buttonOpt.palette = opt->palette;
        updateSpinBoxButtonState(opt, buttonOpt, upIsActive, upIsEnabled);
        bool isPlusMinus = opt->buttonSymbols & QAbstractSpinBox::PlusMinus;

        if (widget && widget->property("_d_dtk_spinBox").toBool()) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(buttonOpt.palette.color(QPalette::Button));
            DDrawUtils::drawRoundedRect(painter, subRect.adjusted(-2, -1, 0, 0), borderRadius, borderRadius,
                                                DDrawUtils::TopRightCorner);

            painter->setPen(getColor(opt, DPalette::FrameBorder, widget));
            painter->drawLine(subRect.x() - 2, subRect.y() - 1, subRect.x() - 2, subRect.y() - 1 + subRect.height());
        } else {
            proxy()->drawControl(CE_PushButton, &buttonOpt, painter, widget);
        }

        if (isPlusMinus) {
            //绘制 "+" 符号
            buttonOpt.rect -= frameExtentMargins();
            QRectF plusRect = proxy()->subElementRect(SE_PushButtonContents, &buttonOpt, widget);
            QRectF cent_rect(0, 0, plusRect.width() / 2, plusRect.width() / 2);
            cent_rect.moveCenter(plusRect.center());

            if (opt->activeSubControls == SC_SpinBoxUp) {
                DDrawUtils::drawPlus(painter, cent_rect, getColor(opt, QPalette::ButtonText), 1);
            } else {
                DDrawUtils::drawPlus(painter, cent_rect,
                                         DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ? QColor("#536076") : QColor("#c5cfe0"), 1);
            }
        } else {
            QStyleOptionButton arrowDrawBtn  = buttonOpt;

            if (widget && widget->property("_d_dtk_spinBox").toBool()) {
                if (opt->activeSubControls == SC_SpinBoxDown) {
                    painter->setPen(buttonOpt.palette.color(QPalette::ButtonText));
                } else {
                    painter->setPen(getColor(opt, QPalette::ButtonText));
                }

                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setWidth(subRect.width() / 2);
                arrowDrawBtn.rect.setHeight(subRect.height() / 1.3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowUp, &arrowDrawBtn, painter, widget);

            } else {
                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setSize(subRect.size() / 3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowUp, &arrowDrawBtn, painter, widget);
            }
        }
    }

    if (opt->subControls & SC_SpinBoxDown) {
        bool downIsActive = opt->activeSubControls == SC_SpinBoxDown;
        bool downIsEnabled = opt->stepEnabled & QAbstractSpinBox::StepDownEnabled && opt->state.testFlag(State_Enabled);
        QRect subRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, widget);
        QStyleOptionButton buttonOpt;
        buttonOpt.rect = subRect;
        buttonOpt.palette = opt->palette;
        updateSpinBoxButtonState(opt, buttonOpt, downIsActive, downIsEnabled);
        bool isPlusMinus = opt->buttonSymbols & QAbstractSpinBox::PlusMinus;

        if (widget && widget->property("_d_dtk_spinBox").toBool()) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(buttonOpt.palette.color(QPalette::Button));
            DDrawUtils::drawRoundedRect(painter, subRect.adjusted(-2, 0, 0, +1), borderRadius, borderRadius,
                                                DDrawUtils::BottomRightCorner);
            painter->setPen(getColor(opt, DPalette::FrameBorder, widget));
            painter->drawLine(subRect.x() - 2, subRect.y(), subRect.x() + subRect.width(), subRect.y());
            painter->drawLine(subRect.x() - 2, subRect.y(), subRect.x() - 2, subRect.y() + subRect.height() + 1);
        } else {
            proxy()->drawControl(CE_PushButton, &buttonOpt, painter, widget);
        }

        if (isPlusMinus) {
            buttonOpt.rect -= frameExtentMargins();
            QRectF subtractRect = proxy()->subElementRect(SE_PushButtonContents, &buttonOpt, widget);
            QRectF cent_rect(0, 0, subtractRect.width() / 2, subtractRect.height() / 2);
            cent_rect.moveCenter(subtractRect.center());

            if (opt->activeSubControls == SC_SpinBoxDown) {
                DDrawUtils::drawSubtract(painter, cent_rect, getColor(opt, QPalette::ButtonText), 1);
            } else {
                DDrawUtils::drawSubtract(painter, cent_rect,
                                         DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ? QColor("#536076") : QColor("#c5cfe0"), 1);
            }
        } else {

            QStyleOptionButton arrowDrawBtn  = buttonOpt;

            if (widget && widget->property("_d_dtk_spinBox").toBool()) {
                if (opt->activeSubControls == SC_SpinBoxDown) {
                    painter->setPen(getColor(opt, QPalette::ButtonText));
                } else {
                    painter->setPen(buttonOpt.palette.color(QPalette::ButtonText));
                }

                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setWidth(subRect.width() / 2);
                arrowDrawBtn.rect.setHeight(subRect.height() / 1.3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowDrawBtn, painter, widget);
            } else {
                //更换了一种绘制箭头方式，使36 *36情况下箭头无瑕疵
                arrowDrawBtn.rect = subRect;
                arrowDrawBtn.rect.setSize(subRect.size() / 3);
                arrowDrawBtn.rect.moveCenter(subRect.center());
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowDrawBtn, painter, widget);
            }
        }
    }

    return true;
}

void ChameleonStyle::updateSpinBoxButtonState(const QStyleOptionSpinBox *opt, QStyleOptionButton &buttonOpt, bool isActive, bool isEnabled) const
{
    State buttonState = opt->state;

    if (!isActive) {
        buttonState &= ~State_MouseOver;
        buttonState &= ~State_Sunken;
        buttonState &= ~State_Active;
        buttonState &= ~State_HasFocus;
    }

    if (!isEnabled) {
        buttonState &= ~State_Enabled;
        buttonState &= ~State_MouseOver;
        buttonState &= ~State_Sunken;
        buttonState &= ~State_On;
    }

    if (buttonState & State_Sunken || buttonState & State_MouseOver || buttonState & State_Active) {
        buttonState &= ~State_HasFocus;
    }

    buttonOpt.state = buttonState;
}

QStyle::SubControl ChameleonStyle::hitTestComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                                         const QPoint &pt, const QWidget *w) const
{
    return DStyle::hitTestComplexControl(cc, opt, pt, w);
}

QRect ChameleonStyle::subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                     QStyle::SubControl sc, const QWidget *w) const
{
    switch (cc) {
    case CC_SpinBox: {
        if (const QStyleOptionSpinBox *option = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {

            switch (sc) {
            case SC_SpinBoxEditField: {
                if (option->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxFrame, w);

                int rightBorder = option->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, opt, w) * 2 : 0;
                int border = w && w->property("_d_dtk_spinBox").toBool() ? 0 : DStyle::pixelMetric(PM_ContentsMargins);
                QRect spinboxRect = option->rect;
                QRect dButtonRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxUp, w);

                spinboxRect.setRight(dButtonRect.left() - rightBorder - frameExtentMargins().left() - border);
                return spinboxRect;
            }
            case SC_SpinBoxUp: {
                if (w && w->property("_d_dtk_spinBox").toBool()) {
                    break;
                }

                if (option->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();

                int rectHeight = option->rect.height();
                int border = w && w->property("_d_dtk_spinBox").toBool() ? 0 : DStyle::pixelMetric(PM_ContentsMargins);
                rectHeight -= (option->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, opt, w) * 2 : 0);

                if (opt->rect.width() < rectHeight * 3)
                    break;

                QRect uButtonRect = proxy()->subControlRect(CC_SpinBox, opt, SC_SpinBoxDown, w);
                QRect buttonRect = option->rect;
                buttonRect.setLeft(uButtonRect.left() - rectHeight - border);
                buttonRect.setRight(uButtonRect.left());
                buttonRect.setSize(QSize(rectHeight, rectHeight));
                return buttonRect;
            }
            case SC_SpinBoxDown: {
                if (w && w->property("_d_dtk_spinBox").toBool()) {
                    break;
                }

                if (option->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return QRect();

                int rectHeight = option->rect.height();
                rectHeight -= (option->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, opt, w) * 2 : 0);

                if (opt->rect.width() < rectHeight * 3)
                    break;

                QRect buttonRect(option->rect.topLeft(), QSize(rectHeight, rectHeight)); //按高度计算
                buttonRect.moveRight(option->rect.right());
                return buttonRect;
            }
            case SC_SpinBoxFrame: {
                return option->rect - frameExtentMargins() ;
            }
            default:
                break;
            }
        }
        break;
    }
    case CC_Slider: {
        if (const QStyleOptionSlider *option = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
            QRectF rect = option->rect;                                                    //Slider控件总的大小矩形
            int slider_size = proxy()->pixelMetric(PM_SliderControlThickness, opt, w);     //滑块的高度
//            int tick_size = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, w);         //刻度的高度
            QRectF slider_handle_rect = rect;                                              //滑块和滑漕的的最小公共矩形 (后面被用作临时且被改变的)


            if (option->orientation == Qt::Horizontal) {
                slider_handle_rect.setHeight(slider_size);
                slider_handle_rect.adjust(margin, 0, -margin, 0);
                if (option->tickPosition == QSlider::TicksAbove) slider_handle_rect.moveBottom(rect.bottom() - margin);
                if (option->tickPosition == QSlider::TicksBelow) slider_handle_rect.moveTop(rect.top() + margin);
                if (option->tickPosition == QSlider::TicksBothSides || option->tickPosition == QSlider::NoTicks)
                    slider_handle_rect.moveCenter(rect.center());
            } else {
                slider_handle_rect.setWidth(slider_size);
                slider_handle_rect.adjust(0, margin, 0, -margin);
                if (option->tickPosition == QSlider::TicksRight) slider_handle_rect.moveLeft(rect.left() + margin);
                if (option->tickPosition == QSlider::TicksLeft)   slider_handle_rect.moveRight(rect.right() - margin);
                if (option->tickPosition == QSlider::TicksBothSides || option->tickPosition == QSlider::NoTicks)
                    slider_handle_rect.moveCenter(rect.center());
            }

            QRectF rectStatic =  slider_handle_rect;   //rectStatic作为 滑块和滑漕的的最小公共矩形(不改变)

            switch (sc) {
            case SC_SliderGroove: {  //滑漕
                qreal groove_size = slider_size / 4.0;
                QRectF groove_rect;

                if (option->orientation == Qt::Horizontal) {
                    groove_rect.setWidth(slider_handle_rect.width());
                    groove_rect.setHeight(groove_size);
                } else {
                    groove_rect.setWidth(groove_size);
                    groove_rect.setHeight(slider_handle_rect.height());
                }

                groove_rect.moveCenter(slider_handle_rect.center());
                return groove_rect.toRect();
            }
            case SC_SliderHandle: {  //滑块
                int sliderPos = 0;
                int len = proxy()->pixelMetric(PM_SliderLength, option, w);
                bool horizontal = option->orientation == Qt::Horizontal;
                sliderPos = sliderPositionFromValue(option->minimum, option->maximum, option->sliderPosition,
                                                    (horizontal ? slider_handle_rect.width() : slider_handle_rect.height()) - len, option->upsideDown);
                if (horizontal) {
                    slider_handle_rect.moveLeft(slider_handle_rect.left() + sliderPos);
                    slider_handle_rect.setWidth(len);
                    slider_handle_rect.moveTop(rectStatic.top());
                } else {
                    slider_handle_rect.moveTop(slider_handle_rect.top() + sliderPos);
                    slider_handle_rect.setHeight(len);
                    slider_handle_rect.moveLeft(rectStatic.left());
                }

                return slider_handle_rect.toRect();
            }
            case SC_SliderTickmarks: {  //刻度的矩形
                if (option->tickPosition & QSlider::NoTicks)
                    return QRect(0, 0, 0, 0);

                QRectF tick_rect = rect;

                if (option->orientation == Qt::Horizontal) {
                    tick_rect.setHeight(rect.height() - slider_handle_rect.height() - ( 2 * margin));
                    tick_rect.adjust(margin, 0, -margin, 0);

                    if (option->tickPosition == QSlider::TicksAbove) {
                        tick_rect.moveTop(rect.top() + margin);
                    } else if (option->tickPosition == QSlider::TicksBelow) {
                        tick_rect.moveBottom(rect.bottom() -margin);
                    }
                } else {
                    tick_rect.setWidth(rect.width() - slider_handle_rect.width() - (2 *margin));
                    tick_rect.adjust(0, margin, 0, -margin);

                    if (option->tickPosition == QSlider::TicksLeft) {
                        tick_rect.moveLeft(rect.left() + margin);
                    } else if (option->tickPosition == QSlider::TicksRight) {
                        tick_rect.moveRight(rect.right() - margin);
                    }
                }

                return tick_rect.toRect();
            }
            default:
                break;
            }
        }
        break;
    }
    case CC_ComboBox: {
        if (qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            DStyleHelper dstyle(proxy());
            int frameMargins = dstyle.pixelMetric(PM_FrameMargins, opt, w);

            switch (sc) {
            case SC_ComboBoxArrow: {
                QRect rect(0, 0, qMax(static_cast<int>(Metrics::ComboBox_ArrowButtonWidth), opt->rect.height()),
                           opt->rect.height()) ;
                int boxHeight = qAbs(rect.height());

                if (w && qobject_cast<const QComboBox *>(w) && !qobject_cast<const QComboBox *>(w)->isEditable())
                    break;
                if (opt->direction == Qt::LeftToRight)
                    rect.moveRight(opt->rect.right());
                else
                    rect.moveLeft(opt->rect.left());

                int buttonRectSize = boxHeight;
                rect.setSize(QSize(buttonRectSize, buttonRectSize));

                return rect;
            }
            case SC_ComboBoxEditField: {
                QRect rect = opt->rect;
                const QRect arrow_rect = proxy()->subControlRect(cc, opt, SC_ComboBoxArrow, w);

                if (opt->direction == Qt::LeftToRight) {
                    rect.setRight(arrow_rect.left());
                    rect.adjust(frameMargins, 0, 0, 0);
                } else {
                    rect.setLeft(arrow_rect.right());
                    rect.adjust(0, 0, -frameMargins, 0);
                }

                return rect;
            }
            case SC_ComboBoxFrame: {
                return opt->rect;
            }
            case SC_ComboBoxListBoxPopup: {
                QSize size = proxy()->sizeFromContents(CT_Menu, opt, opt->rect.size(), w);
                QRect rect = opt->rect;

                if (auto styopt = static_cast<const QStyleOption *>(opt)) {
                    if (auto menuItem = static_cast<const QStyleOptionMenuItem *>(styopt)) {
                        if (menuItem->icon.isNull()) {
                            rect.setWidth(size.width() + Menu_CheckMarkWidth);
                        }
                    }
                }
                return rect;
            }
            default:
                break;
            }
        }
        break;
    }
    default:
        break;
    }
    return DStyle::subControlRect(cc, opt, sc, w);
}

QSize ChameleonStyle::sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt,
                                       const QSize &contentsSize, const QWidget *widget) const
{
    QSize size = DStyle::sizeFromContents(ct, opt, contentsSize, widget);

    switch (ct) {
    case CT_LineEdit: {
        int button_margin = proxy()->pixelMetric(QStyle::PM_ButtonMargin, opt, widget);
        size += QSize(button_margin, button_margin);
        Q_FALLTHROUGH();
    }
    case CT_ComboBox: {
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (cmb->editable) {
                //这是从lineedit设置margin处拿来
                int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
                int left_margins = DStyle::pixelMetric(PM_ContentsMargins, opt, widget);
                size.setWidth(size.width() + frame_margins + left_margins);
            } else {
                size.setWidth(size.width() + opt->fontMetrics.horizontalAdvance("...")); //设置宽度为最小省略号("...")的宽度
            }
        }
        Q_FALLTHROUGH();
    }
    case CT_TabBarTab: {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab*>(opt)) {
            QStyleOptionButton button;
            button.QStyleOption::operator =(*opt);
            button.text = tab->text;
            size = DStyle::sizeFromContents(QStyle::CT_PushButton, &button, tab->fontMetrics.size(0, tab->text), widget);
            int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, widget);
            // 获得Icon引起的增量
            int iconSizeDelta = 0;
            if (!tab->icon.isNull()) {
                 iconSizeDelta += tab->iconSize.width();
                 if (!tab->text.isEmpty())
                     iconSizeDelta += Icon_Margins;
            }
            size.rwidth() +=  2 * frame_radius + proxy()->pixelMetric(PM_TabCloseIndicatorWidth, opt, widget) + TabBar_TabMargin;
            size.rwidth() += iconSizeDelta;
            // TabBar 竖直方向改变其宽高
            if (verticalTabs(tab->shape)) {
               qSwap(size.rwidth(), size.rheight());
            }
        }
        Q_FALLTHROUGH();
    }
    case CT_PushButton: {
        // 只将width加上ButtonMargin，对于height的margin通过frame_margins体现
        size.rheight() -= proxy()->pixelMetric(PM_ButtonMargin, opt, widget);
        int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
        size += QSize(frame_margins * 2, frame_margins * 2);

        if (const QStyleOptionButton *bopt = qstyleoption_cast<const QStyleOptionButton*>(opt)) {
            int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, widget);

            // 为文本添加额外的margin
            if (!bopt->text.isEmpty()) {
                size.rwidth() += 2 * frame_radius;
            }

            if (bopt->features & QStyleOptionButton::HasMenu)
                size.rwidth() += frame_margins;   //qt源码会在带有menu的btn样式中,添加一个箭头矩形的width
        }

        int button_min_size = DStyle::pixelMetric(PM_ButtonMinimizedSize, opt, widget);
        size = size.expandedTo(QSize(button_min_size, button_min_size));
        break;
    }
    case CT_ItemViewItem: {
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            const QMargins &item_margins = qvariant_cast<QMargins>(vopt->index.data(Dtk::MarginsRole));

            if (!item_margins.isNull()) {
                //加上Item自定义的margins
                size = QRect(QPoint(0, 0), size).marginsAdded(item_margins).size();
            }
            size.setWidth(size.width() + opt->fontMetrics.horizontalAdvance("xxx"));

            return size;
        }
        break;
    }
    case CT_Slider: {
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            /*2019-09-19　约定枚举值含义　　　　　中文含义
             * PM_SliderThickness:　　　　　　Slider总的高度　＝　滑块高度＋刻度高度
             * PM_SliderControlThickness:   只是滑块的单独高度
             * PM_SliderLength:             只是滑块的长度
             * PM_SliderTickmarkOffset:     用作slider的刻度线的高度
             * PM_SliderSpaceAvailable      暂时未用到
             */

            int sliderContHeight = proxy()->pixelMetric(PM_SliderControlThickness, opt, widget);
            int tickMarkHeight = proxy()->pixelMetric(PM_SliderTickmarkOffset, opt, widget);
            sliderContHeight += tickMarkHeight;

            if (slider->tickPosition == QSlider::NoTicks) {
                sliderContHeight -= tickMarkHeight;
            } else if (slider->tickPosition == QSlider::TicksBothSides) {
                sliderContHeight += tickMarkHeight;
            } else {
            }

            int margin = DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing);
            if (slider->orientation == Qt::Horizontal){
                size.setHeight(qMax(size.height(), sliderContHeight) + (2 * margin));
                size.setWidth(50);
            } else {
                size.setWidth(qMax(size.width(), sliderContHeight) + (2 * margin));
            }
        }
        break;
    }
    case CT_MenuBarItem: {
        int frame_margins = DStyle::pixelMetric(PM_FrameMargins, opt, widget);
        size += QSize(frame_margins * 2, frame_margins * 2);
        break;
    }
    case CT_MenuItem: {
        if (const QStyleOptionMenuItem *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int m_width = size.width();
            bool hideShortcutText = false;

            if (hideShortcutText) {
                m_width -= menuItemShortcutWidth(menuItem);
                int tabIndex = menuItem->text.indexOf(QLatin1Char('\t'));

                if (tabIndex != -1) {
                    int textWidth = menuItem->fontMetrics.horizontalAdvance(menuItem->text.mid(tabIndex + 1));

                    if (menuItemShortcutWidth(menuItem) == 0)
                        m_width -= textWidth;
                }
            }

            int frameRadius = DStyle::pixelMetric(PM_FrameRadius); //打钩矩形的左侧距离item的左边缘； 也是 打钩矩形的右侧距离 图文内容的左边缘
            int smallIconSize = proxy()->pixelMetric(PM_SmallIconSize, opt, widget); //打钩的宽度
            int realMargins = smallIconSize + 2 * frameRadius; //左侧固定预留的margin，无论是否能够打钩都要预留

            m_width = realMargins;
#ifdef ENABLE_RED_POINT_TEXT
            int redPointWith = opt->fontMetrics.size(Qt::TextSingleLine, QLatin1String("99+")).width();
            m_width += redPointWith;
#endif
            int tabSpacing = MenuItem_TabSpacing;
            if (menuItem->text.contains(QLatin1Char('\t'))) {  //若是项有快捷键，文本内容会以'\t'连接快捷键文本
                if (!hideShortcutText)
                    m_width += tabSpacing;
            } else {
                if (menuItem->menuItemType == QStyleOptionMenuItem::SubMenu) {
                    m_width += 2 * Menu_ArrowHMargin;
                }
            }

            int textWidth = opt->fontMetrics.size(Qt::TextSingleLine, menuItem->text).width();

            if (!menuItem->text.isEmpty())
                m_width += (textWidth + frameRadius);

            if (!menuItem->icon.isNull())
                m_width += (smallIconSize + frameRadius);

            m_width += (smallIconSize + frameRadius);
            size.setWidth(m_width);

            if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
                if (!menuItem->text.isEmpty()) {
                    size.setHeight(menuItem->fontMetrics.height());
                } else {
                    size.setHeight(2);
                }
            } else if (!menuItem->icon.isNull()) {
                if (const QComboBox *combo = qobject_cast<const QComboBox *>(widget)) {
                    size.setHeight(qMax(combo->iconSize().height() + 2, size.height()));
                }
            }
        }

        size.setWidth(qMax(162, size.width()));
        size.setHeight(size.height() + qMax(Menu_ItemVMargin * 2, 0));
        break;
    }
    case CT_ScrollBar: {
        if (size.width() > size.height())
            size.setHeight(ScrollBar_SliderWidth);
        if (size.width() < size.height())
            size.setWidth(ScrollBar_SliderWidth);
        return size;
    }
    case CT_RadioButton:
    case CT_CheckBox: {
        size.rwidth() += 2 * (DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing));
        // fix 当没有文字的时候高度不够绘制焦点圈
        auto button = qobject_cast<const QAbstractButton *>(widget);
        if (button && button->text().isEmpty())
            size.rheight() += 2 * (DStyle::pixelMetric(PM_FocusBorderWidth) + DStyle::pixelMetric(PM_FocusBorderSpacing));
        break;
    }
    case CT_ToolButton: {
        qreal radius = DStyle::pixelMetric(DStyle::PM_FrameRadius);
        if (widget && widget->property("_d_calendarToolBtn").toBool()) {
            return QSize(size.width() + radius, size.height() + radius);
        }
        int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, opt, widget);
        int menuButtonIndicatorWidth = 0;
        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            if (toolbutton->toolButtonStyle == Qt::ToolButtonIconOnly &&
                    !toolbutton->features.testFlag(QStyleOptionToolButton::Arrow) &&
                    !toolbutton->features.testFlag(QStyleOptionToolButton::HasMenu))
                return DSizeModeHelper::element(QSize(24, 24), QSize(36, 36));

            if ((toolbutton->features & QStyleOptionToolButton::HasMenu) &&
                (!(toolbutton->features & QStyleOptionToolButton::MenuButtonPopup))) {
                menuButtonIndicatorWidth = proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator, opt, widget);
            }
        }
        int menuButtonIndicatorMargin = 4;
        return QSize(size.width() + radius + 2 * menuButtonIndicatorMargin + 2 * fw + menuButtonIndicatorWidth,
                     size.height() + radius + 2 * fw);
    }
    case CT_ProgressBar: {
        if (const QStyleOptionProgressBar *pbo = qstyleoption_cast<const QStyleOptionProgressBar*>(opt)) {
            int radius = DStyle::pixelMetric(PM_FrameRadius);

            if (!pbo->textVisible) {
                size.setWidth(qMax(size.width(), 2 * radius));
                size.setHeight(qMax(size.height(), 2 * radius));
            } else {
                QSize text_size = opt->fontMetrics.size(0, pbo->text.isEmpty() ? "%" : pbo->text);
                size.setWidth(qMax(size.width(), 2 * radius + text_size.width()));
                size.setHeight(qMax(size.height(), 2 * radius + text_size.height()));
            }
        }
        break;
    }
    case CT_SpinBox:
        if (auto vopt = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            int spacing = widget && widget->property("_d_dtk_spinBox").toBool() ? 0 : DStyle::pixelMetric(PM_ContentsMargins);
            const int fw = vopt->frame ? proxy()->pixelMetric(PM_SpinBoxFrameWidth, vopt, widget) : 0;
            // 增加左右箭头对应的宽和焦点边框的边距,(正方形箭头宽 + 边距 + 箭头控件左右边距, 焦点边框的宽）.
            size += QSize((size.height() + spacing + fw * 2) * 2, LineEdit_FrameWidth);
            return size;
        }
    break;
    case CT_SizeGrip: {
        size = QSize(16, 16);
        break;
    }
    default:
        break;
    }

    return size;
}

int ChameleonStyle::pixelMetric(QStyle::PixelMetric m, const QStyleOption *opt,
                                const QWidget *widget) const
{
    switch (static_cast<int>(m)) {
    case PM_SpinBoxSliderHeight:
        return SpinBox_MiniHeight;
    case PM_SpinBoxFrameWidth:
        return SpinBox_FrameWidth;
    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
        // 平板需求
        return DGuiApplicationHelper::isTabletEnvironment() ? TabletTabBar_TabButtonSize : TabBar_TabButtonSize;
    case PM_TabBarTabVSpace:
    case PM_TabBarTabHSpace :
        return DStyle::pixelMetric(PM_FrameRadius, opt, widget) * 2;
    case PM_TabBarTabOverlap:
        return TabBar_TabOverlap;
    case PM_TabBarBaseOverlap:
        return TabBar_BaseOverlap;
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        return 0;
    case PM_TabBarScrollButtonWidth:
        // DTabBar有自己的scroll按钮，需要隐藏QTabBar的按钮
        if (widget && qobject_cast<DTabBar*>(widget->parent()))
            return 0;

        return DStyle::pixelMetric(PM_ButtonMinimizedSize, opt, widget);
    case PM_MenuScrollerHeight:
        return 10 + Metrics::Frame_FrameWidth ;
    case PM_MenuPanelWidth:
        //非特效效果则 menu border 为 1
        return DWindowManagerHelper::instance()->hasComposite() ? 0 : 1;
    case PM_SubMenuOverlap:
        return 0;
    case PM_ComboBoxFrameWidth: { //这是ComboBox VMargin
        const QStyleOptionComboBox *comboBoxOption(qstyleoption_cast< const QStyleOptionComboBox *>(opt));
        return comboBoxOption && comboBoxOption->editable ? Metrics::ComboBox_FrameWidth : Metrics::LineEdit_FrameWidth ;
    }
    case PM_MenuVMargin:
        return 8;
    case PM_MenuHMargin:
        return 0;
    default:
        break;
    }

    return DStyle::pixelMetric(m, opt, widget);
}

int ChameleonStyle::styleHint(QStyle::StyleHint sh, const QStyleOption *opt,
                              const QWidget *w, QStyleHintReturn *shret) const
{
    switch (sh) {
    case SH_ItemView_ShowDecorationSelected:
    case SH_ScrollBar_Transient:
        return true;
    //增加菜单鼠标事件跟踪
    case SH_MenuBar_MouseTracking:
    case SH_Menu_MouseTracking:
        return true;
    //增加TabBar超出范围的左右导航按钮
    case SH_TabBar_PreferNoArrows:
        return false;
    case SH_ComboBox_Popup:
        return true;
    case SH_ComboBox_PopupFrameStyle:
        return QFrame::NoFrame | QFrame::Plain;
    case SH_Slider_AbsoluteSetButtons:
        return Qt::LeftButton | Qt::MiddleButton;
    case SH_ToolTipLabel_Opacity:
        return 255;
    default:
        break;
    }

    return DStyle::styleHint(sh, opt, w, shret);
}

void ChameleonStyle::resetAttribute(QWidget *w, bool polish)
{
    if (!w)
        return;

    bool enableHover = w->testAttribute(Qt::WA_Hover);
    if (qobject_cast<QAbstractButton *>(w)
            || qobject_cast<QComboBox *>(w)
            || qobject_cast<QScrollBar *>(w)
            || qobject_cast<QCheckBox *>(w)
            || qobject_cast<QAbstractSpinBox *>(w)
            || qobject_cast<QTabBar *>(w)
            || qobject_cast<QCheckBox *>(w)) {
        enableHover = polish;
    }

    if (auto view = qobject_cast<QAbstractItemView *>(w)) {
        enableHover = polish;
        w = view->viewport();
    }

    // TODO: 是平板环境统一设置所有的控件的 WA_Hover 为 false，
    // 不过在插入鼠标时还是有问题，现阶段先不考虑，以后 Qt 如果优化了如有更好的方案这里再改掉。
    if (DGuiApplicationHelper::isTabletEnvironment())
        enableHover = false;

    w->setAttribute(Qt::WA_Hover, enableHover);

    if (auto scrollbar = qobject_cast<QScrollBar *>(w)) {
        // 默认初始显示滚动条，然后启动隐藏动画
        scrollbar->setProperty("_d_dtk_scrollbar_visible", true);
        scrollbar->setAttribute(Qt::WA_OpaquePaintEvent, !polish);
    }
}

static void updateWeekendTextFormat(QCalendarWidget *calendar, QColor)
{
    if (!calendar)
        return ;

    QTextCharFormat fmt;
    fmt.setForeground(QBrush(calendar->palette().highlight()));
    calendar->setWeekdayTextFormat(Qt::Saturday, fmt);
    calendar->setWeekdayTextFormat(Qt::Sunday, fmt);
}

// 最大圆角18, 原来默认是8
static inline void setWindowRadius(QWidget *w, int radius)
{
    DPlatformWindowHandle handle(w);
    handle.setWindowRadius(radius);
}

void ChameleonStyle::polish(QWidget *w)
{
    DStyle::polish(w);

    resetAttribute(w, true);

    if (qobject_cast<QAbstractButton *>(w)
            || qobject_cast<QComboBox *>(w)
            || qobject_cast<QScrollBar *>(w)
            || qobject_cast<QCheckBox *>(w)
            || qobject_cast<QAbstractSpinBox *>(w)
            || qobject_cast<QTabBar *>(w)
            || qobject_cast<QCheckBox *>(w)) {
        w->setAttribute(Qt::WA_Hover, true);
    }

    if (auto view = qobject_cast<QAbstractItemView *>(w)) {
        view->viewport()->setAttribute(Qt::WA_Hover, true);
    }

    if (auto listview = qobject_cast<QListView *>(w)) {
        if (listview->parentWidget() == nullptr)
            setWindowRadius(listview, DStyle::pixelMetric(PM_FrameRadius));
    }

    if (auto container = qobject_cast<QComboBoxPrivateContainer *>(w)) {
        if (DWindowManagerHelper::instance()->hasComposite())
            setWindowRadius(container, DStyle::pixelMetric(PM_FrameRadius));

        if (!DGuiApplicationHelper::isTabletEnvironment())
            container->setFrameStyle(QFrame::NoFrame);
    }

    if (auto calendar = qobject_cast<QCalendarWidget* >(w)) {
        int radius = DStyle::pixelMetric(PM_TopLevelWindowRadius);
        // 只有dtk的应用绘制日历窗口圆角
        if (dynamic_cast<DApplication *>(QCoreApplication::instance()))
            setWindowRadius(calendar, radius);

        calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

        // 更改日历Saturday　Sunday　活动色改变时跟随
        DPlatformTheme *theme = DGuiApplicationHelper::instance()->applicationTheme();
        updateWeekendTextFormat(calendar, QColor());
        connect(theme, &DPlatformTheme::activeColorChanged, calendar,
                std::bind(&updateWeekendTextFormat, calendar, std::placeholders::_1));

        auto topWidget = calendar->findChild<QWidget *>("qt_calendar_navigationbar");
        topWidget->setBackgroundRole(QPalette::Base);

        auto layout = qobject_cast<QLayout*>(topWidget->layout());
        int margin = radius / 2;
        layout->setContentsMargins(margin, margin, margin, margin);
    }

    if (w && (w->objectName() == "qt_calendar_yearbutton"
                        || w->objectName() == "qt_calendar_monthbutton")) {
        w->setProperty("_d_calendarToolBtn", true);

        DFontSizeManager *fontManager =  DFontSizeManager::instance();
        fontManager->bind(w, DFontSizeManager::T5, QFont::Normal);
    }

    if (w && w->objectName() == "qt_calendar_yearedit") {
        w->setProperty("_d_dtk_spinBox", true);
        //直接取用spinBox最大年限
        int width = w->fontMetrics().horizontalAdvance("9999");
        w->setMaximumWidth(width * 3);
    }

    if (w && (w->objectName() == "qt_calendar_prevmonth"
            || w->objectName() == "qt_calendar_nextmonth")) {
        int btnWidget = DStyle::pixelMetric(DStyle::PM_ButtonMinimizedSize);
        w->setMinimumSize(btnWidget, btnWidget);
    }

    if (w && w->objectName() == "qt_calendar_calendarview") {
        auto view = qobject_cast<QTableView *>(w);
        view->setItemDelegate(new QStyledItemDelegate);
    }

    if (DApplication::isDXcbPlatform() || (qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool())) {
        bool is_menu = qobject_cast<QMenu *>(w);
        bool is_tip = w->inherits("QTipLabel");

        // 当窗口已经创建对应的native窗口，要判断当前是否已经设置了窗口背景透明
        // Bug: https://github.com/linuxdeepin/internal-discussion/issues/323
        if (is_menu && w->windowHandle()) {
            if (const QPlatformWindow *handle = w->windowHandle()->handle()) {
                if (!w->testAttribute(Qt::WA_TranslucentBackground) && !handle->isExposed()) {
                    // 销毁现有的native窗口，否则设置Qt::WA_TranslucentBackground不会生效
                    class DQWidget : public QWidget {public: using QWidget::destroy;};
                    reinterpret_cast<DQWidget *>(w)->destroy(true, false);
                }
            }
        }

        // fix qtcreator QDesignerMenu setnotitlebar qmainwindow
        if (is_menu && w->isWindow()) {
            DPlatformWindowHandle handle(w);

            if (DPlatformWindowHandle::isEnabledDXcb(w)) {
                handle.setEnableBlurWindow(true);

                DPlatformTheme *theme = DGuiApplicationHelper::instance()->applicationTheme();
                if (theme->isValid())
                    setWindowRadius(w, qMax(0, qMin(theme->windowRadius(), 18)));

                connect(theme, &DPlatformTheme::windowRadiusChanged, w, [w](int r){
                   setWindowRadius(w, qMax(0, qMin(r, 18)));
                });
                w->setAttribute(Qt::WA_TranslucentBackground);
                connect(DWindowManagerHelper::instance(), SIGNAL(hasCompositeChanged()), w, SLOT(update()));
            }
        } else if (is_tip) {
            if (DWindowManagerHelper::instance()->hasComposite()) {
                DPlatformWindowHandle handle(w);
                handle.setWindowRadius(DStyle::pixelMetric(PM_FrameRadius));
            }
            QLabel *label = qobject_cast<QLabel *>(w);
            label->setTextFormat(DToolTip::toolTipTextFormat());
        }
    }
}

void ChameleonStyle::unpolish(QWidget *w)
{
    DStyle::unpolish(w);

    resetAttribute(w, false);

    if (w && qobject_cast<QLineEdit *>(w)) {
        w->setProperty("_d_dtk_lineeditActionWidth", QVariant());
        w->setProperty("_d_dtk_lineeditActionMargin", QVariant());
    }
}

void ChameleonStyle::unpolish(QApplication *application)
{
    DStyle::unpolish(application);
}

void ChameleonStyle::drawShadow(QPainter *p, const QRect &rect, const QColor &color) const
{
    int frame_radius = DStyle::pixelMetric(PM_FrameRadius);
    int shadow_xoffset = DStyle::pixelMetric(PM_ShadowHOffset);
    int shadow_yoffset = DStyle::pixelMetric(PM_ShadowVOffset);

    QRect shadow = rect;
    QPoint pointOffset(rect.center().x() + shadow_xoffset, rect.center().y() + shadow_yoffset);
    shadow.moveCenter(pointOffset);

    p->setBrush(color);
    p->setPen(Qt::NoPen);
    p->setRenderHint(QPainter::Antialiasing);
    p->drawRoundedRect(shadow, frame_radius, frame_radius);
}

void ChameleonStyle::drawBorder(QPainter *p, const QStyleOption *opt, const QWidget *w) const
{
    const QColor &focus_color = getColor(opt, QPalette::Highlight);

    if (!focus_color.isValid() || focus_color.alpha() == 0)
        return;

    int frame_radius = DStyle::pixelMetric(PM_FrameRadius, opt, w);
    int margins = DStyle::pixelMetric(PM_FrameMargins, opt, w);

    QRect border = opt->rect.adjusted(margins, margins, -margins, -margins);
    p->setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(focus_color);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    if (w && w->property("_d_dtk_tabbartab_type").toBool()) {
        p->drawRect(opt->rect.adjusted(-1, 1, 1, -1));
        return;
    }

    if (w && w->parentWidget() && w->parentWidget()->property("_d_dtk_spinBox").toBool()) {
        DDrawUtils::drawRoundedRect(p, opt->rect.adjusted(1, 1, -1, -1), frame_radius, frame_radius,
                                    DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
        return;
    }

    bool table = qobject_cast<const QTableView *>(w) && !w->property("_d_dtk_enable_tableviewitem_radius").toBool();
    //QCalendarWidget的QTableView焦点状态与QTableView不同
    bool calendar = w && (w->objectName() == "qt_calendar_calendarview");
    // DButtonBoxButton在不同位置焦点不同
    const DButtonBoxButton *buttonBoxButton = qobject_cast<const DButtonBoxButton *>(w);
    const DStyleOptionButtonBoxButton *btnopt = qstyleoption_cast<const DStyleOptionButtonBoxButton *>(opt);

    pen.setWidth(2);
    int offset = 1;
    QRect rect = border.adjusted(offset, offset, -offset, -offset);

    // 先画内框（黑or白）
    pen.setColor(getColor(opt, QPalette::Base));
    p->setPen(pen);
    if (calendar) {
        pen.setWidth(3);
        p->setPen(pen);
        offset = 2;
        drawCalenderEllipse(p, opt->rect, offset);
    } else if (table) {
        p->drawRect(rect);
    } else if (buttonBoxButton && btnopt) {
        drawButtonBoxButton(btnopt, p, rect, frame_radius);
    } else {
        p->drawRoundedRect(rect, frame_radius, frame_radius);
    }

    // 再画外框（活动色即焦点）
    pen.setColor(focus_color);
    p->setPen(pen);
    if (calendar) {
        pen.setWidth(2);
        p->setPen(pen);
        offset = 1;
        drawCalenderEllipse(p, opt->rect, offset);
    } else if (table) {
        p->drawRect(border);
    } else if (buttonBoxButton && btnopt) {
        drawButtonBoxButton(btnopt, p, border, frame_radius);
    } else {
        // FIX focus rect not covering corners [frame_radius - margins]
        p->drawRoundedRect(border, frame_radius - margins, frame_radius - margins);
    }
}

void ChameleonStyle::drawCalenderEllipse(QPainter *p, const QRect &rect, int offset) const
{
    QRect ellipese = rect;
    ellipese.setWidth(ellipese.height());
    ellipese.moveCenter(rect.center());
    p->drawEllipse(ellipese.adjusted(offset, offset, -offset, -offset));
}

void ChameleonStyle::drawButtonBoxButton(const DStyleOptionButtonBoxButton *btnopt, QPainter *p, const QRect &rect, int radius) const
{
    if (btnopt->position == DStyleOptionButtonBoxButton::Beginning) {
        // Begin
        if (btnopt->orientation == Qt::Horizontal) {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius,
                                        DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
        } else {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius,
                                        DDrawUtils::TopLeftCorner | DDrawUtils::TopRightCorner);
        }
    } else if (btnopt->position == DStyleOptionButtonBoxButton::End) {
        // End
        if (btnopt->orientation == Qt::Horizontal) {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius,
                                        DDrawUtils::TopRightCorner | DDrawUtils::BottomRightCorner);
        } else {
            DDrawUtils::drawRoundedRect(p, rect, radius, radius,
                                        DDrawUtils::BottomLeftCorner | DDrawUtils::BottomRightCorner);
        }
    } else if (btnopt->position == DStyleOptionButtonBoxButton::Middle) {
        // Middle
        p->drawRect(rect);
    } else if (btnopt->position == DStyleOptionButtonBoxButton::OnlyOne) {
        // OnlyOne
        p->drawRoundedRect(rect, radius, radius);
    }
}

bool ChameleonStyle::isNoticks(const QStyleOptionSlider *slider, QPainter *p, const QWidget *w) const
{
    Q_UNUSED(p)
    if (const DSlider *dslider = qobject_cast<const DSlider *>(w)) {
        QSlider::TickPosition tickPosition = slider->tickPosition;

        if (dslider)
            tickPosition = dslider->tickPosition();

        return tickPosition == QSlider::NoTicks;
    }
    return false;
}

QBrush ChameleonStyle::generatedBrush(StateFlags flags, const QBrush &base, QPalette::ColorGroup cg, QPalette::ColorRole role, const QStyleOption *option) const
{
    return DStyle::generatedBrush(flags, base, cg, role, option);
}

QBrush ChameleonStyle::generatedBrush(DStyle::StateFlags flags, const QBrush &base, QPalette::ColorGroup cg, DPalette::ColorType type, const QStyleOption *option) const
{
    return DStyle::generatedBrush(flags, base, cg, type, option);
}

QColor ChameleonStyle::getColor(const QStyleOption *option, QPalette::ColorRole role) const
{
    return DStyle::generatedBrush(option, option->palette.brush(role), option->palette.currentColorGroup(), role).color();
}

QColor ChameleonStyle::getColor(const QStyleOption *option, DPalette::ColorType type, const QWidget *widget) const
{

    const DPalette &pa = DPaletteHelper::instance()->palette(widget, option->palette);

    return DStyle::generatedBrush(option, pa.brush(type), pa.currentColorGroup(), type).color();
}

QBrush ChameleonStyle::getBrush(const QStyleOption *option, DPalette::ColorRole type) const
{
    QWidget *widget = qobject_cast<QWidget *>(option->styleObject);
    if (widget && !widget->testAttribute(Qt::WA_Hover) && DGuiApplicationHelper::isTabletEnvironment()) {
        return QBrush(Qt::NoBrush);
    }
    return QBrush(getColor(option, type));
}

QMargins ChameleonStyle::frameExtentMargins() const
{
    int margins = DStyle::pixelMetric(PM_FrameMargins);

    return QMargins(margins, margins, margins, margins);
}

QRect ChameleonStyle::drawButtonDownArrow(const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt);

    if (!btn)
        return QRect(-1, -1, -1, -1);

    QRect rectOpt = btn->rect;                      //实际绘画箭头所占的小矩形
    int arrowWidth = DStyle::pixelMetric(PM_MenuButtonIndicator, opt, w);
    int arrowHeight = arrowWidth;
    QRect rectArrow(0, 0 , arrowWidth, arrowHeight);
    rectArrow.moveCenter(rectOpt.center());

    QStyleOptionButton newBtn = *btn;                 //绘画箭头的大矩形(不要竖线)
    QRect &newRect = newBtn.rect;
    newRect.setHeight(rectOpt.height());
    newRect.setWidth(arrowWidth);
    newRect.moveCenter(rectOpt.center());

    if (btn->direction == Qt::LeftToRight) {
        rectArrow.moveRight(rectOpt.right());
        newRect.moveRight(rectOpt.right());
    } else {
        rectArrow.moveLeft(rectOpt.left());
        newRect.moveLeft(rectOpt.left());
    }

    if (p == nullptr || w == nullptr)
        return newRect;

    QStyleOptionButton arrowDrawBtn  = newBtn;
    arrowDrawBtn.rect = rectArrow;
    proxy()->drawPrimitive(PE_IndicatorArrowDown, &arrowDrawBtn, p, w);

    return newRect;
}

} // namespace chameleon
