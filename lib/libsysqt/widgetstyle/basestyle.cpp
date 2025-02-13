/*
 * Copyright (C) 2020 Reven Martin
 * Copyright (C) 2020 KeePassXC Team <team@keepassxc.org>
 * Copyright (C) 2019 Andrew Richards
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Derived from Phantomstyle and relicensed under the GPLv2 or v3.
 * https://github.com/randrew/phantomstyle
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 or (at your option)
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "basestyle.h"
#include "phantomcolor.h"
#include "shadowhelper.h"
#include "blurhelper.h"

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QHeaderView>
#include <QListView>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QPolygon>
#include <QPushButton>
#include <QProgressBar>
#include <QScrollBar>
#include <QSharedData>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QString>
#include <QStyleOption>
#include <QTableView>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QWindow>
#include <QWizard>
#include <QtMath>
#include <qdrawutil.h>

#include <QSettings>

#include <cmath>

#include <QDBusInterface>

QT_BEGIN_NAMESPACE
Q_GUI_EXPORT int qt_defaultDpiX();
QT_END_NAMESPACE

// Define colors
QColor m_blueColor = QColor(34, 119, 255); // #2277FF
QColor m_redColor = QColor(255, 92, 109); // #FF5C6D
QColor m_greenColor = QColor(53, 191, 86); // #35BF56
QColor m_purpleColor = QColor(130, 102, 255); // #8266FF
QColor m_pinkColor = QColor(202, 100, 172); // #CA64AC
QColor m_orangeColor = QColor(254, 160, 66); // #FEA042
QColor m_greyColor = QColor(79, 89, 107); // #4F596B

QColor baseColor;

void setBaseColorFromAccentColor(int accentColorID) {
    switch (accentColorID) {
    case 0:
        baseColor = m_blueColor;
        break;
    case 1:
        baseColor = m_redColor;
        break;
    case 2:
        baseColor = m_greenColor;
        break;
    case 3:
        baseColor = m_purpleColor;
        break;
    case 4:
        baseColor = m_pinkColor;
        break;
    case 5:
        baseColor = m_orangeColor;
        break;
    case 6:
        baseColor = m_greyColor;
        break;
    default:
        baseColor = m_blueColor;
        break;
    }
}

// 处理 accentColor 变化的槽函数
void onAccentColorChanged() {
    QDBusInterface iface("com.lingmo.Settings",
                         "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        int accentColorID = iface.property("accentColor").toInt();
        setBaseColorFromAccentColor(accentColorID);

        // 更新所有窗口部件
        for (QWidget *widget : qApp->allWidgets()) {
            // 重新应用样式
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
    }
}

void initializeBaseColor(QObject *context) {
    QDBusInterface iface("com.lingmo.Settings",
                         "/Theme",
                         "com.lingmo.Theme",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        int accentColorID = iface.property("accentColor").toInt();
        setBaseColorFromAccentColor(accentColorID);

        // 连接 accentColorChanged 信号到槽函数
        QDBusConnection::sessionBus().connect("com.lingmo.Settings",
                                              "/Theme",
                                              "com.lingmo.Theme",
                                              "accentColorChanged",
                                              context,
                                              SLOT(onAccentColorChanged()));
    }
}

static QObject *hintsSettings()
{
    return reinterpret_cast<QObject*>(qvariant_cast<quintptr>(qApp->property("_hints_settings_object")));
}

QRectF strokedRect( const QRectF &rect, const int penWidth )
{
    /* With a pen stroke width of 1, the rectangle should have each of its
     * sides moved inwards by half a pixel. This allows the stroke to be
     * pixel perfect instead of blurry from sitting between pixels and
     * prevents the rectangle with a stroke from becoming larger than the
     * original size of the rectangle.
     */
    qreal adjustment = 0.5 * penWidth;
    return QRectF( rect ).adjusted( adjustment, adjustment, -adjustment, -adjustment );
}

QRectF strokedRect( const QRect &rect, const int penWidth )
{
    return strokedRect(QRectF(rect), penWidth);
}

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

// Redefine Q_FALLTHROUGH for older Qt versions
#ifndef Q_FALLTHROUGH
#if (defined(Q_CC_GNU) && Q_CC_GNU >= 700) && !defined(Q_CC_INTEL)
#define Q_FALLTHROUGH() __attribute__((fallthrough))
#else
#define Q_FALLTHROUGH() (void)0
#endif
#endif

namespace Phantom
{
    namespace
    {
        constexpr qint16 DefaultFrameWidth = 6;
        constexpr qint16 SplitterMaxLength = 100; // Length of splitter handle (not thickness)
        constexpr qint16 MenuMinimumWidth = 20; // Smallest width that menu items can have
        constexpr qint16 MenuBar_FrameWidth = 6;
        constexpr qint16 MenuBar_ItemSpacing = 10;
        constexpr qint16 SpinBox_ButtonWidth = 15;

        // These two are currently not based on font, but could be
        constexpr qint16 LineEdit_ContentsHPad = 5;
        constexpr qint16 ComboBox_NonEditable_ContentsHPad = 7;
        constexpr qint16 HeaderSortIndicator_HOffset = 1;
        constexpr qint16 HeaderSortIndicator_VOffset = 2;
        constexpr qint16 TabBar_InctiveVShift = 0;

        constexpr qreal DefaultFrame_Radius = 11.0;
        constexpr qreal TabBarTab_Rounding = 12.0;
        constexpr qreal SpinBox_Rounding = 12.0;
        constexpr qreal LineEdit_Rounding = 12.0;
        constexpr qreal FrameFocusRect_Rounding = 5.0;
        constexpr qreal PushButton_Rounding = 12.0;
        constexpr qreal ToolButton_Rounding = 11.0;
        constexpr qreal ProgressBar_Rounding = 12.0;
        constexpr qreal GroupBox_Rounding = 12.0;
        constexpr qreal SliderGroove_Rounding = 12.0;
        constexpr qreal SliderHandle_Rounding = 12.0;

        constexpr qreal CheckMark_WidthOfHeightScale = 0.8;
        constexpr qreal PushButton_HorizontalPaddingFontHeightRatio = 1.0;
        constexpr qreal TabBar_HPaddingFontRatio = 1.25;
        constexpr qreal TabBar_VPaddingFontRatio = 1.0 / 1.25;
        constexpr qreal GroupBox_LabelBottomMarginFontRatio = 1.0 / 4.0;
        constexpr qreal ComboBox_ArrowMarginRatio = 1.0 / 3.25;

        constexpr qreal MenuBar_HorizontalPaddingFontRatio = 1.0 / 2.0;
        constexpr qreal MenuBar_VerticalPaddingFontRatio = 1.0 / 3.0;

        constexpr qreal MenuItem_LeftMarginFontRatio = 1.0 / 2.0;
        constexpr qreal MenuItem_RightMarginForTextFontRatio = 1.0 / 1.5;
        constexpr qreal MenuItem_RightMarginForArrowFontRatio = 1.0 / 4.0;
        constexpr qreal MenuItem_VerticalMarginsFontRatio = 1.0 / 5.0;
        // Number that's multiplied with a font's height to get the space between a
        // menu item's checkbox (or other sign) and its text (or icon).
        constexpr qreal MenuItem_CheckRightSpaceFontRatio = 1.0 / 4.0;
        constexpr qreal MenuItem_TextMnemonicSpaceFontRatio = 1.5;
        constexpr qreal MenuItem_SubMenuArrowSpaceFontRatio = 1.0 / 1.5;
        constexpr qreal MenuItem_SubMenuArrowWidthFontRatio = 1.0 / 2.75;
        constexpr qreal MenuItem_SeparatorHeightFontRatio = 1.0 / 1.5;
        constexpr qreal MenuItem_CheckMarkVerticalInsetFontRatio = 1.0 / 5.0;
        constexpr qreal MenuItem_IconRightSpaceFontRatio = 1.0 / 3.0;

        constexpr bool BranchesOnEdge = false;
        constexpr bool OverhangShadows = false;
        constexpr bool IndicatorShadows = false;
        constexpr bool MenuExtraBottomMargin = true;
        constexpr bool MenuBarLeftMargin = false;
        constexpr bool MenuBarDrawBorder = false;
        constexpr bool AllowToolBarAutoRaise = true;
        // Note that this only applies to the disclosure etc. decorators in tree views.
        constexpr bool ShowItemViewDecorationSelected = false;
        constexpr bool UseQMenuForComboBoxPopup = true;
        constexpr bool ItemView_UseFontHeightForDecorationSize = true;

        // Whether or not the non-raised tabs in a tab bar have shininess/highlights to
        // them. Setting this to false adds an extra visual hint for distinguishing
        // between the current and non-current tabs, but makes the non-current tabs
        // appear less clickable. Other ways to increase the visual differences could
        // be to increase the color contrast for the background fill color, or increase
        // the vertical offset. However, increasing the vertical offset comes with some
        // layout challenges, and increasing the color contrast further may visually
        // imply an incorrect layout structure. Not sure what's best.
        //
        // This doesn't disable creating the color/brush resource, even though it's
        // currently a compile-time-only option, because it may be changed to be part
        // of some dynamic config system for Phantom in the future, or have a
        // per-widget style hint associated with it.
        const bool TabBar_InactiveTabsHaveSpecular = false;

        struct Grad
        {
            Grad(const QColor& from, const QColor& to)
            {
                rgbA = Rgb::ofQColor(from);
                rgbB = Rgb::ofQColor(to);
                lA = rgbA.toHsl().l;
                lB = rgbB.toHsl().l;
            }
            QColor sample(qreal alpha) const
            {
                Hsl hsl = Rgb::lerp(rgbA, rgbB, alpha).toHsl();
                hsl.l = Phantom::lerp(lA, lB, alpha);
                return hsl.toQColor();
            }
            Rgb rgbA, rgbB;
            qreal lA, lB;
        };

        namespace DeriveColors
        {
            Q_NEVER_INLINE QColor adjustLightness(const QColor& qcolor, qreal ld)
            {
                Hsl hsl = Hsl::ofQColor(qcolor);
                const qreal gamma = 3.0;
                hsl.l = std::pow(Phantom::saturate(std::pow(hsl.l, 1.0 / gamma) + ld * 0.8), gamma);
                return hsl.toQColor();
            }
            bool hack_isLightPalette(const QPalette& pal)
            {
                Hsl hsl0 = Hsl::ofQColor(pal.color(QPalette::WindowText));
                Hsl hsl1 = Hsl::ofQColor(pal.color(QPalette::Window));
                return hsl0.l < hsl1.l;
            }
            QColor buttonColor(const QPalette& pal)
            {
                // temp hack
                if (pal.color(QPalette::Button) == pal.color(QPalette::Window))
                    return adjustLightness(pal.color(QPalette::Button), 0.01);
                return pal.color(QPalette::Button);
            }
            QColor highlightedOutlineOf(const QPalette& pal)
            {
                return adjustLightness(pal.color(QPalette::Highlight), -0.08);
            }
            QColor dividerColor(const QColor& underlying)
            {
                return adjustLightness(underlying, -0.05);
            }
            QColor lightDividerColor(const QColor& underlying)
            {
                return adjustLightness(underlying, 0.02);
            }
            QColor outlineOf(const QPalette& pal)
            {
                return adjustLightness(pal.color(QPalette::Window), -0.1);
            }
            QColor gutterColorOf(const QPalette& pal)
            {
                return adjustLightness(pal.color(QPalette::Window), -0.05);
            }
            QColor darkGutterColorOf(const QPalette& pal)
            {
                return adjustLightness(pal.color(QPalette::Window), -0.08);
            }
            QColor lightShadeOf(const QColor& underlying)
            {
                return adjustLightness(underlying, 0.08);
            }
            QColor darkShadeOf(const QColor& underlying)
            {
                return adjustLightness(underlying, -0.08);
            }
            QColor overhangShadowOf(const QColor& underlying)
            {
                return adjustLightness(underlying, -0.05);
            }
            QColor sliderGutterShadowOf(const QColor& underlying)
            {
                return adjustLightness(underlying, -0.01);
            }
            QColor specularOf(const QColor& underlying)
            {
                return adjustLightness(underlying, 0.01);
            }
            QColor lightSpecularOf(const QColor& underlying)
            {
                return adjustLightness(underlying, 0.05);
            }
            QColor pressedOf(const QColor& color)
            {
                return adjustLightness(color, -0.05);
            }
            QColor darkPressedOf(const QColor& color)
            {
                return adjustLightness(color, -0.08);
            }
            QColor lightOnOf(const QColor& color)
            {
                return adjustLightness(color, -0.04);
            }
            QColor onOf(const QColor& color)
            {
                return adjustLightness(color, -0.08);
            }
            QColor indicatorColorOf(const QPalette& palette, QPalette::ColorGroup group = QPalette::Current)
            {
                if (hack_isLightPalette(palette)) {
                    qreal adjust = (palette.currentColorGroup() == QPalette::Disabled) ? 0.09 : 0.32;
                    return adjustLightness(palette.color(group, QPalette::WindowText), adjust);
                }
                return adjustLightness(palette.color(group, QPalette::WindowText), -0.05);
            }
            QColor inactiveTabFillColorOf(const QColor& underlying)
            {
                // used to be -0.01
                return adjustLightness(underlying, -0.025);
            }
            QColor progressBarOutlineColorOf(const QPalette& pal)
            {
                // Pretty wasteful
                Hsl hsl0 = Hsl::ofQColor(pal.color(QPalette::Window));
                Hsl hsl1 = Hsl::ofQColor(pal.color(QPalette::Highlight));
                hsl1.l = Phantom::saturate(qMin(hsl0.l - 0.1, hsl1.l - 0.2));
                return hsl1.toQColor();
            }
            QColor itemViewMultiSelectionCurrentBorderOf(const QPalette& pal)
            {
                return adjustLightness(pal.color(QPalette::Highlight), -0.15);
            }
            QColor itemViewHeaderOnLineColorOf(const QPalette& pal)
            {
                return hack_isLightPalette(pal)
                           ? highlightedOutlineOf(pal)
                           : Grad(pal.color(QPalette::WindowText), pal.color(QPalette::Window)).sample(0.5);
            }
        } // namespace DeriveColors

        namespace SwatchColors
        {
            enum SwatchColor
            {
                S_none = 0,
                S_window,
                S_button,
                S_base,
                S_text,
                S_windowText,
                S_highlight,
                S_highlightedText,
                S_scrollbarGutter,
                S_scrollbarSlider,
                S_scrollbarSlider_hover,
                S_scrollbarSlider_pressed,
                S_window_outline,
                S_window_specular,
                S_window_divider,
                S_window_lighter,
                S_window_darker,
                S_frame_outline,
                S_button_specular,
                S_button_pressed,
                S_button_on,
                S_button_pressed_specular,
                S_sliderHandle,
                S_sliderHandle_pressed,
                S_sliderHandle_specular,
                S_sliderHandle_pressed_specular,
                S_base_shadow,
                S_base_divider,
                S_windowText_disabled,
                S_highlight_outline,
                S_highlight_specular,
                S_progressBar_outline,
                S_inactiveTabYesFrame,
                S_inactiveTabNoFrame,
                S_inactiveTabYesFrame_specular,
                S_inactiveTabNoFrame_specular,
                S_indicator_current,
                S_indicator_disabled,
                S_itemView_multiSelection_currentBorder,
                S_itemView_headerOnLine,
                S_scrollbarGutter_disabled,

                // Aliases
                S_progressBar = S_highlight,
                S_progressBar_specular = S_highlight_specular,
                S_tabFrame = S_window,
                S_tabFrame_specular = S_window_specular,
            };
        }

        using Swatchy = SwatchColors::SwatchColor;

        enum
        {
            Num_SwatchColors = SwatchColors::S_scrollbarGutter_disabled + 1,
            Num_ShadowSteps = 3,
        };

        struct PhSwatch : public QSharedData
        {
            // The pens store the brushes within them, so storing the brushes here as
            // well is redundant. However, QPen::brush() does not return its brush by
            // reference, so we'd end up doing a bunch of inc/dec work every time we use
            // one. Also, it saves us the indirection of chasing two pointers (Swatch ->
            // QPen -> QBrush) every time we want to get a QColor.
            QBrush brushes[Num_SwatchColors];
            QPen pens[Num_SwatchColors];
            QColor scrollbarShadowColors[Num_ShadowSteps];

            // Note: the casts to int in the assert macros are to suppress a false
            // positive warning for tautological comparison in the clang linter.
            inline const QColor& color(Swatchy swatchValue) const
            {
                Q_ASSERT(swatchValue >= 0 && static_cast<int>(swatchValue) < Num_SwatchColors);
                return brushes[swatchValue].color();
            }
            inline const QBrush& brush(Swatchy swatchValue) const
            {
                Q_ASSERT(swatchValue >= 0 && static_cast<int>(swatchValue) < Num_SwatchColors);
                return brushes[swatchValue];
            }
            inline const QPen& pen(Swatchy swatchValue) const
            {
                Q_ASSERT(swatchValue >= 0 && static_cast<int>(swatchValue) < Num_SwatchColors);
                return pens[swatchValue];
            }

            void loadFromQPalette(const QPalette& pal);
        };

        using PhSwatchPtr = QExplicitlySharedDataPointer<PhSwatch>;
        using PhCacheEntry = QPair<uint, PhSwatchPtr>;
        enum : int
        {
            Num_ColorCacheEntries = 10,
        };
        using PhSwatchCache = QVarLengthArray<PhCacheEntry, Num_ColorCacheEntries>;
        Q_NEVER_INLINE void PhSwatch::loadFromQPalette(const QPalette& pal)
        {
            using namespace SwatchColors;
            namespace Dc = DeriveColors;
            bool isLight = Dc::hack_isLightPalette(pal);
            QColor colors[Num_SwatchColors];
            colors[S_none] = QColor();

            colors[S_window] = pal.color(QPalette::Window);
            colors[S_button] = pal.color(QPalette::Button);
            if (colors[S_button] == colors[S_window])
                colors[S_button] = Dc::adjustLightness(colors[S_button], 0.01);
            colors[S_base] = pal.color(QPalette::Base);
            colors[S_text] = pal.color(QPalette::Text);
            colors[S_windowText] = pal.color(QPalette::WindowText);
            colors[S_highlight] = pal.color(QPalette::Highlight);
            colors[S_highlightedText] = pal.color(QPalette::HighlightedText);
            colors[S_scrollbarGutter] = isLight ? Dc::gutterColorOf(pal) : Dc::darkGutterColorOf(pal);
            colors[S_scrollbarSlider] = isLight ? colors[S_button] : Dc::adjustLightness(colors[S_window], 0.1);
            colors[S_scrollbarSlider_hover] = isLight ? Dc::adjustLightness(colors[S_button], -0.1) : Dc::adjustLightness(colors[S_window], 0.2);
            colors[S_scrollbarSlider_pressed] = isLight ? Dc::adjustLightness(colors[S_button], -0.2) : Dc::adjustLightness(colors[S_window], 0.25);

            colors[S_window_outline] =
                isLight ? Dc::adjustLightness(colors[S_window], -0.1) : Dc::adjustLightness(colors[S_window], 0.03);
            colors[S_window_specular] = Dc::specularOf(colors[S_window]);
            colors[S_window_divider] =
                isLight ? Dc::dividerColor(colors[S_window]) : Dc::lightDividerColor(colors[S_window]);
            colors[S_window_lighter] = Dc::lightShadeOf(colors[S_window]);
            colors[S_window_darker] = Dc::darkShadeOf(colors[S_window]);
            colors[S_frame_outline] = isLight ? colors[S_window_outline] : Dc::adjustLightness(colors[S_window], 0.08);
            colors[S_button_specular] =
                isLight ? Dc::specularOf(colors[S_button]) : Dc::lightSpecularOf(colors[S_button]);
            colors[S_button_pressed] = isLight ? Dc::pressedOf(colors[S_button]) : Dc::darkPressedOf(colors[S_button]);
            colors[S_button_on] = isLight ? Dc::lightOnOf(colors[S_button]) : Dc::onOf(colors[S_button]);
            colors[S_button_pressed_specular] =
                isLight ? Dc::specularOf(colors[S_button_pressed]) : Dc::lightSpecularOf(colors[S_button_pressed]);

            colors[S_sliderHandle] = isLight ? colors[S_button] : Dc::adjustLightness(colors[S_button], -0.03);
            colors[S_sliderHandle_specular] =
                isLight ? Dc::specularOf(colors[S_sliderHandle]) : Dc::lightSpecularOf(colors[S_sliderHandle]);
            colors[S_sliderHandle_pressed] =
                isLight ? colors[S_button_pressed] : Dc::adjustLightness(colors[S_button_pressed], 0.03);
            colors[S_sliderHandle_pressed_specular] = isLight ? Dc::specularOf(colors[S_sliderHandle_pressed])
                                                              : Dc::lightSpecularOf(colors[S_sliderHandle_pressed]);

            colors[S_base_shadow] = Dc::overhangShadowOf(colors[S_base]);
            colors[S_base_divider] = colors[S_window_divider];
            colors[S_windowText_disabled] = pal.color(QPalette::Disabled, QPalette::WindowText);
            colors[S_highlight_outline] = isLight ? Dc::adjustLightness(colors[S_highlight], -0.02)
                                                  : Dc::adjustLightness(colors[S_highlight], 0.05);
            colors[S_highlight_specular] = Dc::specularOf(colors[S_highlight]);
            colors[S_progressBar_outline] = Dc::progressBarOutlineColorOf(pal);
            colors[S_inactiveTabYesFrame] = Dc::inactiveTabFillColorOf(colors[S_tabFrame]);
            colors[S_inactiveTabNoFrame] = Dc::inactiveTabFillColorOf(colors[S_window]);
            colors[S_inactiveTabYesFrame_specular] = Dc::specularOf(colors[S_inactiveTabYesFrame]);
            colors[S_inactiveTabNoFrame_specular] = Dc::specularOf(colors[S_inactiveTabNoFrame]);
            colors[S_indicator_current] = Dc::indicatorColorOf(pal, QPalette::Current);
            colors[S_indicator_disabled] = Dc::indicatorColorOf(pal, QPalette::Disabled);
            colors[S_itemView_multiSelection_currentBorder] = Dc::itemViewMultiSelectionCurrentBorderOf(pal);
            colors[S_itemView_headerOnLine] = Dc::itemViewHeaderOnLineColorOf(pal);
            colors[S_scrollbarGutter_disabled] = colors[S_window];

            brushes[S_none] = Qt::NoBrush;
            for (int i = S_none + 1; i < Num_SwatchColors; ++i) {
                // todo try to reuse
                brushes[i] = colors[i];
            }
            pens[S_none] = Qt::NoPen;
            // QPen::setColor constructs a QBrush behind the scenes, so better to just
            // re-use the ones we already made.
            for (int i = S_none + 1; i < Num_SwatchColors; ++i) {
                pens[i].setBrush(brushes[i]);
                // Width is already 1, don't need to set it. Caps and joins already fine at
                // their defaults, too.
            }

            Grad gutterGrad(Dc::sliderGutterShadowOf(colors[S_scrollbarGutter]), colors[S_scrollbarGutter]);
            for (int i = 0; i < Num_ShadowSteps; ++i) {
                scrollbarShadowColors[i] = gutterGrad.sample(i / static_cast<qreal>(Num_ShadowSteps));
            }
        }

        // This is the "hash" (not really a hash) function we'll use on the happy fast
        // path when looking up a PhSwatch for a given QPalette. It's fragile, because
        // it uses QPalette::cacheKey(), so it may not match even when the contents
        // (currentColorGroup + the RGB colors) of the QPalette are actually a match.
        // But it's cheaper to calculate, so we'll store a single one of these "hashes"
        // for the head (most recently used) cached PhSwatch, and check to see if it
        // matches. This is the most common case, so we can usually save some work by
        // doing this. (The second most common case is probably having a different
        // ColorGroup but the rest of the contents are the same, but we don't have a
        // special path for that.)
        inline quint64 fastfragile_hash_qpalette(const QPalette& p)
        {
            union
            {
                qint64 i;
                quint64 u;
            } x;
            x.i = p.cacheKey();
            // QPalette::ColorGroup has range 0..5 (inclusive), so it only uses 3 bits.
            // The high 32 bits in QPalette::cacheKey() are a global incrementing serial
            // number for the QPalette creation. We don't store (2^29-1) things in our
            // cache, and I doubt that many will ever be created in a real application
            // while also retaining some of them across such a broad time range, so it's
            // really unlikely that repurposing these top 3 bits to also include the
            // QPalette::currentColorGroup() (which the cacheKey doesn't include for some
            // reason...) will generate a collision.
            //
            // This may not be true in the future if the way the QPalette::cacheKey() is
            // generated changes. If that happens, change to use the definition of
            // `fastfragile_hash_qpalette` below, which is less likely to collide with an
            // arbitrarily numbered key but also does more work.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            x.u = x.u ^ (static_cast<quint64>(p.currentColorGroup()) << (64 - 3));
            return x.u;
#else
            // Use this definition here if the contents/layout of QPalette::cacheKey()
            // (as in, the C++ code in qpalette.cpp) are changed. We'll also put a Qt6
            // guard for it, so that it will default to a more safe definition on the
            // next guaranteed big breaking change for Qt. A warning will hopefully get
            // someone to double-check it at some point in the future.
#warning "Verify contents and layout of QPalette::cacheKey() have not changed"
            QtPrivate::QHashCombine c;
            uint h = qHash(p.currentColorGroup());
            h = c(h, (uint)(x.u & 0xFFFFFFFFu));
            h = c(h, (uint)((x.u >> 32) & 0xFFFFFFFFu));
            return h;
#endif
        }

        // This hash function is for when we want an actual accurate hash of a
        // QPalette. QPalette's cacheKey() isn't very reliable -- it seems to change to
        // a new random number whenever it's modified, with the exception of the
        // currentColorGroup being changed. This kind of sucks for us, because it means
        // two QPalette's can have the same contents but hash to different values. And
        // this actually happens a lot! We'll do the hashing ourselves. Also, we're not
        // interested in all of the colors, only some of them, and we ignore
        // pens/brushes.
        uint accurate_hash_qpalette(const QPalette& p)
        {
            // Probably shouldn't use this, could replace with our own guy. It's not a
            // great hasher anyway.
            QtPrivate::QHashCombine c;
            uint h = qHash(p.currentColorGroup());
            QPalette::ColorRole const roles[] = {QPalette::Window,
                                                 QPalette::Button,
                                                 QPalette::Base,
                                                 QPalette::Text,
                                                 QPalette::WindowText,
                                                 QPalette::Highlight,
                                                 QPalette::HighlightedText};
            for (auto role : roles) {
                h = c(h, p.color(role).rgb());
            }
            return h;
        }

        Q_NEVER_INLINE PhSwatchPtr
        deep_getCachedSwatchOfQPalette(PhSwatchCache* cache,
                                       int cacheCount, // Just saving a call to cache->count()
                                       const QPalette& qpalette)
        {
            // Calculate our hash key from the QPalette's current ColorGroup and the
            // actual RGBA values that we use. We have to mix the ColorGroup in
            // ourselves, because QPalette does not account for it in the cache key.
            uint key = accurate_hash_qpalette(qpalette);
            int n = cacheCount;
            int idx = -1;
            for (int i = 0; i < n; ++i) {
                const auto& x = cache->at(i);
                if (x.first == key) {
                    idx = i;
                    break;
                }
            }
            if (idx == -1) {
                PhSwatchPtr ptr;
                if (n < Num_ColorCacheEntries) {
                    ptr = new PhSwatch;
                } else {
                    // Remove the oldest guy from the cache. Remember that because we may
                    // re-enter QStyle functions multiple times when drawing or calculating
                    // something, we may have to load several swaitches derived from
                    // different QPalettes on different stack frames at the same time. But as
                    // an extra cost-savings measure, we'll check and see if something else
                    // has a reference to the removed guy. If there aren't any references to
                    // it, then we'll re-use it directly instead of allocating a new one. (We
                    // will only ever run into the case where we can't re-use it directly if
                    // some other stack frame has a reference to it.) This is nice because
                    // then the QPens and QBrushes don't all also have to reallocate their d
                    // ptr stuff.
                    ptr = cache->last().second;
                    cache->removeLast();
                    ptr.detach();
                }
                ptr->loadFromQPalette(qpalette);
                cache->prepend(PhCacheEntry(key, ptr));
                return ptr;
            } else {
                if (idx == 0) {
                    return cache->at(idx).second;
                }
                PhCacheEntry e = cache->at(idx);
                // Using std::move from algorithm could be more efficient here, but I don't
                // want to depend on algorithm or write this myself. Small N with a movable
                // type means it doesn't really matter in this case.
                cache->remove(idx);
                cache->prepend(e);
                return e.second;
            }
        }

        Q_NEVER_INLINE PhSwatchPtr
        getCachedSwatchOfQPalette(PhSwatchCache* cache,
                                  quint64* headSwatchFastKey, // Optimistic fast-path quick hash key
                                  const QPalette& qpalette)
        {
            quint64 ck = fastfragile_hash_qpalette(qpalette);
            int cacheCount = cache->count();
            // This hint is counter-productive if we're being called in a way that
            // interleaves different QPalettes. But misses to this optimistic path were
            // rare in my tests. (Probably not going to amount to any significant
            // difference, anyway.)
            if (Q_LIKELY(cacheCount > 0 && *headSwatchFastKey == ck)) {
                return cache->at(0).second;
            }
            *headSwatchFastKey = ck;
            return deep_getCachedSwatchOfQPalette(cache, cacheCount, qpalette);
        }

    } // namespace
} // namespace Phantom

class BaseStylePrivate
{
public:
    BaseStylePrivate();

    // A fast'n'easy hash of QPalette::cacheKey()+QPalette::currentColorGroup()
    // of only the head element of swatchCache list. The most common thing that
    // happens when deriving a PhSwatch from a QPalette is that we just end up
    // re-using the last one that we used. For that case, we can potentially save
    // calling `accurate_hash_qpalette()` and instead use the value returned by
    // QPalette::cacheKey() (and QPalette::currentColorGroup()) and compare it to
    // the last one that we used. If it matches, then we know we can just use the
    // head of the cache list without having to do any further checks, which
    // saves a few hundred (!) nanoseconds.
    //
    // However, the `QPalette::cacheKey()` value is fragile and may change even
    // if none of the colors in the QPalette have changed. In other words, all of
    // the colors in a QPalette may match another QPalette (or a derived
    // PhSwatch) even if the `QPalette::cacheKey()` value is different.
    //
    // So if `QPalette::cacheKey()+currentColorGroup()` doesn't match, then we'll
    // use our more accurate `accurate_hash_qpalette()` to get a more accurate
    // comparison key, and then search through the cache list to find a matching
    // cached PhSwatch. (The more accurate cache key is what we store alongside
    // each PhSwatch element, as the `.first` in each QPair. The
    // QPalette::cacheKey() that we associate with the PhSwatch in the head
    // position, `headSwatchFastKey`, is only stored for our single head element,
    // as a special fast case.) If we find it, we'll move it to the head of the
    // cache list. If not, we'll make a new one, and put it at the head. Either
    // way, the `headSwatchFastKey` will be updated to the
    // `fastfragile_qpalette_hash()` of the QPalette that we needed to derive a
    // PhSwatch from, so that if we get called with the same QPalette again next
    // time (which is probably going to be the case), it'll match and we can take
    // the fast path.
    quint64 headSwatchFastKey;

    Phantom::PhSwatchCache swatchCache;
    QPen checkBox_pen_scratch;
};

namespace Phantom
{
    namespace
    {

        // Minimal QPainter save/restore just for pen, brush, and AA render hint. If
        // you touch more than that, this won't help you. But if you're only touching
        // those things, this will save you some typing from manually storing/saving
        // those properties each time.
        struct PSave final
        {
            Q_DISABLE_COPY(PSave)

            explicit PSave(QPainter* painter_)
            {
                Q_ASSERT(painter_);
                painter = painter_;
                pen = painter_->pen();
                brush = painter_->brush();
                hintAA = painter_->testRenderHint(QPainter::Antialiasing);
            }
            Q_NEVER_INLINE void restore()
            {
                QPainter* p = painter;
                if (!p)
                    return;
                bool hintAA_ = hintAA;
                // QPainter will check both pen and brush for equality when setting, so we
                // should set it unconditionally here.
                p->setPen(pen);
                p->setBrush(brush);
                // But it won't check the render hint to guard against doing extra work.
                // We'll do that ourselves. (Though at least for the raster engine, this
                // doesn't cause very much work to occur. But it still chases a few
                // pointers.)
                if (p->testRenderHint(QPainter::Antialiasing) != hintAA_) {
                    p->setRenderHint(QPainter::Antialiasing, hintAA_);
                }
                painter = nullptr;
                pen = QPen();
                brush = QBrush();
                hintAA = false;
            }
            ~PSave()
            {
                restore();
            }

        private:
            QPainter* painter;
            QPen pen;
            QBrush brush;
            bool hintAA;
        };

        const qreal Pi = M_PI;

        qreal dpiScaled(qreal value)
        {
#ifdef Q_OS_MAC
            // On mac the DPI is always 72 so we should not scale it
            return value;
#else
            const qreal scale = qt_defaultDpiX() / 96.0;
            return value * scale;
#endif
        }

        struct MenuItemMetrics
        {
            int fontHeight;
            int frameThickness;
            int leftMargin;
            int rightMarginForText;
            int rightMarginForArrow;
            int topMargin;
            int bottomMargin;
            int checkWidth;
            int checkRightSpace;
            int iconRightSpace;
            int mnemonicSpace;
            int arrowSpace;
            int arrowWidth;
            int separatorHeight;
            int totalHeight;

            static MenuItemMetrics ofFontHeight(int fontHeight);

        private:
            MenuItemMetrics()
            {
            }
        };

        MenuItemMetrics MenuItemMetrics::ofFontHeight(int fontHeight)
        {
            MenuItemMetrics m;
            m.fontHeight = fontHeight;
            m.frameThickness = dpiScaled(1.0);
            m.leftMargin = static_cast<int>(fontHeight * MenuItem_LeftMarginFontRatio);
            m.rightMarginForText = static_cast<int>(fontHeight * MenuItem_RightMarginForTextFontRatio);
            m.rightMarginForArrow = static_cast<int>(fontHeight * MenuItem_RightMarginForArrowFontRatio);
            m.topMargin = static_cast<int>(fontHeight * MenuItem_VerticalMarginsFontRatio);
            m.bottomMargin = static_cast<int>(fontHeight * MenuItem_VerticalMarginsFontRatio);
            int checkVMargin = static_cast<int>(fontHeight * MenuItem_CheckMarkVerticalInsetFontRatio);
            int checkHeight = fontHeight - checkVMargin * 2;
            if (checkHeight < 0)
                checkHeight = 0;
            m.checkWidth = static_cast<int>(checkHeight * CheckMark_WidthOfHeightScale);
            m.checkRightSpace = static_cast<int>(fontHeight * MenuItem_CheckRightSpaceFontRatio);
            m.iconRightSpace = static_cast<int>(fontHeight * MenuItem_IconRightSpaceFontRatio);
            m.mnemonicSpace = static_cast<int>(fontHeight * MenuItem_TextMnemonicSpaceFontRatio);
            m.arrowSpace = static_cast<int>(fontHeight * MenuItem_SubMenuArrowSpaceFontRatio);
            m.arrowWidth = static_cast<int>(fontHeight * MenuItem_SubMenuArrowWidthFontRatio);
            m.separatorHeight = static_cast<int>(fontHeight * MenuItem_SeparatorHeightFontRatio);
            // Odd numbers only
            m.separatorHeight = (m.separatorHeight / 2) * 2 + 1;
            m.totalHeight = fontHeight + m.frameThickness * 2 + m.topMargin + m.bottomMargin;
            return m;
        }

        QRect menuItemContentRect(const MenuItemMetrics& metrics, QRect itemRect, bool hasArrow)
        {
            QRect r = itemRect;
            int ft = metrics.frameThickness;
            int rm = hasArrow ? metrics.rightMarginForArrow : metrics.rightMarginForText;
            r.adjust(ft + metrics.leftMargin, ft + metrics.topMargin, -(ft + rm), -(ft + metrics.bottomMargin));
            return r.isValid() ? r : QRect();
        }
        QRect
        menuItemCheckRect(const MenuItemMetrics& metrics, Qt::LayoutDirection direction, QRect itemRect, bool hasArrow)
        {
            QRect r = menuItemContentRect(metrics, itemRect, hasArrow);
            int checkVMargin = static_cast<int>(metrics.fontHeight * MenuItem_CheckMarkVerticalInsetFontRatio);
            if (checkVMargin < 0)
                checkVMargin = 0;
            r.setSize(QSize(metrics.checkWidth, metrics.fontHeight));
            r.adjust(0, checkVMargin, 0, -checkVMargin);
            return QStyle::visualRect(direction, itemRect, r) & itemRect;
        }
        QRect
        menuItemIconRect(const MenuItemMetrics& metrics, Qt::LayoutDirection direction, QRect itemRect, bool hasArrow)
        {
            QRect r = menuItemContentRect(metrics, itemRect, hasArrow);
            r.setX(r.x() + metrics.checkWidth + metrics.checkRightSpace);
            r.setSize(QSize(metrics.fontHeight, metrics.fontHeight));
            return QStyle::visualRect(direction, itemRect, r) & itemRect;
        }
        QRect menuItemTextRect(const MenuItemMetrics& metrics,
                               Qt::LayoutDirection direction,
                               QRect itemRect,
                               bool hasArrow,
                               bool hasIcon,
                               int tabWidth)
        {
            QRect r = menuItemContentRect(metrics, itemRect, hasArrow);
            r.setX(r.x() + metrics.checkWidth + metrics.checkRightSpace);
            if (hasIcon) {
                r.setX(r.x() + metrics.fontHeight + metrics.iconRightSpace);
            }
            r.setWidth(r.width() - tabWidth);
            r.setHeight(metrics.fontHeight);
            r &= itemRect;
            return QStyle::visualRect(direction, itemRect, r);
        }
        QRect menuItemMnemonicRect(const MenuItemMetrics& metrics,
                                   Qt::LayoutDirection direction,
                                   QRect itemRect,
                                   bool hasArrow,
                                   int tabWidth)
        {
            QRect r = menuItemContentRect(metrics, itemRect, hasArrow);
            int x = r.x() + r.width() - tabWidth;
            if (hasArrow)
                x -= metrics.arrowSpace + metrics.arrowWidth;
            r.setX(x);
            r.setHeight(metrics.fontHeight);
            r &= itemRect;
            return QStyle::visualRect(direction, itemRect, r);
        }
        QRect menuItemArrowRect(const MenuItemMetrics& metrics, Qt::LayoutDirection direction, QRect itemRect)
        {
            QRect r = menuItemContentRect(metrics, itemRect, true);
            int x = r.x() + r.width() - metrics.arrowWidth;
            r.setX(x);
            r &= itemRect;
            return QStyle::visualRect(direction, itemRect, r);
        }

        Q_NEVER_INLINE
        void progressBarFillRects(const QStyleOptionProgressBar* bar,
                                  // The rect that represents the filled/completed region
                                  QRect& outFilled,
                                  // The rect that represents the incomplete region
                                  QRect& outNonFilled,
                                  // Whether or not the progress bar is indeterminate
                                  bool& outIsIndeterminate)
        {
            QRect ra = bar->rect;
            QRect rb = ra;
            bool isHorizontal = bar->orientation != Qt::Vertical;
            bool isInverted = bar->invertedAppearance;
            bool isIndeterminate = bar->minimum == 0 && bar->maximum == 0;
            bool isForward = !isHorizontal || bar->direction != Qt::RightToLeft;
            if (isInverted)
                isForward = !isForward;
            int maxLen = isHorizontal ? ra.width() : ra.height();
            const auto availSteps = qMax(Q_INT64_C(1), qint64(bar->maximum) - bar->minimum);
            const auto progress = qMax(bar->progress, bar->minimum); // workaround for bug in QProgressBar
            const auto progressSteps = qint64(progress) - bar->minimum;
            const auto progressBarWidth = progressSteps * maxLen / availSteps;
            int barLen = isIndeterminate ? maxLen : progressBarWidth;
            if (isHorizontal) {
                if (isForward) {
                    ra.setWidth(barLen);
                    rb.setX(barLen);
                } else {
                    ra.setX(ra.x() + ra.width() - barLen);
                    rb.setWidth(rb.width() - barLen);
                }
            } else {
                if (isForward) {
                    ra.setY(ra.y() + ra.height() - barLen);
                    rb.setHeight(rb.height() - barLen);
                } else {
                    ra.setHeight(barLen);
                    rb.setY(barLen);
                }
            }
            outFilled = ra;
            outNonFilled = rb;
            outIsIndeterminate = isIndeterminate;
        }

        int calcBigLineSize(int radius)
        {
            int bigLineSize = radius / 6;
            if (bigLineSize < 4)
                bigLineSize = 4;
            if (bigLineSize > radius / 2)
                bigLineSize = radius / 2;
            return bigLineSize;
        }
        Q_NEVER_INLINE QPointF calcRadialPos(const QStyleOptionSlider* dial, qreal offset)
        {
            const int width = dial->rect.width();
            const int height = dial->rect.height();
            const int r = qMin(width, height) / 2;
            const int currentSliderPosition =
                dial->upsideDown ? dial->sliderPosition : (dial->maximum - dial->sliderPosition);
            qreal a = 0;
            if (dial->maximum == dial->minimum)
                a = Pi / 2;
            else if (dial->dialWrapping)
                a = Pi * 3 / 2 - (currentSliderPosition - dial->minimum) * 2 * Pi / (dial->maximum - dial->minimum);
            else
                a = (Pi * 8 - (currentSliderPosition - dial->minimum) * 10 * Pi / (dial->maximum - dial->minimum)) / 6;
            qreal xc = width / 2.0;
            qreal yc = height / 2.0;
            qreal len = r - calcBigLineSize(r) - 3;
            qreal back = offset * len;
            QPointF pos(QPointF(xc + back * qCos(a), yc - back * qSin(a)));
            return pos;
        }
        Q_NEVER_INLINE QPolygonF calcLines(const QStyleOptionSlider* dial)
        {
            QPolygonF poly;
            qreal width = dial->rect.width();
            qreal height = dial->rect.height();
            qreal r = qMin(width, height) / 2.0;
            int bigLineSize = calcBigLineSize(r);

            qreal xc = width / 2.0 + 0.5;
            qreal yc = height / 2.0 + 0.5;
            const int ns = dial->tickInterval;
            if (!ns) // Invalid values may be set by Qt Designer.
                return poly;
            int notches = (dial->maximum + ns - 1 - dial->minimum) / ns;
            if (notches <= 0)
                return poly;
            if (dial->maximum < dial->minimum || dial->maximum - dial->minimum > 1000) {
                int maximum = dial->minimum + 1000;
                notches = (maximum + ns - 1 - dial->minimum) / ns;
            }
            poly.resize(2 + 2 * notches);
            int smallLineSize = bigLineSize / 2;
            for (int i = 0; i <= notches; ++i) {
                qreal angle =
                    dial->dialWrapping ? Pi * 3 / 2 - i * 2 * Pi / notches : (Pi * 8 - i * 10 * Pi / notches) / 6;
                qreal s = qSin(angle);
                qreal c = qCos(angle);
                if (i == 0 || (((ns * i) % (dial->pageStep ? dial->pageStep : 1)) == 0)) {
                    poly[2 * i] = QPointF(xc + (r - bigLineSize) * c, yc - (r - bigLineSize) * s);
                    poly[2 * i + 1] = QPointF(xc + r * c, yc - r * s);
                } else {
                    poly[2 * i] = QPointF(xc + (r - 1 - smallLineSize) * c, yc - (r - 1 - smallLineSize) * s);
                    poly[2 * i + 1] = QPointF(xc + (r - 1) * c, yc - (r - 1) * s);
                }
            }
            return poly;
        }
        // This will draw a nice and shiny QDial for us. We don't want
        // all the shinyness in QWindowsStyle, hence we place it here
        Q_NEVER_INLINE void drawDial(const QStyleOptionSlider* option, QPainter* painter)
        {
            namespace Dc = Phantom::DeriveColors;
            const QPalette& pal = option->palette;
            QColor buttonColor = Dc::buttonColor(option->palette);
            const int width = option->rect.width();
            const int height = option->rect.height();
            const bool enabled = option->state & QStyle::State_Enabled;
            qreal r = qMin(width, height) / 2.0;
            r -= r / 50.0;
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            // Draw notches
            if (option->subControls & QStyle::SC_DialTickmarks) {
                painter->setPen(pal.color(QPalette::Disabled, QPalette::Text));
                painter->drawLines(calcLines(option));
            }
            const qreal d_ = r / 6;
            const qreal dx = option->rect.x() + d_ + (width - 2 * r) / 2 + 1;
            const qreal dy = option->rect.y() + d_ + (height - 2 * r) / 2 + 1;
            QRectF br = QRectF(dx + 0.5, dy + 0.5, int(r * 2 - 2 * d_ - 2), int(r * 2 - 2 * d_ - 2));
            if (enabled) {
                painter->setBrush(buttonColor);
            } else {
                painter->setBrush(Qt::NoBrush);
            }
            painter->setPen(Dc::outlineOf(option->palette));
            painter->drawEllipse(br);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Dc::specularOf(buttonColor));
            painter->drawEllipse(br.adjusted(1, 1, -1, -1));
            if (option->state & QStyle::State_HasFocus) {
                QColor highlight = pal.highlight().color();
                highlight.setHsv(highlight.hue(), qMin(160, highlight.saturation()), qMax(230, highlight.value()));
                highlight.setAlpha(127);
                painter->setPen(QPen(highlight, 2.0));
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(br.adjusted(-1, -1, 1, 1));
            }
            QPointF dp = calcRadialPos(option, 0.70);
            const qreal ds = r / 7.0;
            QRectF dialRect(dp.x() - ds, dp.y() - ds, 2 * ds, 2 * ds);
            painter->setBrush(option->palette.color(QPalette::Window));
            painter->setPen(Dc::outlineOf(option->palette));
            painter->drawEllipse(dialRect.adjusted(-1, -1, 1, 1));
            painter->restore();
        }

        int fontMetricsWidth(const QFontMetrics& fontMetrics, const QString& text)
        {
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
            return fontMetrics.width(text, text.size(), Qt::TextBypassShaping);
#else
            return fontMetrics.horizontalAdvance(text);
#endif
        }

        // This always draws the arrow with the correct aspect ratio, even if the
        // provided bounding rect is non-square. The base edge of the triangle is
        // snapped to a whole pixel to avoid anti-aliasing making it look soft.
        //
        // Expected time (release): 5usecs for regular-sized arrows
        Q_NEVER_INLINE void drawArrow(QPainter* p, QRect rect, Qt::ArrowType arrowDirection, const QBrush& brush)
        {
            const qreal ArrowBaseRatio = 0.70;
            qreal irx, iry, irw, irh;
            QRectF(rect).getRect(&irx, &iry, &irw, &irh);
            if (irw < 1.0 || irh < 1.0)
                return;
            qreal dw, dh;
            if (arrowDirection == Qt::LeftArrow || arrowDirection == Qt::RightArrow) {
                dw = ArrowBaseRatio;
                dh = 1.0;
            } else {
                dw = 1.0;
                dh = ArrowBaseRatio;
            }
            QSizeF sz = QSizeF(dw, dh).scaled(irw, irh, Qt::KeepAspectRatio);
            qreal aw = sz.width();
            qreal ah = sz.height();
            qreal ax, ay;
            ax = irx + (irw - aw) / 2;
            ay = iry + (irh - ah) / 2;
            QRectF arrowRect(ax, ay, aw, ah);
            QPointF points[3];
            switch (arrowDirection) {
            case Qt::DownArrow:
                arrowRect.setTop(std::round(arrowRect.top()));
                points[0] = arrowRect.topLeft();
                points[1] = arrowRect.topRight();
                points[2] = QPointF(arrowRect.center().x(), arrowRect.bottom());
                break;
            case Qt::RightArrow: {
                arrowRect.setLeft(std::round(arrowRect.left()));
                points[0] = arrowRect.topLeft();
                points[1] = arrowRect.bottomLeft();
                points[2] = QPointF(arrowRect.right(), arrowRect.center().y());
                break;
            }
            case Qt::LeftArrow:
                arrowRect.setRight(std::round(arrowRect.right()));
                points[0] = arrowRect.topRight();
                points[1] = arrowRect.bottomRight();
                points[2] = QPointF(arrowRect.left(), arrowRect.center().y());
                break;
            case Qt::UpArrow:
            default:
                arrowRect.setBottom(std::round(arrowRect.bottom()));
                points[0] = arrowRect.bottomLeft();
                points[1] = arrowRect.bottomRight();
                points[2] = QPointF(arrowRect.center().x(), arrowRect.top());
                break;
            }
            auto oldPen = p->pen();
            auto oldBrush = p->brush();
            bool oldAA = p->testRenderHint(QPainter::Antialiasing);
            p->setPen(Qt::NoPen);
            p->setBrush(brush);
            if (!oldAA) {
                p->setRenderHint(QPainter::Antialiasing);
            }
            p->drawConvexPolygon(points, 3);
            p->setPen(oldPen);
            p->setBrush(oldBrush);
            if (!oldAA) {
                p->setRenderHint(QPainter::Antialiasing, false);
            }
        }

        // Pass allowEnabled as false to always draw the arrow with the disabled color,
        // even if the underlying palette's current color group is not disabled. Useful
        // for parts of widgets which may want to be drawn as disabled even if the
        // actual widget is not set as disabled, such as scrollbar step buttons when
        // the scrollbar has no movable range.
        Q_NEVER_INLINE void drawArrow(QPainter* painter,
                                      QRect rect,
                                      Qt::ArrowType type,
                                      const PhSwatch& swatch,
                                      bool allowEnabled = true,
                                      qreal lightnessAdjustment = 0.0)
        {
            if (rect.isEmpty())
                return;
            using namespace SwatchColors;
            auto brush = swatch.brush(allowEnabled ? S_indicator_current : S_indicator_disabled);
            brush.setColor(DeriveColors::adjustLightness(brush.color(), lightnessAdjustment));
            Phantom::drawArrow(painter, rect, type, brush);
        }

        // This draws exactly within the rect provided. If you provide a square rect,
        // it will appear too wide -- you probably want to shrink the width of your
        // square first by multiplying it with CheckMark_WidthOfHeightScale.
        Q_NEVER_INLINE void
        drawCheck(QPainter* painter, QPen& scratchPen, const QRectF& r, const PhSwatch& swatch, Swatchy color)
        {
            using namespace Phantom::SwatchColors;
            qreal rx, ry, rw, rh;
            QRectF(r).getRect(&rx, &ry, &rw, &rh);
            qreal penWidth = 0.25 * qMin(rw, rh);
            qreal dimx = rw - penWidth;
            qreal dimy = rh - penWidth;
            if (dimx < 0.5 || dimy < 0.5)
                return;
            qreal x = (rw - dimx) / 2 + rx;
            qreal y = (rh - dimy) / 2 + ry;
            QPointF points[3];
            points[0] = QPointF(0.0, 0.55);
            points[1] = QPointF(0.4, 1.0);
            points[2] = QPointF(1.0, 0);
            for (int i = 0; i < 3; ++i) {
                QPointF pnt = points[i];
                pnt.setX(pnt.x() * dimx + x);
                pnt.setY(pnt.y() * dimy + y);
                points[i] = pnt;
            }
            scratchPen.setBrush(swatch.brush(color));
            scratchPen.setCapStyle(Qt::RoundCap);
            scratchPen.setJoinStyle(Qt::RoundJoin);
            scratchPen.setWidthF(penWidth);
            Phantom::PSave save(painter);
            if (!painter->testRenderHint(QPainter::Antialiasing))
                painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(scratchPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPolyline(points, 3);
        }

        Q_NEVER_INLINE void
        drawHyphen(QPainter* painter, QPen& scratchPen, const QRectF& r, const PhSwatch& swatch, Swatchy color)
        {
            using namespace Phantom::SwatchColors;
            qreal rx, ry, rw, rh;
            QRectF(r).getRect(&rx, &ry, &rw, &rh);
            qreal penWidth = 0.25 * qMin(rw, rh);
            qreal dimx = rw - penWidth;
            qreal dimy = rh - penWidth;
            if (dimx < 0.5 || dimy < 0.5)
                return;
            qreal x = (rw - dimx) / 2 + rx;
            qreal y = (rh - dimy) / 2 + ry;
            QPointF p0(0.0 * dimx + x, 0.5 * dimy + y);
            QPointF p1(1.0 * dimx + x, 0.5 * dimy + y);
            scratchPen.setBrush(swatch.brush(color));
            scratchPen.setCapStyle(Qt::RoundCap);
            scratchPen.setWidthF(penWidth);
            Phantom::PSave save(painter);
            if (!painter->testRenderHint(QPainter::Antialiasing))
                painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(scratchPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawLine(p0, p1);
        }

        Q_NEVER_INLINE void
        drawMdiButton(QPainter* painter, const QStyleOptionTitleBar* option, QRect tmp, bool hover, bool sunken)
        {
            QColor dark;
            dark.setHsv(option->palette.button().color().hue(),
                        qMin<int>(255, (option->palette.button().color().saturation())),
                        qMin<int>(255, option->palette.button().color().value() * 0.7));
            QColor highlight = option->palette.highlight().color();
            bool active = (option->titleBarState & QStyle::State_Active);
            QColor titleBarHighlight(255, 255, 255, 60);
            if (sunken)
                painter->fillRect(tmp.adjusted(1, 1, -1, -1), option->palette.highlight().color().darker(120));
            else if (hover)
                painter->fillRect(tmp.adjusted(1, 1, -1, -1), QColor(255, 255, 255, 20));
            if (sunken)
                titleBarHighlight = highlight.darker(130);
            QColor mdiButtonBorderColor(active ? option->palette.highlight().color().darker(180) : dark.darker(110));
            painter->setPen(QPen(mdiButtonBorderColor));
            const QLine lines[4] = {QLine(tmp.left() + 2, tmp.top(), tmp.right() - 2, tmp.top()),
                                    QLine(tmp.left() + 2, tmp.bottom(), tmp.right() - 2, tmp.bottom()),
                                    QLine(tmp.left(), tmp.top() + 2, tmp.left(), tmp.bottom() - 2),
                                    QLine(tmp.right(), tmp.top() + 2, tmp.right(), tmp.bottom() - 2)};
            painter->drawLines(lines, 4);
            const QPoint points[4] = {QPoint(tmp.left() + 1, tmp.top() + 1),
                                      QPoint(tmp.right() - 1, tmp.top() + 1),
                                      QPoint(tmp.left() + 1, tmp.bottom() - 1),
                                      QPoint(tmp.right() - 1, tmp.bottom() - 1)};
            painter->drawPoints(points, 4);
            painter->setPen(titleBarHighlight);
            painter->drawLine(tmp.left() + 2, tmp.top() + 1, tmp.right() - 2, tmp.top() + 1);
            painter->drawLine(tmp.left() + 1, tmp.top() + 2, tmp.left() + 1, tmp.bottom() - 2);
        }

        Q_NEVER_INLINE void fillRectOutline(QPainter* p, QRect rect, QMargins margins, const QColor& brush)
        {
            int x, y, w, h;
            rect.getRect(&x, &y, &w, &h);
            int ml = margins.left();
            int mt = margins.top();
            int mr = margins.right();
            int mb = margins.bottom();
            QRect r0(x, y, w, mt);
            QRect r1(x, y + mt, ml, h - (mt + mb));
            QRect r2((x + w) - mr, y + mt, mr, h - (mt + mb));
            QRect r3(x, (y + h) - mb, w, mb);
            p->fillRect(r0 & rect, brush);
            p->fillRect(r1 & rect, brush);
            p->fillRect(r2 & rect, brush);
            p->fillRect(r3 & rect, brush);
        }
        void fillRectOutline(QPainter* p, QRect rect, int thickness, const QColor& color)
        {
            fillRectOutline(p, rect, QMargins(thickness, thickness, thickness, thickness), color);
        }
        Q_NEVER_INLINE void
        fillRectEdges(QPainter* p, QRect rect, Qt::Edges edges, QMargins margins, const QColor& color)
        {
            int x, y, w, h;
            rect.getRect(&x, &y, &w, &h);
            if (edges & Qt::LeftEdge) {
                int ml = margins.left();
                QRect r0(x, y, ml, h);
                p->fillRect(r0 & rect, color);
            }
            if (edges & Qt::TopEdge) {
                int mt = margins.top();
                QRect r1(x, y, w, mt);
                p->fillRect(r1 & rect, color);
            }
            if (edges & Qt::RightEdge) {
                int mr = margins.right();
                QRect r2((x + w) - mr, y, mr, h);
                p->fillRect(r2 & rect, color);
            }
            if (edges & Qt::BottomEdge) {
                int mb = margins.bottom();
                QRect r3(x, (y + h) - mb, w, mb);
                p->fillRect(r3 & rect, color);
            }
        }
        void fillRectEdges(QPainter* p, QRect rect, Qt::Edges edges, int thickness, const QColor& color)
        {
            fillRectEdges(p, rect, edges, QMargins(thickness, thickness, thickness, thickness), color);
        }
        inline QRect expandRect(QRect rect, Qt::Edges edges, int delta)
        {
            int l = edges & Qt::LeftEdge ? -delta : 0;
            int t = edges & Qt::TopEdge ? -delta : 0;
            int r = edges & Qt::RightEdge ? delta : 0;
            int b = edges & Qt::BottomEdge ? delta : 0;
            return rect.adjusted(l, t, r, b);
        }
        inline Qt::Edge oppositeEdge(Qt::Edge edge)
        {
            switch (edge) {
            case Qt::LeftEdge:
                return Qt::RightEdge;
            case Qt::TopEdge:
                return Qt::BottomEdge;
            case Qt::RightEdge:
                return Qt::LeftEdge;
            case Qt::BottomEdge:
                return Qt::TopEdge;
            }
            return Qt::TopEdge;
        }
        inline QRect rectTranslatedTowardEdge(QRect rect, Qt::Edge edge, int delta)
        {
            switch (edge) {
            case Qt::LeftEdge:
                return rect.translated(-delta, 0);
            case Qt::TopEdge:
                return rect.translated(0, -delta);
            case Qt::RightEdge:
                return rect.translated(delta, 0);
            case Qt::BottomEdge:
                return rect.translated(0, delta);
            }
            return rect;
        }
        Q_NEVER_INLINE QRect rectFromInnerEdgeWithThickness(QRect rect, Qt::Edge edge, int thickness)
        {
            int x, y, w, h;
            rect.getRect(&x, &y, &w, &h);
            QRect r;
            switch (edge) {
            case Qt::LeftEdge:
                r = QRect(x, y, thickness, h);
                break;
            case Qt::TopEdge:
                r = QRect(x, y, w, thickness);
                break;
            case Qt::RightEdge:
                r = QRect((x + w) - thickness, y, thickness, h);
                break;
            case Qt::BottomEdge:
                r = QRect(x, (y + h) - thickness, w, thickness);
                break;
            }
            return r & rect;
        }
        Q_NEVER_INLINE void
        paintSolidRoundRect(QPainter* p, QRect rect, qreal radius, const PhSwatch& swatch, Swatchy fill)
        {
            if (!fill)
                return;
            bool aa = p->testRenderHint(QPainter::Antialiasing);
            if (radius > 0.5) {
                if (!aa)
                    p->setRenderHint(QPainter::Antialiasing);
                p->setPen(swatch.pen(SwatchColors::S_none));
                p->setBrush(swatch.brush(fill));
                p->drawRoundedRect(rect, radius, radius);
            } else {
                if (aa)
                    p->setRenderHint(QPainter::Antialiasing, false);
                p->fillRect(rect, swatch.color(fill));
            }
        }
        Q_NEVER_INLINE void paintBorderedRoundRect(QPainter* p,
                                                   QRect rect,
                                                   qreal radius,
                                                   const PhSwatch& swatch,
                                                   Swatchy stroke,
                                                   Swatchy fill)
        {
            if (rect.width() < 1 || rect.height() < 1)
                return;
            if (!stroke && !fill)
                return;
            bool aa = p->testRenderHint(QPainter::Antialiasing);
            if (radius > 0.5) {
                if (!aa)
                    p->setRenderHint(QPainter::Antialiasing);
                p->setPen(swatch.pen(stroke));
                p->setBrush(swatch.brush(fill));
                QRectF rf(rect.x() + 0.5, rect.y() + 0.5, rect.width() - 1.0, rect.height() - 1.0);
                p->drawRoundedRect(rf, radius, radius);
            } else {
                if (aa)
                    p->setRenderHint(QPainter::Antialiasing, false);
                if (stroke) {
                    fillRectOutline(p, rect, 1, swatch.color(stroke));
                }
                if (fill) {
                    p->fillRect(rect.adjusted(1, 1, -1, -1), swatch.color(fill));
                }
            }
        }
    } // namespace
} // namespace Phantom

BaseStylePrivate::BaseStylePrivate()
    : headSwatchFastKey(0)
{
}

BaseStyle::BaseStyle()
    : d(new BaseStylePrivate),
      m_shadowHelper(new ShadowHelper(this)),
      m_blurHelper(new BlurHelper(this))
{
    setObjectName(QLatin1String("Phantom"));

    m_shadowHelper->setFrameRadius(Phantom::DefaultFrame_Radius);
    // Call initializeBaseColor() during initialization
    initializeBaseColor(this);
}

BaseStyle::~BaseStyle()
{
    delete d;
}

// Draw text in a rectangle. The current pen set on the painter is used, unless
// an explicit textRole is set, in which case the palette will be used. The
// enabled bool indicates whether the text is enabled or not, and can influence
// how the text is drawn outside of just color. Wrapping and alignment flags
// can be passed in `alignment`.
void BaseStyle::drawItemText(QPainter* painter,
                             const QRect& rect,
                             int alignment,
                             const QPalette& pal,
                             bool enabled,
                             const QString& text,
                             QPalette::ColorRole textRole) const
{
    Q_UNUSED(enabled);
    if (text.isEmpty())
        return;
    if (textRole == QPalette::NoRole) {
        painter->drawText(rect, alignment, text);
        return;
    }
    QPen savedPen = painter->pen();
    const QBrush& newBrush = pal.brush(textRole);
    bool changed = false;
    if (savedPen.brush() != newBrush) {
        changed = true;
        painter->setPen(QPen(newBrush, savedPen.widthF()));
    }
    painter->drawText(rect, alignment, text);
    if (changed) {
        painter->setPen(savedPen);
    }
}

void BaseStyle::drawPrimitive(PrimitiveElement elem,
                              const QStyleOption* option,
                              QPainter* painter,
                              const QWidget* widget) const
{
    Q_ASSERT(option);
    if (!option)
        return;
#ifdef BUILD_WITH_EASY_PROFILER
    EASY_BLOCK("drawPrimitive");
    const char* elemCString = QMetaEnum::fromType<QStyle::PrimitiveElement>().valueToKey(elem);
    EASY_TEXT("Element", elemCString);
#endif
    using Swatchy = Phantom::Swatchy;
    using namespace Phantom::SwatchColors;
    namespace Ph = Phantom;
    auto ph_swatchPtr = getCachedSwatchOfQPalette(&d->swatchCache, &d->headSwatchFastKey, option->palette);
    const Ph::PhSwatch& swatch = *ph_swatchPtr.data();
    const int state = option->state;
    // Cast to int here to suppress warnings about cases listed which are not in
    // the original enum. This is for custom primitive elements.
    switch (static_cast<int>(elem)) {
    case PE_Frame: {
        if (widget && widget->inherits("QComboBoxPrivateContainer")) {
            QStyleOption copy = *option;
            copy.state |= State_Raised;
            proxy()->drawPrimitive(PE_PanelMenu, &copy, painter, widget);
            break;
        }
        Ph::fillRectOutline(painter, option->rect, 1, swatch.color(S_frame_outline));
        break;
    }
    case PE_FrameMenu: {
        break;
    }
    case PE_FrameDockWidget: {
        painter->save();
        QColor softshadow = option->palette.window().color().darker(120);
        QRect r = option->rect;
        painter->setPen(softshadow);
        painter->drawRect(r.adjusted(0, 0, -1, -1));
        painter->setPen(QPen(option->palette.light(), 1));
        painter->drawLine(QPoint(r.left() + 1, r.top() + 1), QPoint(r.left() + 1, r.bottom() - 1));
        painter->setPen(QPen(option->palette.window().color().darker(120)));
        painter->drawLine(QPoint(r.left() + 1, r.bottom() - 1), QPoint(r.right() - 2, r.bottom() - 1));
        painter->drawLine(QPoint(r.right() - 1, r.top() + 1), QPoint(r.right() - 1, r.bottom() - 1));
        painter->restore();
        break;
    }
    case PE_FrameGroupBox: {
        QRect frame = option->rect;
        Ph::PSave save(painter);
        bool isFlat = false;
        if (auto groupBox = qstyleoption_cast<const QStyleOptionGroupBox*>(option)) {
            isFlat = groupBox->features & QStyleOptionFrame::Flat;
        } else if (auto frameOpt = qstyleoption_cast<const QStyleOptionFrame*>(option)) {
            isFlat = frameOpt->features & QStyleOptionFrame::Flat;
        }
        if (isFlat) {
            Ph::fillRectEdges(painter, frame, Qt::TopEdge, 1, swatch.color(S_window_divider));
        } else {
            Ph::paintBorderedRoundRect(painter, frame, Ph::GroupBox_Rounding, swatch, S_frame_outline, S_none);
        }
        break;
    }
    case PE_IndicatorBranch: {
        if (!(option->state & State_Children))
            break;
        Qt::ArrowType arrow;
        if (option->state & State_Open) {
            arrow = Qt::DownArrow;
        } else if (option->direction != Qt::RightToLeft) {
            arrow = Qt::RightArrow;
        } else {
            arrow = Qt::LeftArrow;
        }
        bool useSelectionColor = false;
        if (option->state & State_Selected) {
            if (auto ivopt = qstyleoption_cast<const QStyleOptionViewItem*>(option)) {
                useSelectionColor = ivopt->showDecorationSelected;
            }
        }
        Swatchy color = useSelectionColor ? S_highlightedText : S_indicator_current;
        QRect r = option->rect;
        if (Ph::BranchesOnEdge) {
            // TODO RTL
            r.moveLeft(0);
            if (r.width() < r.height())
                r.setWidth(r.height());
        }
        int adj = qMin(r.width(), r.height()) / 4;
        r.adjust(adj, adj, -adj, -adj);
        Ph::drawArrow(painter, r, arrow, swatch.brush(color));
        break;
    }
    case PE_IndicatorMenuCheckMark: {
        // For this PE, QCommonStyle treats State_On as drawing the check with the
        // highlighted text color, and otherwise with the regular text color. I
        // guess we should match that behavior, even though it's not consistent
        // with other check box/mark drawing in QStyle (buttons and item view
        // items.) QCommonStyle also doesn't care about tri-state or unchecked
        // states -- it seems that if you call this, you want a check, and nothing
        // else.
        //
        // We'll also catch State_Selected and treat it equivalently (the way you'd
        // expect.) We'll use windowText instead of text, though -- probably
        // doesn't matter.
        Swatchy fgColor = S_windowText;
        bool isSelected = option->state & (State_Selected | State_On);
        bool isEnabled = option->state & State_Enabled;
        if (isSelected) {
            fgColor = S_highlightedText;
        } else if (!isEnabled) {
            fgColor = S_windowText_disabled;
        }
        qreal rx, ry, rw, rh;
        QRectF(option->rect).getRect(&rx, &ry, &rw, &rh);
        qreal dim = qMin(rw, rh);
        const qreal insetScale = 0.8;
        qreal dimx = dim * insetScale * Ph::CheckMark_WidthOfHeightScale;
        qreal dimy = dim * insetScale;
        QRectF r_(rx + (rw - dimx) / 2, ry + (rh - dimy) / 2, dimx, dimy);
        Ph::drawCheck(painter, d->checkBox_pen_scratch, r_, swatch, fgColor);
        break;
    }
    case PE_PanelTipLabel: {
        // force registration of widget
        if (widget && widget->window()) {
            m_shadowHelper->registerWidget(widget->window(), true);
        }

        const auto &palette = option->palette;
        const auto &background = palette.color(QPalette::ToolTipBase);
        const qreal radius = Phantom::DefaultFrame_Radius;
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(background);
        painter->drawRoundedRect(option->rect, radius, radius);
        painter->restore();
        break;
    }
        // Called for the content area on tree view rows that are selected
    case PE_PanelItemViewItem: {
        QCommonStyle::drawPrimitive(elem, option, painter, widget);
        break;
    }
        // Called for left-of-item-content-area on tree view rows that are selected
    case PE_PanelItemViewRow: {
        QCommonStyle::drawPrimitive(elem, option, painter, widget);
        break;
    }
    case PE_FrameTabBarBase: {
        auto tbb = qstyleoption_cast<const QStyleOptionTabBarBase*>(option);
        if (!tbb)
            break;
        Qt::Edge edge = Qt::TopEdge;
        switch (tbb->shape) {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
            edge = Qt::TopEdge;
            break;
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
            edge = Qt::BottomEdge;
            break;
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
            edge = Qt::LeftEdge;
            break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
            edge = Qt::RightEdge;
            break;
        }
        Ph::fillRectEdges(painter, option->rect, edge, 1, swatch.color(S_frame_outline));
        // TODO need to check here if we're drawing with window or button color as
        // the frame fill. Assuming window right now, but could be wrong.
        Ph::fillRectEdges(painter, Ph::expandRect(option->rect, edge, -1), edge, 1, swatch.color(S_tabFrame_specular));
        break;
    }
    case PE_PanelScrollAreaCorner: {
        bool isLeftToRight = option->direction != Qt::RightToLeft;
        Qt::Edges edges = Qt::TopEdge;
        QRect bgRect = option->rect;
        if (isLeftToRight) {
            edges |= Qt::LeftEdge;
            bgRect.setX(bgRect.x() + 1);
        } else {
            edges |= Qt::RightEdge;
            bgRect.setWidth(bgRect.width() - 1);
        }
        painter->fillRect(bgRect, swatch.color(S_window));
        Ph::fillRectEdges(painter, option->rect, edges, 1, swatch.color(S_window_outline));
        break;
    }
    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowLeft: {
        int rx, ry, rw, rh;
        option->rect.getRect(&rx, &ry, &rw, &rh);
        if (rw <= 1 || rh <= 1)
            break;
        Qt::ArrowType arrow = Qt::UpArrow;
        switch (elem) {
        case PE_IndicatorArrowUp:
            arrow = Qt::UpArrow;
            break;
        case PE_IndicatorArrowDown:
            arrow = Qt::DownArrow;
            break;
        case PE_IndicatorArrowRight:
            arrow = Qt::RightArrow;
            break;
        case PE_IndicatorArrowLeft:
            arrow = Qt::LeftArrow;
            break;
        default:
            break;
        }
        // The caller may give us a huge rect and expect a normal-sized icon inside
        // of it, so we don't want to fill the entire thing with an arrow,
        // otherwise certain buttons will look weird, like the tab bar scroll
        // buttons. Might want to break these out into editable parameters?
        const int MaxArrowExt = Ph::dpiScaled(12);
        const int MinMargin = qMin(rw, rh) / 4;
        int aw, ah;
        aw = qMin(MaxArrowExt, rw) - MinMargin;
        ah = qMin(MaxArrowExt, rh) - MinMargin;
        if (aw <= 2 || ah <= 2)
            break;
        // QCommonStyle's implementation of CC_ToolButton for non-instant popups
        // gives us a pretty big rectangle to draw the arrow in -- shrink it. This
        // is kind of a dirty temp hack thing until we do something smarter, like
        // fully reimplement CC_ToolButton. Note that it passes us a regular
        // QStyleOption and not a QStyleOptionToolButton in this case, so try to
        // save some work before doing the inherits test.
        if (arrow == Qt::DownArrow && !qstyleoption_cast<const QStyleOptionToolButton*>(option) && widget) {
            auto tbutton = qobject_cast<const QToolButton*>(widget);
            if (tbutton && tbutton->popupMode() != QToolButton::InstantPopup && tbutton->defaultAction()) {
                int dim = static_cast<int>(qMin(rw, rh) * 0.25);
                aw -= dim;
                ah -= dim;
                // We have another hack in PE_IndicatorButtonDropDown where we shift
                // the edge left or right by 1px to avoid having two borders touching
                // (we make it overlap instead.) So we'll need to compensate for that
                // in the arrow's position to avoid it looking off-center.
                rw += 1;
                if (option->direction != Qt::RightToLeft) {
                    rx -= 1;
                }
            }
        }
        aw += (rw - aw) % 2;
        ah += (rh - ah) % 2;
        int ax = (rw - aw) / 2 + rx;
        int ay = (rh - ah) / 2 + ry;
        Ph::drawArrow(painter, QRect(ax, ay, aw, ah), arrow, swatch);
        break;
    }
    case PE_IndicatorItemViewItemCheck: {
        QStyleOptionButton button;
        button.QStyleOption::operator=(*option);
        button.state &= ~State_MouseOver;
        proxy()->drawPrimitive(PE_IndicatorCheckBox, &button, painter, widget);
        return;
    }
    case PE_IndicatorHeaderArrow: {
        auto header = qstyleoption_cast<const QStyleOptionHeader*>(option);
        if (!header)
            return;
        QRect r = header->rect;
        QPoint offset = QPoint(Phantom::HeaderSortIndicator_HOffset, Phantom::HeaderSortIndicator_VOffset);
        qreal lightness = Phantom::DeriveColors::hack_isLightPalette(widget->palette()) ? 0.03 : 0.0;
        if (header->sortIndicator & QStyleOptionHeader::SortUp) {
            Ph::drawArrow(painter, r.translated(offset), Qt::DownArrow, swatch, true, lightness);
        } else if (header->sortIndicator & QStyleOptionHeader::SortDown) {
            Ph::drawArrow(painter, r.translated(offset), Qt::UpArrow, swatch, true, lightness);
        }
        break;
    }
    case PE_IndicatorButtonDropDown: {
        // Temp hack until we implement CC_ToolButton: avoid double-stacked border
        // by clipping off one edge slightly.
        QStyleOption opt0 = *option;
        if (opt0.direction != Qt::RightToLeft) {
            opt0.rect.adjust(-1, 0, 0, 0);
        } else {
            opt0.rect.adjust(0, 0, 1, 0);
        }
        proxy()->drawPrimitive(PE_PanelButtonTool, &opt0, painter, widget);
        break;
    }

    case PE_IndicatorToolBarSeparator: {
        QRect r = option->rect;
        if (option->state & State_Horizontal) {
            if (r.height() >= 10)
                r.adjust(0, 3, 0, -3);
            r.setWidth(r.width() / 2 + 1);
            Ph::fillRectEdges(painter, r, Qt::RightEdge, 1, swatch.color(S_window_divider));
        } else {
            // TODO replace with new code
            const int margin = 6;
            const int offset = r.height() / 2;
            painter->setPen(QPen(option->palette.window().color().darker(110)));
            painter->drawLine(r.topLeft().x() + margin,
                              r.topLeft().y() + offset,
                              r.topRight().x() - margin,
                              r.topRight().y() + offset);
            painter->setPen(QPen(option->palette.window().color().lighter(110)));
            painter->drawLine(r.topLeft().x() + margin,
                              r.topLeft().y() + offset + 1,
                              r.topRight().x() - margin,
                              r.topRight().y() + offset + 1);
        }
        break;
    }
    case PE_PanelButtonTool: {
        bool isDown = option->state & State_Sunken;
        bool isOn = option->state & State_On;
        bool hasFocus = (option->state & State_HasFocus && option->state & State_KeyboardFocusChange);
        const qreal rounding = Ph::ToolButton_Rounding;
        Swatchy outline = S_window_outline;
        Swatchy fill = S_button;
        Swatchy specular = S_button_specular;
        if (isDown) {
            fill = S_button_pressed;
            specular = S_button_pressed_specular;
        } else if (isOn) {
            fill = S_button_on;
            specular = S_none;
        }
        if (hasFocus) {
            outline = S_highlight_outline;
        }
        QRect r = option->rect;
        Ph::PSave save(painter);
        Ph::paintBorderedRoundRect(painter, r, rounding, swatch, outline, fill);
        Ph::paintBorderedRoundRect(painter, r.adjusted(1, 1, -1, -1), rounding, swatch, specular, S_none);
        break;
    }
    case PE_IndicatorDockWidgetResizeHandle: {
        QStyleOption dockWidgetHandle = *option;
        bool horizontal = option->state & State_Horizontal;
        dockWidgetHandle.state =
            !horizontal ? (dockWidgetHandle.state | State_Horizontal) : (dockWidgetHandle.state & ~State_Horizontal);
        proxy()->drawControl(CE_Splitter, &dockWidgetHandle, painter, widget);
        break;
    }
    case PE_FrameWindow: {
        break;
    }
    case PE_FrameLineEdit: {
        QRect r = option->rect;
        bool hasFocus = option->state & State_HasFocus;
        bool isEnabled = option->state & State_Enabled;
        const qreal rounding = Ph::LineEdit_Rounding;
        auto pen = hasFocus ? S_highlight_outline : S_window_outline;
        Ph::PSave save(painter);
        Ph::paintBorderedRoundRect(painter, r, rounding, swatch, pen, S_none);
        save.restore();
        if (Ph::OverhangShadows && !hasFocus && isEnabled) {
            // Imperfect when rounded, may leave a gap on left and right. Going
            // closer would eat into the outline, though.
            Ph::fillRectEdges(painter,
                              r.adjusted(qRound(rounding / 2) + 1, 1, -(qRound(rounding / 2) + 1), -1),
                              Qt::TopEdge,
                              1,
                              swatch.color(S_base_shadow));
        }
        break;
    }
    case PE_PanelLineEdit: {
        auto lineEdit = qstyleoption_cast<const QStyleOptionFrame*>(option);
        if (!lineEdit)
            break;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = lineEdit->rect;
        bool enabled = lineEdit->state & State_Enabled;
        bool hasFocus = lineEdit->state & State_HasFocus;
        bool mouseOver = lineEdit->state & State_MouseOver;
        bool isReadOnly = lineEdit->state & State_ReadOnly;

        // 获取基础颜色
        QColor baseColor = isDarkMode() ? QColor("#1E1E1E") : QColor("#FFFFFF");
        QColor borderColor = isDarkMode() ? QColor("#3C3C3C") : QColor("#CCCCCC");

        // 背景颜色
        QColor bgColor = baseColor;
        if (!enabled || isReadOnly) {
            bgColor = isDarkMode() ? QColor("#2A2A2A") : QColor("#F5F5F5");
            borderColor.setAlpha(180);
        } else if (hasFocus) {
            borderColor = ::baseColor;  // 使用全局变量 baseColor
        } else if (mouseOver) {
            borderColor = borderColor.darker(110);
        }

        // 绘制背景
        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 11, 11);

        // 如果有焦点，绘制发光效果
        if (hasFocus && enabled && !isReadOnly) {
            QColor glowColor = ::baseColor;  // 使用全局变量 baseColor
            glowColor.setAlpha(50);
            
            painter->setPen(Qt::NoPen);
            painter->setBrush(glowColor);
            painter->drawRoundedRect(rect.adjusted(-2, -2, 1, 1), 11, 11);
        }

        painter->restore();
        break;
    }
    case PE_IndicatorCheckBox: {
        auto checkbox = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!checkbox)
            break;
        QRect r = option->rect;
        bool isHighlighted = option->state & State_HasFocus && option->state & State_KeyboardFocusChange;
        bool isSelected = option->state & State_Selected;
        bool isFlat = checkbox->features & QStyleOptionButton::Flat;
        bool isEnabled = option->state & State_Enabled;
        bool isPressed = state & State_Sunken;
        Swatchy outlineColor = isHighlighted ? S_highlight_outline : S_window_outline;
        Swatchy bgFillColor = isPressed ? S_highlight : S_base;
        Swatchy fgColor = isFlat ? S_windowText : S_text;
        if (isPressed && !isFlat) {
            fgColor = S_highlightedText;
        }
        // Bare checkmarks that are selected should draw with the highlighted text
        // color.
        if (isSelected && isFlat) {
            fgColor = S_highlightedText;
        }
        if (!isFlat) {
            QRect fillR = r;
            Ph::fillRectOutline(painter, fillR, 1, swatch.color(outlineColor));
            fillR.adjust(1, 1, -1, -1);
            if (Ph::IndicatorShadows && !isPressed && isEnabled) {
                Ph::fillRectEdges(painter, fillR, Qt::TopEdge, 1, swatch.color(S_base_shadow));
                fillR.adjust(0, 1, 0, 0);
            }
            painter->fillRect(fillR, swatch.color(bgFillColor));
        }
        if (checkbox->state & State_NoChange) {
            const qreal insetScale = 0.7;
            qreal rx, ry, rw, rh;
            QRectF(r.adjusted(1, 1, -1, -1)).getRect(&rx, &ry, &rw, &rh);
            qreal dimx = rw * insetScale;
            qreal dimy = rh * insetScale;
            QRectF r_(rx + (rw - dimx) / 2, ry + (rh - dimy) / 2, dimx, dimy);
            Ph::drawHyphen(painter, d->checkBox_pen_scratch, r_, swatch, fgColor);
        } else if (checkbox->state & State_On) {
            const qreal insetScale = 0.8;
            qreal rx, ry, rw, rh;
            QRectF(r.adjusted(1, 1, -1, -1)).getRect(&rx, &ry, &rw, &rh);
            // kinda wrong, assumes we're already square, but we probably are
            qreal dimx = rw * insetScale * Ph::CheckMark_WidthOfHeightScale;
            qreal dimy = rh * insetScale;
            QRectF r_(rx + (rw - dimx) / 2, ry + (rh - dimy) / 2, dimx, dimy);
            Ph::drawCheck(painter, d->checkBox_pen_scratch, r_, swatch, fgColor);
        }
        break;
    }
    case PE_IndicatorRadioButton: {
        auto radioButton = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!radioButton)
            break;

        QRect rect = radioButton->rect;
        bool enabled = radioButton->state & State_Enabled;
        bool mouseOver = radioButton->state & State_MouseOver;
        bool sunken = radioButton->state & State_Sunken;
        bool checked = radioButton->state & State_On;

        // 获取指示器矩形 - 使用 subElementRect 而不是 subControlRect
        QRect indicatorRect = proxy()->subElementRect(SE_RadioButtonIndicator, option, widget);
        
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 绘制外圈
        int diameter = qMin(indicatorRect.width(), indicatorRect.height());
        QRectF circleRect = QRectF(
            indicatorRect.x() + (indicatorRect.width() - diameter) / 2.0,
            indicatorRect.y() + (indicatorRect.height() - diameter) / 2.0,
            diameter,
            diameter
        );

        QColor circleColor = isDarkMode() ? QColor("#404040") : QColor("#FFFFFF");
        QColor borderColor = isDarkMode() ? QColor("#606060") : QColor("#999999");

        if (!enabled) {
            circleColor.setAlpha(180);
            borderColor.setAlpha(180);
        } else if (checked) {
            borderColor = baseColor;
        } else if (mouseOver) {
            borderColor = borderColor.lighter(120);
        }

        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(circleColor);
        painter->drawEllipse(circleRect);

        // 绘制内部选中点
        if (checked) {
            QColor dotColor = baseColor;
            if (!enabled) {
                dotColor.setAlpha(180);
            }

            painter->setPen(Qt::NoPen);
            painter->setBrush(dotColor);
            
            // 调整内部点的大小和位置以确保居中
            qreal margin = diameter * 0.35;  // 调整点的大小比例
            QRectF innerRect = circleRect.adjusted(margin, margin, -margin, -margin);
            painter->drawEllipse(innerRect);
        }

        painter->restore();
        break;
    }
    case PE_IndicatorToolBarHandle: {
        if (!option)
            break;
        QRect r = option->rect;
        if (r.width() < 3 || r.height() < 3)
            break;
        int rows = 3;
        int columns = 2;
        if (option->state & State_Horizontal) {
        } else {
            qSwap(columns, rows);
        }
        int dotLen = Ph::dpiScaled(2);
        QSize occupied(dotLen * (columns * 2 - 1), dotLen * (rows * 2 - 1));
        QRect rr = QStyle::alignedRect(option->direction, Qt::AlignCenter, QSize(occupied), r);
        int x = rr.x();
        int y = rr.y();
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < columns; ++col) {
                int x_ = x + col * 2 * dotLen;
                int y_ = y + row * 2 * dotLen;
                painter->fillRect(x_, y_, dotLen, dotLen, swatch.color(S_window_divider));
            }
        }
        break;
    }
    case PE_FrameDefaultButton:
        break;
    case PE_FrameFocusRect: {
        auto fropt = qstyleoption_cast<const QStyleOptionFocusRect*>(option);
        if (!fropt)
            break;
        //### check for d->alt_down
        if (!(fropt->state & State_KeyboardFocusChange))
            return;
        if (fropt->state & State_Item) {
            if (auto itemView = qobject_cast<const QAbstractItemView*>(widget)) {
                // TODO either our grid line hack is interfering, or Qt has a bug, but
                // in RTL layout the grid borders can leave junk behind in the grid
                // areas and the right edge of the focus rect may not get painted.
                // (Sometimes it will, though.) To replicate, set to RTL mode, and move
                // the current around in a table view without the selection being on
                // the current.
                if (option->state & QStyle::State_Selected) {
                    bool showCurrent = true;
                    bool hasTableGrid = false;
                    const auto selectionMode = itemView->selectionMode();
                    if (selectionMode == QAbstractItemView::SingleSelection) {
                        showCurrent = false;
                    } else {
                        // Table views will can have a "current" frame drawn even if the
                        // "current" is within the selected range. Other item views won't,
                        // which means the "current" frame will be invisible if it's on a
                        // selected item. This is a compromise between the broken drawing
                        // behavior of Qt item views of drawing "current" frames when they
                        // don't make sense (like a tree view where you can only select
                        // entire rows, but Qt will the frame rect around whatever column
                        // was last clicked on by the mouse, but using keyboard navigation
                        // has no effect) and not drawing them at all.
                        bool isTableView = false;
                        if (auto tableView = qobject_cast<const QTableView*>(itemView)) {
                            hasTableGrid = tableView->showGrid();
                            isTableView = true;
                        }
                        const auto selectionModel = itemView->selectionModel();
                        if (selectionModel) {
                            const auto selection = selectionModel->selection();
                            if (selection.count() == 1) {
                                const auto& range = selection.at(0);
                                if (isTableView) {
                                    // For table views, we don't draw the "current" frame if
                                    // there is exactly one cell selected and the "current" is
                                    // that cell, or if there is exactly one row or one column
                                    // selected with the behavior set to the corresponding
                                    // selection, and the "current" is that one row or column.
                                    const auto selectionBehavior = itemView->selectionBehavior();
                                    if ((range.width() == 1 && range.height() == 1)
                                        || (selectionBehavior == QAbstractItemView::SelectRows && range.height() == 1)
                                        || (selectionBehavior == QAbstractItemView::SelectColumns
                                            && range.width() == 1)) {
                                        showCurrent = false;
                                    }
                                } else {
                                    // For any other type of item view, don't draw the "current"
                                    // frame if there is a single contiguous selection, and the
                                    // "current" is within that selection. If there's a
                                    // discontiguous selection, that means the user is probably
                                    // doing something more advanced, and we should just draw the
                                    // focus frame, even if Qt might be doing it badly in some
                                    // cases.
                                    showCurrent = false;
                                }
                            }
                        }
                    }
                    if (showCurrent) {
                        // TODO handle dark-highlight-light-text
                        const QColor& borderColor = swatch.color(S_itemView_multiSelection_currentBorder);
                        const int thickness = hasTableGrid ? 2 : 1;
                        Ph::fillRectOutline(painter, option->rect, thickness, borderColor);
                    }
                } else {
                    Ph::fillRectOutline(painter, option->rect, 1, swatch.color(S_highlight_outline));
                }
                break;
            }
        }
        // It would be nice to also handle QTreeView's allColumnsShowFocus thing in
        // the above code, in addition to the normal cases for focus rects in item
        // views. Unfortunately, with allColumnsShowFocus set to true,
        // QTreeView::drawRow() calls the style to paint with PE_FrameFocusRect for
        // the row frame with the widget set to nullptr. This makes it basically
        // impossible to figure out that we need to draw a special frame for it.
        // So, if any application code is using that mode in a QTreeView, it won't
        // get special item view frames. Too bad.
        Ph::PSave save(painter);
        Ph::paintBorderedRoundRect(
            painter, option->rect, Ph::FrameFocusRect_Rounding, swatch, S_highlight_outline, S_none);
        break;
    }
    case PE_PanelButtonCommand:
    case PE_PanelButtonBevel: {
        bool isDefault = false;
        bool isFlat = false;
        bool isDown = option->state & State_Sunken;
        bool isOn = option->state & State_On;
        if (auto button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            isDefault = (button->features & QStyleOptionButton::DefaultButton) && (button->state & State_Enabled);
            isFlat = (button->features & QStyleOptionButton::Flat);
        }
        if (isFlat && !isDown && !isOn)
            break;
        bool isEnabled = option->state & State_Enabled;
        Q_UNUSED(isEnabled);
        bool hasFocus = (option->state & State_HasFocus && option->state & State_KeyboardFocusChange);
        const qreal rounding = Ph::PushButton_Rounding;
        Swatchy outline = S_window_outline;
        Swatchy fill = S_button;
        Swatchy specular = S_button_specular;
        if (isDown) {
            fill = S_button_pressed;
            specular = S_button_pressed_specular;
        } else if (isOn) {
            // kinda repurposing this, hmm
            fill = S_scrollbarGutter;
            specular = S_button_pressed_specular;
        }
        if (hasFocus || isDefault) {
            outline = S_highlight_outline;
        }
        QRect r = option->rect;
        Ph::PSave save(painter);
        Ph::paintBorderedRoundRect(painter, r, rounding, swatch, outline, fill);
        Ph::paintBorderedRoundRect(painter, r.adjusted(1, 1, -1, -1), rounding, swatch, specular, S_none);
        break;
    }
    case PE_FrameTabWidget: {
        QRect bgRect = option->rect.adjusted(1, 1, -1, -1);
        painter->fillRect(bgRect, swatch.color(S_tabFrame));
        auto twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(option);
        if (!twf)
            break;
        Ph::fillRectOutline(painter, option->rect, 1, swatch.color(S_frame_outline));
        Ph::fillRectOutline(painter, bgRect, 1, swatch.color(S_tabFrame_specular));
        break;
    }
    case PE_FrameStatusBarItem:
        break;
    case PE_IndicatorTabClose:
    case Phantom_PE_IndicatorTabNew: {
        Swatchy fg = S_windowText;
        Swatchy bg = S_none;
        if ((option->state & State_Enabled) && (option->state & State_MouseOver)) {
            fg = S_highlightedText;
            bg = option->state & State_Sunken ? S_highlight_outline : S_highlight;
        }
        // temp code
        Ph::PSave save(painter);
        if (bg) {
            Ph::paintSolidRoundRect(painter, option->rect, Ph::PushButton_Rounding, swatch, bg);
        }
        QPen pen = swatch.pen(fg);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidthF(1.5);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing);
        QRect r = option->rect;
        // int adj = (int)((qreal)qMin(r.width(), r.height()) * (1.0 / 2.5));
        int adj = Ph::dpiScaled(5.0);
        r.adjust(adj, adj, -adj, -adj);
        qreal x, y, w, h;
        QRectF(r).getRect(&x, &y, &w, &h);
        // painter->translate(-0.5, -0.5);
        switch (static_cast<int>(elem)) {
        case PE_IndicatorTabClose:
            painter->drawLine(QPointF(x - 0.5, y - 0.5), QPointF(x + 0.5 + w, y + 0.5 + h));
            painter->drawLine(QPointF(x - 0.5, y + h + 0.5), QPointF(x + 0.5 + w, y - 0.5));
            break;
        case Phantom_PE_IndicatorTabNew:
            // kinda hacky here on extra len
            painter->drawLine(QPointF(x + w / 2, y - 1.0), QPointF(x + w / 2, y + h + 1.0));
            painter->drawLine(QPointF(x - 1.0, y + h / 2), QPointF(x + w + 1.0, y + h / 2));
            break;
        }
        save.restore();
        // painter->fillRect(option->rect, QColor(255, 0, 0, 30));
        break;
    }
    case PE_PanelMenu: {
        // bool isBelowMenuBar = false;
        // works but currently unused
        // QPoint gp = widget->mapToGlobal(widget->rect().topLeft());
        // gp.setY(gp.y() - 1);
        // QWidget* bar = qApp->widgetAt(gp);
        // if (bar && bar->inherits("QMenuBar")) {
        //   isBelowMenuBar = true;
        // }

        // Ph::fillRectOutline(painter, option->rect, 1, swatch.color(S_window_divider));
        // QRect bgRect = option->rect.adjusted(1, isBelowMenuBar ? 0 : 1, -1, -1);
        // painter->fillRect(bgRect, swatch.color(S_window));

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        QColor background(swatch.color(S_window));
        background.setAlpha(200);
        painter->setBrush(background);
        QRectF frameRect = strokedRect(option->rect, 1);
        painter->drawRoundedRect(frameRect, Phantom::DefaultFrame_Radius, Phantom::DefaultFrame_Radius);

        // blur
        if (widget && widget->window()) {
//            QPainterPath path;
//            path.addRoundedRect(option->rect, Phantom::DefaultFrame_Radius, Phantom::DefaultFrame_Radius);
//            const_cast<QWidget *>(widget)->setMask(path.toFillPolygon().toPolygon());
//            m_blurHelper->update(const_cast<QWidget *>(widget));

            m_blurHelper->enableBlurBehind(const_cast<QWidget *>(widget), true, Phantom::DefaultFrame_Radius);
        }

        painter->restore();
        break;
    }
    case Phantom_PE_ScrollBarSliderVertical: {
        bool isLeftToRight = option->direction != Qt::RightToLeft;
        bool isSunken = option->state & State_Sunken;
        Swatchy thumbFill, thumbSpecular;
        if (isSunken) {
            thumbFill = S_button_pressed;
            thumbSpecular = S_button_pressed_specular;
        } else {
            thumbFill = S_scrollbarSlider;
            thumbSpecular = S_button_specular;
        }
        Qt::Edges edges;
        QRect edgeRect = option->rect;
        QRect mainRect = option->rect;
        edgeRect.adjust(0, -1, 0, 1);
        if (isLeftToRight) {
            edges = Qt::LeftEdge | Qt::TopEdge | Qt::BottomEdge;
            mainRect.setX(mainRect.x() + 1);
        } else {
            edges = Qt::TopEdge | Qt::BottomEdge | Qt::RightEdge;
            mainRect.setWidth(mainRect.width() - 1);
        }
        Ph::fillRectEdges(painter, edgeRect, edges, 1, swatch.color(S_window_outline));
        painter->fillRect(mainRect, swatch.color(thumbFill));
        Ph::fillRectOutline(painter, mainRect, 1, swatch.color(thumbSpecular));
        break;
    }
    case Phantom_PE_WindowFrameColor: {
        painter->fillRect(option->rect, swatch.color(S_window_outline));
        break;
    }
    // case CE_LineEdit: {
    //     auto lineEdit = qstyleoption_cast<const QStyleOptionFrame*>(option);
    //     if (!lineEdit)
    //         break;

    //     painter->save();
    //     painter->setRenderHint(QPainter::Antialiasing);

    //     QRect rect = lineEdit->rect;
    //     bool enabled = lineEdit->state & State_Enabled;
    //     bool hasFocus = lineEdit->state & State_HasFocus;
    //     bool mouseOver = lineEdit->state & State_MouseOver;
    //     bool isReadOnly = lineEdit->state & State_ReadOnly;

    //     // 获取基础颜色
    //     QColor baseColor = isDarkMode() ? QColor("#1E1E1E") : QColor("#FFFFFF");
    //     QColor borderColor = isDarkMode() ? QColor("#3C3C3C") : QColor("#CCCCCC");

    //     // 背景颜色
    //     QColor bgColor = baseColor;
    //     if (!enabled || isReadOnly) {
    //         bgColor = isDarkMode() ? QColor("#2A2A2A") : QColor("#F5F5F5");
    //         borderColor.setAlpha(180);
    //     } else if (hasFocus) {
    //         borderColor = this->baseColor;
    //     } else if (mouseOver) {
    //         borderColor = borderColor.darker(110);
    //     }

    //     // 绘制背景
    //     painter->setPen(QPen(borderColor, 1));
    //     painter->setBrush(bgColor);
    //     painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 11, 11);

    //     // 如果有焦点，绘制发光效果
    //     if (hasFocus && enabled && !isReadOnly) {
    //         QColor glowColor = this->baseColor;
    //         glowColor.setAlpha(50);
            
    //         painter->setPen(Qt::NoPen);
    //         painter->setBrush(glowColor);
    //         painter->drawRoundedRect(rect.adjusted(-2, -2, 1, 1), 11, 11);
    //     }

    //     painter->restore();
    //     break;
    // }
    default:
        QCommonStyle::drawPrimitive(elem, option, painter, widget);
        break;
    }
}

void BaseStyle::drawControl(ControlElement element,
                            const QStyleOption* option,
                            QPainter* painter,
                            const QWidget* widget) const
{
#ifdef BUILD_WITH_EASY_PROFILER
    EASY_BLOCK("drawControl");
    const char* elemCString = QMetaEnum::fromType<QStyle::ControlElement>().valueToKey(element);
    EASY_TEXT("Element", elemCString);
#endif
    using Swatchy = Phantom::Swatchy;
    using namespace Phantom::SwatchColors;
    namespace Ph = Phantom;
    auto ph_swatchPtr = Ph::getCachedSwatchOfQPalette(&d->swatchCache, &d->headSwatchFastKey, option->palette);
    const Ph::PhSwatch& swatch = *ph_swatchPtr.data();

    switch (element) {
    case CE_CheckBox:
    case CE_RadioButton: {
        auto button = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!button)
            break;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 获取复选框的矩形区域
        QRect checkRect = proxy()->subElementRect(element == CE_RadioButton ? SE_RadioButtonIndicator 
                                                                           : SE_CheckBoxIndicator, 
                                                button, widget);
        
        // 确保矩形是正方形，并且边缘宽度一致
        int size = qMin(checkRect.width(), checkRect.height());
        checkRect = QRect(checkRect.x(), checkRect.y() + (checkRect.height() - size) / 2, size, size);
        
        // 计算内部矩形（所有边缘保持相同的边框宽度）
        QRect innerRect = checkRect.adjusted(2, 2, -2, -2);
        
        bool isRadio = (element == CE_RadioButton);
        bool isChecked = button->state & State_On;
        bool isEnabled = button->state & State_Enabled;
        bool isHovered = button->state & State_MouseOver;

        painter->setPen(Qt::NoPen);
        
        // 背景颜色
        QColor bgColor;
        if (!isEnabled) {
            bgColor = swatch.color(S_window);
        } else if (isChecked) {
            bgColor = baseColor;
            if (isHovered) {
                bgColor = bgColor.lighter(110);
            }
        } else if (isHovered) {
            bgColor = swatch.color(S_window);
        } else {
            bgColor = swatch.color(S_window);
        }

        // 边框颜色
        QColor borderColor;
        if (!isEnabled) {
            borderColor = swatch.color(S_window_outline);
        } else if (isChecked) {
            borderColor = baseColor;
            if (isHovered) {
                borderColor = borderColor.lighter(110);
            }
        } else if (isHovered) {
            borderColor = baseColor.lighter(110);
        } else {
            borderColor = swatch.color(S_window_outline);
        }

        if (isRadio) {
            // 单选框
            painter->setPen(QPen(borderColor, 1));
            painter->setBrush(bgColor);
            QRect smallerCheckRect = checkRect.adjusted(
                checkRect.width() * 0.05,   // 左边界向右调整5%
                checkRect.height() * 0.05, // 上边界向下调整5%
                -checkRect.width() * 0.05, // 右边界向左调整5%
                -checkRect.height() * 0.05 // 下边界向上调整5%
            );

            painter->drawEllipse(smallerCheckRect);

            if (isChecked && isEnabled) {
                // 内部圆点大小为外圈的50%
                int dotSize = qRound(smallerCheckRect.width() * 0.5);  // 内圆大小为缩小后外圆宽度的50%

                // 使用内圆的中心点来绘制
                QPointF dotCenter = smallerCheckRect.center();  // 获取外圆的中心点
                QRectF dotRect(
                    dotCenter.x() - dotSize / 2.5,  // 左上角x坐标
                    dotCenter.y() - dotSize / 2.5,  // 左上角y坐标
                    dotSize, dotSize               // 宽度和高度
                );

                painter->setPen(Qt::NoPen);
                painter->setBrush(Qt::white);
                painter->drawEllipse(dotRect);
            }
        } else {
            // 复选框
            if (isChecked) {
                // 选中状态下不显示边框，只有背景色
                painter->setBrush(bgColor);
                painter->drawRoundedRect(checkRect, 3, 3);

                // 绘制对勾
                QPen checkPen(Qt::white, 2);
                checkPen.setCapStyle(Qt::RoundCap);
                checkPen.setJoinStyle(Qt::RoundJoin);
                painter->setPen(checkPen);

                // macOS 风格的对勾
                QPainterPath path;
                qreal x = checkRect.x() + checkRect.width() * 0.15;
                qreal y = checkRect.y() + checkRect.height() * 0.5;
                
                path.moveTo(x, y);
                path.lineTo(x + checkRect.width() * 0.3, y + checkRect.height() * 0.3);
                path.lineTo(x + checkRect.width() * 0.7, y - checkRect.height() * 0.3);
                
                painter->drawPath(path);
            } else {
                painter->setPen(QPen(borderColor, 1)); // 减小边框宽度
                painter->setBrush(bgColor);
                painter->drawRoundedRect(checkRect, 2.5, 2.5);
            }
        }

        // 绘制文本标签
        QRect textRect = proxy()->subElementRect(element == CE_RadioButton ? SE_RadioButtonContents 
                                                                          : SE_CheckBoxContents, 
                                               button, widget);
        if (!button->text.isEmpty()) {
            painter->setPen(button->palette.color(isEnabled ? QPalette::Active : QPalette::Disabled, 
                                                QPalette::WindowText));
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, button->text);
        }

        painter->restore();
        break;
    }
    case CE_ComboBoxLabel: {
        auto cb = qstyleoption_cast<const QStyleOptionComboBox*>(option);
        if (!cb)
            break;
        QRect editRect = proxy()->subControlRect(CC_ComboBox, cb, SC_ComboBoxEditField, widget);
        painter->save();
        painter->setClipRect(editRect);
        if (!cb->currentIcon.isNull()) {
            QIcon::Mode mode = cb->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
            QPixmap pixmap = cb->currentIcon.pixmap(cb->iconSize, mode);
            QRect iconRect(editRect);
            iconRect.setWidth(cb->iconSize.width() + 4);
            iconRect = alignedRect(cb->direction, Qt::AlignLeft | Qt::AlignVCenter, iconRect.size(), editRect);
            if (cb->editable)
                painter->fillRect(iconRect, cb->palette.brush(QPalette::Base));
            proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);

            if (cb->direction == Qt::RightToLeft)
                editRect.translate(-4 - cb->iconSize.width(), 0);
            else
                editRect.translate(cb->iconSize.width() + 4, 0);
        }
        if (!cb->currentText.isEmpty() && !cb->editable) {
            proxy()->drawItemText(painter,
                                  editRect.adjusted(1, 0, -1, 0),
                                  visualAlignment(cb->direction, Qt::AlignLeft | Qt::AlignVCenter),
                                  cb->palette,
                                  cb->state & State_Enabled,
                                  cb->currentText,
                                  cb->editable ? QPalette::Text : QPalette::ButtonText);
        }
        painter->restore();
        break;
    }
    case CE_Splitter: {
        QRect r = option->rect;
        // We don't have anything useful to draw if it's too thin
        if (r.width() < 5 || r.height() < 5)
            break;
        int length = Ph::dpiScaled(Ph::SplitterMaxLength);
        // int thickness = Ph::dpiScaled(1);
        qreal radius = 0;
        QSize size;
        if (option->state & State_Horizontal) {
            if (r.height() < length)
                length = r.height();
            // size = QSize(thickness, length);
            size = QSize(r.width() / 2, length);
            radius = r.width() * 0.3;
        } else {
            if (r.width() < length)
                length = r.width();
            // size = QSize(length, thickness);
            size = QSize(length, r.height() / 2);
            radius = r.height() * 0.3;
        }
        QRect filledRect = QStyle::alignedRect(option->direction, Qt::AlignCenter, size, r);
        Ph::paintSolidRoundRect(painter, filledRect, radius, swatch, S_button_specular);
        // Ph::fillRectOutline(painter, filledRect.adjusted(-1, 0, 1, 0), 1, swatch.color(S_window_divider));
        break;
    }
    // TODO update this for phantom
    case CE_RubberBand: {
        if (!qstyleoption_cast<const QStyleOptionRubberBand*>(option))
            break;
        QColor highlight = option->palette.color(QPalette::Active, QPalette::Highlight);
        painter->save();
        QColor penColor = highlight.darker(120);
        penColor.setAlpha(180);
        painter->setPen(penColor);
        QColor dimHighlight(qMin(highlight.red() / 2 + 110, 255),
                            qMin(highlight.green() / 2 + 110, 255),
                            qMin(highlight.blue() / 2 + 110, 255));
        dimHighlight.setAlpha(widget && widget->isTopLevel() ? 255 : 80);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(0.5, 0.5);
        painter->setBrush(dimHighlight);
        painter->drawRoundedRect(option->rect.adjusted(0, 0, -1, -1), 1, 1);
        QColor innerLine = Qt::white;
        innerLine.setAlpha(40);
        painter->setPen(innerLine);
        painter->drawRoundedRect(option->rect.adjusted(1, 1, -2, -2), 1, 1);
        painter->restore();
        break;
    }
    case CE_SizeGrip: {
        Qt::LayoutDirection dir = option->direction;
        QRect rect = option->rect;
        int rcx = rect.center().x();
        int rcy = rect.center().y();
        // draw grips
        for (int i = -6; i < 12; i += 3) {
            for (int j = -6; j < 12; j += 3) {
                if ((dir == Qt::LeftToRight && i > -j) || (dir == Qt::RightToLeft && j > i)) {
                    painter->fillRect(rcx + i, rcy + j, 2, 2, swatch.color(S_window_lighter));
                    painter->fillRect(rcx + i, rcy + j, 1, 1, swatch.color(S_window_darker));
                }
            }
        }
        break;
    }
    case CE_ToolBar: {
        auto toolBar = qstyleoption_cast<const QStyleOptionToolBar*>(option);
        if (!toolBar)
            break;
        painter->fillRect(option->rect, option->palette.window().color());
        bool isFloating = false;
        if (auto tb = qobject_cast<const QToolBar*>(widget)) {
            isFloating = tb->isFloating();
        }
        if (isFloating) {
            Ph::fillRectOutline(painter, option->rect, 1, swatch.color(S_window_outline));
        }
        break;
    }
    case CE_DockWidgetTitle: {
        auto dwOpt = qstyleoption_cast<const QStyleOptionDockWidget*>(option);
        if (!dwOpt)
            break;
        painter->save();
        bool verticalTitleBar = dwOpt->verticalTitleBar;

        QRect titleRect = subElementRect(SE_DockWidgetTitleBarText, option, widget);
        if (verticalTitleBar) {
            QRect r = dwOpt->rect;
            QRect rtrans = {r.x(), r.y(), r.height(), r.width()};
            titleRect = QRect(rtrans.left() + r.bottom() - titleRect.bottom(),
                              rtrans.top() + titleRect.left() - r.left(),
                              titleRect.height(),
                              titleRect.width());
            painter->translate(rtrans.left(), rtrans.top() + rtrans.width());
            painter->rotate(-90);
            painter->translate(-rtrans.left(), -rtrans.top());
        }
        if (!dwOpt->title.isEmpty()) {
            QString titleText = painter->fontMetrics().elidedText(dwOpt->title, Qt::ElideRight, titleRect.width());
            proxy()->drawItemText(painter,
                                  titleRect,
                                  Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                                  dwOpt->palette,
                                  dwOpt->state & State_Enabled,
                                  titleText,
                                  QPalette::WindowText);
        }
        painter->restore();
        break;
    }
    case CE_HeaderSection: {
        auto header = qstyleoption_cast<const QStyleOptionHeader*>(option);
        if (!header)
            break;
        QRect rect = header->rect;
        Qt::Orientation orientation = header->orientation;
        QStyleOptionHeader::SectionPosition position = header->position;
        // See the "Table header layout reference" comment block at the bottom of
        // this file for more information to help understand what's going on.
        bool isLeftToRight = header->direction != Qt::RightToLeft;
        bool isHorizontal = orientation == Qt::Horizontal;
        bool isVertical = orientation == Qt::Vertical;
        bool isEnd = position == QStyleOptionHeader::End;
        bool isBegin = position == QStyleOptionHeader::Beginning;
        bool isOnlyOne = position == QStyleOptionHeader::OnlyOneSection;
        Qt::Edges edges;
        bool spansToEnd = false;
        bool isSpecialCorner = false;
        if ((isHorizontal && isLeftToRight && isEnd) || (isHorizontal && !isLeftToRight && isBegin)
            || (isVertical && isEnd) || isOnlyOne) {
            auto hv = qobject_cast<const QHeaderView*>(widget);
            if (hv) {
                spansToEnd = hv->stretchLastSection();
                // In the case where the header item is not stretched to the end, but
                // could plausibly be in a position where it could happen to be exactly
                // the right width or height to be appear to be stretched to the end,
                // we'll check to see if it actually does exactly meet the right (or
                // bottom in vertical, or left in RTL) edge, and omit drawing the edge
                // if that's the case. This can commonly happen if you have a tree or
                // list view and don't set it to stretch, but the widget is still sized
                // exactly to hold the one column. (It could also happen if there's
                // user code running to manually stretch the last section as
                // necessary.)
                if (!spansToEnd) {
                    QRect viewBound = hv->contentsRect();
                    if (isHorizontal) {
                        if (isLeftToRight) {
                            spansToEnd = rect.right() == viewBound.right();
                        } else {
                            spansToEnd = rect.left() == viewBound.left();
                        }
                    } else if (isVertical) {
                        spansToEnd = rect.bottom() == viewBound.bottom();
                    }
                }
            } else {
                // We only need to do this check in RTL, because the corner button in
                // RTL *doesn't* need hacks applied. In LTR, we can just treat the
                // corner button like anything else on the horizontal header bar, and
                // can skip doing this inherits check.
                if (isOnlyOne && !isLeftToRight && widget && widget->inherits("QTableCornerButton")) {
                    isSpecialCorner = true;
                }
            }
        }

        if (isSpecialCorner) {
            // In RTL layout, the corner button in a table view doesn't have any
            // offset problems. This branch we're on is only taken if we're in RTL
            // layout and this is the corner button being drawn.
            edges |= Qt::BottomEdge;
            if (isLeftToRight)
                edges |= Qt::RightEdge;
            else
                edges |= Qt::LeftEdge;
        } else if (isHorizontal) {
            // This branch is taken for horizontal headers in either layout direction
            // or for the corner button in LTR.
            edges |= Qt::BottomEdge;
            if (isLeftToRight) {
                // In LTR, this code path may be for both the corner button *and* the
                // actual header item. It doesn't matter in this case, and we were able
                // to avoid doing an extra inherits call earlier.
                if (!spansToEnd) {
                    edges |= Qt::RightEdge;
                }
            } else {
                // Note: in right-to-left layouts for horizontal headers, the header
                // view will unfortunately be shifted to the right by 1 pixel, due to
                // what appears to be a Qt bug. This causes the vertical lines we draw
                // in the header view to misalign with the grid, and causes the
                // rightmost section to have its right edge clipped off. Therefore,
                // we'll draw the separator on the on the right edge instead of the
                // left edge. (We would have expected to draw it on the left edge in
                // RTL layout.) This makes it line up with the grid again, except for
                // the last section. right by 1 pixel.
                //
                // In RTL, the "Begin" position is on the left side for some reason
                // (the same as LTR.) So "End" is always on the right. Ok, whatever.
                // See the table at the bottom of this file if you're confused.
                if (!isOnlyOne && !isEnd) {
                    edges |= Qt::RightEdge;
                }
                // The leftmost section in RTL has to draw on both its right and left
                // edges, instead of just 1 edge like every other configuration. The
                // left edge will be offset by 1 pixel from the grid, but it's the best
                // we can do.
                if (isBegin && !spansToEnd) {
                    edges |= Qt::LeftEdge;
                }
            }
        } else if (isVertical) {
            if (isLeftToRight) {
                edges |= Qt::RightEdge;
            } else {
                edges |= Qt::LeftEdge;
            }
            if (!spansToEnd) {
                edges |= Qt::BottomEdge;
            }
        }
        QRect bgRect = Ph::expandRect(rect, edges, -1);
        painter->fillRect(bgRect, swatch.color(S_window));
        Ph::fillRectEdges(painter, rect, edges, 1, swatch.color(S_frame_outline));
        break;
    }
    case CE_HeaderLabel: {
        auto header = qstyleoption_cast<const QStyleOptionHeader*>(option);
        if (!header)
            break;
        QRect rect = header->rect;
        if (!header->icon.isNull()) {
            int iconExtent = qMin(qMin(rect.height(), rect.width()), option->fontMetrics.height());
            auto window = widget ? widget->window()->windowHandle() : nullptr;
            QPixmap pixmap = header->icon.pixmap(window,
                                                 QSize(iconExtent, iconExtent),
                                                 (header->state & State_Enabled) ? QIcon::Normal : QIcon::Disabled);
            int pixw = static_cast<int>(pixmap.width() / pixmap.devicePixelRatio());
            QRect aligned = alignedRect(
                header->direction, QFlag(header->iconAlignment), pixmap.size() / pixmap.devicePixelRatio(), rect);
            QRect inter = aligned.intersected(rect);
            painter->drawPixmap(inter.x(),
                                inter.y(),
                                pixmap,
                                inter.x() - aligned.x(),
                                inter.y() - aligned.y(),
                                static_cast<int>(aligned.width() * pixmap.devicePixelRatio()),
                                static_cast<int>(pixmap.height() * pixmap.devicePixelRatio()));
            int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, option, widget);
            if (header->direction == Qt::LeftToRight)
                rect.setLeft(rect.left() + pixw + margin);
            else
                rect.setRight(rect.right() - pixw - margin);
        }
        proxy()->drawItemText(painter,
                              rect,
                              header->textAlignment,
                              header->palette,
                              (header->state & State_Enabled),
                              header->text,
                              QPalette::ButtonText);

        // But we still need some kind of indicator, so draw a line
        bool drawHighlightLine = option->state & State_On;
        // Special logic: if the selection mode of the item view is to select every
        // row or every column, there's no real need to draw special "this
        // row/column is selected" highlight indicators in the header view. The
        // application programmer can also disable this explicitly on the header
        // view, but it's nice to have it done automatically, I think.
        if (drawHighlightLine) {
            const QAbstractItemView* itemview = nullptr;
            // Header view itself is an item view, and we don't care about its
            // selection behavior -- we care about the actual item view. So try to
            // get the widget as the header first, then find the item view from
            // there.
            auto headerview = qobject_cast<const QHeaderView*>(widget);
            if (headerview) {
                // Also don't care about highlights if there's only one row or column.
                drawHighlightLine = headerview->count() > 1;
                itemview = qobject_cast<const QAbstractItemView*>(headerview->parentWidget());
            }
            if (drawHighlightLine && itemview) {
                auto selBehavior = itemview->selectionBehavior();
                if (selBehavior == QAbstractItemView::SelectRows && header->orientation == Qt::Horizontal)
                    drawHighlightLine = false;
                else if (selBehavior == QAbstractItemView::SelectColumns && header->orientation == Qt::Vertical)
                    drawHighlightLine = false;
            }
        }

        if (drawHighlightLine) {
            QRect r = option->rect;
            Qt::Edge edge;
            if (header->orientation == Qt::Horizontal) {
                edge = Qt::BottomEdge;
                r.adjust(-2, 1, 1, 1);
            } else {
                bool isLeftToRight = option->direction != Qt::RightToLeft;
                if (isLeftToRight) {
                    edge = Qt::RightEdge;
                    r.adjust(1, -2, 1, 1);
                } else {
                    edge = Qt::LeftEdge;
                    r.adjust(-1, -2, -1, 1);
                }
            }
            Ph::fillRectEdges(painter, r, edge, 1, swatch.color(S_itemView_headerOnLine));
        }
        break;
    }
    case CE_ProgressBarGroove: {
        const qreal rounding = Ph::ProgressBar_Rounding;
        QRect rect = option->rect;
        Ph::PSave save(painter);
        Ph::paintBorderedRoundRect(painter, rect, rounding, swatch, S_window_outline, S_base);
        save.restore();
        if (Ph::OverhangShadows && option->state & State_Enabled) {
            // Inner shadow
            const QColor& shadowColor = swatch.color(S_base_shadow);
            // We can either have the shadow cut into the rounded corners, or leave a
            // 1px gap, due to AA.
            Ph::fillRectEdges(painter,
                              rect.adjusted(qRound(rounding / 2) + 1, 1, -(qRound(rounding / 2) + 1), -1),
                              Qt::TopEdge,
                              1,
                              shadowColor);
        }
        break;
    }
    case CE_ProgressBarContents: {
        auto bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
        if (!bar)
            break;
        const qreal rounding = Ph::ProgressBar_Rounding;
        QRect filled, nonFilled;
        bool isIndeterminate = false;
        Ph::progressBarFillRects(bar, filled, nonFilled, isIndeterminate);
        if (isIndeterminate || bar->progress > bar->minimum) {
            Ph::PSave save(painter);
            Ph::paintBorderedRoundRect(painter, filled, rounding, swatch, S_progressBar_outline, S_progressBar);
            Ph::paintBorderedRoundRect(
                painter, filled.adjusted(1, 1, -1, -1), rounding, swatch, S_progressBar_specular, S_none);
            if (isIndeterminate) {
                // TODO paint indeterminate indicator
            }
        }
        break;
    }
    case CE_ProgressBarLabel: {
        auto bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
        if (!bar)
            break;
        if (bar->text.isEmpty())
            break;
        QRect r = bar->rect.adjusted(2, 2, -2, -2);
        if (r.isEmpty() || !r.isValid())
            break;
        QSize textSize = option->fontMetrics.size(Qt::TextBypassShaping, bar->text);
        QRect textRect = QStyle::alignedRect(option->direction, Qt::AlignCenter, textSize, option->rect);
        textRect &= r;
        if (textRect.isEmpty())
            break;
        QRect filled, nonFilled;
        bool isIndeterminate = false;
        Ph::progressBarFillRects(bar, filled, nonFilled, isIndeterminate);
        QRect textNonFilledR = textRect & nonFilled;
        QRect textFilledR = textRect & filled;
        bool needsNonFilled = !textNonFilledR.isEmpty();
        bool needsFilled = !textFilledR.isEmpty();
        bool needsMasking = needsNonFilled && needsFilled;
        Ph::PSave save(painter);
        if (needsNonFilled) {
            if (needsMasking) {
                painter->save();
                painter->setClipRect(textNonFilledR);
            }
            painter->setPen(swatch.pen(S_text));
            painter->setBrush(Qt::NoBrush);
            painter->drawText(textRect, bar->text, Qt::AlignHCenter | Qt::AlignVCenter);
            if (needsMasking) {
                painter->restore();
            }
        }
        if (needsFilled) {
            if (needsMasking) {
                painter->save();
                painter->setClipRect(textFilledR);
            }
            painter->setPen(swatch.pen(S_highlightedText));
            painter->setBrush(Qt::NoBrush);
            painter->drawText(textRect, bar->text, Qt::AlignHCenter | Qt::AlignVCenter);
            if (needsMasking) {
                painter->restore();
            }
        }
        break;
    }
    case CE_MenuBarItem: {
        auto mbi = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
        if (!mbi)
            break;
        const QRect r = option->rect;
        QRect textRect = r;
        textRect.setY(textRect.y() + (r.height() - option->fontMetrics.height()) / 2);
        int alignment = Qt::AlignHCenter | Qt::AlignTop | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
        if (!proxy()->styleHint(SH_UnderlineShortcut, mbi, widget))
            alignment |= Qt::TextHideMnemonic;
        const auto itemState = mbi->state;
        bool maybeHasAltKeyNavFocus = itemState & State_Selected && itemState & State_HasFocus;
        bool isSelected = itemState & State_Selected || itemState & State_Sunken;
        if (!isSelected && maybeHasAltKeyNavFocus && widget) {
            isSelected = widget->hasFocus();
        }
        Swatchy fill = isSelected ? S_highlight : S_window;
        painter->fillRect(r, swatch.color(fill));
        QPalette::ColorRole textRole = isSelected ? QPalette::HighlightedText : QPalette::Text;
        proxy()->drawItemText(
            painter, textRect, alignment, mbi->palette, mbi->state & State_Enabled, mbi->text, textRole);
        if (Phantom::MenuBarDrawBorder && !isSelected) {
            Ph::fillRectEdges(painter, r, Qt::BottomEdge, 1, swatch.color(S_window_divider));
        }
        break;
    }

    case CE_MenuItem: {
        auto menuItem = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
        if (!menuItem)
            break;
        const auto metrics = Ph::MenuItemMetrics::ofFontHeight(option->fontMetrics.height());
        // Draws one item in a popup menu.
        if (menuItem->menuItemType == QStyleOptionMenuItem::Separator) {
            // Phantom ignores text and icons in menu separators, because
            // 1) The text and icons for separators don't render on Mac native menus
            // 2) There doesn't seem to be a way to account for the width of the text
            // properly (Fusion will often draw separator text clipped off)
            // 3) Setting text on separators also seems to mess up the metrics for
            // menu items on Mac native menus
            QRect r = option->rect;
            r.setHeight(r.height() / 2 + 1);
            Ph::fillRectEdges(painter, r, Qt::BottomEdge, 1, swatch.color(S_window_divider));
            break;
        }
        const QRect itemRect = option->rect;
        painter->save();
        bool isSelected = menuItem->state & State_Selected && menuItem->state & State_Enabled;
        bool isCheckable = menuItem->checkType != QStyleOptionMenuItem::NotCheckable;
        bool isChecked = menuItem->checked;
        bool isSunken = menuItem->state & State_Sunken;
        bool isEnabled = menuItem->state & State_Enabled;
        bool hasSubMenu = menuItem->menuItemType == QStyleOptionMenuItem::SubMenu;

        if (isSelected) {
            QColor fillColor = baseColor;
            if (isSunken) {
                fillColor = baseColor.lighter(120); // Increase brightness by 20% when pressed
            }
            qreal item_radius = Phantom::DefaultFrame_Radius / 2;
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(fillColor);
            if (!isSunken) {
                painter->setOpacity(1.0); // No transparency when hovered
            } else {
                painter->setOpacity(0.4); // Transparency when pressed
            }
            painter->setRenderHint(QPainter::Antialiasing);
            painter->drawRoundedRect(option->rect, item_radius, item_radius);
            painter->restore();
        }

        if (isCheckable) {
            // Note: check rect might be misaligned vertically if it's a menu from a
            // combo box. Probably a bug in Qt code?
            QRect checkRect = Ph::menuItemCheckRect(metrics, option->direction, itemRect, hasSubMenu);
            Swatchy signColor = !isEnabled ? S_windowText : isSelected ? S_windowText /*S_highlightedText*/ : S_windowText;
            if (menuItem->checkType & QStyleOptionMenuItem::Exclusive) {
                // Radio button
                if (isChecked) {
                    painter->setRenderHint(QPainter::Antialiasing);
                    painter->setPen(Qt::NoPen);
                    QPalette::ColorRole textRole =
                        !isEnabled ? QPalette::Text : isSelected ? /*QPalette::HighlightedText*/ QPalette::ButtonText : QPalette::ButtonText;
                    painter->setBrush(option->palette.brush(option->palette.currentColorGroup(), textRole));
                    qreal rx, ry, rw, rh;
                    QRectF(checkRect).getRect(&rx, &ry, &rw, &rh);
                    qreal dim = qMin(checkRect.width(), checkRect.height()) * 0.75;
                    QRectF rf(rx + rw / dim, ry + rh / dim, dim, dim);
                    painter->drawEllipse(rf);
                }
            } else {
                // If we want mouse-down to immediately show the item as
                // checked/unchecked (kinda bad if the user is click-holding on the
                // menu instead of click-clicking.)
                //
                // if ((isChecked && !isSunken) || (!isChecked && isSunken)) {
                if (isChecked) {
                    Ph::drawCheck(painter, d->checkBox_pen_scratch, checkRect, swatch, signColor);
                }
            }
        }

        const bool hasIcon = !menuItem->icon.isNull();

        if (hasIcon) {
            QRect iconRect = Ph::menuItemIconRect(metrics, option->direction, itemRect, hasSubMenu);
            QIcon::Mode mode = isEnabled ? QIcon::Normal : QIcon::Disabled;
            if (isSelected && isEnabled)
                mode = QIcon::Selected;
            QIcon::State state = isChecked ? QIcon::On : QIcon::Off;

            // TODO hmm, we might be ending up with blurry icons at size 15 instead
            // of 16 for example on Windows.
            //
            // int smallIconSize =
            //     proxy()->pixelMetric(PM_SmallIconSize, option, widget);
            // QSize iconSize(smallIconSize, smallIconSize);
            int iconExtent = qMin(iconRect.width(), iconRect.height());
            QSize iconSize(iconExtent, iconExtent);
            if (auto combo = qobject_cast<const QComboBox*>(widget)) {
                iconSize = combo->iconSize();
            }
            QWindow* window = widget ? widget->windowHandle() : nullptr;
            QPixmap pixmap = menuItem->icon.pixmap(window, iconSize, mode, state);
            const int pixw = static_cast<int>(pixmap.width() / pixmap.devicePixelRatio());
            const int pixh = static_cast<int>(pixmap.height() / pixmap.devicePixelRatio());
            QRect pixmapRect = QStyle::alignedRect(option->direction, Qt::AlignCenter, QSize(pixw, pixh), iconRect);
            painter->drawPixmap(pixmapRect.topLeft(), pixmap);
        }

        // Draw main text and mnemonic text
        QStringRef s(&menuItem->text);
        if (!s.isEmpty()) {
            QRect textRect =
                Ph::menuItemTextRect(metrics, option->direction, itemRect, hasSubMenu, hasIcon, menuItem->tabWidth);
            int t = s.indexOf(QLatin1Char('\t'));
            int text_flags =
                Qt::AlignLeft | Qt::AlignTop | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
            if (!styleHint(SH_UnderlineShortcut, menuItem, widget))
                text_flags |= Qt::TextHideMnemonic;
#if 0
                painter->save();
#endif
            painter->setPen(isSelected ? QColor(Qt::white) : swatch.pen(S_text /*S_highlightedText*/));

            // My comment:
            //
            // What actually looks like is happening is that the qplatformtheme may
            // have set a per-class font for menus. The QComboMenuDelegate sets the
            // combo box's own font on the QStyleOptionMenuItem when passing it in
            // here and when calling sizeFromContents with CT_MenuItem, but the
            // QPainter we're called with hasn't had its font set to it -- it's still
            // set to the QMenu/QMenuItem app fonts hash font. So if it's a menu
            // coming from a combo box, let's just go ahead and set the font for it
            // if it doesn't match, since that's probably what it wanted to do. I
            // think. And as described above, we have to do the weird dance with the
            // resolve mask... which is some internal Qt detail that we aren't
            // supposed to have to deal with, but here we are.
            //
            // Ok, there's another problem, and QFusionStyle also suffers from it: in
            // high DPI, setting the pointSizeF and setting the font again won't
            // necessarily give us the right font (at least in Windows.) The font
            // might have too thin of a weight, and probably other problems. So just
            // forget about it: we'll have Phantom return 0 for the style hint that
            // the combo box uses to determine if it should use a QMenu popup instead
            // of a regular dropdown menu thing. The popup menu might actually be
            // better for usability in some cases, and it's how combos work on Mac
            // and BeOS, but it won't work anyway for editable combo boxes in Qt, and
            // the font issues just make it not worth it. So we'll have a dropdown
            // guy like a traditional Windows thing.
            //
            // If you want to try it out again, go to SH_ComboBox_Popup and have it
            // return 1.
            //
            // Alternatively, we could instead have the CT_MenuItem handling code try
            // to be aggressively clever and use the qt app font hash to look up the
            // expected font for a QMenu and use that for calculating its metrics.
            // Unfortunately, that probably won't work so great if the combo/menu
            // actually wants to use custom fonts in its listing, since we'd be
            // ignoring it. That's how UseQMenuForComboBoxPopup currently works,
            // though it tests for Qt::WA_SetFont as an attempt at recognizing when
            // it shouldn't use the qt font hash for QMenu.
#if 0
            if (qobject_cast<const QComboBox*>(widget)) {
                QFont font = menuItem->font;
                font.setPointSizeF(QFontInfo(menuItem->font).pointSizeF());
                painter->setFont(font);
            }
#endif

            // Draw mnemonic text
            if (t >= 0) {
                QRect mnemonicR =
                    Ph::menuItemMnemonicRect(metrics, option->direction, itemRect, hasSubMenu, menuItem->tabWidth);
                const QStringRef textToDrawRef = s.mid(t + 1);
                const QString unsafeTextToDraw = QString::fromRawData(textToDrawRef.constData(), textToDrawRef.size());
                painter->drawText(mnemonicR, text_flags, unsafeTextToDraw);
                s = s.left(t);
            }
            const QStringRef textToDrawRef = s.left(t);
            const QString unsafeTextToDraw = QString::fromRawData(textToDrawRef.constData(), textToDrawRef.size());
            painter->drawText(textRect, text_flags, unsafeTextToDraw);
        }

        // SubMenu Arrow
        if (hasSubMenu) {
            Qt::ArrowType arrow = option->direction == Qt::RightToLeft ? Qt::LeftArrow : Qt::RightArrow;
            QRect arrowRect = Ph::menuItemArrowRect(metrics, option->direction, itemRect);
            Swatchy arrowColor = isSelected ? S_highlightedText : S_indicator_current;
            Ph::drawArrow(painter, arrowRect, arrow, swatch.brush(arrowColor));
        }
        painter->restore();
        break;
    }
    case CE_MenuHMargin:
    case CE_MenuVMargin:
    case CE_MenuEmptyArea:
        break;
    case CE_PushButton: {
        auto btn = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!btn)
            break;
        // proxy()->drawControl(CE_PushButtonBevel, btn, painter, widget);
        QStyleOptionButton subopt = *btn;
        subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
        proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
        break;
    }
    case CE_PushButtonLabel: {
        auto button = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!button)
            break;
        // Apply stylesheet to set default background color and hover effect
        if (widget) {
            auto nonConstWidget = const_cast<QWidget*>(widget);
            QString baseColorString;
            QString hoverColorString;
            QString pressedColorString;
            QString textColorString;
            QString disabledTextColorString = QColor("#A0A0A0").name(); // Light gray color for disabled text

            if (button->features & QStyleOptionButton::DefaultButton) {
                // Use grey color for Cancel button
                baseColorString = baseColor.name(); // Convert QColor to string
                hoverColorString = baseColor.lighter(110).name(); // Increase brightness by 20%
                pressedColorString = baseColor.darker(120).name(); // Decrease brightness by 20%
                textColorString = "white";
            } else {
                // Use baseColor for other buttons
                if (isDarkMode()) {
                    baseColorString = QColor("#3C3C3D").name(); // Dark mode color
                    textColorString = "white";
                } else {
                    baseColorString = QColor("#DEDEDE").name(); // Light mode color
                    textColorString = "black";
                }
                hoverColorString = QColor(baseColorString).lighter(120).name(); // Increase brightness by 20%
                pressedColorString = QColor(baseColorString).darker(120).name(); // Decrease brightness by 20%
            }

            // 根据文本内容计算按钮宽度
            QFontMetrics fm(widget->font());
            int textWidth = fm.horizontalAdvance(button->text);
            int buttonWidth = textWidth + 12; // 每边添加 6 个像素内边距

            nonConstWidget->setStyleSheet(
                QString(
                    "QPushButton {"
                    "    background-color: %1;" // Use baseColor for default background color
                    "    border-radius: 11px;" // 调整圆角大小
                    "    padding: 4px 12px;" // 减小上下padding
                    "    min-height: 26px;" // 减小最小高度
                    "    color: %4;"
                    "}"
                    "QPushButton:hover {"
                    "    background-color: %2;" // Background color on hover
                    "}"
                    "QPushButton:pressed {"
                    "    background-color: %3;" // Background color when pressed
                    "}"
                    "QPushButton:disabled {"
                    "    background-color: %1;" // Use baseColor for disabled background color
                    "    color: %5;" // Set text color for disabled state
                    "}")
                    .arg(baseColorString)
                    .arg(hoverColorString)
                    .arg(pressedColorString)
                    .arg(textColorString)
                    .arg(disabledTextColorString)
            );
        }
        QRect textRect = button->rect;
        int tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
        if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
            tf |= Qt::TextHideMnemonic;
        if (!button->icon.isNull()) {
            // Center both icon and text
            QRect iconRect;
            QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
            QIcon::State state = button->state & State_On ? QIcon::On : QIcon::Off;
            auto window = widget ? widget->window()->windowHandle() : nullptr;
            QPixmap pixmap = button->icon.pixmap(window, button->iconSize, mode, state);
            int pixmapWidth = static_cast<int>(pixmap.width() / pixmap.devicePixelRatio());
            int pixmapHeight = static_cast<int>(pixmap.height() / pixmap.devicePixelRatio());
            int labelWidth = pixmapWidth;
            int labelHeight = pixmapHeight;
            // 4 is hardcoded in QPushButton::sizeHint()
            int iconSpacing = 4;
            int textWidth = button->fontMetrics.boundingRect(option->rect, tf, button->text).width();
            if (!button->text.isEmpty())
                labelWidth += (textWidth + iconSpacing);
            iconRect = QRect(textRect.x() + (textRect.width() - labelWidth) / 2,
                             textRect.y() + (textRect.height() - labelHeight) / 2,
                             pixmapWidth,
                             pixmapHeight);
            iconRect = visualRect(button->direction, textRect, iconRect);
            tf |= Qt::AlignLeft; // left align, we adjust the text-rect instead
            if (button->direction == Qt::RightToLeft)
                textRect.setRight(iconRect.left() - iconSpacing);
            else
                textRect.setLeft(iconRect.left() + iconRect.width() + iconSpacing);
            if (button->state & (State_On | State_Sunken))
                iconRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                                   proxy()->pixelMetric(PM_ButtonShiftVertical, option, widget));
            painter->drawPixmap(iconRect, pixmap);
        } else {
            tf |= Qt::AlignHCenter;
        }
        if (button->state & (State_On | State_Sunken))
            textRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, option, widget),
                               proxy()->pixelMetric(PM_ButtonShiftVertical, option, widget));
        if (button->features & QStyleOptionButton::HasMenu) {
            int indicatorSize = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
            if (button->direction == Qt::LeftToRight)
                textRect = textRect.adjusted(0, 0, -indicatorSize, 0);
            else
                textRect = textRect.adjusted(indicatorSize, 0, 0, 0);
        }
        proxy()->drawItemText(painter,
                              textRect,
                              tf,
                              button->palette,
                              (button->state & State_Enabled),
                              button->text,
                              QPalette::ButtonText);
        break;
    }
    case CE_MenuBarEmptyArea: {
        QRect rect = option->rect;
        if (Phantom::MenuBarDrawBorder) {
            Ph::fillRectEdges(painter, rect, Qt::BottomEdge, 1, swatch.color(S_window_divider));
        }
        painter->fillRect(rect.adjusted(0, 0, 0, -1), swatch.color(S_window));
        break;
    }
    case CE_TabBarTabShape: {
        auto tab = qstyleoption_cast<const QStyleOptionTab*>(option);
        if (!tab)
            break;
        bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                           && (tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::RoundedSouth));
        bool isSelected = tab->state & State_Selected;
        bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                        || (rtlHorTabs && tab->position == QStyleOptionTab::Beginning));
        bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
        int tabOverlap = pixelMetric(PM_TabBarTabOverlap, option, widget);
        const qreal rounding = Ph::TabBarTab_Rounding;
        Qt::Edge outerEdge = Qt::TopEdge;
        Qt::Edge edgeTowardNextTab = Qt::RightEdge;
        switch (tab->shape) {
        case QTabBar::RoundedNorth:
            outerEdge = Qt::TopEdge;
            edgeTowardNextTab = Qt::RightEdge;
            break;
        case QTabBar::RoundedSouth:
            outerEdge = Qt::BottomEdge;
            edgeTowardNextTab = Qt::RightEdge;
            break;
        case QTabBar::RoundedWest:
            outerEdge = Qt::LeftEdge;
            edgeTowardNextTab = Qt::BottomEdge;
            break;
        case QTabBar::RoundedEast:
            outerEdge = Qt::RightEdge;
            edgeTowardNextTab = Qt::BottomEdge;
            break;
        default:
            QCommonStyle::drawControl(element, tab, painter, widget);
            return;
        }
        Qt::Edge innerEdge = Ph::oppositeEdge(outerEdge);
        Qt::Edge edgeAwayNextTab = Ph::oppositeEdge(edgeTowardNextTab);
        QRect shapeClipRect = Ph::expandRect(option->rect, innerEdge, -2);
        QRect drawRect = Ph::expandRect(shapeClipRect, innerEdge, 3 + 2 * rounding + 1);
        if (!onlyOne && !lastTab) {
            drawRect = Ph::expandRect(drawRect, edgeTowardNextTab, tabOverlap);
            shapeClipRect = Ph::expandRect(shapeClipRect, edgeTowardNextTab, tabOverlap);
        }
        if (!isSelected) {
            int offset = proxy()->pixelMetric(PM_TabBarTabShiftVertical, option, widget);
            drawRect = Ph::expandRect(drawRect, outerEdge, -offset);
        }
        painter->save();
        painter->setClipRect(shapeClipRect);
        bool hasFrame = tab->features & QStyleOptionTab::HasFrame && !tab->documentMode;
        Swatchy tabFrameColor, thisFillColor, specular;
        if (hasFrame) {
            tabFrameColor = S_tabFrame;
            if (isSelected) {
                thisFillColor = S_tabFrame;
                specular = S_tabFrame_specular;
            } else {
                thisFillColor = S_inactiveTabYesFrame;
                specular = Ph::TabBar_InactiveTabsHaveSpecular ? S_inactiveTabYesFrame_specular : S_none;
            }
        } else {
            tabFrameColor = S_window;
            if (isSelected) {
                thisFillColor = S_window;
                specular = S_window_specular;
            } else {
                thisFillColor = S_inactiveTabNoFrame;
                specular = Ph::TabBar_InactiveTabsHaveSpecular ? S_inactiveTabNoFrame_specular : S_none;
            }
        }
        auto frameColor = isSelected ? S_frame_outline : S_window_outline;
        Ph::paintBorderedRoundRect(painter, drawRect, rounding, swatch, frameColor, thisFillColor);
        Ph::paintBorderedRoundRect(painter, drawRect.adjusted(1, 1, -1, -1), rounding, swatch, specular, S_none);
        painter->restore();
        if (isSelected) {
            QRect highlightRect = drawRect.adjusted(2, 1, -2, 0);
            highlightRect.setHeight(Ph::dpiScaled(2.0));
            QRect highlightRectSpec = highlightRect.adjusted(-1, -1, 1, 0);
            painter->fillRect(highlightRectSpec, Ph::DeriveColors::lightSpecularOf(swatch.color(S_highlight)));
            painter->fillRect(highlightRect, swatch.color(S_highlight));

            QRect refillRect = Ph::rectFromInnerEdgeWithThickness(shapeClipRect, innerEdge, 2);
            refillRect = Ph::rectTranslatedTowardEdge(refillRect, innerEdge, 2);
            refillRect = Ph::expandRect(refillRect, edgeAwayNextTab | edgeTowardNextTab, -1);
            painter->fillRect(refillRect, swatch.color(tabFrameColor));
            Ph::fillRectEdges(painter, refillRect, edgeAwayNextTab | edgeTowardNextTab, 1, swatch.color(specular));
        }
        break;
    }
    case CE_ItemViewItem: {
        auto ivopt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
        if (!ivopt)
            break;
        // Hack to work around broken grid line drawing in Qt's table view code:
        //
        // We tell it that the grid line color is a color via
        // SH_Table_GridLineColor. It draws the grid lines, but it in high DPI it's
        // broken because it uses a pen/path to draw the line, which makes it too
        // narrow, subpixel-incorrectly-antialiased, and/or offset from its correct
        // position. So when we draw the item view items in a table view, we'll
        // also try to paint 1 pixel outside of our current rect to try to fill in
        // the incorrectly painted areas where the grid lines are.
        //
        // Also note that the table views with the bad drawing code, when
        // scrolling, will leave garbage behind in the incorrectly-drawn grid line
        // areas. This will also paint over that.
        bool overdrawGridHack = false;
        if (auto tableWidget = qobject_cast<const QTableView*>(widget)) {
            overdrawGridHack = tableWidget->showGrid() && tableWidget->gridStyle() == Qt::SolidLine;
        }
        if (overdrawGridHack) {
            QRect r = option->rect.adjusted(-1, -1, 1, 1);
            Ph::fillRectOutline(painter, r, 1, swatch.color(S_base_divider));
        }
        QCommonStyle::drawControl(element, option, painter, widget);
        break;
    }
    case CE_ShapedFrame: {
        auto frameopt = qstyleoption_cast<const QStyleOptionFrame*>(option);
        if (frameopt) {
            if (frameopt->frameShape == QFrame::HLine) {
                QRect r = option->rect;
                r.setY(r.y() + r.height() / 2);
                r.setHeight(2);
                painter->fillRect(r, swatch.color(S_tabFrame_specular));
                r.setHeight(1);
                painter->fillRect(r, swatch.color(S_frame_outline));
                break;
            } else if (frameopt->frameShape == QFrame::VLine) {
                QRect r = option->rect;
                r.setX(r.x() + r.width() / 2);
                r.setWidth(2);
                painter->fillRect(r, swatch.color(S_tabFrame_specular));
                r.setWidth(1);
                painter->fillRect(r, swatch.color(S_frame_outline));
                break;
            }
        }
        QCommonStyle::drawControl(element, option, painter, widget);
        break;
    }
    case CE_ProgressBar: {
        auto progressBar = qstyleoption_cast<const QStyleOptionProgressBar*>(option);
        if (!progressBar)
            break;

        QRect rect = progressBar->rect;
        QRect filled;
        QRect nonFilled;
        bool isIndeterminate;
        Ph::progressBarFillRects(progressBar, filled, nonFilled, isIndeterminate);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // 绘制背景
        painter->setPen(Qt::NoPen);
        QColor bgColor = isDarkMode() ? QColor(60, 60, 60) : QColor(230, 230, 230);
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect, Ph::ProgressBar_Rounding, Ph::ProgressBar_Rounding);

        // 绘制进度条
        if (!isIndeterminate && !filled.isEmpty()) {
            // 创建渐变 - 从左到右
            QLinearGradient gradient;
            if (progressBar->orientation == Qt::Horizontal) {
                gradient = QLinearGradient(filled.topLeft(), filled.topRight());
            } else {
                gradient = QLinearGradient(filled.bottomLeft(), filled.topLeft());
            }
            
            // 设置渐变颜色 - 从基础颜色逐渐变亮
            gradient.setColorAt(0, baseColor);
            gradient.setColorAt(1, baseColor.lighter(115)); // 结束点变亮15%

            // 使用渐变填充整个进度条
            painter->setBrush(gradient);
            painter->drawRoundedRect(filled, Ph::ProgressBar_Rounding, Ph::ProgressBar_Rounding);
        }

        // 如果有文本，绘制文本
        if (progressBar->textVisible) {
            QString text = progressBar->text;
            if (text.isEmpty() && !isIndeterminate) {
                text = QString::number(progressBar->progress) + QStringLiteral("%");
            }
            
            // 根据进度条的选中状态设置文字颜色
            QColor textColor;
            if (progressBar->state & State_Selected) {
                textColor = swatch.color(S_highlightedText);
            } else {
                if (filled.contains(rect.center())) {
                    textColor = Qt::white;  // 在填充区域内使用白色
                } else {
                    textColor = isDarkMode() ? Qt::white : Qt::black;  // 在未填充区域根据主题使用对应颜色
                }
            }
            
            painter->setPen(textColor);
            painter->drawText(rect, Qt::AlignCenter, text);
        }

        painter->restore();
        break;
    }
    case CE_TabBarTab: {
        auto tab = qstyleoption_cast<const QStyleOptionTab*>(option);
        if (!tab)
            break;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        bool selected = tab->state & State_Selected;
        bool hovered = tab->state & State_MouseOver;
        bool enabled = tab->state & State_Enabled;
        bool pressed = tab->state & State_Sunken;
        
        // 获取标签区域
        QRect rect = tab->rect;
        
        // macOS 风格尺寸和间距调整
        const int tabHeight = 32;
        const int verticalPadding = 6;
        rect.setHeight(tabHeight);

        // 获取基础背景色（与按钮相同）
        QColor baseTabColor = isDarkMode() ? QColor("#3C3C3D") : QColor("#DEDEDE");
        
        // 背景颜色处理
        QColor bgColor;
        if (!enabled) {
            bgColor = baseTabColor;
        } else if (selected) {
            bgColor = baseColor;
            if (pressed) {
                bgColor = baseColor.darker(110);  // 选中状态下按压变暗10%
            } else if (hovered) {
                bgColor = baseColor.lighter(110);  // 选中状态下悬停变亮10%
            }
        } else {
            if (pressed) {
                bgColor = baseTabColor.darker(110);  // 未选中状态下按压变暗10%
            } else if (hovered) {
                bgColor = baseTabColor.lighter(110);  // 未选中状态下悬停变亮10%
            } else {
                bgColor = baseTabColor;
            }
        }

        // 绘制标签背景
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColor);
        
        // 绘制圆角矩形
        const int radius = 11;
        
        // 按压时的位移效果
        if (pressed && !selected) {
            rect.translate(0, 1);
        }
        
        painter->drawRoundedRect(rect, radius, radius);
        
        // 绘制分隔线（仅在未选中且未悬停且未按压时）
        if (!selected && !hovered && !pressed) {
            painter->setPen(QPen(swatch.color(S_window_outline).lighter(120), 1));
            painter->drawLine(rect.right(), rect.top() + 8, rect.right(), rect.bottom() - 8);
        }

        // 绘制文本
        if (!tab->text.isEmpty()) {
            QRect textRect = rect.adjusted(12, verticalPadding, -12, -verticalPadding);
            
            // 文本颜色
            QColor textColor;
            if (!enabled) {
                textColor = swatch.color(S_windowText).lighter(150);
            } else if (selected) {
                textColor = Qt::white;
            } else {
                textColor = swatch.color(S_windowText);
                if (pressed) {
                    textColor.setAlpha(180);  // 按压时文本更透明
                } else if (hovered) {
                    textColor = textColor.lighter(110);  // 悬停时文本变亮
                }
            }
            
            // 文本跟随按压位移
            if (pressed && !selected) {
                textRect.translate(0, 1);
            }
            
            painter->setPen(textColor);
            painter->drawText(textRect, Qt::AlignCenter, tab->text);
        }

        painter->restore();
        break;
    }
    
    default:
        QCommonStyle::drawControl(element, option, painter, widget);
        break;
    }
}

QPalette BaseStyle::lightModePalette()
{
    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::Window, QRgb(0xF7F7F7));
    palette.setColor(QPalette::Inactive, QPalette::Window, QRgb(0xFCFCFC));
    palette.setColor(QPalette::Disabled, QPalette::Window, QRgb(0xEDEDED));

    palette.setColor(QPalette::Active, QPalette::WindowText, QRgb(0x1D1D20));
    palette.setColor(QPalette::Inactive, QPalette::WindowText, QRgb(0x252528));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QRgb(0x8C8C92));

    palette.setColor(QPalette::Active, QPalette::Text, QRgb(0x1D1D20));
    palette.setColor(QPalette::Inactive, QPalette::Text, QRgb(0x252528));
    palette.setColor(QPalette::Disabled, QPalette::Text, QRgb(0x8C8C92));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, QRgb(0x71727D));
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, QRgb(0x878893));
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, QRgb(0xA3A4AC));
#endif

    palette.setColor(QPalette::Active, QPalette::BrightText, QRgb(0xF3F3F4));
    palette.setColor(QPalette::Inactive, QPalette::BrightText, QRgb(0xEAEAEB));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, QRgb(0xE4E5E7));

    palette.setColor(QPalette::Active, QPalette::Base, QRgb(0xF9F9F9));
    palette.setColor(QPalette::Inactive, QPalette::Base, QRgb(0xFCFCFC));
    palette.setColor(QPalette::Disabled, QPalette::Base, QRgb(0xEFEFF2));

    palette.setColor(QPalette::Active, QPalette::AlternateBase, QRgb(0xECF3E8));
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, QRgb(0xF1F6EE));
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, QRgb(0xE1E9DD));

    palette.setColor(QPalette::All, QPalette::ToolTipBase, QRgb(0xF7F7F7));
    palette.setColor(QPalette::All, QPalette::ToolTipText, QRgb(0x1D1D20));

    palette.setColor(QPalette::Active, QPalette::Button, QRgb(0xD4D5DD));
    palette.setColor(QPalette::Inactive, QPalette::Button, QRgb(0xDCDCE0));
    palette.setColor(QPalette::Disabled, QPalette::Button, QRgb(0xE5E5E6));

    palette.setColor(QPalette::Active, QPalette::ButtonText, QRgb(0x181A18));
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, QRgb(0x454A54));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QRgb(0x97979B));

    palette.setColor(QPalette::Active, QPalette::Highlight, QRgb(0x549CFF));
    palette.setColor(QPalette::Inactive, QPalette::Highlight, QRgb(0x96C2FF));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QRgb(0xBFDAFF));

    palette.setColor(QPalette::Active, QPalette::HighlightedText, QRgb(0xFFFFFF));
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, QRgb(0x252528));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QRgb(0x8C8C92));

    palette.setColor(QPalette::All, QPalette::Light, QRgb(0xF9F9F9));
    palette.setColor(QPalette::All, QPalette::Midlight, QRgb(0xE9E9EB));
    palette.setColor(QPalette::All, QPalette::Mid, QRgb(0xC9C9CF));
    palette.setColor(QPalette::All, QPalette::Dark, QRgb(0xBBBBC2));
    palette.setColor(QPalette::All, QPalette::Shadow, QRgb(0x6C6D79));

    palette.setColor(QPalette::All, QPalette::Link, QRgb(0x4090FF));
    palette.setColor(QPalette::Disabled, QPalette::Link, QRgb(0x3388FF));
    palette.setColor(QPalette::All, QPalette::LinkVisited, QRgb(0x4090FF));
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, QRgb(0x3388FF));

    return palette;
}

QPalette BaseStyle::darkModePalette()
{
    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::Window, QRgb(0x2C2C2D));
    palette.setColor(QPalette::Inactive, QPalette::Window, QRgb(0x2C2C2D));
    palette.setColor(QPalette::Disabled, QPalette::Window, QRgb(0x424242));

    palette.setColor(QPalette::Active, QPalette::WindowText, QRgb(0xCACBCE));
    palette.setColor(QPalette::Inactive, QPalette::WindowText, QRgb(0xC8C8C6));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QRgb(0x707070));

    palette.setColor(QPalette::Active, QPalette::Text, QRgb(0xCACBCE));
    palette.setColor(QPalette::Inactive, QPalette::Text, QRgb(0xC8C8C6));
    palette.setColor(QPalette::Disabled, QPalette::Text, QRgb(0x707070));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, QRgb(0x7D7D82));
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, QRgb(0x87888C));
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, QRgb(0x737373));
#endif

    palette.setColor(QPalette::Active, QPalette::BrightText, QRgb(0x252627));
    palette.setColor(QPalette::Inactive, QPalette::BrightText, QRgb(0x2D2D2F));
    palette.setColor(QPalette::Disabled, QPalette::BrightText, QRgb(0x333333));

    palette.setColor(QPalette::Active, QPalette::Base, QRgb(0x27272A));
    palette.setColor(QPalette::Inactive, QPalette::Base, QRgb(0x2A2A2D));
    palette.setColor(QPalette::Disabled, QPalette::Base, QRgb(0x343437));

    palette.setColor(QPalette::Active, QPalette::AlternateBase, QRgb(0x2C2C30));
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, QRgb(0x2B2B2F));
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, QRgb(0x36363A));

    palette.setColor(QPalette::All, QPalette::ToolTipBase, QRgb(0x3B3B3D));
    palette.setColor(QPalette::All, QPalette::ToolTipText, QRgb(0xCACBCE));

    palette.setColor(QPalette::Active, QPalette::Button, QRgb(0x28282B));
    palette.setColor(QPalette::Inactive, QPalette::Button, QRgb(0x28282B));
    palette.setColor(QPalette::Disabled, QPalette::Button, QRgb(0x2B2A2A));

    palette.setColor(QPalette::Active, QPalette::ButtonText, QRgb(0xB9B9BE));
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, QRgb(0x9E9FA5));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QRgb(0x73747E));

    palette.setColor(QPalette::Active, QPalette::Highlight, QRgb(0x447FCF));
    palette.setColor(QPalette::Inactive, QPalette::Highlight, QRgb(0x3B6EB3));
    palette.setColor(QPalette::Disabled, QPalette::Highlight, QRgb(0x315B94));

    palette.setColor(QPalette::Active, QPalette::HighlightedText, QRgb(0xCCCCCC));
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, QRgb(0xCECECE));
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QRgb(0x707070));

    palette.setColor(QPalette::All, QPalette::Light, QRgb(0x414145));
    palette.setColor(QPalette::All, QPalette::Midlight, QRgb(0x39393C));
    palette.setColor(QPalette::All, QPalette::Mid, QRgb(0x2F2F32));
    palette.setColor(QPalette::All, QPalette::Dark, QRgb(0x202022));
    palette.setColor(QPalette::All, QPalette::Shadow, QRgb(0x19191A));

    palette.setColor(QPalette::All, QPalette::Link, QRgb(0x68B668));
    palette.setColor(QPalette::Disabled, QPalette::Link, QRgb(0x74A474));
    palette.setColor(QPalette::All, QPalette::LinkVisited, QRgb(0x75B875));
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, QRgb(0x77A677));

    return palette;
}

QPalette BaseStyle::standardPalette() const
{
    return isDarkMode() ? darkModePalette() : lightModePalette();

    // QColor backGround(251, 251, 251);
    // QColor light = backGround.lighter(150);
    // QColor mid(backGround.darker(130));
    // QColor midLight = mid.lighter(110);
    // QColor base = Qt::white;
    // QColor disabledBase(backGround);
    // QColor dark = backGround.darker(150);
    // QColor darkDisabled = QColor(209, 209, 209).darker(110);
    // QColor text = Qt::black;
    // QColor hightlightedText = Qt::white;
    // QColor disabledText = QColor(190, 190, 190);
    // QColor button = QColor(242, 242, 242);
    // QColor shadow = dark.darker(135);
    // QColor disabledShadow = shadow.lighter(150);
    // QColor placeholder = text;
    // QColor highlightColor(84, 156, 255);
    // placeholder.setAlpha(128);

    // QPalette fusionPalette(Qt::black, backGround, light, dark, mid, text, base);
    // fusionPalette.setBrush(QPalette::Midlight, midLight);
    // fusionPalette.setBrush(QPalette::Button, button);
    // fusionPalette.setBrush(QPalette::Shadow, shadow);
    // fusionPalette.setBrush(QPalette::HighlightedText, hightlightedText);

    // fusionPalette.setBrush(QPalette::Disabled, QPalette::Text, disabledText);
    // fusionPalette.setBrush(QPalette::Disabled, QPalette::WindowText, disabledText);
    // fusionPalette.setBrush(QPalette::Disabled, QPalette::ButtonText, disabledText);
    // fusionPalette.setBrush(QPalette::Disabled, QPalette::Base, disabledBase);
    // fusionPalette.setBrush(QPalette::Disabled, QPalette::Dark, darkDisabled);
    // fusionPalette.setBrush(QPalette::Disabled, QPalette::Shadow, disabledShadow);

    // fusionPalette.setBrush(QPalette::Highlight, highlightColor);
    // fusionPalette.setBrush(QPalette::Active, QPalette::Highlight, highlightColor);
    // fusionPalette.setBrush(QPalette::Inactive, QPalette::Highlight, highlightColor);
    // fusionPalette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(145, 145, 145));

    // fusionPalette.setBrush(QPalette::Base, base);
    // fusionPalette.setBrush(QPalette::Window, base);

    // fusionPalette.setBrush(QPalette::PlaceholderText, placeholder);

    // return fusionPalette;
}

void BaseStyle::drawComplexControl(ComplexControl control,
                                   const QStyleOptionComplex* option,
                                   QPainter* painter,
                                   const QWidget* widget) const
{
#ifdef BUILD_WITH_EASY_PROFILER
    EASY_BLOCK("drawControl");
    const char* controlCString = QMetaEnum::fromType<QStyle::ComplexControl>().valueToKey(control);
    EASY_TEXT("ComplexControl", controlCString);
#endif
    using Swatchy = Phantom::Swatchy;
    using namespace Phantom::SwatchColors;
    namespace Ph = Phantom;
    auto ph_swatchPtr = Ph::getCachedSwatchOfQPalette(&d->swatchCache, &d->headSwatchFastKey, option->palette);
    const Ph::PhSwatch& swatch = *ph_swatchPtr.data();

    switch (control) {
    case CC_GroupBox: {
        auto groupBox = qstyleoption_cast<const QStyleOptionGroupBox*>(option);
        if (!groupBox)
            break;
        painter->save();
        // Draw frame
        QRect textRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
        QRect checkBoxRect = proxy()->subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);

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
            unsigned alignment = groupBox->textAlignment;
            if (!proxy()->styleHint(QStyle::SH_UnderlineShortcut, option, widget))
                alignment |= Qt::TextHideMnemonic;

            proxy()->drawItemText(painter,
                                  textRect,
                                  alignment | Qt::TextShowMnemonic | Qt::AlignLeft,
                                  groupBox->palette,
                                  groupBox->state & State_Enabled,
                                  groupBox->text,
                                  QPalette::NoRole);

            if (groupBox->state & State_HasFocus) {
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*groupBox);
                fropt.rect = textRect.adjusted(-1, 0, 1, 0);
                proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
            }
        }

        // Draw checkbox
        if (groupBox->subControls & SC_GroupBoxCheckBox) {
            QStyleOptionButton box;
            box.QStyleOption::operator=(*groupBox);
            box.rect = checkBoxRect;
            proxy()->drawPrimitive(PE_IndicatorCheckBox, &box, painter, widget);
        }
        painter->restore();
        break;
    }
    case CC_SpinBox: {
        auto spinBox = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
        if (!spinBox)
            break;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = spinBox->rect;
        bool enabled = spinBox->state & State_Enabled;
        bool hasFocus = spinBox->state & State_HasFocus;
        bool mouseOver = spinBox->state & State_MouseOver;
        bool upPressed = spinBox->activeSubControls == SC_SpinBoxUp && (spinBox->state & State_Sunken);
        bool downPressed = spinBox->activeSubControls == SC_SpinBoxDown && (spinBox->state & State_Sunken);
        bool upHovered = spinBox->activeSubControls == SC_SpinBoxUp && mouseOver;
        bool downHovered = spinBox->activeSubControls == SC_SpinBoxDown && mouseOver;

        // 获取基础颜色
        QColor baseSpinBoxColor = isDarkMode() ? QColor("#1E1E1E") : QColor("#FFFFFF");
        QColor borderColor = isDarkMode() ? QColor("#3C3C3C") : QColor("#CCCCCC");
        
        // 背景颜色
        QColor bgColor = baseSpinBoxColor;
        if (!enabled) {
            bgColor.setAlpha(180);
            borderColor.setAlpha(180);
        }

        // 绘制主体背景和边框
        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 11, 11); // 修改圆角为11

        if (spinBox->buttonSymbols != QAbstractSpinBox::NoButtons) {
            // 获取按钮区域
            QRect upRect = proxy()->subControlRect(CC_SpinBox, spinBox, SC_SpinBoxUp, widget);
            QRect downRect = proxy()->subControlRect(CC_SpinBox, spinBox, SC_SpinBoxDown, widget);

            // 绘制分隔线
            painter->setPen(borderColor);
            painter->drawLine(upRect.left(), upRect.top() + 6, upRect.left(), downRect.bottom() - 6);

            // 绘制上下箭头
            QColor arrowColor = enabled ? (isDarkMode() ? QColor("#CCCCCC") : QColor("#666666")) 
                                      : QColor(120, 120, 120);
            
            // 上箭头
            if (upPressed) {
                arrowColor.setAlpha(200);
            } else if (upHovered) {
                arrowColor = baseColor;
            }
            painter->setPen(QPen(arrowColor, 1));
            int centerX = upRect.center().x();
            int centerY = upRect.center().y();
            QPolygonF upArrow;
            upArrow << QPointF(centerX - 3, centerY + 1)
                    << QPointF(centerX, centerY - 2)
                    << QPointF(centerX + 3, centerY + 1);
            painter->drawPolyline(upArrow);

            // 下箭头
            arrowColor = enabled ? (isDarkMode() ? QColor("#CCCCCC") : QColor("#666666")) 
                               : QColor(120, 120, 120);
            if (downPressed) {
                arrowColor.setAlpha(200);
            } else if (downHovered) {
                arrowColor = baseColor;
            }
            painter->setPen(QPen(arrowColor, 1));
            centerX = downRect.center().x();
            centerY = downRect.center().y();
            QPolygonF downArrow;
            downArrow << QPointF(centerX - 3, centerY - 1)
                      << QPointF(centerX, centerY + 2)
                      << QPointF(centerX + 3, centerY - 1);
            painter->drawPolyline(downArrow);
        }

        painter->restore();
        break;
    }
    case CC_TitleBar: {
        auto titleBar = qstyleoption_cast<const QStyleOptionTitleBar*>(option);
        if (!titleBar)
            break;
        painter->save();
        const int buttonMargin = 5;
        bool active = (titleBar->titleBarState & State_Active);
        QRect fullRect = titleBar->rect;
        QPalette palette = option->palette;
        QColor highlight = option->palette.highlight().color();
        QColor outline = option->palette.dark().color();

        QColor titleBarFrameBorder(active ? highlight.darker(180) : outline.darker(110));
        QColor titleBarHighlight(active ? highlight.lighter(120) : palette.window().color().lighter(120));
        QColor textColor(active ? 0xffffff : 0xff000000);
        QColor textAlphaColor(active ? 0xffffff : 0xff000000);

        {
            // Fill title
            QColor titlebarColor = QColor(active ? highlight : palette.window().color());
            painter->fillRect(option->rect.adjusted(1, 1, -1, 0), titlebarColor);
            // Frame and rounded corners
            painter->setPen(titleBarFrameBorder);

            // top outline
            painter->drawLine(fullRect.left() + 5, fullRect.top(), fullRect.right() - 5, fullRect.top());
            painter->drawLine(fullRect.left(), fullRect.top() + 4, fullRect.left(), fullRect.bottom());
            const QPoint points[5] = {QPoint(fullRect.left() + 4, fullRect.top() + 1),
                                      QPoint(fullRect.left() + 3, fullRect.top() + 1),
                                      QPoint(fullRect.left() + 2, fullRect.top() + 2),
                                      QPoint(fullRect.left() + 1, fullRect.top() + 3),
                                      QPoint(fullRect.left() + 1, fullRect.top() + 4)};
            painter->drawPoints(points, 5);

            painter->drawLine(fullRect.right(), fullRect.top() + 4, fullRect.right(), fullRect.bottom());
            const QPoint points2[5] = {QPoint(fullRect.right() - 3, fullRect.top() + 1),
                                       QPoint(fullRect.right() - 4, fullRect.top() + 1),
                                       QPoint(fullRect.right() - 2, fullRect.top() + 2),
                                       QPoint(fullRect.right() - 1, fullRect.top() + 3),
                                       QPoint(fullRect.right() - 1, fullRect.top() + 4)};
            painter->drawPoints(points2, 5);

            // draw bottomline
            painter->drawLine(fullRect.right(), fullRect.bottom(), fullRect.left(), fullRect.bottom());

            // top highlight
            painter->setPen(titleBarHighlight);
            painter->drawLine(fullRect.left() + 6, fullRect.top() + 1, fullRect.right() - 6, fullRect.top() + 1);
        }
        // draw title
        QRect textRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarLabel, widget);
        painter->setPen(active ? (titleBar->palette.text().color().lighter(120)) : titleBar->palette.text().color());
        // Note workspace also does elliding but it does not use the correct font
        QString title = painter->fontMetrics().elidedText(titleBar->text, Qt::ElideRight, textRect.width() - 14);
        painter->drawText(textRect.adjusted(1, 1, 1, 1), title, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        painter->setPen(Qt::white);
        if (active)
            painter->drawText(textRect, title, QTextOption(Qt::AlignHCenter | Qt::AlignVCenter));
        // min button
        if ((titleBar->subControls & SC_TitleBarMinButton) && (titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint)
            && !(titleBar->titleBarState & Qt::WindowMinimized)) {
            QRect minButtonRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarMinButton, widget);
            if (minButtonRect.isValid()) {
                bool hover =
                    (titleBar->activeSubControls & SC_TitleBarMinButton) && (titleBar->state & State_MouseOver);
                bool sunken = (titleBar->activeSubControls & SC_TitleBarMinButton) && (titleBar->state & State_Sunken);
                Ph::drawMdiButton(painter, titleBar, minButtonRect, hover, sunken);
                QRect minButtonIconRect =
                    minButtonRect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);
                painter->setPen(textColor);
                painter->drawLine(minButtonIconRect.center().x() - 2,
                                  minButtonIconRect.center().y() + 3,
                                  minButtonIconRect.center().x() + 3,
                                  minButtonIconRect.center().y() + 3);
                painter->drawLine(minButtonIconRect.center().x() - 2,
                                  minButtonIconRect.center().y() + 4,
                                  minButtonIconRect.center().x() + 3,
                                  minButtonIconRect.center().y() + 4);
                painter->setPen(textAlphaColor);
                painter->drawLine(minButtonIconRect.center().x() - 3,
                                  minButtonIconRect.center().y() + 3,
                                  minButtonIconRect.center().x() - 3,
                                  minButtonIconRect.center().y() + 4);
                painter->drawLine(minButtonIconRect.center().x() + 4,
                                  minButtonIconRect.center().y() + 3,
                                  minButtonIconRect.center().x() + 4,
                                  minButtonIconRect.center().y() + 4);
            }
        }
        // max button
        if ((titleBar->subControls & SC_TitleBarMaxButton) && (titleBar->titleBarFlags & Qt::WindowMaximizeButtonHint)
            && !(titleBar->titleBarState & Qt::WindowMaximized)) {
            QRect maxButtonRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarMaxButton, widget);
            if (maxButtonRect.isValid()) {
                bool hover =
                    (titleBar->activeSubControls & SC_TitleBarMaxButton) && (titleBar->state & State_MouseOver);
                bool sunken = (titleBar->activeSubControls & SC_TitleBarMaxButton) && (titleBar->state & State_Sunken);
                Ph::drawMdiButton(painter, titleBar, maxButtonRect, hover, sunken);

                QRect maxButtonIconRect =
                    maxButtonRect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);

                painter->setPen(textColor);
                painter->drawRect(maxButtonIconRect.adjusted(0, 0, -1, -1));
                painter->drawLine(maxButtonIconRect.left() + 1,
                                  maxButtonIconRect.top() + 1,
                                  maxButtonIconRect.right() - 1,
                                  maxButtonIconRect.top() + 1);
                painter->setPen(textAlphaColor);
                const QPoint points[4] = {maxButtonIconRect.topLeft(),
                                          maxButtonIconRect.topRight(),
                                          maxButtonIconRect.bottomLeft(),
                                          maxButtonIconRect.bottomRight()};
                painter->drawPoints(points, 4);
            }
        }

        // close button
        if ((titleBar->subControls & SC_TitleBarCloseButton) && (titleBar->titleBarFlags & Qt::WindowSystemMenuHint)) {
            QRect closeButtonRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarCloseButton, widget);
            if (closeButtonRect.isValid()) {
                bool hover =
                    (titleBar->activeSubControls & SC_TitleBarCloseButton) && (titleBar->state & State_MouseOver);
                bool sunken =
                    (titleBar->activeSubControls & SC_TitleBarCloseButton) && (titleBar->state & State_Sunken);
                Ph::drawMdiButton(painter, titleBar, closeButtonRect, hover, sunken);
                QRect closeIconRect =
                    closeButtonRect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);
                painter->setPen(textAlphaColor);
                const QLine lines[4] = {QLine(closeIconRect.left() + 1,
                                              closeIconRect.top(),
                                              closeIconRect.right(),
                                              closeIconRect.bottom() - 1),
                                        QLine(closeIconRect.left(),
                                              closeIconRect.top() + 1,
                                              closeIconRect.right() - 1,
                                              closeIconRect.bottom()),
                                        QLine(closeIconRect.right() - 1,
                                              closeIconRect.top(),
                                              closeIconRect.left(),
                                              closeIconRect.bottom() - 1),
                                        QLine(closeIconRect.right(),
                                              closeIconRect.top() + 1,
                                              closeIconRect.left() + 1,
                                              closeIconRect.bottom())};
                painter->drawLines(lines, 4);
                const QPoint points[4] = {closeIconRect.topLeft(),
                                          closeIconRect.topRight(),
                                          closeIconRect.bottomLeft(),
                                          closeIconRect.bottomRight()};
                painter->drawPoints(points, 4);

                painter->setPen(textColor);
                painter->drawLine(closeIconRect.left() + 1,
                                  closeIconRect.top() + 1,
                                  closeIconRect.right() - 1,
                                  closeIconRect.bottom() - 1);
                painter->drawLine(closeIconRect.left() + 1,
                                  closeIconRect.bottom() - 1,
                                  closeIconRect.right() - 1,
                                  closeIconRect.top() + 1);
            }
        }

        // normalize button
        if ((titleBar->subControls & SC_TitleBarNormalButton)
            && (((titleBar->titleBarFlags & Qt::WindowMinimizeButtonHint)
                 && (titleBar->titleBarState & Qt::WindowMinimized))
                || ((titleBar->titleBarFlags & Qt::WindowMaximizeButtonHint)
                    && (titleBar->titleBarState & Qt::WindowMaximized)))) {
            QRect normalButtonRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarNormalButton, widget);
            if (normalButtonRect.isValid()) {

                bool hover =
                    (titleBar->activeSubControls & SC_TitleBarNormalButton) && (titleBar->state & State_MouseOver);
                bool sunken =
                    (titleBar->activeSubControls & SC_TitleBarNormalButton) && (titleBar->state & State_Sunken);
                QRect normalButtonIconRect =
                    normalButtonRect.adjusted(buttonMargin, buttonMargin, -buttonMargin, -buttonMargin);
                Ph::drawMdiButton(painter, titleBar, normalButtonRect, hover, sunken);

                QRect frontWindowRect = normalButtonIconRect.adjusted(0, 3, -3, 0);
                painter->setPen(textColor);
                painter->drawRect(frontWindowRect.adjusted(0, 0, -1, -1));
                painter->drawLine(frontWindowRect.left() + 1,
                                  frontWindowRect.top() + 1,
                                  frontWindowRect.right() - 1,
                                  frontWindowRect.top() + 1);
                painter->setPen(textAlphaColor);
                const QPoint points[4] = {frontWindowRect.topLeft(),
                                          frontWindowRect.topRight(),
                                          frontWindowRect.bottomLeft(),
                                          frontWindowRect.bottomRight()};
                painter->drawPoints(points, 4);

                QRect backWindowRect = normalButtonIconRect.adjusted(3, 0, 0, -3);
                QRegion clipRegion = backWindowRect;
                clipRegion -= frontWindowRect;
                painter->save();
                painter->setClipRegion(clipRegion);
                painter->setPen(textColor);
                painter->drawRect(backWindowRect.adjusted(0, 0, -1, -1));
                painter->drawLine(backWindowRect.left() + 1,
                                  backWindowRect.top() + 1,
                                  backWindowRect.right() - 1,
                                  backWindowRect.top() + 1);
                painter->setPen(textAlphaColor);
                const QPoint points2[4] = {backWindowRect.topLeft(),
                                           backWindowRect.topRight(),
                                           backWindowRect.bottomLeft(),
                                           backWindowRect.bottomRight()};
                painter->drawPoints(points2, 4);
                painter->restore();
            }
        }

        // context help button
        if (titleBar->subControls & SC_TitleBarContextHelpButton
            && (titleBar->titleBarFlags & Qt::WindowContextHelpButtonHint)) {
            QRect contextHelpButtonRect =
                proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarContextHelpButton, widget);
            if (contextHelpButtonRect.isValid()) {
                bool hover =
                    (titleBar->activeSubControls & SC_TitleBarContextHelpButton) && (titleBar->state & State_MouseOver);
                bool sunken =
                    (titleBar->activeSubControls & SC_TitleBarContextHelpButton) && (titleBar->state & State_Sunken);
                Ph::drawMdiButton(painter, titleBar, contextHelpButtonRect, hover, sunken);
                // This is lame, but I doubt it will get used often. Previously, XPM
                // icon was used here (very poorly, by re-allocating a QImage over and
                // over and modifying/painting it)
                QIcon helpIcon = QCommonStyle::standardIcon(QStyle::SP_DialogHelpButton);
                helpIcon.paint(painter, contextHelpButtonRect.adjusted(4, 4, -4, -4));
            }
        }

        // shade button
        if (titleBar->subControls & SC_TitleBarShadeButton && (titleBar->titleBarFlags & Qt::WindowShadeButtonHint)) {
            QRect shadeButtonRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarShadeButton, widget);
            if (shadeButtonRect.isValid()) {
                bool hover =
                    (titleBar->activeSubControls & SC_TitleBarShadeButton) && (titleBar->state & State_MouseOver);
                bool sunken =
                    (titleBar->activeSubControls & SC_TitleBarShadeButton) && (titleBar->state & State_Sunken);
                Ph::drawMdiButton(painter, titleBar, shadeButtonRect, hover, sunken);
                Ph::drawArrow(painter, shadeButtonRect.adjusted(5, 7, -5, -7), Qt::UpArrow, swatch);
            }
        }

        // unshade button
        if (titleBar->subControls & SC_TitleBarUnshadeButton && (titleBar->titleBarFlags & Qt::WindowShadeButtonHint)) {
            QRect unshadeButtonRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarUnshadeButton, widget);
            if (unshadeButtonRect.isValid()) {
                bool hover =
                    (titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_MouseOver);
                bool sunken =
                    (titleBar->activeSubControls & SC_TitleBarUnshadeButton) && (titleBar->state & State_Sunken);
                Ph::drawMdiButton(painter, titleBar, unshadeButtonRect, hover, sunken);
                Ph::drawArrow(painter, unshadeButtonRect.adjusted(5, 7, -5, -7), Qt::DownArrow, swatch);
            }
        }

        if ((titleBar->subControls & SC_TitleBarSysMenu) && (titleBar->titleBarFlags & Qt::WindowSystemMenuHint)) {
            QRect iconRect = proxy()->subControlRect(CC_TitleBar, titleBar, SC_TitleBarSysMenu, widget);
            if (iconRect.isValid()) {
                if (!titleBar->icon.isNull()) {
                    titleBar->icon.paint(painter, iconRect);
                } else {
                    QStyleOption tool = *titleBar;
                    QPixmap pm = proxy()->standardIcon(SP_TitleBarMenuButton, &tool, widget).pixmap(16, 16);
                    tool.rect = iconRect;
                    painter->save();
                    proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pm);
                    painter->restore();
                }
            }
        }
        painter->restore();
        break;
    }
    case CC_ScrollBar: {
        auto scrollBar = qstyleoption_cast<const QStyleOptionSlider*>(option);
        if (!scrollBar)
            break;
        auto pr = proxy();
        QRect scrollBarSubLine = pr->subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget);
        QRect scrollBarAddLine = pr->subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget);
        QRect scrollBarSlider = pr->subControlRect(control, scrollBar, SC_ScrollBarSlider, widget);
        QRect scrollBarGroove = pr->subControlRect(control, scrollBar, SC_ScrollBarGroove, widget);

        int padding = Ph::dpiScaled(4);
        scrollBarSlider.setX(scrollBarSlider.x() + padding);
        scrollBarSlider.setY(scrollBarSlider.y() + padding);
        // Width and height should be reduced by 2 * padding, but somehow padding is enough.
        scrollBarSlider.setWidth(scrollBarSlider.width() - padding);
        scrollBarSlider.setHeight(scrollBarSlider.height() - padding);

        // Groove/gutter/trench area
        if (scrollBar->subControls & SC_ScrollBarGroove) {
            painter->fillRect(scrollBarGroove, swatch.color(S_window));
        }

        // Slider thumb
        if (scrollBar->subControls & SC_ScrollBarSlider) {
            qreal radius =
                (scrollBar->orientation == Qt::Horizontal ? scrollBarSlider.height() : scrollBarSlider.width()) / 2.0;
            bool mouseOver((option->state & State_Active) && option->state & State_MouseOver);
            bool mousePress(option->state & State_Sunken);
            painter->fillRect(scrollBarSlider, swatch.color(S_window));
            // Ph::paintSolidRoundRect(painter, scrollBarSlider, radius, swatch, S_scrollbarSlider);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(mouseOver ? swatch.brush(S_scrollbarSlider_hover) : swatch.brush(S_scrollbarSlider));
            if (mousePress)
                painter->setBrush(swatch.brush(S_scrollbarSlider_pressed));
            painter->drawRoundedRect(scrollBarSlider, radius, radius);
            painter->restore();
        }

        // The SubLine (up/left) buttons
        if (scrollBar->subControls & SC_ScrollBarSubLine) {
            painter->fillRect(scrollBarSubLine, swatch.color(S_window));
        }

        // The AddLine (down/right) button
        if (scrollBar->subControls & SC_ScrollBarAddLine) {
            painter->fillRect(scrollBarAddLine, swatch.color(S_window));
        }
        break;
    }
    case CC_ComboBox: {
        auto comboBox = qstyleoption_cast<const QStyleOptionComboBox*>(option);
        if (!comboBox)
            break;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::TextAntialiasing);  // 添加文本抗锯齿

        QRect rect = comboBox->rect;
        bool enabled = comboBox->state & State_Enabled;
        bool hasFocus = comboBox->state & State_HasFocus;
        bool mouseOver = comboBox->state & State_MouseOver;
        bool pressed = comboBox->state & State_Sunken;
        bool editable = comboBox->editable;

        // 获取箭头区域
        QRect arrowRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxArrow, widget);
        QRect editRect = proxy()->subControlRect(CC_ComboBox, comboBox, SC_ComboBoxEditField, widget);

        // 获取基础颜色
        QColor baseComboColor = isDarkMode() ? QColor("#1E1E1E") : QColor("#FFFFFF");
        QColor borderColor = isDarkMode() ? QColor("#3C3C3C") : QColor("#CCCCCC");
        QColor arrowColor = isDarkMode() ? QColor("#CCCCCC") : QColor("#666666");

        // 背景颜色
        QColor bgColor = baseComboColor;
        if (!enabled) {
            bgColor.setAlpha(180);
            borderColor.setAlpha(180);
        } else if (pressed) {
            bgColor = bgColor.darker(105);
        } else if (mouseOver) {
            bgColor = bgColor.lighter(103);
        }

        // 绘制主体背景和边框
        painter->setPen(QPen(borderColor, 1));
        painter->setBrush(bgColor);
        painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 11, 11);

        // 绘制分隔线（如果是可编辑的）
        if (editable) {
            painter->setPen(borderColor);
            painter->drawLine(arrowRect.left(), arrowRect.top() + 6,
                             arrowRect.left(), arrowRect.bottom() - 6);
        }

        // 绘制下拉箭头
        if (comboBox->subControls & SC_ComboBoxArrow) {
            if (!enabled) {
                arrowColor.setAlpha(180);
            } else if (pressed) {
                arrowColor.setAlpha(200);
            } else if (mouseOver) {
                arrowColor = baseColor;
            }

            painter->setPen(QPen(arrowColor, 1.2));
            
            // 绘制箭头
            int centerX = arrowRect.center().x();
            int centerY = arrowRect.center().y();
            const int arrowSize = 4;
            
            QPolygonF arrow;
            arrow << QPointF(centerX - arrowSize, centerY - arrowSize/2)
                  << QPointF(centerX, centerY + arrowSize/2)
                  << QPointF(centerX + arrowSize, centerY - arrowSize/2);
            
            painter->drawPolyline(arrow);
        }

        // 绘制文本（如果不是可编辑的）
        // if (!editable && !comboBox->currentText.isEmpty()) {
        //     QRect textRect = editRect;
        //     textRect.adjust(6, 0, -6, 0); // 添加水平内边距
            
        //     QString text = comboBox->currentText;
        //     QFontMetrics fm(comboBox->fontMetrics);
        //     text = fm.elidedText(text, Qt::ElideRight, textRect.width());

        //     QColor textColor;
        //     if (!enabled) {
        //         textColor = isDarkMode() ? QColor("#808080") : QColor("#A0A0A0");
        //     } else {
        //         textColor = isDarkMode() ? Qt::white : Qt::black;
        //     }

        //     // 清除之前的文本渲染设置
        //     painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            
        //     // 设置文本颜色和绘制文本
        //     painter->setPen(textColor);
        //     painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
        // }

        painter->restore();
        break;
    }
    case CC_Slider: {
        auto slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
        if (!slider)
            break;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect groove = proxy()->subControlRect(CC_Slider, slider, SC_SliderGroove, widget);
        QRect handle = proxy()->subControlRect(CC_Slider, slider, SC_SliderHandle, widget);

        bool horizontal = slider->orientation == Qt::Horizontal;
        bool enabled = slider->state & State_Enabled;
        bool pressed = slider->state & State_Sunken;
        bool hovered = slider->state & State_MouseOver;

        // 绘制轨道
        if (slider->subControls & SC_SliderGroove) {
            // 轨道高度
            const int grooveHeight = 4;
            
            // 调整轨道位置使其垂直居中
            if (horizontal) {
                groove.setHeight(grooveHeight);
                groove.moveCenter(slider->rect.center());
            } else {
                groove.setWidth(grooveHeight);
                groove.moveCenter(slider->rect.center());
            }

            // 计算已填充部分的矩形
            QRect filledGroove = groove;
            if (horizontal) {
                filledGroove.setWidth(handle.center().x() - groove.left());
            } else {
                filledGroove.setTop(handle.center().y());
                filledGroove.setBottom(groove.bottom());
            }

            // 绘制未填充部分（灰色背景）
            QColor grooveColor = isDarkMode() ? QColor(60, 60, 60) : QColor(200, 200, 200);
            painter->setPen(Qt::NoPen);
            painter->setBrush(grooveColor);
            painter->drawRoundedRect(groove, grooveHeight/2, grooveHeight/2);

            // 绘制已填充部分（主题色）
            QColor activeColor = baseColor;
            if (!enabled) {
                activeColor.setAlpha(120);
            } else if (hovered || pressed) {
                activeColor = activeColor.lighter(110);
            }
            painter->setBrush(activeColor);
            painter->drawRoundedRect(filledGroove, grooveHeight/2, grooveHeight/2);
        }

        // 绘制滑块
        if (slider->subControls & SC_SliderHandle) {
            // 滑块大小
            const int handleSize = 16;
            QRect handleRect = handle;
            
            // 调整滑块大小
            if (horizontal) {
                handleRect.setWidth(handleSize);
                handleRect.setHeight(handleSize);
                handleRect.moveCenter(QPoint(handle.center().x(), groove.center().y()));
            } else {
                handleRect.setWidth(handleSize);
                handleRect.setHeight(handleSize);
                handleRect.moveCenter(QPoint(groove.center().x(), handle.center().y()));
            }

            // 滑块颜色
            QColor handleColor = Qt::white;
            if (!enabled) {
                handleColor.setAlpha(180);
            }

            // 绘制滑块阴影
            if (enabled) {
                QColor shadowColor(0, 0, 0, 30);
                painter->setPen(Qt::NoPen);
                painter->setBrush(shadowColor);
                painter->drawEllipse(handleRect.adjusted(0, 1, 0, 1));
            }

            // 绘制滑块
            painter->setPen(Qt::NoPen);
            painter->setBrush(handleColor);
            painter->drawEllipse(handleRect);

            // 如果被按下或悬停，添加边框效果
            if (enabled && (pressed || hovered)) {
                painter->setPen(QPen(baseColor.lighter(110), 1));
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(handleRect);
            }
        }

        painter->restore();
        break;
    }
    case CC_ToolButton: {
        auto tbopt = qstyleoption_cast<const QStyleOptionToolButton*>(option);
        if (!tbopt || !widget) {
            break;
        }

        QPainterPath path;
        QRect rect = tbopt->rect;
        bool enabled = tbopt->state & QStyle::State_Enabled;
        bool pressed = tbopt->state & QStyle::State_Sunken;
        bool hovered = tbopt->state & QStyle::State_MouseOver;
        bool checked = tbopt->state & QStyle::State_On;

        // macOS 风格的工具按钮通常具有圆角
        int radius = 6;

        // 绘制背景
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QColor bgColor;
        if (!enabled) {
            // 禁用状态
            bgColor = isDarkMode() ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 40);
        } else if (pressed || checked) {
            // 按下或选中状态
            bgColor = baseColor;
        } else if (hovered) {
            // 悬停状态
            bgColor = baseColor;
            bgColor.setAlpha(50);
        } else {
            // 正常状态
            bgColor = isDarkMode() ? QColor(255, 255, 255, 15) : QColor(0, 0, 0, 15);
        }

        if (bgColor != Qt::transparent) {
            path.addRoundedRect(rect, radius, radius);
            painter->fillPath(path, bgColor);
        }

        // 判断显示内容
        bool hasIcon = !tbopt->icon.isNull();  // 是否有图标
        bool hasText = !tbopt->text.isEmpty(); // 是否有文字
        bool textBesideIcon = tbopt->toolButtonStyle == Qt::ToolButtonTextBesideIcon;
        bool textUnderIcon = tbopt->toolButtonStyle == Qt::ToolButtonTextUnderIcon;
        bool textOnly = tbopt->toolButtonStyle == Qt::ToolButtonTextOnly;
        bool iconOnly = tbopt->toolButtonStyle == Qt::ToolButtonIconOnly;

        // 绘制图标
        if (hasIcon && !textOnly) {
            QRect iconRect;
            if (textBesideIcon) {
                iconRect = rect;
                iconRect.setWidth(rect.height());
            } else if (textUnderIcon) {
                iconRect = rect;
                iconRect.setHeight(rect.height() * 0.7);
            } else {
                iconRect = rect;
            }

            QIcon::Mode mode = enabled ? QIcon::Normal : QIcon::Disabled;
            if (pressed || checked) {
                painter->setBrush(Qt::white);
                mode = QIcon::Normal;
            } else if (hovered) {
                mode = QIcon::Active;
            }

            QIcon::State state = checked ? QIcon::On : QIcon::Off;
            QPixmap pixmap = tbopt->icon.pixmap(tbopt->iconSize, mode, state);
            proxy()->drawItemPixmap(painter, iconRect, Qt::AlignCenter, pixmap);
        }

        // 绘制文字
        if (hasText && !iconOnly) {
            QRect textRect;
            if (textBesideIcon && hasIcon) {
                textRect = rect;
                textRect.setLeft(rect.height() + 4);
            } else if (textUnderIcon && hasIcon) {
                textRect = rect;
                textRect.setTop(rect.height() * 0.7);
            } else {
                textRect = rect;
            }

            // 设置文字颜色
            QColor textColor;
            if (!enabled) {
                textColor = isDarkMode() ? QColor(255, 255, 255, 80) : QColor(0, 0, 0, 80);
            } else if (pressed || checked) {
                // 按下或选中状态文字变白
                textColor = Qt::white;
            } else {
                textColor = isDarkMode() ? Qt::white : Qt::black;
            }

            painter->setPen(textColor);
            painter->drawText(textRect, Qt::AlignCenter, tbopt->text);
        }

        painter->restore();
        break;
    }
    case CC_Dial:
        if (auto dial = qstyleoption_cast<const QStyleOptionSlider*>(option))
            Ph::drawDial(dial, painter);
        break;
    default:
        QCommonStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

int BaseStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    // Calculate pixel metrics.
    // Use immediate return if value is not supposed to be dpi-scaled.
    int val = -1;
    switch (metric) {
    case PM_SliderTickmarkOffset:
        val = 6;
        break;
    case PM_ToolTipLabelFrameWidth:
    case PM_HeaderMargin:
    case PM_ButtonMargin:
    case PM_SpinBoxFrameWidth:
        val = Phantom::DefaultFrameWidth;
        break;
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
        val = 0;
        break;
    case PM_ButtonShiftVertical:
        if (qobject_cast<const QToolButton*>(widget)) {
            return 0;
        }
        val = 1;
        break;
    case PM_ComboBoxFrameWidth:
        return 1;
    case PM_DefaultFrameWidth:
        // Original comment from fusion:
        // Do not dpi-scale because the drawn frame is always exactly 1 pixel thick
        // My note:
        // I seriously doubt, with all of the hacky add-or-remove-1 things
        // everywhere in fusion (and still in phantom), and the fact that fusion is
        // totally broken in high dpi, that this actually holds true.
        if (qobject_cast<const QAbstractItemView*>(widget)) {
            return 1;
        }
        val = qMax(1, Phantom::DefaultFrameWidth - 2);
        break;
    case PM_MessageBoxIconSize:
        val = 48;
        break;
    case PM_DialogButtonsSeparator:
    case PM_ScrollBarSliderMin:
        val = 26;
        break;
    case PM_TitleBarHeight:
        val = 24;
        break;
    case PM_ScrollBarExtent:
        val = 14;
        break;
    case PM_SliderThickness:
    case PM_SliderLength:
        val = 15;
        break;
    case PM_DockWidgetTitleMargin:
        val = 1;
        break;
    case PM_MenuVMargin:
        val = Phantom::DefaultFrame_Radius / 2;
        break;
    case PM_MenuHMargin:
        val = Phantom::DefaultFrame_Radius / 2;
        break;
    case PM_MenuPanelWidth:
        val = 0;
        break;
    case PM_MenuBarItemSpacing:
        val = Phantom::MenuBar_ItemSpacing / 2;
        break;
    case PM_MenuBarHMargin:
        // option is usually nullptr, use widget instead to get font metrics
        if (!Phantom::MenuBarLeftMargin || !widget) {
            val = 0;
            break;
        }
        return widget->fontMetrics().height() * Phantom::MenuBar_HorizontalPaddingFontRatio;
    case PM_MenuBarVMargin:
    case PM_MenuBarPanelWidth:
        val = 0;
        break;
    case PM_ToolBarSeparatorExtent:
        val = 9;
        break;
    case PM_ToolBarHandleExtent: {
        int dotLen = Phantom::dpiScaled(2);
        return dotLen * (3 * 2 - 1);
    }
    case PM_ToolBarItemSpacing:
        val = 1;
        break;
    case PM_ToolBarFrameWidth:
        val = Phantom::MenuBar_FrameWidth;
        break;
    case PM_ToolBarItemMargin:
        val = 1;
        break;
    case PM_ToolBarExtensionExtent:
        val = 32;
        break;
    case PM_ListViewIconSize:
    case PM_SmallIconSize:
        if (Phantom::ItemView_UseFontHeightForDecorationSize && widget
            && qobject_cast<const QAbstractItemView*>(widget)) {
            // QAbstractItemView::viewOptions() always uses nullptr for the
            // styleoption when querying for PM_SmallIconSize. The best we can do is
            // use the font set on the widget itself, which is obviously going to be
            // wrong if the row has a custom font set on it. Hmm.
            return widget->fontMetrics().height();
        }
        val = 16;
        break;
    case PM_ButtonIconSize: {
        if (option)
            return option->fontMetrics.height();
        if (widget)
            return widget->fontMetrics().height();
        val = 16;
        break;
    }
    case PM_DockWidgetTitleBarButtonMargin:
        val = 2;
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    case PM_TitleBarButtonSize:
        val = 19;
        break;
#endif
    case PM_MaximumDragDistance:
        return -1; // Do not dpi-scale because the value is magic
    case PM_TabCloseIndicatorWidth:
    case PM_TabCloseIndicatorHeight:
        val = 16;
        break;
    case PM_TabBarTabHSpace:
        // Contents may clip out horizontally if we don't some extra pixels here or
        // in sizeFromContents for CT_TabBarTab.
        if (!option)
            break;
        return static_cast<int>(option->fontMetrics.height() * Phantom::TabBar_HPaddingFontRatio)
               + static_cast<int>(Phantom::dpiScaled(4));
    case PM_TabBarTabVSpace:
        if (!option)
            break;
        return static_cast<int>(option->fontMetrics.height() * Phantom::TabBar_VPaddingFontRatio)
               + static_cast<int>(Phantom::dpiScaled(2));
    case PM_TabBarTabOverlap:
        val = 1;
        break;
    case PM_TabBarBaseOverlap:
        val = 2;
        break;
    case PM_TabBarIconSize: {
        if (!widget)
            break;
        return widget->fontMetrics().height();
    }
    case PM_TabBarTabShiftVertical: {
        val = Phantom::TabBar_InctiveVShift;
        break;
    }
    case PM_SubMenuOverlap:
        val = 0;
        break;
    case PM_DockWidgetHandleExtent:
    case PM_SplitterWidth:
        val = 5;
        break;
    case PM_IndicatorHeight:
    case PM_IndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
        if (option)
            return option->fontMetrics.height();
        if (widget)
            return widget->fontMetrics().height();
        val = 14;
        break;
    case PM_ScrollView_ScrollBarOverlap:
    case PM_ScrollView_ScrollBarSpacing:
        val = 0;
        break;
    case PM_TreeViewIndentation: {
        if (widget)
            return widget->fontMetrics().height();
        val = 12;
        break;
    }
    default:
        val = QCommonStyle::pixelMetric(metric, option, widget);
    }
    return Phantom::dpiScaled(val);
}

QSize BaseStyle::sizeFromContents(ContentsType type,
                                  const QStyleOption* option,
                                  const QSize& size,
                                  const QWidget* widget) const
{
    namespace Ph = Phantom;
    // Cases which do not rely on the parent class to do any work
    switch (type) {
    case CT_RadioButton:
    case CT_CheckBox: {
        auto btn = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!btn)
            break;
        bool isRadio = type == CT_RadioButton;
        int w = proxy()->pixelMetric(isRadio ? PM_ExclusiveIndicatorWidth : PM_IndicatorWidth, btn, widget);
        int h = proxy()->pixelMetric(isRadio ? PM_ExclusiveIndicatorHeight : PM_IndicatorHeight, btn, widget);
        int margins = 0;
        if (!btn->icon.isNull() || !btn->text.isEmpty())
            margins =
                proxy()->pixelMetric(isRadio ? PM_RadioButtonLabelSpacing : PM_CheckBoxLabelSpacing, option, widget);
        return QSize(size.width() + w + margins, qMax(size.height(), h));
    }
    case CT_MenuBarItem: {
        return QSize(QCommonStyle::sizeFromContents(type, option, size, widget) + QSize(8, 5));

        // int fontHeight = option ? option->fontMetrics.height() : size.height();
        // int w = static_cast<int>(fontHeight * Ph::MenuBar_HorizontalPaddingFontRatio);
        // int h = static_cast<int>(fontHeight * Ph::MenuBar_VerticalPaddingFontRatio);
        // int line = Ph::dpiScaled(1);
        // return QSize(size.width() + w * 2, size.height() + h * 2 + line);
    }
    case CT_MenuItem: {
        auto menuItem = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
        if (!menuItem)
            return size;
        bool hasTabChar = menuItem->text.contains(QLatin1Char('\t'));
        bool hasSubMenu = menuItem->menuItemType == QStyleOptionMenuItem::SubMenu;
        bool isSeparator = menuItem->menuItemType == QStyleOptionMenuItem::Separator;
        int fontMetricsHeight = -1;
        // See notes at CE_MenuItem and SH_ComboBox_Popup for more information
        if (Ph::UseQMenuForComboBoxPopup && qobject_cast<const QComboBox*>(widget)) {
            if (!widget->testAttribute(Qt::WA_SetFont))
                fontMetricsHeight = QFontMetrics(qApp->font("QMenu")).height();
        }
        if (fontMetricsHeight == -1) {
            fontMetricsHeight = option->fontMetrics.height();
        }
        auto metrics = Ph::MenuItemMetrics::ofFontHeight(fontMetricsHeight);
        // Incoming width is the sum of the visual widths of the main item text and
        // the mnemonic text (if any). To this width we will add the widths of the
        // other features for this menu item -- the icon/checkbox, spacing between
        // icon/text/mnemonic, etc. For cases like separators without any text, we
        // may disregard the width.
        //
        // Height is the text height, probably.
        int w = size.width();
        // Frame
        w += metrics.frameThickness * 2;
        // Left margins don't depend on whether or not we have a submenu arrow.
        // Calculating the right margins requires knowing whether or not the menu
        // item has a submenu arrow.
        w += metrics.leftMargin;
        // Phantom treats every menu item with the same space on the left for a
        // check mark, even if it doesn't have the checkable property.
        w += metrics.checkWidth + metrics.checkRightSpace;

        if (!menuItem->icon.isNull()) {
            // Phantom disregards any user-specified icon sizing at the moment.
            w += metrics.fontHeight;
            w += metrics.iconRightSpace;
        }

        // Tab character is used for separating the shortcut text
        if (hasTabChar)
            w += metrics.mnemonicSpace;
        if (hasSubMenu)
            w += metrics.arrowSpace + metrics.arrowWidth + metrics.rightMarginForArrow;
        else
            w += metrics.rightMarginForText;
        int h;
        if (isSeparator) {
            h = metrics.separatorHeight;
        } else {
            h = metrics.totalHeight;
        }
        if (!menuItem->icon.isNull()) {
            if (auto combo = qobject_cast<const QComboBox*>(widget)) {
                h = qMax(combo->iconSize().height() + 2, h);
            }
        }
        QSize sz;
        sz.setWidth(qMax<int>(w, Ph::dpiScaled(Ph::MenuMinimumWidth)));
        sz.setHeight(h);
        return sz;
    }
    case CT_Menu: {
        if (!Ph::MenuExtraBottomMargin || !option || !widget)
            break;
        // Trick the QMenu into putting a margin only at the bottom by adding extra
        // height to the contents size. We only want to add this tricky space if
        // there is at least more than 1 item in the menu.
        const auto acts = widget->actions();
        if (acts.count() < 2)
            break;
        // We only want to add the tricky space if there's at least 1 separator,
        // otherwise it looks weird.
        bool anySeps = false;
        for (auto act : acts) {
            if (act->isSeparator()) {
                anySeps = true;
                break;
            }
        }
        if (!anySeps)
            break;
        // int fheight = option->fontMetrics.height();
        // int vmargin = static_cast<int>(fheight * Ph::MenuItem_SeparatorHeightFontRatio) / 2;
        // QSize sz = size;
        // sz.setHeight(sz.height() + vmargin);
        return size;
    }
    case CT_TabBarTab: {
        // Placeholder in case we change this in the future
        return size;
    }
    case CT_Slider: {
        QSize sz = size;
        // mitigate zero-pointer dereference
        if (!widget || qobject_cast<const QSlider*>(widget)->orientation() == Qt::Horizontal) {
            sz.setHeight(sz.height() + PM_SliderTickmarkOffset);
        } else {
            sz.setWidth(sz.width() + PM_SliderTickmarkOffset);
        }
        return sz;
    }
    case CT_GroupBox: {
        // This doesn't seem to get used except once by QGroupBox for
        // minimumSizeHint(). After that, the sizing/layout calculations seem to
        // use the rects given by subControlRect().
        auto opt = qstyleoption_cast<const QStyleOptionGroupBox*>(option);
        if (!opt)
            break;
        // Checkbox and text height already accounted for, but margin between text
        // and frame isn't.
        int xadd = 0;
        int yadd = 0;
        if (opt->subControls & (SC_GroupBoxCheckBox | SC_GroupBoxLabel)) {
            int fontHeight = option->fontMetrics.height();
            yadd += static_cast<int>(fontHeight * Phantom::GroupBox_LabelBottomMarginFontRatio);
        }
        // We can test for the frame in general, but unfortunately testing to see
        // if it's the 1-line "flat" style or 4-line box/rect "anything else" style
        // doesn't seem to be possible here, only when painting.
        if (opt->subControls & SC_GroupBoxFrame) {
            xadd += 2;
            yadd += 2;
        }
        return QSize(size.width() + xadd, size.height() + yadd);
    }
    case CT_ItemViewItem: {
        auto vopt = qstyleoption_cast<const QStyleOptionViewItem*>(option);
        if (!vopt)
            break;
        QSize sz = QCommonStyle::sizeFromContents(type, option, size, widget);
        sz += QSize(0, Phantom::DefaultFrameWidth);
        // QCommonStyle has a bunch of complicated logic for laying out/calculating
        // rects of view items, which is locked behind a private data guy. In
        // sizeFromContents for CT_ItemViewItem, it unions all of the item row's
        // rects together and then, if the decoration height is exactly the same as
        // the row height, it adds 2 pixels (not dpi scaled) to the height. The
        // comment says it's to prevent "icons from overlapping" but I have no idea
        // how that's supposed to help. And we don't necessarily want those extra 2
        // pixels. Anyway, I don't want to copy and paste all of that code into
        // Phantom and then maintain it. So when Phantom is in the mode where we're
        // basing the item view decoration sizes off of the font size, we'll just
        // take a guess when QCommonStyle has added 2 to the height (because the
        // row height and decoration height are both the font height), and
        // re-remove those two pixels.
#if 1
        if (Phantom::ItemView_UseFontHeightForDecorationSize) {
            int fh = vopt->fontMetrics.height();
            if (sz.height() == fh + 2 && vopt->decorationSize.height() == fh) {
                sz.setHeight(fh);
            }
        }
#endif
        return sz;
    }
    case CT_HeaderSection: {
        auto hdr = qstyleoption_cast<const QStyleOptionHeader*>(option);
        if (!hdr)
            break;
        // This is pretty crummy. Should also check if we need multi-line support
        // or not.
        bool nullIcon = hdr->icon.isNull();
        int margin = proxy()->pixelMetric(QStyle::PM_HeaderMargin, hdr, widget);
        int iconSize = nullIcon ? 0 : option->fontMetrics.height();
        QSize txt = hdr->fontMetrics.size(Qt::TextSingleLine | Qt::TextBypassShaping, hdr->text);
        QSize sz;
        sz.setHeight(margin + qMax(iconSize, txt.height()) + margin);
        sz.setWidth((nullIcon ? 0 : margin) + iconSize + (hdr->text.isNull() ? 0 : margin) + txt.width() + margin);
        if (hdr->sortIndicator != QStyleOptionHeader::None) {
            if (hdr->orientation == Qt::Horizontal)
                sz.rwidth() += sz.height() + margin;
            else
                sz.rheight() += sz.width() + margin;
        }
        return sz;
    }
    default:
        break;
    }

    // Cases which modify the size given by the parent class
    QSize newSize = QCommonStyle::sizeFromContents(type, option, size, widget);
    switch (type) {
    case CT_PushButton: {
        auto pbopt = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!pbopt || pbopt->text.isEmpty())
            break;
        int hpad = static_cast<int>(pbopt->fontMetrics.height() * Phantom::PushButton_HorizontalPaddingFontHeightRatio);
        newSize.rwidth() += hpad * 2;
        if (widget && qobject_cast<const QDialogButtonBox*>(widget->parent())) {
            int dialogButtonMinWidth = Phantom::dpiScaled(80);
            newSize.rwidth() = qMax(newSize.width(), dialogButtonMinWidth);
        }
        break;
    }
    case CT_ToolButton:
#if defined(Q_OS_MACOS)
        newSize += QSize(Ph::dpiScaled(6 + Phantom::DefaultFrameWidth), Ph::dpiScaled(6 + Phantom::DefaultFrameWidth));
#elif defined(Q_OS_WIN)
        newSize += QSize(Ph::dpiScaled(4 + Phantom::DefaultFrameWidth), Ph::dpiScaled(4 + Phantom::DefaultFrameWidth));
#else
        newSize += QSize(Ph::dpiScaled(3 + Phantom::DefaultFrameWidth), Ph::dpiScaled(3 + Phantom::DefaultFrameWidth));
#endif
        break;
    case CT_ComboBox: {
        newSize += QSize(0, Ph::dpiScaled(4 + Phantom::DefaultFrameWidth));
        auto cb = qstyleoption_cast<const QStyleOptionComboBox*>(option);
        // Non-editable combo boxes have some extra padding on the left side,
        // similar to push buttons. We should account for that here to avoid text
        // being clipped off.
        if (cb) {
            int pad = 0;
            if (cb->editable) {
                pad = Ph::dpiScaled(Ph::LineEdit_ContentsHPad);
            } else {
                pad = Ph::dpiScaled(Ph::ComboBox_NonEditable_ContentsHPad);
            }
            newSize.rwidth() += pad * 2;
        }
        break;
    }
    case CT_LineEdit: {
        newSize += QSize(0, 4);
        int pad = Ph::dpiScaled(Ph::LineEdit_ContentsHPad);
        newSize.rwidth() += pad * 2;
        break;
    }
    case CT_SpinBox:
        // No changes needed
        break;
    case CT_SizeGrip:
        newSize += QSize(4, 4);
        break;
    case CT_MdiControls:
        newSize -= QSize(1, 0);
        break;
    default:
        break;
    }
    return newSize;
}

void BaseStyle::polish(QApplication* app)
{
    QCommonStyle::polish(app);

    app->setPalette(standardPalette());

    // if (QObject *obj = hintsSettings()) {
    //     connect(obj, SIGNAL(systemFontChanged(QString)), this, SLOT(updateAppFont()));
    //     connect(obj, SIGNAL(systemFontPointSizeChanged(qreal)), this, SLOT(updateAppFont()));
    // }
}

void BaseStyle::unpolish(QApplication* app)
{
    QCommonStyle::unpolish(app);

    // if (QObject *obj = hintsSettings()) {
    //     disconnect(obj, SIGNAL(systemFontChanged(QString)), this, SLOT(updateAppFont()));
    //     disconnect(obj, SIGNAL(systemFontPointSizeChanged(qreal)), this, SLOT(updateAppFont()));
    // }
}

void BaseStyle::polish(QWidget *widget)
{
    QCommonStyle::polish(widget);
    if (false
            || qobject_cast<QAbstractButton*>(widget)
            || qobject_cast<QComboBox *>(widget)
            || qobject_cast<QProgressBar *>(widget)
            || qobject_cast<QScrollBar *>(widget)
            || qobject_cast<QSplitterHandle *>(widget)
            || qobject_cast<QAbstractSlider *>(widget)
            || qobject_cast<QAbstractSpinBox *>(widget)
            || (widget->inherits("QDockSeparator"))
            || (widget->inherits("QDockWidgetSeparator"))
            ) {
        widget->setAttribute(Qt::WA_Hover, true);
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);

        // 监听主题变化
        if (qobject_cast<QPushButton*>(widget)) {
            QDBusConnection::sessionBus().connect(
                "com.lingmo.Settings",
                "/Theme",
                "com.lingmo.Theme",
                "darkModeChanged",
                widget,
                SLOT(update())
            );
        }
    }

    if (qobject_cast<QMenu *>(widget)) {
        widget->setAttribute(Qt::WA_TranslucentBackground, true);
        // m_blurHelper->registerWidget(widget);
    }

    if (widget->inherits("QTipLabel") || widget->inherits("QComboBoxPrivateContainer")) {
        widget->setAttribute(Qt::WA_TranslucentBackground, true);
    }

    m_shadowHelper->registerWidget(widget);
}

void BaseStyle::unpolish(QWidget *widget)
{
    QCommonStyle::unpolish(widget);
    if (false
            || qobject_cast<QAbstractButton*>(widget)
            || qobject_cast<QComboBox *>(widget)
            || qobject_cast<QProgressBar *>(widget)
            || qobject_cast<QScrollBar *>(widget)
            || qobject_cast<QSplitterHandle *>(widget)
            || qobject_cast<QAbstractSlider *>(widget)
            || qobject_cast<QAbstractSpinBox *>(widget)
            || (widget->inherits("QDockSeparator"))
            || (widget->inherits("QDockWidgetSeparator"))
            ) {
        widget->setAttribute(Qt::WA_Hover, false);

        // 断开主题变化的连接
        if (qobject_cast<QPushButton*>(widget)) {
            QDBusConnection::sessionBus().disconnect(
                "com.lingmo.Settings",
                "/Theme",
                "com.lingmo.Theme",
                "darkModeChanged",
                widget,
                SLOT(update())
            );
        }
    }

    if (qobject_cast<QMenu *>(widget)) {
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        // m_blurHelper->unregisterWidget(widget);
    }

    if (widget->inherits("QTipLabel")) {
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
    }

    m_shadowHelper->unregisterWidget(widget);
}

bool BaseStyle::isDarkMode() const
{
    QSettings settings(QSettings::UserScope, "lingmoos", "theme");
    return settings.value("DarkMode", false).toBool();
}

QRect BaseStyle::subControlRect(ComplexControl control,
                                const QStyleOptionComplex* option,
                                SubControl subControl,
                                const QWidget* widget) const
{
    namespace Ph = Phantom;
    QRect rect = QCommonStyle::subControlRect(control, option, subControl, widget);
    switch (control) {
    case CC_Slider: {
        auto slider = qstyleoption_cast<const QStyleOptionSlider*>(option);
        if (!slider)
            break;
        int tickSize = proxy()->pixelMetric(PM_SliderTickmarkOffset, option, widget);
        switch (subControl) {
        case SC_SliderHandle: {
            if (slider->orientation == Qt::Horizontal) {
                rect.setHeight(proxy()->pixelMetric(PM_SliderThickness));
                rect.setWidth(proxy()->pixelMetric(PM_SliderLength));
                int centerY = slider->rect.center().y() - rect.height() / 2;
                if (slider->tickPosition & QSlider::TicksAbove)
                    centerY += tickSize;
                if (slider->tickPosition & QSlider::TicksBelow)
                    centerY -= tickSize;
                rect.moveTop(centerY);
            } else {
                rect.setWidth(proxy()->pixelMetric(PM_SliderThickness));
                rect.setHeight(proxy()->pixelMetric(PM_SliderLength));
                int centerX = slider->rect.center().x() - rect.width() / 2;
                if (slider->tickPosition & QSlider::TicksAbove)
                    centerX += tickSize;
                if (slider->tickPosition & QSlider::TicksBelow)
                    centerX -= tickSize;
                rect.moveLeft(centerX);
            }
            break;
        }
        case SC_SliderGroove: {
            QPoint grooveCenter = slider->rect.center();
            const int grooveThickness = Ph::dpiScaled(7);
            if (slider->orientation == Qt::Horizontal) {
                rect.setHeight(grooveThickness);
                if (slider->tickPosition & QSlider::TicksAbove)
                    grooveCenter.ry() += tickSize;
                if (slider->tickPosition & QSlider::TicksBelow)
                    grooveCenter.ry() -= tickSize;
            } else {
                rect.setWidth(grooveThickness);
                if (slider->tickPosition & QSlider::TicksAbove)
                    grooveCenter.rx() += tickSize;
                if (slider->tickPosition & QSlider::TicksBelow)
                    grooveCenter.rx() -= tickSize;
            }
            rect.moveCenter(grooveCenter);
            break;
        }
        default:
            break;
        }
        break;
    }
    case CC_SpinBox: {
        auto spinbox = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
        if (!spinbox)
            break;
        // Some leftover Fusion code here. Should clean up this mess.
        int center = spinbox->rect.height() / 2;
        int fw = spinbox->frame ? 1 : 0;
        int y = fw;
        const int buttonWidth = static_cast<int>(Ph::dpiScaled(Ph::SpinBox_ButtonWidth)) + 2;
        int x, lx, rx;
        x = spinbox->rect.width() - y - buttonWidth + 2;
        lx = fw;
        rx = x - fw;
        switch (subControl) {
        case SC_SpinBoxUp:
            if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons)
                return {};
            rect = QRect(x, fw, buttonWidth, center - fw);
            break;
        case SC_SpinBoxDown:
            if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons)
                return QRect();

            rect = QRect(x, center, buttonWidth, spinbox->rect.bottom() - center - fw + 1);
            break;
        case SC_SpinBoxEditField:
            if (spinbox->buttonSymbols == QAbstractSpinBox::NoButtons) {
                rect = QRect(lx, fw, spinbox->rect.width() - 2 * fw, spinbox->rect.height() - 2 * fw);
            } else {
                rect = QRect(lx, fw, rx - qMax(fw - 1, 0), spinbox->rect.height() - 2 * fw);
            }
            break;
        case SC_SpinBoxFrame:
            rect = spinbox->rect;
            break;
        default:
            break;
        }
        rect = visualRect(spinbox->direction, spinbox->rect, rect);
        break;
    }
    case CC_GroupBox: {
        auto groupBox = qstyleoption_cast<const QStyleOptionGroupBox*>(option);
        if (!groupBox)
            break;
        switch (subControl) {
        case SC_GroupBoxFrame:
        case SC_GroupBoxContents: {
            QRect r = option->rect;
            if (groupBox->subControls & (SC_GroupBoxLabel | SC_GroupBoxCheckBox)) {
                int fontHeight = option->fontMetrics.height();
                int topMargin = qMax(pixelMetric(PM_ExclusiveIndicatorHeight), fontHeight);
                topMargin += static_cast<int>(fontHeight * Ph::GroupBox_LabelBottomMarginFontRatio);
                r.setTop(r.top() + topMargin);
            }
            if (subControl == SC_GroupBoxContents && groupBox->subControls & SC_GroupBoxFrame) {
                // Testing against groupBox->features for the frame type doesn't seem
                // to work here.
                r.adjust(1, 1, -1, -1);
            }
            return r;
        }
        case SC_GroupBoxCheckBox:
        case SC_GroupBoxLabel: {
            // Accurate height doesn't matter -- the other group box style
            // implementations also fail with multi-line or too-tall text.
            int textHeight = option->fontMetrics.height();
            // width()/horizontalAdvance() is faster than size() and good enough for
            // us, since we only support a single line of text here anyway.
            int textWidth = Phantom::fontMetricsWidth(option->fontMetrics, groupBox->text);
            int indicatorWidth = proxy()->pixelMetric(PM_IndicatorWidth, option, widget);
            int indicatorHeight = proxy()->pixelMetric(PM_IndicatorHeight, option, widget);
            int margin = 0;
            int indicatorRightSpace = textHeight / 3;
            int contentWidth = textWidth;
            if (option->subControls & QStyle::SC_GroupBoxCheckBox) {
                contentWidth += indicatorWidth + indicatorRightSpace;
            }
            int x = margin;
            int y = 0;
            switch (groupBox->textAlignment & Qt::AlignHorizontal_Mask) {
            case Qt::AlignHCenter:
                x += (option->rect.width() - contentWidth) / 2;
                break;
            case Qt::AlignRight:
                x += option->rect.width() - contentWidth;
                break;
            default:
                break;
            }
            int w, h;
            if (subControl == SC_GroupBoxCheckBox) {
                w = indicatorWidth;
                h = indicatorHeight;
                if (textHeight > indicatorHeight) {
                    y = (textHeight - indicatorHeight) / 2;
                }
            } else {
                w = contentWidth;
                h = textHeight;
                if (option->subControls & QStyle::SC_GroupBoxCheckBox) {
                    x += indicatorWidth + indicatorRightSpace;
                    w -= indicatorWidth + indicatorRightSpace;
                }
            }
            return visualRect(option->direction, option->rect, QRect(x, y, w, h));
        }
        default:
            break;
        }
        break;
    }
    case CC_ComboBox: {
        auto cb = qstyleoption_cast<const QStyleOptionComboBox*>(option);
        if (!cb)
            return QRect();
        int frame = cb->frame ? proxy()->pixelMetric(PM_ComboBoxFrameWidth, cb, widget) : 0;
        QRect r = option->rect;
        r.adjust(frame, frame, -frame, -frame);
        int dim = qMin(r.width(), r.height());
        if (dim < 1)
            return QRect();
        switch (subControl) {
        case SC_ComboBoxFrame:
            return cb->rect;
        case SC_ComboBoxArrow: {
            QRect r0 = r;
            r0.setX((r0.x() + r0.width()) - dim + 1);
            return visualRect(option->direction, option->rect, r0);
        }
        case SC_ComboBoxEditField: {
            // Add extra padding if not editable
            int pad = 0;
            if (cb->editable) {
                // Line edit padding already added
            } else {
                pad = Ph::dpiScaled(Ph::ComboBox_NonEditable_ContentsHPad);
            }
            r.adjust(pad, 0, -dim, 0);
            return visualRect(option->direction, option->rect, r);
        }
        case SC_ComboBoxListBoxPopup: {
            //return cb->rect;
            //rekols
            QSize size = proxy()->sizeFromContents(CT_Menu, option, option->rect.size(), widget);
            QRect rect = option->rect;
            if (auto styleOption = static_cast<const QStyleOption *>(option)) {
                if (auto menuItem = static_cast<const QStyleOptionMenuItem *>(styleOption)) {
                    if (menuItem->icon.isNull()) {
                        rect.setWidth(size.width());
                    }
                }
            }
            return rect;
        }
        default:
            break;
        }
        break;
    }
    case CC_TitleBar: {
        auto tb = qstyleoption_cast<const QStyleOptionTitleBar*>(option);
        if (!tb)
            break;
        SubControl sc = subControl;
        QRect& ret = rect;
        const int indent = 3;
        const int controlTopMargin = 3;
        const int controlBottomMargin = 3;
        const int controlWidthMargin = 2;
        const int controlHeight = tb->rect.height() - controlTopMargin - controlBottomMargin;
        const int delta = controlHeight + controlWidthMargin;
        int offset = 0;
        bool isMinimized = tb->titleBarState & Qt::WindowMinimized;
        bool isMaximized = tb->titleBarState & Qt::WindowMaximized;
        switch (sc) {
        case SC_TitleBarLabel:
            if (tb->titleBarFlags & (Qt::WindowTitleHint | Qt::WindowSystemMenuHint)) {
                ret = tb->rect;
                if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                    ret.adjust(delta, 0, -delta, 0);
                if (tb->titleBarFlags & Qt::WindowMinimizeButtonHint)
                    ret.adjust(0, 0, -delta, 0);
                if (tb->titleBarFlags & Qt::WindowMaximizeButtonHint)
                    ret.adjust(0, 0, -delta, 0);
                if (tb->titleBarFlags & Qt::WindowShadeButtonHint)
                    ret.adjust(0, 0, -delta, 0);
                if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                    ret.adjust(0, 0, -delta, 0);
            }
            break;
        case SC_TitleBarContextHelpButton:
            if (tb->titleBarFlags & Qt::WindowContextHelpButtonHint)
                offset += delta;
            Q_FALLTHROUGH();
        case SC_TitleBarMinButton:
            if (!isMinimized && (tb->titleBarFlags & Qt::WindowMinimizeButtonHint))
                offset += delta;
            else if (sc == SC_TitleBarMinButton)
                break;
            Q_FALLTHROUGH();
        case SC_TitleBarNormalButton:
            if (isMinimized && (tb->titleBarFlags & Qt::WindowMinimizeButtonHint))
                offset += delta;
            else if (isMaximized && (tb->titleBarFlags & Qt::WindowMaximizeButtonHint))
                offset += delta;
            else if (sc == SC_TitleBarNormalButton)
                break;
            Q_FALLTHROUGH();
        case SC_TitleBarMaxButton:
            if (!isMaximized && (tb->titleBarFlags & Qt::WindowMaximizeButtonHint))
                offset += delta;
            else if (sc == SC_TitleBarMaxButton)
                break;
            Q_FALLTHROUGH();
        case SC_TitleBarShadeButton:
            if (!isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
                offset += delta;
            else if (sc == SC_TitleBarShadeButton)
                break;
            Q_FALLTHROUGH();
        case SC_TitleBarUnshadeButton:
            if (isMinimized && (tb->titleBarFlags & Qt::WindowShadeButtonHint))
                offset += delta;
            else if (sc == SC_TitleBarUnshadeButton)
                break;
            Q_FALLTHROUGH();
        case SC_TitleBarCloseButton:
            if (tb->titleBarFlags & Qt::WindowSystemMenuHint)
                offset += delta;
            else if (sc == SC_TitleBarCloseButton)
                break;
            ret.setRect(
                tb->rect.right() - indent - offset, tb->rect.top() + controlTopMargin, controlHeight, controlHeight);
            break;
        case SC_TitleBarSysMenu:
            if (tb->titleBarFlags & Qt::WindowSystemMenuHint) {
                ret.setRect(tb->rect.left() + controlWidthMargin + indent,
                            tb->rect.top() + controlTopMargin,
                            controlHeight,
                            controlHeight);
            }
            break;
        default:
            break;
        }
        ret = visualRect(tb->direction, tb->rect, ret);
        break;
    }
    default:
        break;
    }

    return rect;
}

QRect BaseStyle::itemPixmapRect(const QRect& r, int flags, const QPixmap& pixmap) const
{
    return QCommonStyle::itemPixmapRect(r, flags, pixmap);
}

void BaseStyle::drawItemPixmap(QPainter* painter, const QRect& rect, int alignment, const QPixmap& pixmap) const
{
    QCommonStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

QStyle::SubControl BaseStyle::hitTestComplexControl(ComplexControl cc,
                                                    const QStyleOptionComplex* opt,
                                                    const QPoint& pt,
                                                    const QWidget* w) const
{
    return QCommonStyle::hitTestComplexControl(cc, opt, pt, w);
}

QPixmap BaseStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, const QStyleOption* opt) const
{
    // Default icon highlight is way too subtle
    if (iconMode == QIcon::Selected) {
        QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&img);

        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);

        QColor color =
            Phantom::DeriveColors::adjustLightness(opt->palette.color(QPalette::Normal, QPalette::Highlight), .25);
        color.setAlphaF(0.25);
        painter.fillRect(0, 0, img.width(), img.height(), color);

        painter.end();

        return QPixmap::fromImage(img);
    }
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

int BaseStyle::styleHint(StyleHint hint,
                         const QStyleOption* option,
                         const QWidget* widget,
                         QStyleHintReturn* returnData) const
{
    if (auto menu = qobject_cast<const QMenu *>(widget)) {
        const_cast<QWidget *>(widget)->setAttribute(Qt::WA_TranslucentBackground);
    }

    switch (hint) {
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_FontDialog_SelectAssociatedText:
    case SH_ComboBox_ListMouseTracking:
    case SH_Slider_StopMouseOverSlider:
    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_TitleBar_AutoRaise:
    case SH_TitleBar_NoBorder:
    case SH_ItemView_ArrowKeysNavigateIntoChildren:
    case SH_ItemView_ChangeHighlightOnFocus:
    case SH_MenuBar_MouseTracking:
    case SH_Menu_MouseTracking:
        return 1;
    case SH_Menu_SupportsSections:
        return 0;
#ifndef Q_OS_MAC
    case SH_MenuBar_AltKeyNavigation:
        return 1;
#endif
#if defined(QT_PLATFORM_UIKIT)
    case SH_ComboBox_UseNativePopup:
        return 1;
#endif
    case SH_ItemView_ShowDecorationSelected:
        // QWindowsStyle does this as well -- QCommonStyle seems to have some
        // internal confusion buried within its private implementation of laying
        // out and drawing item views where it can't keep track of what's
        // considered a decoration and what's not. For tree views, if you give 0
        // for ShowDecorationSelected, it applies only to the disclosure indicator
        // and not to the QIcon/pixmap that might be present for the item. So
        // selecting an item in a tree view will have the selection color drawn
        // underneath the icon/pixmap, but not the disclosure indicator. However,
        // in list views, if you give 0 for ShowDecorationSelected, it will *not*
        // draw the selection color underneath the icon/pixmap. There's no way to
        // access this internal logic in QCommonStyle without fully reimplementing
        // the huge mass of stuff for item view layout and drawing. Therefore, the
        // best we can do is at least try to get consistent behavior: if it's a
        // list view, just always return 1 for ShowDecorationSelected.
        if (!Phantom::ShowItemViewDecorationSelected && qobject_cast<const QListView*>(widget))
            return 1;
        return Phantom::ShowItemViewDecorationSelected;
    case SH_ItemView_MovementWithoutUpdatingSelection:
        return 1;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    case SH_ItemView_ScrollMode:
        return QAbstractItemView::ScrollPerPixel;
#endif
    case SH_ScrollBar_ContextMenu:
#ifdef Q_OS_MAC
        return 0;
#else
        return 1;
#endif
    // Some Linux distros might want to enable this, but it doesn't behave very
    // consistently with varied QPalettes, depending on how the QPA and icons
    // deal with both light and dark themes. It might seem weird to just disable
    // this, but none of (Mac, Windows, BeOS/Haiku) show icons in dialog buttons,
    // and the results on Linux are generally pretty messy -- not sure why it's
    // historically been the default, especially when other button types
    // generally don't have any icons.
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return 0;
    case SH_ScrollBar_Transient:
        return 1;
    case SH_EtchDisabledText:
    case SH_DitherDisabledText:
    case SH_ToolBox_SelectedPageTitleBold:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_MainWindow_SpaceBelowMenuBar:
    case SH_MessageBox_CenterButtons:
    case SH_RubberBand_Mask:
    case SH_ScrollView_FrameOnlyAroundContents:
        return 0;
    case SH_ComboBox_Popup: {
        return Phantom::UseQMenuForComboBoxPopup;
        // Fusion did this, but we don't because of font bugs (especially in high
        // DPI) with the QMenu that the combo box will create instead of a dropdown
        // view. See notes in CE_MenuItem for more details.
        if (auto cmb = qstyleoption_cast<const QStyleOptionComboBox*>(option))
            return !cmb->editable;
        return 0;
    }
    case SH_Table_GridLineColor: {
        using namespace Phantom::SwatchColors;
        namespace Ph = Phantom;
        if (!option)
            return 0;
        auto ph_swatchPtr = Ph::getCachedSwatchOfQPalette(&d->swatchCache, &d->headSwatchFastKey, option->palette);
        const Ph::PhSwatch& swatch = *ph_swatchPtr.data();
        // Qt code in table views for drawing grid lines is broken. See case for
        // CE_ItemViewItem painting for more information.
        return static_cast<int>(swatch.color(S_base_divider).rgb());
    }
    case SH_MessageBox_TextInteractionFlags:
        return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;
    case SH_WizardStyle:
        return QWizard::ClassicStyle;
    case SH_Menu_SubMenuPopupDelay:
        // Returning 0 will break sloppy submenus even if they're enabled
        return 10;
    case SH_Menu_SloppySubMenus:
        return true;
    case SH_Menu_SubMenuSloppyCloseTimeout:
        return 500;
    case SH_Menu_SubMenuDontStartSloppyOnLeave:
        return 1;
    case SH_Menu_SubMenuSloppySelectOtherActions:
        return 1;
    case SH_Menu_SubMenuUniDirection:
        return 1;
    case SH_Menu_SubMenuUniDirectionFailCount:
        return 1;
    case SH_Menu_SubMenuResetWhenReenteringParent:
        return 0;
#ifdef Q_OS_MAC
    case SH_Menu_FlashTriggeredItem:
        return 1;
    case SH_Menu_FadeOutOnHide:
        return 0;
#endif
    case SH_WindowFrame_Mask:
        return 0;
    case SH_UnderlineShortcut: {
        return false;
    }
    case SH_Widget_Animate:
        return 1;
    default:
        break;
    }
    return QCommonStyle::styleHint(hint, option, widget, returnData);
}

QRect BaseStyle::subElementRect(SubElement sr, const QStyleOption* opt, const QWidget* w) const
{
    switch (sr) {
    case SE_ProgressBarLabel:
    case SE_ProgressBarContents:
    case SE_ProgressBarGroove:
        return opt->rect;
    case SE_PushButtonFocusRect: {
        QRect r = QCommonStyle::subElementRect(sr, opt, w);
        r.adjust(0, 1, 0, -1);
        return r;
    }
    case SE_DockWidgetTitleBarText: {
        auto titlebar = qstyleoption_cast<const QStyleOptionDockWidget*>(opt);
        if (!titlebar)
            break;
        QRect r = QCommonStyle::subElementRect(sr, opt, w);
        bool verticalTitleBar = titlebar->verticalTitleBar;
        if (verticalTitleBar) {
            r.adjust(0, 0, 0, -4);
        } else {
            if (opt->direction == Qt::LeftToRight)
                r.adjust(4, 0, 0, 0);
            else
                r.adjust(0, 0, -4, 0);
        }
        return r;
    }
    case SE_TreeViewDisclosureItem: {
        if (Phantom::BranchesOnEdge) {
            // Shove it all the way to the left (or right) side, probably outside of
            // the rect it gave us. Old-school.
            QRect rect = opt->rect;
            if (opt->direction != Qt::RightToLeft) {
                rect.moveLeft(0);
                if (rect.width() < rect.height())
                    rect.setWidth(rect.height());
            } else {
                // todo
            }
            return rect;
        }
        break;
    }
    case SE_LineEditContents: {
        QRect r = QCommonStyle::subElementRect(sr, opt, w);
        int pad = Phantom::dpiScaled(Phantom::LineEdit_ContentsHPad);
        return r.adjusted(pad, 0, -pad, 0);
    }
    default:
        break;
    }
    return QCommonStyle::subElementRect(sr, opt, w);
}
