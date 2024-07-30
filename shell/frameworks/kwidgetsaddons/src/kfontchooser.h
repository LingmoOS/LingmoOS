/*
    SPDX-FileCopyrightText: 1997 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1999 Mario Weilguni <mweilguni@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef K_FONT_CHOOSER_H
#define K_FONT_CHOOSER_H

#include <QStringList>
#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

class QFont;

/**
 * @class KFontChooser kfontchooser.h KFontChooser
 *
 * @short A font selection widget.
 *
 * While KFontChooser as an ordinary widget can be embedded in
 * custom dialogs and therefore is very flexible, in most cases
 * it is preferable to use the convenience functions in
 * QFontDialog.
 *
 * \image html kfontchooser.png "KFontChooser Widget"
 *
 * @see KFontRequester
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 */
class KWIDGETSADDONS_EXPORT KFontChooser : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontSelected USER true)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QString sampleText READ sampleText WRITE setSampleText)

public:
    /**
     * Displayed columns.
     */
    enum FontColumn {
        FamilyList = 0x01, ///< Identifies the family (leftmost) list.
        StyleList = 0x02, ///< Identifies the style (center) list.
        SizeList = 0x04, ///< Identifies the size (rightmost) list.
    };

    /**
     * Flags for selecting which font attributes to change
     * @see FontDiffFlags
     */
    enum FontDiff {
        NoFontDiffFlags = 0, ///< No flags set
        FontDiffFamily = 1, ///< Identifies a requested change in the font family.
        FontDiffStyle = 2, ///< Identifies a requested change in the font style.
        FontDiffSize = 4, ///< Identifies a requested change in the font size.
        AllFontDiffs = FontDiffFamily | FontDiffStyle | FontDiffSize,
    };
    /**
     * Stores an combination of #FontDiff values.
     */
    Q_DECLARE_FLAGS(FontDiffFlags, FontDiff)

    /**
     * Flags for selecting what is displayed in the widget.
     * @see DisplayFlags
     */
    enum DisplayFlag {
        NoDisplayFlags = 0, ///< No flags set
        FixedFontsOnly = 1, ///< Only show monospaced/fixed-width fonts, excluding proportional fonts, (the
                            ///< checkbox to toggle showing only monospaced fonts is not shown in this case)
        DisplayFrame = 2, ///< Show a visual frame around the chooser
        ShowDifferences = 4, ///< Display the font differences interfaces
    };
    /**
     * Stores a combination of #DisplayFlag values.
     */
    Q_DECLARE_FLAGS(DisplayFlags, DisplayFlag)

    /**
     * Constructs a font picker widget.
     *
     * @param parent the parent widget
     *
     * @since 5.86
     */
    explicit KFontChooser(QWidget *parent = nullptr);

    /**
     * Create a font picker widget.
     *
     * @param flags a combination of OR-ed values from the @c KFontChooser::DisplayFlags enum,
     * the default is @c DisplayFonts::NoDisplayFlags
     * @param parent the parent widget, if not nullptr the windowing system will use it to position
     * the chooser widget relative to it
     *
     * @since 5.86
     */
    explicit KFontChooser(DisplayFlags flags, QWidget *parent = nullptr);

    /**
     * Destructor.
     */
    ~KFontChooser() override;

    /**
     * Enables or disables a column (family, style, size) in the widget.
     *
     * Use this function if your application does not need or support all font properties.
     *
     * @param column specify the column(s) to enable/disable, an OR-ed combination of
     * @c KFontChooser::FontColumn enum values
     * @param state if @p false the columns are disabled, and vice-versa
     */
    void enableColumn(int column, bool state);

    /**
     * Sets the currently selected font in the widget.
     *
     * @param font the font to select
     * @param onlyFixed if @c true, the font list will only display fixed-width fonts,
     * otherwise all fonts are displayed. The default is @c false.
     */
    void setFont(const QFont &font, bool onlyFixed = false);

    /**
     * Returns the bitmask corresponding to the attributes the user wishes to change.
     */
    FontDiffFlags fontDiffFlags() const;

    /**
     * Returns the currently selected font in the chooser.
     */
    QFont font() const;

    /**
     * Sets the color to use for the font in the preview area.
     */
    void setColor(const QColor &col);

    /**
     * Returns the color currently used for the font in the preview
     * area (default: the text color of the active color group).
     */
    QColor color() const;

    /**
     * Sets the background color to use in the preview area.
     */
    void setBackgroundColor(const QColor &col);

    /**
     * Returns the background color currently used in the preview area
     * (default: the base color of the active colorgroup)
     */
    QColor backgroundColor() const;

    /**
     * @return The current text in the sample text input area.
     */
    QString sampleText() const;

    /**
     * Sets the sample text in the preview area; this is useful if you
     * want to use text in your native language.
     *
     * @param text the new sample text (it will replace the current text)
     */
    void setSampleText(const QString &text);

    /**
     * If @p visible is @c true the preview area will be shown, and vice-versa
     * is it's @c false.
     */
    void setSampleBoxVisible(bool visible);

    /**
     * The selection criteria for the font families shown in the dialog.
     */
    enum FontListCriteria {
        /**
         * If set, only show fixed fixed-width (monospace) fonts.
         */
        FixedWidthFonts = 0x01,
        /**
         * If set, only show scalable fonts.
         * Certain configurations allow bitmap fonts to remain unscaled
         * and thus these fonts have limited number of sizes.
         */
        ScalableFonts = 0x02,
        /**
         * If set, only show smooth scalable fonts.
         * This will return only non-bitmap fonts which are scalable to any size requested.
         * Setting this option means the @c ScalableFonts flag is ignored.
         */
        SmoothScalableFonts = 0x04
    };

    /**
     * Returns a list of font faimly name strings filtered based on @p fontListCriteria.
     *
     * @param fontListCriteria specifies the criteria used to select fonts to add to
     * the list, a combination of OR-ed values from @ref KFontChooser::FontListCriteria
     *
     * @since 5.86
     */
    static QStringList createFontList(uint fontListCriteria);

    /**
     * Uses @p fontList to fill the font family list in the widget.
     *
     * You can create a custom list of fonts using the static @c createFontList(uint
     * criteria) to only include fonts that meet certain criteria (e.g. only
     * smooth-scalable fonts).
     *
     * @see KFontChooser::createFontList(uint), KFontChooser::FontListCriteria
     *
     * Note that if @p fontList is empty, the font list in the chooser will show
     * all the available fonts on the system.
     * @since 5.86
     */
    void setFontListItems(const QStringList &fontList);

    /**
     * Sets the minimum number of items that should be visible in the
     * child list widgets; this number will be used to compute and set
     * the minimum heights for those widgets.
     *
     * @since 5.86
     */
    void setMinVisibleItems(int visibleItems);

    /**
     * Reimplemented for internal reasons.
     */
    QSize sizeHint(void) const override;

Q_SIGNALS:
    /**
     * Emitted when the selected font changes.
     */
    void fontSelected(const QFont &font);

private:
    std::unique_ptr<class KFontChooserPrivate> const d;

    Q_DISABLE_COPY(KFontChooser)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFontChooser::DisplayFlags)

#endif
