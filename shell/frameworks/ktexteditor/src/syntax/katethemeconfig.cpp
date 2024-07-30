/*
    SPDX-FileCopyrightText: 2007, 2008 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001-2003 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN Includes
#include "katethemeconfig.h"

#include "katecolortreewidget.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katehighlight.h"
#include "katestyletreewidget.h"
#include "katesyntaxmanager.h"
#include "kateview.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageWidget>

#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QJsonObject>
#include <QLabel>
#include <QMetaEnum>
#include <QPushButton>
#include <QShowEvent>
#include <QTabWidget>

// END

/**
 * Return the translated name of default style @p n.
 */
static inline QString defaultStyleName(KSyntaxHighlighting::Theme::TextStyle style)
{
    using namespace KTextEditor;
    switch (style) {
    case KSyntaxHighlighting::Theme::TextStyle::Normal:
        return i18nc("@item:intable Text context", "Normal");
    case KSyntaxHighlighting::Theme::TextStyle::Keyword:
        return i18nc("@item:intable Text context", "Keyword");
    case KSyntaxHighlighting::Theme::TextStyle::Function:
        return i18nc("@item:intable Text context", "Function");
    case KSyntaxHighlighting::Theme::TextStyle::Variable:
        return i18nc("@item:intable Text context", "Variable");
    case KSyntaxHighlighting::Theme::TextStyle::ControlFlow:
        return i18nc("@item:intable Text context", "Control Flow");
    case KSyntaxHighlighting::Theme::TextStyle::Operator:
        return i18nc("@item:intable Text context", "Operator");
    case KSyntaxHighlighting::Theme::TextStyle::BuiltIn:
        return i18nc("@item:intable Text context", "Built-in");
    case KSyntaxHighlighting::Theme::TextStyle::Extension:
        return i18nc("@item:intable Text context", "Extension");
    case KSyntaxHighlighting::Theme::TextStyle::Preprocessor:
        return i18nc("@item:intable Text context", "Preprocessor");
    case KSyntaxHighlighting::Theme::TextStyle::Attribute:
        return i18nc("@item:intable Text context", "Attribute");

    case KSyntaxHighlighting::Theme::TextStyle::Char:
        return i18nc("@item:intable Text context", "Character");
    case KSyntaxHighlighting::Theme::TextStyle::SpecialChar:
        return i18nc("@item:intable Text context", "Special Character");
    case KSyntaxHighlighting::Theme::TextStyle::String:
        return i18nc("@item:intable Text context", "String");
    case KSyntaxHighlighting::Theme::TextStyle::VerbatimString:
        return i18nc("@item:intable Text context", "Verbatim String");
    case KSyntaxHighlighting::Theme::TextStyle::SpecialString:
        return i18nc("@item:intable Text context", "Special String");
    case KSyntaxHighlighting::Theme::TextStyle::Import:
        return i18nc("@item:intable Text context", "Imports, Modules, Includes");

    case KSyntaxHighlighting::Theme::TextStyle::DataType:
        return i18nc("@item:intable Text context", "Data Type");
    case KSyntaxHighlighting::Theme::TextStyle::DecVal:
        return i18nc("@item:intable Text context", "Decimal/Value");
    case KSyntaxHighlighting::Theme::TextStyle::BaseN:
        return i18nc("@item:intable Text context", "Base-N Integer");
    case KSyntaxHighlighting::Theme::TextStyle::Float:
        return i18nc("@item:intable Text context", "Floating Point");
    case KSyntaxHighlighting::Theme::TextStyle::Constant:
        return i18nc("@item:intable Text context", "Constant");

    case KSyntaxHighlighting::Theme::TextStyle::Comment:
        return i18nc("@item:intable Text context", "Comment");
    case KSyntaxHighlighting::Theme::TextStyle::Documentation:
        return i18nc("@item:intable Text context", "Documentation");
    case KSyntaxHighlighting::Theme::TextStyle::Annotation:
        return i18nc("@item:intable Text context", "Annotation");
    case KSyntaxHighlighting::Theme::TextStyle::CommentVar:
        return i18nc("@item:intable Text context", "Comment Variable");
    case KSyntaxHighlighting::Theme::TextStyle::RegionMarker:
        // this next one is for denoting the beginning/end of a user defined folding region
        return i18nc("@item:intable Text context", "Region Marker");
    case KSyntaxHighlighting::Theme::TextStyle::Information:
        return i18nc("@item:intable Text context", "Information");
    case KSyntaxHighlighting::Theme::TextStyle::Warning:
        return i18nc("@item:intable Text context", "Warning");
    case KSyntaxHighlighting::Theme::TextStyle::Alert:
        return i18nc("@item:intable Text context", "Alert");

    case KSyntaxHighlighting::Theme::TextStyle::Others:
        return i18nc("@item:intable Text context", "Others");
    case KSyntaxHighlighting::Theme::TextStyle::Error:
        // this one is for marking invalid input
        return i18nc("@item:intable Text context", "Error");
    };
    Q_UNREACHABLE();
}

/**
 * Helper to get json object for given valid theme.
 * @param theme theme to get json object for
 * @return json object of theme
 */
static QJsonObject jsonForTheme(const KSyntaxHighlighting::Theme &theme)
{
    // load json content, shall work, as the theme as valid, but still abort on errors
    QFile loadFile(theme.filePath());
    if (!loadFile.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }
    const QByteArray jsonData = loadFile.readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return QJsonObject();
    }
    return jsonDoc.object();
}

/**
 * Helper to write json data to given file path.
 * After the function returns, stuff is flushed to disk for sure.
 * @param json json object with theme data
 * @param themeFileName file name to write to
 * @return did writing succeed?
 */
static bool writeJson(const QJsonObject &json, const QString &themeFileName)
{
    QFile saveFile(themeFileName);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    saveFile.write(QJsonDocument(json).toJson());
    return true;
}

// BEGIN KateThemeConfigColorTab -- 'Colors' tab
KateThemeConfigColorTab::KateThemeConfigColorTab()
{
    QGridLayout *l = new QGridLayout(this);

    ui = new KateColorTreeWidget(this);
    QPushButton *btnUseColorScheme = new QPushButton(i18n("Use Default Colors"), this);

    l->addWidget(ui, 0, 0, 1, 2);
    l->addWidget(btnUseColorScheme, 1, 1);

    l->setColumnStretch(0, 1);
    l->setColumnStretch(1, 0);

    connect(btnUseColorScheme, &QPushButton::clicked, ui, &KateColorTreeWidget::selectDefaults);
    connect(ui, &KateColorTreeWidget::changed, this, &KateThemeConfigColorTab::changed);
}

static QList<KateColorItem> colorItemList(const KSyntaxHighlighting::Theme &theme)
{
    QList<KateColorItem> items;

    //
    // editor background colors
    //
    KateColorItem ci(KSyntaxHighlighting::Theme::BackgroundColor);
    ci.category = i18n("Editor Background Colors");

    ci.name = i18n("Text Area");
    ci.key = QStringLiteral("Color Background");
    ci.whatsThis = i18n("<p>Sets the background color of the editing area.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::TextSelection;
    ci.name = i18n("Selected Text");
    ci.key = QStringLiteral("Color Selection");
    ci.whatsThis = i18n(
        "<p>Sets the background color of the selection.</p><p>To set the text color for selected text, use the &quot;<b>Configure Highlighting</b>&quot; "
        "dialog.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::CurrentLine;
    ci.name = i18n("Current Line");
    ci.key = QStringLiteral("Color Highlighted Line");
    ci.whatsThis = i18n("<p>Sets the background color of the currently active line, which means the line where your cursor is positioned.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::SearchHighlight;
    ci.name = i18n("Search Highlight");
    ci.key = QStringLiteral("Color Search Highlight");
    ci.whatsThis = i18n("<p>Sets the background color of search results.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::ReplaceHighlight;
    ci.name = i18n("Replace Highlight");
    ci.key = QStringLiteral("Color Replace Highlight");
    ci.whatsThis = i18n("<p>Sets the background color of replaced text.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    //
    // icon border
    //
    ci.category = i18n("Icon Border");

    ci.role = KSyntaxHighlighting::Theme::IconBorder;
    ci.name = i18n("Background Area");
    ci.key = QStringLiteral("Color Icon Bar");
    ci.whatsThis = i18n("<p>Sets the background color of the icon border.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::LineNumbers;
    ci.name = i18n("Line Numbers");
    ci.key = QStringLiteral("Color Line Number");
    ci.whatsThis = i18n("<p>This color will be used to draw the line numbers (if enabled).</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::CurrentLineNumber;
    ci.name = i18n("Current Line Number");
    ci.key = QStringLiteral("Color Current Line Number");
    ci.whatsThis = i18n("<p>This color will be used to draw the number of the current line (if enabled).</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::Separator;
    ci.name = i18n("Separator");
    ci.key = QStringLiteral("Color Separator");
    ci.whatsThis = i18n("<p>This color will be used to draw the line between line numbers and the icon borders, if both are enabled.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::WordWrapMarker;
    ci.name = i18n("Word Wrap Marker");
    ci.key = QStringLiteral("Color Word Wrap Marker");
    ci.whatsThis = i18n(
        "<p>Sets the color of Word Wrap-related markers:</p><dl><dt>Static Word Wrap</dt><dd>A vertical line which shows the column where text is going to be "
        "wrapped</dd><dt>Dynamic Word Wrap</dt><dd>An arrow shown to the left of "
        "visually-wrapped lines</dd></dl>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::CodeFolding;
    ci.name = i18n("Code Folding");
    ci.key = QStringLiteral("Color Code Folding");
    ci.whatsThis = i18n("<p>Sets the color of the code folding bar.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::ModifiedLines;
    ci.name = i18n("Modified Lines");
    ci.key = QStringLiteral("Color Modified Lines");
    ci.whatsThis = i18n("<p>Sets the color of the line modification marker for modified lines.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::SavedLines;
    ci.name = i18n("Saved Lines");
    ci.key = QStringLiteral("Color Saved Lines");
    ci.whatsThis = i18n("<p>Sets the color of the line modification marker for saved lines.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    //
    // text decorations
    //
    ci.category = i18n("Text Decorations");

    ci.role = KSyntaxHighlighting::Theme::SpellChecking;
    ci.name = i18n("Spelling Mistake Line");
    ci.key = QStringLiteral("Color Spelling Mistake Line");
    ci.whatsThis = i18n("<p>Sets the color of the line that is used to indicate spelling mistakes.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::TabMarker;
    ci.name = i18n("Tab and Space Markers");
    ci.key = QStringLiteral("Color Tab Marker");
    ci.whatsThis = i18n("<p>Sets the color of the tabulator marks.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::IndentationLine;
    ci.name = i18n("Indentation Line");
    ci.key = QStringLiteral("Color Indentation Line");
    ci.whatsThis = i18n("<p>Sets the color of the vertical indentation lines.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::BracketMatching;
    ci.name = i18n("Bracket Highlight");
    ci.key = QStringLiteral("Color Highlighted Bracket");
    ci.whatsThis = i18n(
        "<p>Sets the bracket matching color. This means, if you place the cursor e.g. at a <b>(</b>, the matching <b>)</b> will be highlighted with this "
        "color.</p>");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    //
    // marker colors
    //
    ci.category = i18n("Marker Colors");

    const QString markerNames[KSyntaxHighlighting::Theme::MarkError - KSyntaxHighlighting::Theme::MarkBookmark + 1] = {i18n("Bookmark"),
                                                                                                                       i18n("Active Breakpoint"),
                                                                                                                       i18n("Reached Breakpoint"),
                                                                                                                       i18n("Disabled Breakpoint"),
                                                                                                                       i18n("Execution"),
                                                                                                                       i18n("Warning"),
                                                                                                                       i18n("Error")};

    ci.whatsThis = i18n("<p>Sets the background color of mark type.</p><p><b>Note</b>: The marker color is displayed lightly because of transparency.</p>");
    for (int i = 0; i <= KSyntaxHighlighting::Theme::MarkError - KSyntaxHighlighting::Theme::MarkBookmark; ++i) {
        ci.role = static_cast<KSyntaxHighlighting::Theme::EditorColorRole>(i + KSyntaxHighlighting::Theme::MarkBookmark);
        ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
        ci.name = markerNames[i];
        ci.key = QLatin1String("Color MarkType ") + QString::number(i + 1);
        items.append(ci);
    }

    //
    // text templates
    //
    ci.category = i18n("Text Templates & Snippets");

    ci.whatsThis = QString(); // TODO: add whatsThis for text templates

    ci.role = KSyntaxHighlighting::Theme::TemplateBackground;
    ci.name = i18n("Background");
    ci.key = QStringLiteral("Color Template Background");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::TemplatePlaceholder;
    ci.name = i18n("Editable Placeholder");
    ci.key = QStringLiteral("Color Template Editable Placeholder");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::TemplateFocusedPlaceholder;
    ci.name = i18n("Focused Editable Placeholder");
    ci.key = QStringLiteral("Color Template Focused Editable Placeholder");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    ci.role = KSyntaxHighlighting::Theme::TemplateReadOnlyPlaceholder;
    ci.name = i18n("Not Editable Placeholder");
    ci.key = QStringLiteral("Color Template Not Editable Placeholder");
    ci.defaultColor = QColor::fromRgba(theme.editorColor(ci.role));
    items.append(ci);

    //
    // finally, add all elements
    //
    return items;
}

void KateThemeConfigColorTab::schemaChanged(const QString &newSchema)
{
    // ensure invalid or read-only stuff can't be changed
    const auto theme = KateHlManager::self()->repository().theme(newSchema);
    ui->setReadOnly(!theme.isValid() || theme.isReadOnly());

    // save current schema
    if (!m_currentSchema.isEmpty()) {
        auto it = m_schemas.find(m_currentSchema);
        if (it != m_schemas.end()) {
            m_schemas.erase(m_currentSchema); // clear this color schema
        }

        // now add it again
        m_schemas[m_currentSchema] = ui->colorItems();
    }

    if (newSchema == m_currentSchema) {
        return;
    }

    // switch
    m_currentSchema = newSchema;

    // If we havent this schema, read in from config file
    if (m_schemas.find(newSchema) == m_schemas.end()) {
        QList<KateColorItem> items = colorItemList(theme);
        for (auto &item : items) {
            item.color = QColor::fromRgba(theme.editorColor(item.role));
        }
        m_schemas[newSchema] = std::move(items);
    }

    // first block signals otherwise setColor emits changed
    const bool blocked = blockSignals(true);

    ui->clear();
    ui->addColorItems(m_schemas[m_currentSchema]);

    blockSignals(blocked);
}

/**
 * @brief Converts @p c to its hex value, if @p c has alpha then the returned string
 * will be of the format #AARRGGBB other wise #RRGGBB
 */
static QString hexName(const QColor &c)
{
    return c.alpha() == 0xFF ? c.name() : c.name(QColor::HexArgb);
}

void KateThemeConfigColorTab::apply()
{
    schemaChanged(m_currentSchema);

    // we use meta-data of enum for computing the json keys
    static const auto idx = KSyntaxHighlighting::Theme::staticMetaObject.indexOfEnumerator("EditorColorRole");
    Q_ASSERT(idx >= 0);
    const auto metaEnum = KSyntaxHighlighting::Theme::staticMetaObject.enumerator(idx);

    // export all themes we cached data for
    for (auto it = m_schemas.cbegin(); it != m_schemas.cend(); ++it) {
        // get theme for key, skip invalid or read-only themes for writing
        const auto theme = KateHlManager::self()->repository().theme(it->first);
        if (!theme.isValid() || theme.isReadOnly()) {
            continue;
        }

        // get current theme data from disk
        QJsonObject newThemeObject = jsonForTheme(theme);

        // patch the editor-colors part
        QJsonObject colors;
        const auto &colorItems = it->second;
        for (const KateColorItem &item : colorItems) {
            QColor c = item.useDefault ? item.defaultColor : item.color;
            colors[QLatin1String(metaEnum.key(item.role))] = hexName(c);
        }
        newThemeObject[QLatin1String("editor-colors")] = colors;

        // write json back to file
        writeJson(newThemeObject, theme.filePath());
    }

    // all colors are written, so throw away all cached schemas
    m_schemas.clear();
}

void KateThemeConfigColorTab::reload()
{
    // drop all cached data
    m_schemas.clear();

    // trigger re-creation of ui from theme
    const auto backupName = m_currentSchema;
    m_currentSchema.clear();
    schemaChanged(backupName);
}

QColor KateThemeConfigColorTab::backgroundColor() const
{
    return ui->findColor(QStringLiteral("Color Background"));
}

QColor KateThemeConfigColorTab::selectionColor() const
{
    return ui->findColor(QStringLiteral("Color Selection"));
}
// END KateThemeConfigColorTab

// BEGIN FontColorConfig -- 'Normal Text Styles' tab
KateThemeConfigDefaultStylesTab::KateThemeConfigDefaultStylesTab(KateThemeConfigColorTab *colorTab)
{
    m_colorTab = colorTab;

    // size management
    QGridLayout *grid = new QGridLayout(this);

    m_defaultStyles = new KateStyleTreeWidget(this);
    connect(m_defaultStyles, &KateStyleTreeWidget::changed, this, &KateThemeConfigDefaultStylesTab::changed);
    grid->addWidget(m_defaultStyles, 0, 0);

    m_defaultStyles->setWhatsThis(
        i18n("<p>This list displays the default styles for the current color theme and "
             "offers the means to edit them. The style name reflects the current "
             "style settings.</p>"
             "<p>To edit the colors, click the colored squares, or select the color "
             "to edit from the popup menu.</p><p>You can unset the Background and Selected "
             "Background colors from the popup menu when appropriate.</p>"));
}

KateAttributeList *KateThemeConfigDefaultStylesTab::attributeList(const QString &schema)
{
    auto it = m_defaultStyleLists.find(schema);
    if (it == m_defaultStyleLists.end()) {
        // get list of all default styles
        const auto numStyles = QMetaEnum::fromType<KSyntaxHighlighting::Theme::TextStyle>().keyCount();
        KateAttributeList list;
        list.reserve(numStyles);
        const KSyntaxHighlighting::Theme currentTheme = KateHlManager::self()->repository().theme(schema);
        for (int z = 0; z < numStyles; z++) {
            KTextEditor::Attribute::Ptr i(new KTextEditor::Attribute());
            const auto style = static_cast<KSyntaxHighlighting::Theme::TextStyle>(z);

            if (const auto col = currentTheme.textColor(style)) {
                i->setForeground(QColor::fromRgba(col));
            }

            if (const auto col = currentTheme.selectedTextColor(style)) {
                i->setSelectedForeground(QColor::fromRgba(col));
            }

            if (const auto col = currentTheme.backgroundColor(style)) {
                i->setBackground(QColor::fromRgba(col));
            } else {
                i->clearBackground();
            }

            if (const auto col = currentTheme.selectedBackgroundColor(style)) {
                i->setSelectedBackground(QColor::fromRgba(col));
            } else {
                i->clearProperty(SelectedBackground);
            }

            i->setFontBold(currentTheme.isBold(style));
            i->setFontItalic(currentTheme.isItalic(style));
            i->setFontUnderline(currentTheme.isUnderline(style));
            i->setFontStrikeOut(currentTheme.isStrikeThrough(style));
            list.append(i);
        }
        it = m_defaultStyleLists.emplace(schema, list).first;
    }

    return &(it->second);
}

void KateThemeConfigDefaultStylesTab::schemaChanged(const QString &schema)
{
    // ensure invalid or read-only stuff can't be changed
    const auto theme = KateHlManager::self()->repository().theme(schema);
    m_defaultStyles->setReadOnly(!theme.isValid() || theme.isReadOnly());

    m_currentSchema = schema;

    m_defaultStyles->clear();

    KateAttributeList *l = attributeList(schema);
    updateColorPalette(l->at(0)->foreground().color());

    // normal text and source code
    QTreeWidgetItem *parent = new QTreeWidgetItem(m_defaultStyles, QStringList() << i18nc("@item:intable", "Normal Text & Source Code"));
    parent->setFirstColumnSpanned(true);
    for (int i = (int)KSyntaxHighlighting::Theme::TextStyle::Normal; i <= (int)KSyntaxHighlighting::Theme::TextStyle::Attribute; ++i) {
        m_defaultStyles->addItem(parent, defaultStyleName(static_cast<KSyntaxHighlighting::Theme::TextStyle>(i)), l->at(i));
    }

    // Number, Types & Constants
    parent = new QTreeWidgetItem(m_defaultStyles, QStringList() << i18nc("@item:intable", "Numbers, Types & Constants"));
    parent->setFirstColumnSpanned(true);
    for (int i = (int)KSyntaxHighlighting::Theme::TextStyle::DataType; i <= (int)KSyntaxHighlighting::Theme::TextStyle::Constant; ++i) {
        m_defaultStyles->addItem(parent, defaultStyleName(static_cast<KSyntaxHighlighting::Theme::TextStyle>(i)), l->at(i));
    }

    // strings & characters
    parent = new QTreeWidgetItem(m_defaultStyles, QStringList() << i18nc("@item:intable", "Strings & Characters"));
    parent->setFirstColumnSpanned(true);
    for (int i = (int)KSyntaxHighlighting::Theme::TextStyle::Char; i <= (int)KSyntaxHighlighting::Theme::TextStyle::Import; ++i) {
        m_defaultStyles->addItem(parent, defaultStyleName(static_cast<KSyntaxHighlighting::Theme::TextStyle>(i)), l->at(i));
    }

    // comments & documentation
    parent = new QTreeWidgetItem(m_defaultStyles, QStringList() << i18nc("@item:intable", "Comments & Documentation"));
    parent->setFirstColumnSpanned(true);
    for (int i = (int)KSyntaxHighlighting::Theme::TextStyle::Comment; i <= (int)KSyntaxHighlighting::Theme::TextStyle::Alert; ++i) {
        m_defaultStyles->addItem(parent, defaultStyleName(static_cast<KSyntaxHighlighting::Theme::TextStyle>(i)), l->at(i));
    }

    // Misc
    parent = new QTreeWidgetItem(m_defaultStyles, QStringList() << i18nc("@item:intable", "Miscellaneous"));
    parent->setFirstColumnSpanned(true);
    for (int i = (int)KSyntaxHighlighting::Theme::TextStyle::Others; i <= (int)KSyntaxHighlighting::Theme::TextStyle::Error; ++i) {
        m_defaultStyles->addItem(parent, defaultStyleName(static_cast<KSyntaxHighlighting::Theme::TextStyle>(i)), l->at(i));
    }

    m_defaultStyles->expandAll();
}

void KateThemeConfigDefaultStylesTab::updateColorPalette(const QColor &textColor)
{
    QPalette p(m_defaultStyles->palette());
    p.setColor(QPalette::Base, m_colorTab->backgroundColor());
    p.setColor(QPalette::Highlight, m_colorTab->selectionColor());
    p.setColor(QPalette::Text, textColor);
    m_defaultStyles->setPalette(p);
}

void KateThemeConfigDefaultStylesTab::reload()
{
    m_defaultStyles->clear();
    m_defaultStyleLists.clear();

    schemaChanged(m_currentSchema);
}

void KateThemeConfigDefaultStylesTab::apply()
{
    // get enum meta data for json keys
    static const auto idx = KSyntaxHighlighting::Theme::staticMetaObject.indexOfEnumerator("TextStyle");
    Q_ASSERT(idx >= 0);
    const auto metaEnum = KSyntaxHighlighting::Theme::staticMetaObject.enumerator(idx);

    // export all configured styles of the cached themes
    for (const auto &kv : m_defaultStyleLists) {
        // get theme for key, skip invalid or read-only themes for writing
        const auto theme = KateHlManager::self()->repository().theme(kv.first);
        if (!theme.isValid() || theme.isReadOnly()) {
            continue;
        }

        // get current theme data from disk
        QJsonObject newThemeObject = jsonForTheme(theme);

        // patch the text-styles part
        QJsonObject styles;
        const auto numStyles = QMetaEnum::fromType<KSyntaxHighlighting::Theme::TextStyle>().keyCount();
        for (int z = 0; z < numStyles; z++) {
            QJsonObject style;
            KTextEditor::Attribute::Ptr p = kv.second.at(z);
            if (p->hasProperty(QTextFormat::ForegroundBrush)) {
                style[QLatin1String("text-color")] = hexName(p->foreground().color());
            }
            if (p->hasProperty(QTextFormat::BackgroundBrush)) {
                style[QLatin1String("background-color")] = hexName(p->background().color());
            }
            if (p->hasProperty(SelectedForeground)) {
                style[QLatin1String("selected-text-color")] = hexName(p->selectedForeground().color());
            }
            if (p->hasProperty(SelectedBackground)) {
                style[QLatin1String("selected-background-color")] = hexName(p->selectedBackground().color());
            }
            if (p->hasProperty(QTextFormat::FontWeight) && p->fontBold()) {
                style[QLatin1String("bold")] = true;
            }
            if (p->hasProperty(QTextFormat::FontItalic) && p->fontItalic()) {
                style[QLatin1String("italic")] = true;
            }
            if (p->hasProperty(QTextFormat::TextUnderlineStyle) && p->fontUnderline()) {
                style[QLatin1String("underline")] = true;
            }
            if (p->hasProperty(QTextFormat::FontStrikeOut) && p->fontStrikeOut()) {
                style[QLatin1String("strike-through")] = true;
            }
            styles[QLatin1String(metaEnum.key((z)))] = style;
        }
        newThemeObject[QLatin1String("text-styles")] = styles;

        // write json back to file
        writeJson(newThemeObject, theme.filePath());
    }
}

void KateThemeConfigDefaultStylesTab::showEvent(QShowEvent *event)
{
    if (!event->spontaneous() && !m_currentSchema.isEmpty()) {
        KateAttributeList *l = attributeList(m_currentSchema);
        Q_ASSERT(l != nullptr);
        updateColorPalette(l->at(0)->foreground().color());
    }

    QWidget::showEvent(event);
}
// END FontColorConfig

// BEGIN KateThemeConfigHighlightTab -- 'Highlighting Text Styles' tab
KateThemeConfigHighlightTab::KateThemeConfigHighlightTab(KateThemeConfigDefaultStylesTab *page, KateThemeConfigColorTab *colorTab)
{
    m_defaults = page;
    m_colorTab = colorTab;

    m_hl = 0;

    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    layout->addLayout(headerLayout);

    QLabel *lHl = new QLabel(i18n("H&ighlight:"), this);
    headerLayout->addWidget(lHl);

    hlCombo = new QComboBox(this);
    hlCombo->setEditable(false);
    headerLayout->addWidget(hlCombo);

    lHl->setBuddy(hlCombo);
    connect(hlCombo, &QComboBox::activated, this, &KateThemeConfigHighlightTab::hlChanged);

    headerLayout->addStretch();

    const auto modeList = KateHlManager::self()->modeList();
    for (const auto &hl : modeList) {
        const auto section = hl.translatedSection();
        if (!section.isEmpty()) {
            hlCombo->addItem(section + QLatin1Char('/') + hl.translatedName());
        } else {
            hlCombo->addItem(hl.translatedName());
        }
    }
    hlCombo->setCurrentIndex(0);

    // styles listview
    m_styles = new KateStyleTreeWidget(this, true);
    connect(m_styles, &KateStyleTreeWidget::changed, this, &KateThemeConfigHighlightTab::changed);
    layout->addWidget(m_styles, 999);

    // get current highlighting from the host application
    int hl = 0;
    KTextEditor::ViewPrivate *kv =
        qobject_cast<KTextEditor::ViewPrivate *>(KTextEditor::EditorPrivate::self()->application()->activeMainWindow()->activeView());
    if (kv) {
        const QString hlName = kv->doc()->highlight()->name();
        hl = KateHlManager::self()->nameFind(hlName);
        Q_ASSERT(hl >= 0);
    }

    hlCombo->setCurrentIndex(hl);
    hlChanged(hl);

    m_styles->setWhatsThis(
        i18n("<p>This list displays the contexts of the current syntax highlight mode and "
             "offers the means to edit them. The context name reflects the current "
             "style settings.</p><p>To edit using the keyboard, press "
             "<strong>&lt;SPACE&gt;</strong> and choose a property from the popup menu.</p>"
             "<p>To edit the colors, click the colored squares, or select the color "
             "to edit from the popup menu.</p><p>You can unset the Background and Selected "
             "Background colors from the context menu when appropriate.</p>"));
}

void KateThemeConfigHighlightTab::hlChanged(int z)
{
    m_hl = z;
    schemaChanged(m_schema);
}

/**
 * Helper to get the "default attributes" for the given schema + highlighting.
 * This means all stuff set without taking theme overrides for the highlighting into account.
 */
static KateAttributeList defaultsForHighlighting(const std::vector<KSyntaxHighlighting::Format> &formats, const KateAttributeList &defaultStyleAttributes)
{
    const KSyntaxHighlighting::Theme invalidTheme;
    KateAttributeList defaults;
    for (const auto &format : formats) {
        // create a KTextEditor attribute matching the default style for this format
        // use the default style attribute we got passed to have the one we currently have configured in the settings here
        KTextEditor::Attribute::Ptr newAttribute(new KTextEditor::Attribute(*defaultStyleAttributes.at(format.textStyle())));

        // check for override => if yes, set attribute as overridden, use invalid theme to avoid the usage of theme override!

        if (format.hasTextColorOverride()) {
            newAttribute->setForeground(format.textColor(invalidTheme));
        }
        if (format.hasBackgroundColorOverride()) {
            newAttribute->setBackground(format.backgroundColor(invalidTheme));
        }
        if (format.hasSelectedTextColorOverride()) {
            newAttribute->setSelectedForeground(format.selectedTextColor(invalidTheme));
        }
        if (format.hasSelectedBackgroundColorOverride()) {
            newAttribute->setSelectedBackground(format.selectedBackgroundColor(invalidTheme));
        }
        if (format.hasBoldOverride()) {
            newAttribute->setFontBold(format.isBold(invalidTheme));
        }
        if (format.hasItalicOverride()) {
            newAttribute->setFontItalic(format.isItalic(invalidTheme));
        }
        if (format.hasUnderlineOverride()) {
            newAttribute->setFontUnderline(format.isUnderline(invalidTheme));
        }
        if (format.hasStrikeThroughOverride()) {
            newAttribute->setFontStrikeOut(format.isStrikeThrough(invalidTheme));
        }

        // not really relevant, set it as configured
        newAttribute->setSkipSpellChecking(format.spellCheck());
        defaults.append(newAttribute);
    }
    return defaults;
}

void KateThemeConfigHighlightTab::schemaChanged(const QString &schema)
{
    // ensure invalid or read-only stuff can't be changed
    const auto theme = KateHlManager::self()->repository().theme(schema);

    // NOTE: None (m_hl == 0) can't be changed with the current way
    // TODO: removed it from the list?
    const auto isNoneSchema = m_hl == 0;
    m_styles->setReadOnly(!theme.isValid() || theme.isReadOnly() || isNoneSchema);

    m_schema = schema;

    m_styles->clear();

    auto it = m_hlDict.find(m_schema);
    if (it == m_hlDict.end()) {
        it = m_hlDict.insert(schema, QHash<int, QList<KTextEditor::Attribute::Ptr>>());
    }

    // Set listview colors
    KateAttributeList *l = m_defaults->attributeList(schema);
    updateColorPalette(l->at(0)->foreground().color());

    // create unified stuff
    auto attributes = KateHlManager::self()->getHl(m_hl)->attributesForDefinition(m_schema);
    auto formats = KateHlManager::self()->getHl(m_hl)->formats();
    auto defaults = defaultsForHighlighting(formats, *l);

    for (int i = 0; i < attributes.size(); ++i) {
        // All stylenames have their language mode prefixed, e.g. HTML:Comment
        // split them and put them into nice substructures.
        int c = attributes[i]->name().indexOf(QLatin1Char(':'));
        if (c <= 0) {
            continue;
        }

        QString highlighting = attributes[i]->name().left(c);
        QString name = attributes[i]->name().mid(c + 1);
        auto &uniqueAttribute = m_uniqueAttributes[m_schema][highlighting][name].first;
        auto &uniqueAttributeDefault = m_uniqueAttributes[m_schema][highlighting][name].second;

        if (uniqueAttribute.data()) {
            attributes[i] = uniqueAttribute;
        } else {
            uniqueAttribute = attributes[i];
        }

        if (uniqueAttributeDefault.data()) {
            defaults[i] = uniqueAttributeDefault;
        } else {
            uniqueAttributeDefault = defaults[i];
        }
    }

    auto &subMap = it.value();
    auto it1 = subMap.find(m_hl);
    if (it1 == subMap.end()) {
        it1 = subMap.insert(m_hl, attributes);
    }

    QHash<QString, QTreeWidgetItem *> prefixes;
    const auto &attribs = it1.value();
    auto vec_it = attribs.cbegin();
    int i = 0;
    while (vec_it != attribs.end()) {
        const KTextEditor::Attribute::Ptr itemData = *vec_it;
        Q_ASSERT(itemData);

        // All stylenames have their language mode prefixed, e.g. HTML:Comment
        // split them and put them into nice substructures.
        int c = itemData->name().indexOf(QLatin1Char(':'));
        if (c > 0) {
            QString prefix = itemData->name().left(c);
            QString name = itemData->name().mid(c + 1);

            QTreeWidgetItem *parent = prefixes[prefix];
            if (!parent) {
                parent = new QTreeWidgetItem(m_styles, QStringList() << prefix);
                m_styles->expandItem(parent);
                prefixes.insert(prefix, parent);
            }
            m_styles->addItem(parent, name, defaults.at(i), itemData);
        } else {
            m_styles->addItem(itemData->name(), defaults.at(i), itemData);
        }
        ++vec_it;
        ++i;
    }

    m_styles->resizeColumns();
}

void KateThemeConfigHighlightTab::updateColorPalette(const QColor &textColor)
{
    QPalette p(m_styles->palette());
    p.setColor(QPalette::Base, m_colorTab->backgroundColor());
    p.setColor(QPalette::Highlight, m_colorTab->selectionColor());
    p.setColor(QPalette::Text, textColor);
    m_styles->setPalette(p);
}

void KateThemeConfigHighlightTab::reload()
{
    m_styles->clear();

    m_hlDict.clear();
    m_uniqueAttributes.clear();

    hlChanged(hlCombo->currentIndex());
}

void KateThemeConfigHighlightTab::apply()
{
    // handle all cached themes data
    for (const auto &themeIt : m_uniqueAttributes) {
        // get theme for key, skip invalid or read-only themes for writing
        const auto theme = KateHlManager::self()->repository().theme(themeIt.first);
        if (!theme.isValid() || theme.isReadOnly()) {
            continue;
        }

        // get current theme data from disk
        QJsonObject newThemeObject = jsonForTheme(theme);

        // look at all highlightings we have info stored, important: keep info we did load from file and not overwrite here!
        QJsonObject overrides = newThemeObject[QLatin1String("custom-styles")].toObject();
        for (const auto &highlightingIt : themeIt.second) {
            // start with stuff we know from the loaded json
            const QString definitionName = highlightingIt.first;
            QJsonObject styles = overrides[definitionName].toObject();
            for (const auto &attributeIt : highlightingIt.second) {
                // we need to store even if we have nothing set as long as the value differs from the default, see bug 459093
                QJsonObject style;
                KTextEditor::Attribute::Ptr p = attributeIt.second.first;
                KTextEditor::Attribute::Ptr pDefault = attributeIt.second.second;
                if (p->foreground().color() != pDefault->foreground().color()) {
                    style[QLatin1String("text-color")] = hexName(p->foreground().color());
                }
                if (p->background().color() != pDefault->background().color()) {
                    style[QLatin1String("background-color")] = hexName(p->background().color());
                }
                if (p->selectedForeground().color() != pDefault->selectedForeground().color()) {
                    style[QLatin1String("selected-text-color")] = hexName(p->selectedForeground().color());
                }
                if (p->selectedBackground().color() != pDefault->selectedBackground().color()) {
                    style[QLatin1String("selected-background-color")] = hexName(p->selectedBackground().color());
                }
                if (p->fontBold() != pDefault->fontBold()) {
                    style[QLatin1String("bold")] = p->fontBold();
                }
                if (p->fontItalic() != pDefault->fontItalic()) {
                    style[QLatin1String("italic")] = p->fontItalic();
                }
                if (p->fontUnderline() != pDefault->fontUnderline()) {
                    style[QLatin1String("underline")] = p->fontUnderline();
                }
                if (p->fontStrikeOut() != pDefault->fontStrikeOut()) {
                    style[QLatin1String("strike-through")] = p->fontStrikeOut();
                }

                // either set the new stuff or erase the old entry we might have set from the loaded json
                if (!style.isEmpty()) {
                    styles[attributeIt.first] = style;
                } else {
                    styles.remove(attributeIt.first);
                }
            }

            // either set the new stuff or erase the old entry we might have set from the loaded json
            if (!styles.isEmpty()) {
                overrides[definitionName] = styles;
            } else {
                overrides.remove(definitionName);
            }
        }

        // we set even empty overrides, to ensure we overwrite stuff!
        newThemeObject[QLatin1String("custom-styles")] = overrides;

        // write json back to file
        writeJson(newThemeObject, theme.filePath());
    }
}

QList<int> KateThemeConfigHighlightTab::hlsForSchema(const QString &schema)
{
    auto it = m_hlDict.find(schema);
    if (it != m_hlDict.end()) {
        return it.value().keys();
    }
    return {};
}

void KateThemeConfigHighlightTab::showEvent(QShowEvent *event)
{
    if (!event->spontaneous()) {
        KateAttributeList *l = m_defaults->attributeList(m_schema);
        Q_ASSERT(l != nullptr);
        updateColorPalette(l->at(0)->foreground().color());
    }

    QWidget::showEvent(event);
}
// END KateThemeConfigHighlightTab

// BEGIN KateThemeConfigPage -- Main dialog page
KateThemeConfigPage::KateThemeConfigPage(QWidget *parent)
    : KateConfigPage(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins({});

    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setDocumentMode(true);
    layout->addWidget(tabWidget);

    auto *themeEditor = new QWidget(this);
    auto *themeChooser = new QWidget(this);
    tabWidget->addTab(themeChooser, i18n("Default Theme"));
    tabWidget->addTab(themeEditor, i18n("Theme Editor"));
    layoutThemeChooserTab(themeChooser);
    layoutThemeEditorTab(themeEditor);

    reload();
}

void KateThemeConfigPage::layoutThemeChooserTab(QWidget *tab)
{
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins({});

    auto *comboLayout = new QHBoxLayout;

    auto lHl = new QLabel(i18n("Select theme:"), this);
    comboLayout->addWidget(lHl);

    defaultSchemaCombo = new QComboBox(this);
    comboLayout->addWidget(defaultSchemaCombo);
    defaultSchemaCombo->setEditable(false);
    lHl->setBuddy(defaultSchemaCombo);
    connect(defaultSchemaCombo, &QComboBox::currentIndexChanged, this, &KateThemeConfigPage::slotChanged);
    comboLayout->addStretch();

    layout->addLayout(comboLayout);

    m_doc = new KTextEditor::DocumentPrivate;
    m_doc->setParent(this);

    const auto code = R"sample(/**
* SPDX-FileCopyrightText: 2020 Christoph Cullmann <cullmann@kde.org>
* SPDX-License-Identifier: MIT
*/

// BEGIN
#include <QString>
#include <string>
// END

/**
* TODO: improve documentation
* @param magicArgument some magic argument
* @return magic return value
*/
int main(uint64_t magicArgument)
{
    if (magicArgument > 1) {
        const std::string string = "source file: \"" __FILE__ "\"";
        const QString qString(QStringLiteral("test"));
        return qrand();
    }

    /* BUG: bogus integer constant inside next line */
    const double g = 1.1e12 * 0b01'01'01'01 - 43a + 0x11234 * 0234ULL - 'c' * 42;
    return g > 1.3f;
})sample";

    m_doc->setText(QString::fromUtf8(code));
    m_doc->setHighlightingMode(QStringLiteral("C++"));
    m_themePreview = new KTextEditor::ViewPrivate(m_doc, this);

    layout->addWidget(m_themePreview);

    connect(defaultSchemaCombo, &QComboBox::currentIndexChanged, this, [this](int idx) {
        const QString schema = defaultSchemaCombo->itemData(idx).toString();
        m_themePreview->rendererConfig()->setSchema(schema);
        if (schema.isEmpty()) {
            m_themePreview->rendererConfig()->setValue(KateRendererConfig::AutoColorThemeSelection, true);
        } else {
            m_themePreview->rendererConfig()->setValue(KateRendererConfig::AutoColorThemeSelection, false);
        }
    });
}

void KateThemeConfigPage::layoutThemeEditorTab(QWidget *tab)
{
    QVBoxLayout *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);

    // header
    QHBoxLayout *headerLayout = new QHBoxLayout;
    layout->addLayout(headerLayout);

    QLabel *lHl = new QLabel(i18n("&Theme:"), this);
    headerLayout->addWidget(lHl);

    schemaCombo = new QComboBox(this);
    schemaCombo->setEditable(false);
    lHl->setBuddy(schemaCombo);
    headerLayout->addWidget(schemaCombo);
    connect(schemaCombo, &QComboBox::currentIndexChanged, this, &KateThemeConfigPage::comboBoxIndexChanged);

    auto *copyButton = new QPushButton(i18nc("@action:button", "&Copy…"), this);
    headerLayout->addWidget(copyButton);
    connect(copyButton, &QPushButton::clicked, this, &KateThemeConfigPage::copyTheme);

    btndel = new QPushButton(i18n("&Delete"), this);
    headerLayout->addWidget(btndel);
    connect(btndel, &QPushButton::clicked, this, &KateThemeConfigPage::deleteSchema);

    auto *btnexport = new QPushButton(i18nc("@action:button", "Export…"), this);
    headerLayout->addWidget(btnexport);
    connect(btnexport, &QPushButton::clicked, this, &KateThemeConfigPage::exportFullSchema);

    auto *btnimport = new QPushButton(i18nc("@action:button", "Import…"), this);
    headerLayout->addWidget(btnimport);
    connect(btnimport, &QPushButton::clicked, this, &KateThemeConfigPage::importFullSchema);

    headerLayout->addStretch();

    // label to inform about read-only state
    m_readOnlyThemeLabel = new KMessageWidget(i18n("Bundled read-only theme. To modify the theme, please copy it."), this);
    m_readOnlyThemeLabel->setCloseButtonVisible(false);
    m_readOnlyThemeLabel->setMessageType(KMessageWidget::Information);
    m_readOnlyThemeLabel->hide();
    layout->addWidget(m_readOnlyThemeLabel);

    // tabs
    QTabWidget *tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);

    m_colorTab = new KateThemeConfigColorTab();
    tabWidget->addTab(m_colorTab, i18n("Colors"));
    connect(m_colorTab, &KateThemeConfigColorTab::changed, this, &KateThemeConfigPage::slotChanged);

    m_defaultStylesTab = new KateThemeConfigDefaultStylesTab(m_colorTab);
    tabWidget->addTab(m_defaultStylesTab, i18n("Default Text Styles"));
    connect(m_defaultStylesTab, &KateThemeConfigDefaultStylesTab::changed, this, &KateThemeConfigPage::slotChanged);

    m_highlightTab = new KateThemeConfigHighlightTab(m_defaultStylesTab, m_colorTab);
    tabWidget->addTab(m_highlightTab, i18n("Highlighting Text Styles"));
    connect(m_highlightTab, &KateThemeConfigHighlightTab::changed, this, &KateThemeConfigPage::slotChanged);

    QHBoxLayout *footLayout = new QHBoxLayout;
    layout->addLayout(footLayout);
}

void KateThemeConfigPage::exportFullSchema()
{
    // get save destination
    const QString currentSchemaName = m_currentSchema;
    const QString destName = QFileDialog::getSaveFileName(this,
                                                          i18n("Exporting color theme: %1", currentSchemaName),
                                                          currentSchemaName + QLatin1String(".theme"),
                                                          QStringLiteral("%1 (*.theme)").arg(i18n("Color theme")));
    if (destName.isEmpty()) {
        return;
    }

    // get current theme
    const QString currentThemeName = schemaCombo->itemData(schemaCombo->currentIndex()).toString();
    const auto currentTheme = KateHlManager::self()->repository().theme(currentThemeName);

    // ensure we overwrite
    if (QFile::exists(destName)) {
        QFile::remove(destName);
    }

    // export is easy, just copy the file 1:1
    QFile::copy(currentTheme.filePath(), destName);
}

void KateThemeConfigPage::importFullSchema()
{
    const QString srcName =
        QFileDialog::getOpenFileName(this, i18n("Importing Color Theme"), QString(), QStringLiteral("%1 (*.theme)").arg(i18n("Color theme")));
    if (srcName.isEmpty()) {
        return;
    }

    // location to write theme files to
    const QString themesPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/org.kde.syntax-highlighting/themes");

    // construct file name for imported theme
    const QString themesFullFileName = themesPath + QStringLiteral("/") + QFileInfo(srcName).fileName();

    // if something might be overwritten, as the user
    if (QFile::exists(themesFullFileName)) {
        if (KMessageBox::warningContinueCancel(this,
                                               i18n("Importing will overwrite the existing theme file \"%1\". This can not be undone.", themesFullFileName),
                                               i18n("Possible Data Loss"),
                                               KGuiItem(i18n("Import Nevertheless")),
                                               KStandardGuiItem::cancel())
            != KMessageBox::Continue) {
            return;
        }
    }

    // copy theme file, we might need to create the local dir first
    QDir().mkpath(themesPath);

    // ensure we overwrite
    if (QFile::exists(themesFullFileName)) {
        QFile::remove(themesFullFileName);
    }
    QFile::copy(srcName, themesFullFileName);

    // reload themes DB & clear all attributes
    KateHlManager::self()->reload();
    for (int i = 0; i < KateHlManager::self()->modeList().size(); ++i) {
        KateHlManager::self()->getHl(i)->clearAttributeArrays();
    }

    // KateThemeManager::update() sorts the schema alphabetically, hence the
    // schema indexes change. Thus, repopulate the schema list...
    refillCombos(schemaCombo->itemData(schemaCombo->currentIndex()).toString(), defaultSchemaCombo->itemData(defaultSchemaCombo->currentIndex()).toString());
}

void KateThemeConfigPage::apply()
{
    // remember name + index
    const QString schemaName = schemaCombo->itemData(schemaCombo->currentIndex()).toString();

    // first apply all tabs
    m_colorTab->apply();
    m_defaultStylesTab->apply();
    m_highlightTab->apply();

    // reload themes DB & clear all attributes
    KateHlManager::self()->reload();
    for (int i = 0; i < KateHlManager::self()->modeList().size(); ++i) {
        KateHlManager::self()->getHl(i)->clearAttributeArrays();
    }

    // than reload the whole stuff, special handle auto selection == empty theme name
    const auto defaultTheme = defaultSchemaCombo->itemData(defaultSchemaCombo->currentIndex()).toString();
    if (defaultTheme.isEmpty()) {
        KateRendererConfig::global()->setValue(KateRendererConfig::AutoColorThemeSelection, true);
    } else {
        KateRendererConfig::global()->setValue(KateRendererConfig::AutoColorThemeSelection, false);
        KateRendererConfig::global()->setSchema(defaultTheme);
    }
    KateRendererConfig::global()->reloadSchema();

    // KateThemeManager::update() sorts the schema alphabetically, hence the
    // schema indexes change. Thus, repopulate the schema list...
    refillCombos(schemaCombo->itemData(schemaCombo->currentIndex()).toString(), defaultSchemaCombo->itemData(defaultSchemaCombo->currentIndex()).toString());
    schemaChanged(schemaName);

    // all tabs need to reload to discard all the cached data, as the index
    // mapping may have changed
    m_colorTab->reload();
    m_defaultStylesTab->reload();
    m_highlightTab->reload();
}

void KateThemeConfigPage::reload()
{
    // reinitialize combo boxes
    refillCombos(KateRendererConfig::global()->schema(), KateRendererConfig::global()->schema());

    // finally, activate the current schema again
    schemaChanged(schemaCombo->itemData(schemaCombo->currentIndex()).toString());

    // all tabs need to reload to discard all the cached data, as the index
    // mapping may have changed
    m_colorTab->reload();
    m_defaultStylesTab->reload();
    m_highlightTab->reload();
}

void KateThemeConfigPage::refillCombos(const QString &schemaName, const QString &defaultSchemaName)
{
    schemaCombo->blockSignals(true);
    defaultSchemaCombo->blockSignals(true);

    // reinitialize combo boxes
    schemaCombo->clear();
    defaultSchemaCombo->clear();
    defaultSchemaCombo->addItem(i18n("Follow System Color Scheme"), QString());
    defaultSchemaCombo->insertSeparator(1);
    const auto themes = KateHlManager::self()->sortedThemes();
    for (const auto &theme : themes) {
        schemaCombo->addItem(theme.translatedName(), theme.name());
        defaultSchemaCombo->addItem(theme.translatedName(), theme.name());
    }

    // set the correct indexes again, fallback to always existing default theme
    int schemaIndex = schemaCombo->findData(schemaName);
    if (schemaIndex == -1) {
        schemaIndex = schemaCombo->findData(
            KTextEditor::EditorPrivate::self()->hlManager()->repository().defaultTheme(KSyntaxHighlighting::Repository::LightTheme).name());
    }

    // set the correct indexes again, fallback to auto-selection
    int defaultSchemaIndex = 0;
    if (!KateRendererConfig::global()->value(KateRendererConfig::AutoColorThemeSelection).toBool()) {
        defaultSchemaIndex = defaultSchemaCombo->findData(defaultSchemaName);
        if (defaultSchemaIndex == -1) {
            defaultSchemaIndex = 0;
        }
    }

    Q_ASSERT(schemaIndex != -1);
    Q_ASSERT(defaultSchemaIndex != -1);

    defaultSchemaCombo->setCurrentIndex(defaultSchemaIndex);
    schemaCombo->setCurrentIndex(schemaIndex);

    schemaCombo->blockSignals(false);
    defaultSchemaCombo->blockSignals(false);

    m_themePreview->rendererConfig()->setSchema(defaultSchemaName);
}

void KateThemeConfigPage::reset()
{
    // reload themes DB & clear all attributes
    KateHlManager::self()->reload();
    for (int i = 0; i < KateHlManager::self()->modeList().size(); ++i) {
        KateHlManager::self()->getHl(i)->clearAttributeArrays();
    }

    // reload the view
    reload();
}

void KateThemeConfigPage::defaults()
{
    reset();
}

void KateThemeConfigPage::deleteSchema()
{
    const int comboIndex = schemaCombo->currentIndex();
    const QString schemaNameToDelete = schemaCombo->itemData(comboIndex).toString();

    // KSyntaxHighlighting themes can not be deleted, skip invalid themes, too
    const auto theme = KateHlManager::self()->repository().theme(schemaNameToDelete);
    if (!theme.isValid() || theme.isReadOnly()) {
        return;
    }

    // ask the user again, this can't be undone
    if (KMessageBox::warningContinueCancel(this,
                                           i18n("Do you really want to delete the theme \"%1\"? This can not be undone.", schemaNameToDelete),
                                           i18n("Possible Data Loss"),
                                           KGuiItem(i18n("Delete Nevertheless")),
                                           KStandardGuiItem::cancel())
        != KMessageBox::Continue) {
        return;
    }

    // purge the theme file
    QFile::remove(theme.filePath());

    // reset syntax manager repo to flush deleted theme
    KateHlManager::self()->reload();

    // fallback to Default schema + auto
    schemaCombo->setCurrentIndex(schemaCombo->findData(
        QVariant(KTextEditor::EditorPrivate::self()->hlManager()->repository().defaultTheme(KSyntaxHighlighting::Repository::LightTheme).name())));
    if (defaultSchemaCombo->currentIndex() == defaultSchemaCombo->findData(schemaNameToDelete)) {
        defaultSchemaCombo->setCurrentIndex(0);
    }

    // remove schema from combo box
    schemaCombo->removeItem(comboIndex);
    defaultSchemaCombo->removeItem(comboIndex);

    // Reload the color tab, since it uses cached schemas
    m_colorTab->reload();
}

bool KateThemeConfigPage::copyTheme()
{
    // get current theme data as template
    const QString currentThemeName = schemaCombo->itemData(schemaCombo->currentIndex()).toString();
    const auto currentTheme = KateHlManager::self()->repository().theme(currentThemeName);

    // location to write theme files to
    const QString themesPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/org.kde.syntax-highlighting/themes");

    // get sane name
    QString schemaName;
    QString themeFileName;
    while (schemaName.isEmpty()) {
        QInputDialog newNameDialog(this);
        newNameDialog.setInputMode(QInputDialog::TextInput);
        newNameDialog.setWindowTitle(i18n("Copy theme"));
        newNameDialog.setLabelText(i18n("Name for copy of color theme \"%1\":", currentThemeName));
        newNameDialog.setTextValue(currentThemeName);
        if (newNameDialog.exec() == QDialog::Rejected) {
            return false;
        }
        schemaName = newNameDialog.textValue();

        // try if schema already around => if yes, retry name input
        // we try for duplicated file names, too
        themeFileName = themesPath + QStringLiteral("/") + schemaName + QStringLiteral(".theme");
        if (KateHlManager::self()->repository().theme(schemaName).isValid() || QFile::exists(themeFileName)) {
            KMessageBox::information(this,
                                     i18n("<p>The theme \"%1\" already exists.</p><p>Please choose a different theme name.</p>", schemaName),
                                     i18n("Copy Theme"));
            schemaName.clear();
        }
    }

    // get json for current theme
    QJsonObject newThemeObject = jsonForTheme(currentTheme);
    QJsonObject metaData;
    metaData[QLatin1String("revision")] = 1;
    metaData[QLatin1String("name")] = schemaName;
    newThemeObject[QLatin1String("metadata")] = metaData;

    // write to new theme file, we might need to create the local dir first
    QDir().mkpath(themesPath);
    if (!writeJson(newThemeObject, themeFileName)) {
        return false;
    }

    // reset syntax manager repo to find new theme
    KateHlManager::self()->reload();

    // append items to combo boxes
    schemaCombo->addItem(schemaName, QVariant(schemaName));
    defaultSchemaCombo->addItem(schemaName, QVariant(schemaName));

    // finally, activate new schema (last item in the list)
    schemaCombo->setCurrentIndex(schemaCombo->count() - 1);
    return true;
}

void KateThemeConfigPage::schemaChanged(const QString &schema)
{
    // we can't delete read-only themes, e.g. the stuff shipped inside Qt resources or system wide installed
    const auto theme = KateHlManager::self()->repository().theme(schema);
    btndel->setEnabled(!theme.isReadOnly());
    m_readOnlyThemeLabel->setVisible(theme.isReadOnly());

    // propagate changed schema to all tabs
    m_colorTab->schemaChanged(schema);
    m_defaultStylesTab->schemaChanged(schema);
    m_highlightTab->schemaChanged(schema);

    // save current schema index
    m_currentSchema = schema;
}

void KateThemeConfigPage::comboBoxIndexChanged(int currentIndex)
{
    schemaChanged(schemaCombo->itemData(currentIndex).toString());
}

QString KateThemeConfigPage::name() const
{
    return i18n("Color Themes");
}

QString KateThemeConfigPage::fullName() const
{
    return i18n("Color Themes");
}

QIcon KateThemeConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("preferences-desktop-color"));
}

// END KateThemeConfigPage

#include "moc_katethemeconfig.cpp"
