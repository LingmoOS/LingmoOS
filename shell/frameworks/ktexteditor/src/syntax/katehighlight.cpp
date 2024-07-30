/*
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003, 2004 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2003 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN INCLUDES
#include "katehighlight.h"

#include "katedocument.h"
#include "kateextendedattribute.h"
#include "katesyntaxmanager.h"
// END

// BEGIN KateHighlighting
KateHighlighting::KateHighlighting(const KSyntaxHighlighting::Definition &def)
{
    // get name and section, always works
    iName = def.name();
    iSection = def.translatedSection();

    // get all included definitions, e.g. PHP for HTML highlighting
    auto definitions = def.includedDefinitions();

    // handle the "no highlighting" case
    // it's possible to not have any definitions with malformed file
    if (!def.isValid() || (definitions.isEmpty() && def.formats().isEmpty())) {
        // dummy properties + formats
        m_properties.resize(1);
        m_propertiesForFormat.push_back(&m_properties[0]);
        m_formats.resize(1);
        m_formatsIdToIndex.insert(std::make_pair(m_formats[0].id(), 0));

        // be done, all below is just for the real highlighting variants
        return;
    }

    // handle the real highlighting case
    noHl = false;
    iHidden = def.isHidden();
    identifier = def.filePath();
    iStyle = def.style();
    m_indentation = def.indenter();
    folding = def.foldingEnabled();
    m_foldingIndentationSensitive = def.indentationBasedFoldingEnabled();

    // tell the AbstractHighlighter the definition it shall use
    setDefinition(def);

    embeddedHighlightingModes.reserve(definitions.size());
    // first: handle only really included definitions
    for (const auto &includedDefinition : std::as_const(definitions)) {
        embeddedHighlightingModes.push_back(includedDefinition.name());
    }

    // now: handle all, including this definition itself
    // create the format => attributes mapping
    // collect embedded highlightings, too
    //
    // we start with our definition as we want to have the default format
    // of the initial definition as attribute with index == 0
    //
    // we collect additional properties in the m_properties and
    // map the formats to the right properties in m_propertiesForFormat
    definitions.push_front(definition());
    m_properties.resize(definitions.size());
    size_t propertiesIndex = 0;
    for (const auto &includedDefinition : std::as_const(definitions)) {
        auto &properties = m_properties[propertiesIndex];
        properties.definition = includedDefinition;
        properties.emptyLines.reserve(includedDefinition.foldingIgnoreList().size());
        const auto foldingIgnoreList = includedDefinition.foldingIgnoreList();
        for (const auto &emptyLine : foldingIgnoreList) {
            properties.emptyLines.push_back(QRegularExpression(emptyLine, QRegularExpression::UseUnicodePropertiesOption));
        }
        properties.singleLineCommentMarker = includedDefinition.singleLineCommentMarker();
        properties.singleLineCommentPosition = includedDefinition.singleLineCommentPosition();
        const auto multiLineComment = includedDefinition.multiLineCommentMarker();
        properties.multiLineCommentStart = multiLineComment.first;
        properties.multiLineCommentEnd = multiLineComment.second;

        // collect character characters
        const auto encodings = includedDefinition.characterEncodings();
        for (const auto &enc : encodings) {
            properties.characterEncodingsPrefixStore.addPrefix(enc.second);
            properties.characterEncodings[enc.second] = enc.first;
            properties.reverseCharacterEncodings[enc.first] = enc.second;
        }

        // collect formats
        const auto formats = includedDefinition.formats();
        for (const auto &format : formats) {
            // register format id => internal attributes, we want no clashs
            const auto nextId = m_formats.size();
            m_formatsIdToIndex.insert(std::make_pair(format.id(), int(nextId)));
            m_formats.push_back(format);
            m_propertiesForFormat.push_back(&properties);
        }

        // advance to next properties
        ++propertiesIndex;
    }
}

void KateHighlighting::doHighlight(const Kate::TextLine *prevLine, Kate::TextLine *textLine, bool &ctxChanged, Foldings *foldings)
{
    // default: no context change
    ctxChanged = false;

    // no text line => nothing to do
    if (!textLine) {
        return;
    }

    // in all cases, remove old hl, or we will grow to infinite ;)
    textLine->clearAttributes();

    // reset folding start
    textLine->clearMarkedAsFoldingStartAndEnd();
    if (foldings) {
        foldings->clear();
    }

    // no hl set, nothing to do more than the above cleaning ;)
    if (noHl) {
        return;
    }

    // ensure we arrive in clean state
    Q_ASSERT(!m_textLineToHighlight);
    Q_ASSERT(!m_foldings);
    Q_ASSERT(m_foldingStartToCount.isEmpty());

    // highlight the given line via the abstract highlighter
    // a bit ugly: we set the line to highlight as member to be able to update its stats in the applyFormat and applyFolding member functions
    m_textLineToHighlight = textLine;
    m_foldings = foldings;
    const KSyntaxHighlighting::State initialState(!prevLine ? KSyntaxHighlighting::State() : prevLine->highlightingState());
    const KSyntaxHighlighting::State endOfLineState = highlightLine(textLine->text(), initialState);
    m_textLineToHighlight = nullptr;
    m_foldings = nullptr;

    // update highlighting state if needed
    if (textLine->highlightingState() != endOfLineState) {
        textLine->setHighlightingState(endOfLineState);
        ctxChanged = true;
    }

    // handle folding info computed and cleanup hash again, if there
    // check if folding is not balanced and we have more starts then ends
    // then this line is a possible folding start!
    if (!m_foldingStartToCount.isEmpty()) {
        // possible folding start, if imbalanced, aka hash not empty!
        textLine->markAsFoldingStartAttribute();

        // clear hash for next doHighlight
        m_foldingStartToCount.clear();
    }
}

void KateHighlighting::applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format)
{
    Q_ASSERT(m_textLineToHighlight);
    if (!format.isValid()) {
        return;
    }

    // get internal attribute, must exist
    const auto it = m_formatsIdToIndex.find(format.id());
    Q_ASSERT(it != m_formatsIdToIndex.end());

    // WE ATM assume ascending offset order
    // remember highlighting info in our textline
    m_textLineToHighlight->addAttribute(Kate::TextLine::Attribute(offset, length, it->second));
}

void KateHighlighting::applyFolding(int offset, int length, KSyntaxHighlighting::FoldingRegion region)
{
    Q_ASSERT(m_textLineToHighlight);
    Q_ASSERT(region.isValid());

    // WE ATM assume ascending offset order, we add the length to the offset for the folding ends to have ranges spanning the full folding region
    if (m_foldings) {
        m_foldings->emplace_back(offset + ((region.type() == KSyntaxHighlighting::FoldingRegion::Begin) ? 0 : length), length, region);
    }

    // for each end region, decrement counter for that type, erase if count reaches 0!
    if (region.type() == KSyntaxHighlighting::FoldingRegion::End) {
        QHash<int, int>::iterator end = m_foldingStartToCount.find(region.id());
        if (end != m_foldingStartToCount.end()) {
            if (end.value() > 1) {
                --(end.value());
            } else {
                m_foldingStartToCount.erase(end);
            }
        } else {
            // if we arrive here, we might have some folding end in this line for previous lines
            m_textLineToHighlight->markAsFoldingEndAttribute();
        }
    }

    // increment counter for each begin region!
    else {
        ++m_foldingStartToCount[region.id()];
    }
}

int KateHighlighting::sanitizeFormatIndex(int attrib) const
{
    // sanitize, e.g. one could have old hl info with now invalid attribs
    if (attrib < 0 || size_t(attrib) >= m_formats.size()) {
        return 0;
    }
    return attrib;
}

const QHash<QString, QChar> &KateHighlighting::getCharacterEncodings(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->characterEncodings;
}

const KatePrefixStore &KateHighlighting::getCharacterEncodingsPrefixStore(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->characterEncodingsPrefixStore;
}

const QHash<QChar, QString> &KateHighlighting::getReverseCharacterEncodings(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->reverseCharacterEncodings;
}

bool KateHighlighting::attributeRequiresSpellchecking(int attr)
{
    return m_formats[sanitizeFormatIndex(attr)].spellCheck();
}

KSyntaxHighlighting::Theme::TextStyle KateHighlighting::defaultStyleForAttribute(int attr) const
{
    return m_formats[sanitizeFormatIndex(attr)].textStyle();
}

QString KateHighlighting::nameForAttrib(int attrib) const
{
    const auto &format = m_formats.at(sanitizeFormatIndex(attrib));
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->definition.name() + QLatin1Char(':')
        + QString(format.isValid() ? format.name() : QStringLiteral("Normal"));
}

bool KateHighlighting::isInWord(QChar c, int attrib) const
{
    return !m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->definition.isWordDelimiter(c) && !c.isSpace() && c != QLatin1Char('"')
        && c != QLatin1Char('\'') && c != QLatin1Char('`');
}

bool KateHighlighting::canBreakAt(QChar c, int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->definition.isWordWrapDelimiter(c) && c != QLatin1Char('"') && c != QLatin1Char('\'');
}

const QList<QRegularExpression> &KateHighlighting::emptyLines(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->emptyLines;
}

bool KateHighlighting::canComment(int startAttrib, int endAttrib) const
{
    const auto startProperties = m_propertiesForFormat.at(sanitizeFormatIndex(startAttrib));
    const auto endProperties = m_propertiesForFormat.at(sanitizeFormatIndex(endAttrib));
    return (startProperties == endProperties
            && ((!startProperties->multiLineCommentStart.isEmpty() && !startProperties->multiLineCommentEnd.isEmpty())
                || !startProperties->singleLineCommentMarker.isEmpty()));
}

QString KateHighlighting::getCommentStart(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->multiLineCommentStart;
}

QString KateHighlighting::getCommentEnd(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->multiLineCommentEnd;
}

QString KateHighlighting::getCommentSingleLineStart(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->singleLineCommentMarker;
}

KSyntaxHighlighting::CommentPosition KateHighlighting::getCommentSingleLinePosition(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->singleLineCommentPosition;
}

const QHash<QString, QChar> &KateHighlighting::characterEncodings(int attrib) const
{
    return m_propertiesForFormat.at(sanitizeFormatIndex(attrib))->characterEncodings;
}

void KateHighlighting::clearAttributeArrays()
{
    // just clear the hashed attributes, we create them lazy again
    m_attributeArrays.clear();
}

QList<KTextEditor::Attribute::Ptr> KateHighlighting::attributesForDefinition(const QString &schema) const
{
    // create list of known attributes based on highlighting format & wanted theme
    QList<KTextEditor::Attribute::Ptr> array;
    array.reserve(m_formats.size());
    const auto currentTheme = KateHlManager::self()->repository().theme(schema);
    for (const auto &format : m_formats) {
        // create a KTextEditor attribute matching the given format
        KTextEditor::Attribute::Ptr newAttribute(new KTextEditor::Attribute(nameForAttrib(array.size()), format.textStyle()));

        if (const auto color = format.textColor(currentTheme).rgba()) {
            newAttribute->setForeground(QColor::fromRgba(color));
        }

        if (const auto color = format.selectedTextColor(currentTheme).rgba()) {
            newAttribute->setSelectedForeground(QColor::fromRgba(color));
        }

        if (const auto color = format.backgroundColor(currentTheme).rgba()) {
            newAttribute->setBackground(QColor::fromRgba(color));
        } else {
            newAttribute->clearBackground();
        }

        if (const auto color = format.selectedBackgroundColor(currentTheme).rgba()) {
            newAttribute->setSelectedBackground(QColor::fromRgba(color));
        } else {
            newAttribute->clearProperty(SelectedBackground);
        }

        // Only set attributes if true, otherwise we waste memory
        if (format.isBold(currentTheme)) {
            newAttribute->setFontBold(true);
        }
        if (format.isItalic(currentTheme)) {
            newAttribute->setFontItalic(true);
        }
        if (format.isUnderline(currentTheme)) {
            newAttribute->setFontUnderline(true);
        }
        if (format.isStrikeThrough(currentTheme)) {
            newAttribute->setFontStrikeOut(true);
        }
        if (format.spellCheck()) {
            newAttribute->setSkipSpellChecking(true);
        }
        array.append(newAttribute);
    }
    return array;
}

QList<KTextEditor::Attribute::Ptr> KateHighlighting::attributes(const QString &schema)
{
    // query cache first
    if (m_attributeArrays.contains(schema)) {
        return m_attributeArrays[schema];
    }

    // create new attributes array for wanted theme and cache it
    const auto array = attributesForDefinition(schema);
    m_attributeArrays.insert(schema, array);
    return array;
}

QStringList KateHighlighting::getEmbeddedHighlightingModes() const
{
    return embeddedHighlightingModes;
}

bool KateHighlighting::isEmptyLine(const Kate::TextLine *textline) const
{
    const QString &txt = textline->text();
    if (txt.isEmpty()) {
        return true;
    }

    const auto &l = emptyLines(textline->attribute(0));
    if (l.isEmpty()) {
        return false;
    }

    for (const QRegularExpression &re : l) {
        const QRegularExpressionMatch match = re.match(txt, 0, QRegularExpression::NormalMatch, QRegularExpression::AnchorAtOffsetMatchOption);
        if (match.hasMatch() && match.capturedLength() == txt.length()) {
            return true;
        }
    }

    return false;
}

int KateHighlighting::attributeForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor)
{
    // Validate parameters to prevent out of range access
    if (cursor.line() < 0 || cursor.line() >= doc->lines() || cursor.column() < 0) {
        return 0;
    }

    // get highlighted line
    const auto tl = doc->kateTextLine(cursor.line());

    // either get char attribute or attribute of context still active at end of line
    if (cursor.column() < tl.length()) {
        return sanitizeFormatIndex(tl.attribute(cursor.column()));
    } else if (cursor.column() >= tl.length()) {
        if (!tl.attributesList().empty()) {
            return sanitizeFormatIndex(tl.attributesList().back().attributeValue);
        }
    }
    return 0;
}

QStringList KateHighlighting::keywordsForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor)
{
    // FIXME-SYNTAX: was before more precise, aka context level
    const auto &def = m_propertiesForFormat.at(attributeForLocation(doc, cursor))->definition;
    QStringList keywords;
    keywords.reserve(def.keywordLists().size());
    const auto keyWordLists = def.keywordLists();
    for (const auto &keylist : keyWordLists) {
        keywords += def.keywordList(keylist);
    }
    return keywords;
}

bool KateHighlighting::spellCheckingRequiredForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor)
{
    return m_formats.at(attributeForLocation(doc, cursor)).spellCheck();
}

QString KateHighlighting::higlightingModeForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor)
{
    return m_propertiesForFormat.at(attributeForLocation(doc, cursor))->definition.name();
}

// END
