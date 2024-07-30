/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>
    SPDX-FileCopyrightText: 2008 Thomas McGuire <thomas.mcguire@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "krichtextwidget.h"

#include "krichtextedit_p.h"

// KDE includes
#include <KColorScheme>
#include <KFontAction>
#include <KFontSizeAction>
#include <KLocalizedString>
#include <ktoggleaction.h>

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include <QTextList>

#include "klinkdialog_p.h"

// TODO: Add i18n context

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KRichTextWidgetPrivate : public KRichTextEditPrivate
{
    Q_DECLARE_PUBLIC(KRichTextWidget)

public:
    KRichTextWidgetPrivate(KRichTextWidget *qq)
        : KRichTextEditPrivate(qq)
    {
    }

    QList<QAction *> richTextActionList;
    QTextCharFormat painterFormat;

    KRichTextWidget::RichTextSupport richTextSupport;

    bool painterActive = false;

    bool richTextEnabled = false;
    KToggleAction *enableRichText = nullptr;

    QAction *action_text_foreground_color = nullptr;
    QAction *action_text_background_color = nullptr;

    KToggleAction *action_text_bold = nullptr;
    KToggleAction *action_text_italic = nullptr;
    KToggleAction *action_text_underline = nullptr;
    KToggleAction *action_text_strikeout = nullptr;

    KFontAction *action_font_family = nullptr;
    KFontSizeAction *action_font_size = nullptr;

    KSelectAction *action_list_style = nullptr;
    QAction *action_list_indent = nullptr;
    QAction *action_list_dedent = nullptr;

    QAction *action_manage_link = nullptr;
    QAction *action_insert_horizontal_rule = nullptr;
    QAction *action_format_painter = nullptr;
    QAction *action_to_plain_text = nullptr;

    KToggleAction *action_align_left = nullptr;
    KToggleAction *action_align_right = nullptr;
    KToggleAction *action_align_center = nullptr;
    KToggleAction *action_align_justify = nullptr;

    KToggleAction *action_direction_ltr = nullptr;
    KToggleAction *action_direction_rtl = nullptr;

    KToggleAction *action_text_superscript = nullptr;
    KToggleAction *action_text_subscript = nullptr;

    KSelectAction *action_heading_level = nullptr;

    //
    // Normal functions
    //
    void init();

    //
    // Slots
    //

    /**
     * @brief Opens a dialog to allow the user to select a foreground color.
     */
    void _k_setTextForegroundColor();

    /**
     * @brief Opens a dialog to allow the user to select a background color.
     */
    void _k_setTextBackgroundColor();

    /**
     * Opens a dialog which lets the user turn the currently selected text into
     * a link.
     * If no text is selected, the word under the cursor will be taken.
     * If the cursor is already over a link, the user can edit that link.
     *
     */
    void _k_manageLink();

    /**
     * Activates a format painter to allow the user to copy font/text formatting
     * to different parts of the document.
     *
     */
    void _k_formatPainter(bool active);

    /**
     * @brief Update actions relating to text format (bold, size etc.).
     */
    void updateCharFormatActions(const QTextCharFormat &format);

    /**
     * Update actions not covered by text formatting, such as alignment,
     * list style and level.
     */
    void updateMiscActions();

    /**
     * Change the style of the current list or create a new list with the style given by @a index.
     */
    void _k_setListStyle(int index);

    /**
     * Change the heading level of a current line to a level given by @a level
     */
    void _k_setHeadingLevel(int level);
};
//@endcond

void KRichTextWidgetPrivate::init()
{
    Q_Q(KRichTextWidget);

    q->setRichTextSupport(KRichTextWidget::FullSupport);
}

KRichTextWidget::KRichTextWidget(QWidget *parent)
    : KRichTextEdit(*new KRichTextWidgetPrivate(this), parent)
{
    Q_D(KRichTextWidget);

    d->init();
}

KRichTextWidget::KRichTextWidget(const QString &text, QWidget *parent)
    : KRichTextEdit(*new KRichTextWidgetPrivate(this), text, parent)
{
    Q_D(KRichTextWidget);

    d->init();
}

KRichTextWidget::~KRichTextWidget() = default;

KRichTextWidget::RichTextSupport KRichTextWidget::richTextSupport() const
{
    Q_D(const KRichTextWidget);

    return d->richTextSupport;
}

void KRichTextWidget::setRichTextSupport(const KRichTextWidget::RichTextSupport &support)
{
    Q_D(KRichTextWidget);

    d->richTextSupport = support;
}

QList<QAction *> KRichTextWidget::createActions()
{
    Q_D(KRichTextWidget);

    // Note to maintainers: If adding new functionality here, make sure to disconnect
    // and delete actions which should not be supported.
    //
    // New Actions need to be added to the following places:
    // - possibly the RichTextSupportValues enum
    // - the API documentation for createActions()
    // - this function
    // - the action needs to be added to the private class as a member
    // - the constructor of the private class
    // - depending on the action, some slot that changes the toggle state when
    //   appropriate, such as updateCharFormatActions or updateMiscActions.

    // The list of actions currently supported is also stored internally.
    // This is used to disable all actions at once in setActionsEnabled.
    d->richTextActionList.clear();

    if (d->richTextSupport & SupportTextForegroundColor) {
        // Foreground Color
        d->action_text_foreground_color = new QAction(QIcon::fromTheme(QStringLiteral("format-stroke-color")), i18nc("@action", "Text &Color…"), this);
        d->action_text_foreground_color->setIconText(i18nc("@label stroke color", "Color"));
        d->richTextActionList.append((d->action_text_foreground_color));
        d->action_text_foreground_color->setObjectName(QStringLiteral("format_text_foreground_color"));
        connect(d->action_text_foreground_color, &QAction::triggered, this, [this]() {
            Q_D(KRichTextWidget);
            d->_k_setTextForegroundColor();
        });
    } else {
        d->action_text_foreground_color = nullptr;
    }

    if (d->richTextSupport & SupportTextBackgroundColor) {
        // Background Color
        d->action_text_background_color = new QAction(QIcon::fromTheme(QStringLiteral("format-fill-color")), i18nc("@action", "Text &Highlight…"), this);
        d->richTextActionList.append((d->action_text_background_color));
        d->action_text_background_color->setObjectName(QStringLiteral("format_text_background_color"));
        connect(d->action_text_background_color, &QAction::triggered, this, [this]() {
            Q_D(KRichTextWidget);
            d->_k_setTextBackgroundColor();
        });
    } else {
        d->action_text_background_color = nullptr;
    }

    if (d->richTextSupport & SupportFontFamily) {
        // Font Family
        d->action_font_family = new KFontAction(i18nc("@action", "&Font"), this);
        d->richTextActionList.append((d->action_font_family));
        d->action_font_family->setObjectName(QStringLiteral("format_font_family"));
        connect(d->action_font_family, &KSelectAction::textTriggered, this, &KRichTextWidget::setFontFamily);
    } else {
        d->action_font_family = nullptr;
    }

    if (d->richTextSupport & SupportFontSize) {
        // Font Size
        d->action_font_size = new KFontSizeAction(i18nc("@action", "Font &Size"), this);
        d->richTextActionList.append((d->action_font_size));
        d->action_font_size->setObjectName(QStringLiteral("format_font_size"));
        connect(d->action_font_size, &KFontSizeAction::fontSizeChanged, this, &KRichTextEdit::setFontSize);
    } else {
        d->action_font_size = nullptr;
    }

    if (d->richTextSupport & SupportBold) {
        d->action_text_bold = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-bold")), i18nc("@action boldify selected text", "&Bold"), this);
        QFont bold;
        bold.setBold(true);
        d->action_text_bold->setFont(bold);
        d->richTextActionList.append((d->action_text_bold));
        d->action_text_bold->setObjectName(QStringLiteral("format_text_bold"));
        d->action_text_bold->setShortcut(Qt::CTRL | Qt::Key_B);
        connect(d->action_text_bold, &QAction::triggered, this, &KRichTextEdit::setTextBold);
    } else {
        d->action_text_bold = nullptr;
    }

    if (d->richTextSupport & SupportItalic) {
        d->action_text_italic =
            new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-italic")), i18nc("@action italicize selected text", "&Italic"), this);
        QFont italic;
        italic.setItalic(true);
        d->action_text_italic->setFont(italic);
        d->richTextActionList.append((d->action_text_italic));
        d->action_text_italic->setObjectName(QStringLiteral("format_text_italic"));
        d->action_text_italic->setShortcut(Qt::CTRL | Qt::Key_I);
        connect(d->action_text_italic, &QAction::triggered, this, &KRichTextEdit::setTextItalic);
    } else {
        d->action_text_italic = nullptr;
    }

    if (d->richTextSupport & SupportUnderline) {
        d->action_text_underline =
            new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-underline")), i18nc("@action underline selected text", "&Underline"), this);
        QFont underline;
        underline.setUnderline(true);
        d->action_text_underline->setFont(underline);
        d->richTextActionList.append((d->action_text_underline));
        d->action_text_underline->setObjectName(QStringLiteral("format_text_underline"));
        d->action_text_underline->setShortcut(Qt::CTRL | Qt::Key_U);
        connect(d->action_text_underline, &QAction::triggered, this, &KRichTextEdit::setTextUnderline);
    } else {
        d->action_text_underline = nullptr;
    }

    if (d->richTextSupport & SupportStrikeOut) {
        d->action_text_strikeout = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-strikethrough")), i18nc("@action", "&Strike Out"), this);
        QFont strikeout;
        strikeout.setStrikeOut(true);
        d->action_text_strikeout->setFont(strikeout);
        d->richTextActionList.append((d->action_text_strikeout));
        d->action_text_strikeout->setObjectName(QStringLiteral("format_text_strikeout"));
        d->action_text_strikeout->setShortcut(Qt::CTRL | Qt::Key_L);
        connect(d->action_text_strikeout, &QAction::triggered, this, &KRichTextEdit::setTextStrikeOut);
    } else {
        d->action_text_strikeout = nullptr;
    }

    if (d->richTextSupport & SupportAlignment) {
        // Alignment
        d->action_align_left = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-left")), i18nc("@action", "Align &Left"), this);
        d->action_align_left->setIconText(i18nc("@label left justify", "Left"));
        d->richTextActionList.append((d->action_align_left));
        d->action_align_left->setObjectName(QStringLiteral("format_align_left"));
        connect(d->action_align_left, &QAction::triggered, this, &KRichTextEdit::alignLeft);

        d->action_align_center = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-center")), i18nc("@action", "Align &Center"), this);
        d->action_align_center->setIconText(i18nc("@label center justify", "Center"));
        d->richTextActionList.append((d->action_align_center));
        d->action_align_center->setObjectName(QStringLiteral("format_align_center"));
        connect(d->action_align_center, &QAction::triggered, this, &KRichTextEdit::alignCenter);

        d->action_align_right = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-right")), i18nc("@action", "Align &Right"), this);
        d->action_align_right->setIconText(i18nc("@label right justify", "Right"));
        d->richTextActionList.append((d->action_align_right));
        d->action_align_right->setObjectName(QStringLiteral("format_align_right"));
        connect(d->action_align_right, &QAction::triggered, this, &KRichTextEdit::alignRight);

        d->action_align_justify = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-justify-fill")), i18nc("@action", "&Justify"), this);
        d->action_align_justify->setIconText(i18nc("@label justify fill", "Justify"));
        d->richTextActionList.append((d->action_align_justify));
        d->action_align_justify->setObjectName(QStringLiteral("format_align_justify"));
        connect(d->action_align_justify, &QAction::triggered, this, &KRichTextEdit::alignJustify);

        QActionGroup *alignmentGroup = new QActionGroup(this);
        alignmentGroup->addAction(d->action_align_left);
        alignmentGroup->addAction(d->action_align_center);
        alignmentGroup->addAction(d->action_align_right);
        alignmentGroup->addAction(d->action_align_justify);
    } else {
        d->action_align_left = nullptr;
        d->action_align_center = nullptr;
        d->action_align_right = nullptr;
        d->action_align_justify = nullptr;
    }

    if (d->richTextSupport & SupportDirection) {
        d->action_direction_ltr = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-direction-ltr")), i18nc("@action", "Left-to-Right"), this);
        d->action_direction_ltr->setIconText(i18nc("@label left-to-right", "Left-to-Right"));
        d->richTextActionList.append(d->action_direction_ltr);
        d->action_direction_ltr->setObjectName(QStringLiteral("direction_ltr"));
        connect(d->action_direction_ltr, &QAction::triggered, this, &KRichTextEdit::makeLeftToRight);

        d->action_direction_rtl = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-direction-rtl")), i18nc("@action", "Right-to-Left"), this);
        d->action_direction_rtl->setIconText(i18nc("@label right-to-left", "Right-to-Left"));
        d->richTextActionList.append(d->action_direction_rtl);
        d->action_direction_rtl->setObjectName(QStringLiteral("direction_rtl"));
        connect(d->action_direction_rtl, &QAction::triggered, this, &KRichTextEdit::makeRightToLeft);

        QActionGroup *directionGroup = new QActionGroup(this);
        directionGroup->addAction(d->action_direction_ltr);
        directionGroup->addAction(d->action_direction_rtl);
    } else {
        d->action_direction_ltr = nullptr;
        d->action_direction_rtl = nullptr;
    }

    if (d->richTextSupport & SupportChangeListStyle) {
        d->action_list_style = new KSelectAction(QIcon::fromTheme(QStringLiteral("format-list-unordered")), i18nc("@title:menu", "List Style"), this);
        QStringList listStyles;
        /* clang-format off */
        listStyles << i18nc("@item:inmenu no list style", "None")
                   << i18nc("@item:inmenu disc list style", "Disc")
                   << i18nc("@item:inmenu circle list style", "Circle")
                   << i18nc("@item:inmenu square list style", "Square")
                   << i18nc("@item:inmenu numbered lists", "123")
                   << i18nc("@item:inmenu lowercase abc lists", "abc")
                   << i18nc("@item:inmenu uppercase abc lists", "ABC")
                   << i18nc("@item:inmenu lower case roman numerals", "i ii iii")
                   << i18nc("@item:inmenu upper case roman numerals", "I II III");
        /* clang-format on */

        d->action_list_style->setItems(listStyles);
        d->action_list_style->setCurrentItem(0);
        d->richTextActionList.append((d->action_list_style));
        d->action_list_style->setObjectName(QStringLiteral("format_list_style"));

        connect(d->action_list_style, &KSelectAction::indexTriggered, this, [this](int style) {
            Q_D(KRichTextWidget);
            d->_k_setListStyle(style);
        });
        connect(d->action_list_style, &QAction::triggered, this, [d]() {
            d->updateMiscActions();
        });
    } else {
        d->action_list_style = nullptr;
    }

    if (d->richTextSupport & SupportIndentLists) {
        d->action_list_indent = new QAction(QIcon::fromTheme(QStringLiteral("format-indent-more")), i18nc("@action", "Increase Indent"), this);
        d->richTextActionList.append((d->action_list_indent));
        d->action_list_indent->setObjectName(QStringLiteral("format_list_indent_more"));
        connect(d->action_list_indent, &QAction::triggered, this, &KRichTextEdit::indentListMore);
        connect(d->action_list_indent, &QAction::triggered, this, [d]() {
            d->updateMiscActions();
        });
    } else {
        d->action_list_indent = nullptr;
    }

    if (d->richTextSupport & SupportDedentLists) {
        d->action_list_dedent = new QAction(QIcon::fromTheme(QStringLiteral("format-indent-less")), i18nc("@action", "Decrease Indent"), this);
        d->richTextActionList.append((d->action_list_dedent));
        d->action_list_dedent->setObjectName(QStringLiteral("format_list_indent_less"));
        connect(d->action_list_dedent, &QAction::triggered, this, &KRichTextEdit::indentListLess);
        connect(d->action_list_dedent, &QAction::triggered, this, [d]() {
            d->updateMiscActions();
        });
    } else {
        d->action_list_dedent = nullptr;
    }

    if (d->richTextSupport & SupportRuleLine) {
        d->action_insert_horizontal_rule = new QAction(QIcon::fromTheme(QStringLiteral("insert-horizontal-rule")), i18nc("@action", "Insert Rule Line"), this);
        d->richTextActionList.append((d->action_insert_horizontal_rule));
        d->action_insert_horizontal_rule->setObjectName(QStringLiteral("insert_horizontal_rule"));
        connect(d->action_insert_horizontal_rule, &QAction::triggered, this, &KRichTextEdit::insertHorizontalRule);
    } else {
        d->action_insert_horizontal_rule = nullptr;
    }

    if (d->richTextSupport & SupportHyperlinks) {
        d->action_manage_link = new QAction(QIcon::fromTheme(QStringLiteral("insert-link")), i18nc("@action", "Link"), this);
        d->richTextActionList.append((d->action_manage_link));
        d->action_manage_link->setObjectName(QStringLiteral("manage_link"));
        connect(d->action_manage_link, &QAction::triggered, this, [this]() {
            Q_D(KRichTextWidget);
            d->_k_manageLink();
        });
    } else {
        d->action_manage_link = nullptr;
    }

    if (d->richTextSupport & SupportFormatPainting) {
        d->action_format_painter = new KToggleAction(QIcon::fromTheme(QStringLiteral("draw-brush")), i18nc("@action", "Format Painter"), this);
        d->richTextActionList.append((d->action_format_painter));
        d->action_format_painter->setObjectName(QStringLiteral("format_painter"));
        connect(d->action_format_painter, &QAction::toggled, this, [this](bool state) {
            Q_D(KRichTextWidget);
            d->_k_formatPainter(state);
        });
    } else {
        d->action_format_painter = nullptr;
    }

    if (d->richTextSupport & SupportToPlainText) {
        d->action_to_plain_text = new KToggleAction(i18nc("@action", "To Plain Text"), this);
        d->richTextActionList.append((d->action_to_plain_text));
        d->action_to_plain_text->setObjectName(QStringLiteral("action_to_plain_text"));
        connect(d->action_to_plain_text, &QAction::triggered, this, &KRichTextEdit::switchToPlainText);
    } else {
        d->action_to_plain_text = nullptr;
    }

    if (d->richTextSupport & SupportSuperScriptAndSubScript) {
        d->action_text_subscript = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-subscript")), i18nc("@action", "Subscript"), this);
        d->richTextActionList.append((d->action_text_subscript));
        d->action_text_subscript->setObjectName(QStringLiteral("format_text_subscript"));
        connect(d->action_text_subscript, &QAction::triggered, this, &KRichTextEdit::setTextSubScript);

        d->action_text_superscript = new KToggleAction(QIcon::fromTheme(QStringLiteral("format-text-superscript")), i18nc("@action", "Superscript"), this);
        d->richTextActionList.append((d->action_text_superscript));
        d->action_text_superscript->setObjectName(QStringLiteral("format_text_superscript"));
        connect(d->action_text_superscript, &QAction::triggered, this, &KRichTextEdit::setTextSuperScript);
    } else {
        d->action_text_subscript = nullptr;

        d->action_text_superscript = nullptr;
    }

    if (d->richTextSupport & SupportHeading) {
        // TODO: an icon maybe?
        d->action_heading_level = new KSelectAction(i18nc("@title:menu", "Heading Level"), this);
        const QStringList headingLevels = {i18nc("@item:inmenu no heading", "Basic text"),
                                           i18nc("@item:inmenu heading level 1 (largest)", "Title"),
                                           i18nc("@item:inmenu heading level 2", "Subtitle"),
                                           i18nc("@item:inmenu heading level 3", "Section"),
                                           i18nc("@item:inmenu heading level 4", "Subsection"),
                                           i18nc("@item:inmenu heading level 5", "Paragraph"),
                                           i18nc("@item:inmenu heading level 6 (smallest)", "Subparagraph")};

        d->action_heading_level->setItems(headingLevels);
        d->action_heading_level->setCurrentItem(0);
        d->richTextActionList.append(d->action_heading_level);
        d->action_heading_level->setObjectName(QStringLiteral("format_heading_level"));
        connect(d->action_heading_level, &KSelectAction::indexTriggered, this, [this](int level) {
            Q_D(KRichTextWidget);
            d->_k_setHeadingLevel(level);
        });
    } else {
        d->action_heading_level = nullptr;
    }

    disconnect(this, &QTextEdit::currentCharFormatChanged, this, nullptr);
    disconnect(this, &QTextEdit::cursorPositionChanged, this, nullptr);
    connect(this, &QTextEdit::currentCharFormatChanged, this, [d](const QTextCharFormat &format) {
        d->updateCharFormatActions(format);
    });
    connect(this, &QTextEdit::cursorPositionChanged, this, [d]() {
        d->updateMiscActions();
    });

    d->updateMiscActions();
    d->updateCharFormatActions(currentCharFormat());

    return d->richTextActionList;
}

void KRichTextWidget::setActionsEnabled(bool enabled)
{
    Q_D(KRichTextWidget);

    for (QAction *action : std::as_const(d->richTextActionList)) {
        action->setEnabled(enabled);
    }
    d->richTextEnabled = enabled;
}

void KRichTextWidgetPrivate::_k_setListStyle(int index)
{
    Q_Q(KRichTextWidget);

    q->setListStyle(index);
    updateMiscActions();
}

void KRichTextWidgetPrivate::_k_setHeadingLevel(int level)
{
    Q_Q(KRichTextWidget);

    q->setHeadingLevel(level);
    updateMiscActions();
}

void KRichTextWidgetPrivate::updateCharFormatActions(const QTextCharFormat &format)
{
    Q_Q(KRichTextWidget);

    QFont f = format.font();

    if (richTextSupport & KRichTextWidget::SupportFontFamily) {
        action_font_family->setFont(f.family());
    }
    if (richTextSupport & KRichTextWidget::SupportFontSize) {
        if (f.pointSize() > 0) {
            action_font_size->setFontSize(f.pointSize());
        }
    }

    if (richTextSupport & KRichTextWidget::SupportBold) {
        action_text_bold->setChecked(f.bold());
    }

    if (richTextSupport & KRichTextWidget::SupportItalic) {
        action_text_italic->setChecked(f.italic());
    }

    if (richTextSupport & KRichTextWidget::SupportUnderline) {
        action_text_underline->setChecked(f.underline());
    }

    if (richTextSupport & KRichTextWidget::SupportStrikeOut) {
        action_text_strikeout->setChecked(f.strikeOut());
    }

    if (richTextSupport & KRichTextWidget::SupportSuperScriptAndSubScript) {
        QTextCharFormat::VerticalAlignment vAlign = format.verticalAlignment();
        action_text_superscript->setChecked(vAlign == QTextCharFormat::AlignSuperScript);
        action_text_subscript->setChecked(vAlign == QTextCharFormat::AlignSubScript);
    }
}

void KRichTextWidgetPrivate::updateMiscActions()
{
    Q_Q(KRichTextWidget);

    if (richTextSupport & KRichTextWidget::SupportAlignment) {
        Qt::Alignment a = q->alignment();
        if (a & Qt::AlignLeft) {
            action_align_left->setChecked(true);
        } else if (a & Qt::AlignHCenter) {
            action_align_center->setChecked(true);
        } else if (a & Qt::AlignRight) {
            action_align_right->setChecked(true);
        } else if (a & Qt::AlignJustify) {
            action_align_justify->setChecked(true);
        }
    }

    if (richTextSupport & KRichTextWidget::SupportChangeListStyle) {
        if (q->textCursor().currentList()) {
            action_list_style->setCurrentItem(-q->textCursor().currentList()->format().style());
        } else {
            action_list_style->setCurrentItem(0);
        }
    }

    if (richTextSupport & KRichTextWidget::SupportIndentLists) {
        if (richTextEnabled) {
            action_list_indent->setEnabled(q->canIndentList());
        } else {
            action_list_indent->setEnabled(false);
        }
    }

    if (richTextSupport & KRichTextWidget::SupportDedentLists) {
        if (richTextEnabled) {
            action_list_dedent->setEnabled(q->canDedentList());
        } else {
            action_list_dedent->setEnabled(false);
        }
    }

    if (richTextSupport & KRichTextWidget::SupportDirection) {
        const Qt::LayoutDirection direction = q->textCursor().blockFormat().layoutDirection();
        action_direction_ltr->setChecked(direction == Qt::LeftToRight);
        action_direction_rtl->setChecked(direction == Qt::RightToLeft);
    }

    if (richTextSupport & KRichTextWidget::SupportHeading) {
        action_heading_level->setCurrentItem(q->textCursor().blockFormat().headingLevel());
    }
}

void KRichTextWidgetPrivate::_k_setTextForegroundColor()
{
    Q_Q(KRichTextWidget);

    const QColor currentColor = q->textColor();
    const QColor defaultColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();

    const QColor selectedColor = QColorDialog::getColor(currentColor.isValid() ? currentColor : defaultColor, q);

    if (!selectedColor.isValid() && !currentColor.isValid()) {
        q->setTextForegroundColor(defaultColor);
    } else if (selectedColor.isValid()) {
        q->setTextForegroundColor(selectedColor);
    }
}

void KRichTextWidgetPrivate::_k_setTextBackgroundColor()
{
    Q_Q(KRichTextWidget);

    QTextCharFormat fmt = q->textCursor().charFormat();
    const QColor currentColor = fmt.background().color();
    const QColor defaultColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();

    const QColor selectedColor = QColorDialog::getColor(currentColor.isValid() ? currentColor : defaultColor, q);

    if (!selectedColor.isValid() && !currentColor.isValid()) {
        q->setTextBackgroundColor(defaultColor);
    } else if (selectedColor.isValid()) {
        q->setTextBackgroundColor(selectedColor);
    }
}

void KRichTextWidgetPrivate::_k_manageLink()
{
    Q_Q(KRichTextWidget);

    q->selectLinkText();
    KLinkDialog *linkDialog = new KLinkDialog(q);
    linkDialog->setLinkText(q->currentLinkText());
    linkDialog->setLinkUrl(q->currentLinkUrl());
    linkDialog->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(linkDialog, &QDialog::accepted, linkDialog, [linkDialog, this]() {
        Q_Q(KRichTextWidget);
        q->updateLink(linkDialog->linkUrl(), linkDialog->linkText());
    });

    linkDialog->show();
}

void KRichTextWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(KRichTextWidget);

    if (d->painterActive) {
        // If the painter is active, paint the selection with the
        // correct format.
        if (textCursor().hasSelection()) {
            QTextCursor c = textCursor();
            c.setCharFormat(d->painterFormat);
            setTextCursor(c);
        }
        d->painterActive = false;
        d->action_format_painter->setChecked(false);
    }
    KRichTextEdit::mouseReleaseEvent(event);
}

void KRichTextWidgetPrivate::_k_formatPainter(bool active)
{
    Q_Q(KRichTextWidget);

    if (active) {
        painterFormat = q->currentCharFormat();
        painterActive = true;
        q->viewport()->setCursor(QCursor(QIcon::fromTheme(QStringLiteral("draw-brush")).pixmap(32, 32), 0, 32));
    } else {
        painterFormat = QTextCharFormat();
        painterActive = false;
        q->viewport()->setCursor(Qt::IBeamCursor);
    }
}

void KRichTextWidget::updateActionStates()
{
    Q_D(KRichTextWidget);

    d->updateMiscActions();
    d->updateCharFormatActions(currentCharFormat());
}

#include "moc_krichtextwidget.cpp"
