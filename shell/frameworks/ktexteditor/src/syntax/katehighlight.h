/*
    SPDX-FileCopyrightText: 2001, 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_HIGHLIGHT_H
#define KATE_HIGHLIGHT_H

#include <KSyntaxHighlighting/AbstractHighlighter>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/FoldingRegion>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Theme>

#include <KTextEditor/Attribute>
#include <KTextEditor/Range>

#include "spellcheck/prefixstore.h"

#include <QHash>
#include <QList>

#include <QRegularExpression>
#include <QStringList>

#include <unordered_map>
#include <vector>

namespace KTextEditor
{
class DocumentPrivate;
}
namespace Kate
{
class TextLine;
}

class KateHighlighting : private KSyntaxHighlighting::AbstractHighlighter
{
public:
    explicit KateHighlighting(const KSyntaxHighlighting::Definition &def);

    /**
     * Folding storage
     */
    class Folding
    {
    public:
        /**
         * Construct folding.
         * @param _offset offset of the folding start
         * @param _length length of the folding
         * @param _foldingRegion folding region, includes if we start or end a region and the id
         */
        Folding(int _offset, int _length, KSyntaxHighlighting::FoldingRegion _foldingRegion)
            : offset(_offset)
            , length(_length)
            , foldingRegion(_foldingRegion)
        {
        }

        /**
         * offset
         */
        int offset = 0;

        /**
         * length
         */
        int length = 0;

        /**
         * folding region, includes if we start or end a region and the id
         */
        KSyntaxHighlighting::FoldingRegion foldingRegion;
    };

    /**
     * Folding vector used to pass out folding knowledge
     */
    typedef std::vector<Folding> Foldings;

protected:
    /**
     * Reimplement this to apply formats to your output. The provided @p format
     * is valid for the interval [@p offset, @p offset + @p length).
     *
     * @param offset The start column of the interval for which @p format matches
     * @param length The length of the matching text
     * @param format The Format that applies to the range [offset, offset + length)
     *
     * @note Make sure to set a valid Definition, otherwise the parameter
     *       @p format is invalid for the entire line passed to highlightLine()
     *       (cf. Format::isValid()).
     *
     * @see applyFolding(), highlightLine()
     */
    void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

    /**
     * Reimplement this to apply folding to your output. The provided
     * FoldingRegion @p region either stars or ends a code folding region in the
     * interval [@p offset, @p offset + @p length).
     *
     * @param offset The start column of the FoldingRegion
     * @param length The length of the matching text that starts / ends a
     *       folding region
     * @param region The FoldingRegion that applies to the range [offset, offset + length)
     *
     * @note The FoldingRegion @p region is @e always either of type
     *       FoldingRegion::Type::Begin or FoldingRegion::Type::End.
     *
     * @see applyFormat(), highlightLine(), FoldingRegion
     */
    void applyFolding(int offset, int length, KSyntaxHighlighting::FoldingRegion region) override;

public:
    /**
     * Parse the text and fill in the context array and folding list array
     *
     * @param prevLine The previous line, the context array is picked up from that if present.
     * @param textLine The text line to parse
     * @param ctxChanged will be set to reflect if the context changed
     * @param foldings foldings vector to fill, if that is wanted
     */
    void doHighlight(const Kate::TextLine *prevLine, Kate::TextLine *textLine, bool &ctxChanged, Foldings *foldings = nullptr);

    const QString &name() const
    {
        return iName;
    }
    const QString &section() const
    {
        return iSection;
    }
    bool hidden() const
    {
        return iHidden;
    }
    const QString &style() const
    {
        return iStyle;
    }
    const QString &getIdentifier() const
    {
        return identifier;
    }

    /**
     * @return true if the character @p c is not a deliminator character
     *     for the corresponding highlight.
     */
    bool isInWord(QChar c, int attrib = 0) const;

    /**
     * @return true if the character @p c is a wordwrap deliminator as specified
     * in the general keyword section of the xml file.
     */
    bool canBreakAt(QChar c, int attrib = 0) const;
    /**
     *
     */
    const QList<QRegularExpression> &emptyLines(int attribute = 0) const;

    bool isEmptyLine(const Kate::TextLine *textline) const;

    /**
     * @return true if @p beginAttr and @p endAttr are members of the same
     * highlight, and there are comment markers of either type in that.
     */
    bool canComment(int startAttr, int endAttr) const;

    /**
     * @return the mulitiline comment start marker for the highlight
     * corresponding to @p attrib.
     */
    QString getCommentStart(int attrib = 0) const;

    /**
     * @return the muiltiline comment end marker for the highlight corresponding
     * to @p attrib.
     */
    QString getCommentEnd(int attrib = 0) const;

    /**
     * @return the single comment marker for the highlight corresponding
     * to @p attrib.
     */
    QString getCommentSingleLineStart(int attrib = 0) const;

    const QHash<QString, QChar> &characterEncodings(int attrib = 0) const;

    /**
     * @return the single comment marker position for the highlight corresponding
     * to @p attrib.
     */
    KSyntaxHighlighting::CommentPosition getCommentSingleLinePosition(int attrib = 0) const;

    bool attributeRequiresSpellchecking(int attr);

    /**
     * map attribute to its name
     * @return name of the attribute
     */
    QString nameForAttrib(int attrib) const;

    /**
     * Get attribute for the given cursor position.
     * @param doc document to use
     * @param cursor cursor position in the given document
     * @return attribute valid at that location, default is 0
     */
    int attributeForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor);

    /**
     * Get all keywords valid for the given cursor position.
     * @param doc document to use
     * @param cursor cursor position in the given document
     * @return all keywords valid at that location
     */
    QStringList keywordsForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor);

    /**
     * Is spellchecking required for the tiven cursor position?
     * @param doc document to use
     * @param cursor cursor position in the given document
     * @return spell checking required?
     */
    bool spellCheckingRequiredForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor);

    /**
     * Get highlighting mode for the given cursor position.
     * @param doc document to use
     * @param cursor cursor position in the given document
     * @return mode valid at that location
     */
    QString higlightingModeForLocation(KTextEditor::DocumentPrivate *doc, const KTextEditor::Cursor cursor);

    KSyntaxHighlighting::Theme::TextStyle defaultStyleForAttribute(int attr) const;

    void clearAttributeArrays();

    QList<KTextEditor::Attribute::Ptr> attributes(const QString &schema);

    inline bool noHighlighting() const
    {
        return noHl;
    }

    /**
     * Indentation mode, e.g. c-style, ....
     * @return indentation mode
     */
    const QString &indentation() const
    {
        return m_indentation;
    }

    const QHash<QString, QChar> &getCharacterEncodings(int attrib) const;
    const KatePrefixStore &getCharacterEncodingsPrefixStore(int attrib) const;
    const QHash<QChar, QString> &getReverseCharacterEncodings(int attrib) const;

    /**
     * Returns a list of names of embedded modes.
     */
    QStringList getEmbeddedHighlightingModes() const;

    /**
     * create list of attributes from internal formats with properties as defined in syntax file
     * @param schema The id of the chosen schema
     * @return attributes list with attributes as defined in syntax file
     */
    QList<KTextEditor::Attribute::Ptr> attributesForDefinition(const QString &schema) const;

    /**
     * Retrieve all formats for this highlighting.
     * @return all formats for the highlighting definition of this highlighting includes included formats
     */
    const std::vector<KSyntaxHighlighting::Format> &formats() const
    {
        return m_formats;
    }

private:
    int sanitizeFormatIndex(int attrib) const;

private:
    QStringList embeddedHighlightingModes;

    bool noHl = true;
    bool folding = false;

    QString iName;
    QString iSection;
    bool iHidden = false;
    QString identifier;
    QString iStyle;

    /**
     * Indentation mode, e.g. c-style, ....
     */
    QString m_indentation;

    bool m_foldingIndentationSensitive = false;

    // map schema name to attributes...
    QHash<QString, QList<KTextEditor::Attribute::Ptr>> m_attributeArrays;

    /**
     * This class holds the additional properties for one highlight
     * definition, such as comment strings, deliminators etc.
     */
    class HighlightPropertyBag
    {
    public:
        KSyntaxHighlighting::Definition definition;
        QString singleLineCommentMarker;
        QString multiLineCommentStart;
        QString multiLineCommentEnd;
        KSyntaxHighlighting::CommentPosition singleLineCommentPosition;
        QList<QRegularExpression> emptyLines;
        QHash<QString, QChar> characterEncodings;
        KatePrefixStore characterEncodingsPrefixStore;
        QHash<QChar, QString> reverseCharacterEncodings;
    };

public:
    inline bool foldingIndentationSensitive()
    {
        return m_foldingIndentationSensitive;
    }
    inline bool allowsFolding()
    {
        return folding;
    }

    /**
     * Highlight properties for this definition and each included highlight definition.
     */
    std::vector<HighlightPropertyBag> m_properties;

    /**
     * all formats for the highlighting definition of this highlighting
     * includes included formats
     */
    std::vector<KSyntaxHighlighting::Format> m_formats;

    /**
     * for each format, pointer to the matching HighlightPropertyBag in m_properties
     */
    std::vector<const HighlightPropertyBag *> m_propertiesForFormat;

    /**
     * mapping of format id => index into m_formats
     */
    std::unordered_map<int, int> m_formatsIdToIndex;

    /**
     * textline to do updates on during doHighlight
     */
    Kate::TextLine *m_textLineToHighlight = nullptr;

    /**
     * foldings vector to do updates on during doHighlight
     * might not be set, then we ignor that
     */
    Foldings *m_foldings = nullptr;

    /**
     * check if the folding begin/ends are balanced!
     * updated during doHighlight
     */
    QHash<int, int> m_foldingStartToCount;
};

#endif
