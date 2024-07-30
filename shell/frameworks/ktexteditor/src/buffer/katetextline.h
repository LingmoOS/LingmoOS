/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXTLINE_H
#define KATE_TEXTLINE_H

#include <KSyntaxHighlighting/State>

#include <QList>
#include <QString>

namespace Kate
{
/**
 * Class representing a single text line.
 * For efficiency reasons, not only pure text is stored here, but also additional data.
 */
class TextLine
{
public:
    /**
     * Attribute storage
     */
    class Attribute
    {
    public:
        /**
         * Attribute constructor
         * @param _offset offset
         * @param _length length
         * @param _attributeValue attribute value
         */
        explicit Attribute(int _offset = 0, int _length = 0, int _attributeValue = 0)
            : offset(_offset)
            , length(_length)
            , attributeValue(_attributeValue)
        {
        }

        /**
         * offset
         */
        int offset;

        /**
         * length
         */
        int length;

        /**
         * attribute value (to encode type of this range)
         */
        int attributeValue;
    };

    /**
     * Flags of TextLine
     */
    enum Flags { flagAutoWrapped = 1, flagFoldingStartAttribute = 2, flagFoldingEndAttribute = 4, flagLineModified = 8, flagLineSavedOnDisk = 16 };

    /**
     * Construct an empty text line.
     */
    TextLine() = default;

    /**
     * Construct an text line with given text.
     * @param text text to use for this line
     */
    explicit TextLine(const QString &text)
        : m_text(text)
        , m_flags(0)
    {
    }

    /**
     * Accessor to the text contained in this line.
     * @return text of this line as constant reference
     */
    const QString &text() const
    {
        return m_text;
    }

    /**
     * Accessor to the text contained in this line.
     * @return text of this line as reference
     */
    QString &text()
    {
        return m_text;
    }

    /**
     * Returns the position of the first non-whitespace character
     * @return position of first non-whitespace char or -1 if there is none
     */
    int firstChar() const;

    /**
     * Returns the position of the last non-whitespace character
     * @return position of last non-whitespace char or -1 if there is none
     */
    int lastChar() const;

    /**
     * Find the position of the next char that is not a space.
     * @param pos Column of the character which is examined first.
     * @return True if the specified or a following character is not a space
     *          Otherwise false.
     */
    int nextNonSpaceChar(int pos) const;

    /**
     * Find the position of the previous char that is not a space.
     * @param pos Column of the character which is examined first.
     * @return The position of the first non-whitespace character preceding pos,
     *   or -1 if none is found.
     */
    int previousNonSpaceChar(int pos) const;

    /**
     * Returns the character at the given \e column. If \e column is out of
     * range, the return value is QChar().
     * @param column column you want char for
     * @return char at given column or QChar()
     */
    inline QChar at(int column) const
    {
        if (column >= 0 && column < m_text.length()) {
            return m_text.at(column);
        }

        return QChar();
    }

    inline void markAsModified(bool modified)
    {
        if (modified) {
            m_flags |= flagLineModified;
            m_flags &= (~flagLineSavedOnDisk);
        } else {
            m_flags &= (~flagLineModified);
        }
    }

    inline bool markedAsModified() const
    {
        return m_flags & flagLineModified;
    }

    inline void markAsSavedOnDisk(bool savedOnDisk)
    {
        if (savedOnDisk) {
            m_flags |= flagLineSavedOnDisk;
            m_flags &= (~flagLineModified);
        } else {
            m_flags &= (~flagLineSavedOnDisk);
        }
    }

    inline bool markedAsSavedOnDisk() const
    {
        return m_flags & flagLineSavedOnDisk;
    }

    /**
     * Clear folding start and end status.
     */
    void clearMarkedAsFoldingStartAndEnd()
    {
        m_flags &= ~flagFoldingStartAttribute;
        m_flags &= ~flagFoldingEndAttribute;
    }

    /**
     * Is on this line a folding start per attribute?
     * @return folding start line per attribute? or not?
     */
    bool markedAsFoldingStartAttribute() const
    {
        return m_flags & flagFoldingStartAttribute;
    }

    /**
     * Mark as folding start line of an attribute based folding.
     */
    void markAsFoldingStartAttribute()
    {
        m_flags |= flagFoldingStartAttribute;
    }

    /**
     * Is on this line a folding end per attribute?
     * @return folding end line per attribute? or not?
     */
    bool markedAsFoldingEndAttribute() const
    {
        return m_flags & flagFoldingEndAttribute;
    }

    /**
     * Mark as folding end line of an attribute based folding.
     */
    void markAsFoldingEndAttribute()
    {
        m_flags |= flagFoldingEndAttribute;
    }

    /**
     * Returns the line's length.
     */
    int length() const
    {
        return m_text.length();
    }

    /**
     * Returns \e true, if the line was automagically wrapped, otherwise returns
     * \e false.
     * @return was this line auto-wrapped?
     */
    bool isAutoWrapped() const
    {
        return m_flags & flagAutoWrapped;
    }

    /**
     * Returns the substring with \e length beginning at the given \e column.
     * @param column start column of text to return
     * @param length length of text to return
     * @return wanted part of text
     */
    QString string(int column, int length) const
    {
        return m_text.mid(column, length);
    }

    /**
     * Leading whitespace of this line
     * @return leading whitespace of this line
     */
    QString leadingWhitespace() const;

    /**
     * Returns the indentation depth with each tab expanded into \e tabWidth characters.
     */
    int indentDepth(int tabWidth) const;

    /**
     * Returns the \e column with each tab expanded into \e tabWidth characters.
     */
    int toVirtualColumn(int column, int tabWidth) const;

    /**
     * Returns the "real" column where each tab only counts one character.
     * The conversion calculates with \e tabWidth characters for each tab.
     */
    int fromVirtualColumn(int column, int tabWidth) const;

    /**
     * Returns the text length with each tab expanded into \e tabWidth characters.
     */
    int virtualLength(int tabWidth) const;

    /**
     * Returns \e true, if \e match equals to the text at position \e column,
     * otherwise returns \e false.
     */
    bool matchesAt(int column, const QString &match) const;

    /**
     * Returns \e true, if the line starts with \e match, otherwise returns \e false.
     */
    bool startsWith(const QString &match) const
    {
        return m_text.startsWith(match);
    }

    /**
     * Returns \e true, if the line ends with \e match, otherwise returns \e false.
     */
    bool endsWith(const QString &match) const
    {
        return m_text.endsWith(match);
    }

    /**
     * context stack
     * @return context stack
     */
    const KSyntaxHighlighting::State &highlightingState() const
    {
        return m_highlightingState;
    }

    /**
     * Sets the syntax highlight context number
     * @param val new context array
     */
    void setHighlightingState(const KSyntaxHighlighting::State &val)
    {
        m_highlightingState = val;
    }

    /**
     * Add attribute to this line.
     * @param attribute new attribute to append
     */
    void addAttribute(const Attribute &attribute);

    /**
     * Clear attributes and foldings of this line
     */
    void clearAttributes()
    {
        m_attributesList.clear();
    }

    /**
     * Accessor to attributes
     * @return attributes of this line
     */
    const QList<Attribute> &attributesList() const
    {
        return m_attributesList;
    }

    /**
     * Gets the attribute at the given position
     * use KRenderer::attributes  to get the KTextAttribute for this.
     *
     * @param pos position of attribute requested
     * @return value of attribute
     */
    int attribute(int pos) const;

    /**
     * set auto-wrapped property
     * @param wrapped line was wrapped?
     */
    void setAutoWrapped(bool wrapped)
    {
        if (wrapped) {
            m_flags = m_flags | flagAutoWrapped;
        } else {
            m_flags = m_flags & ~flagAutoWrapped;
        }
    }

private:
    /**
     * text of this line
     */
    QString m_text;

    /**
     * attributes of this line
     */
    QList<Attribute> m_attributesList;

    /**
     * current highlighting state
     */
    KSyntaxHighlighting::State m_highlightingState;

    /**
     * flags of this line
     */
    unsigned int m_flags = 0;
};
}

#endif
