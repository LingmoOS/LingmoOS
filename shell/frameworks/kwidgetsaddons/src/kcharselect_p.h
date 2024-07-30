/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCHARSELECT_P_H
#define KCHARSELECT_P_H

#include <QAbstractTableModel>
#include <QFont>
#include <QMimeData>
#include <QTableView>
#include <memory>

#include "kcharselectdata_p.h"

class KCharSelectTablePrivate;

/**
 * @short Character selection table
 *
 * A table widget which displays the characters of a font. Internally
 * used by KCharSelect. See the KCharSelect documentation for further
 * details.
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 * @author Daniel Laidig <d.laidig@gmx.de>
 */

class KCharSelectTable : public QTableView
{
    Q_OBJECT

public:
    /**
     * Constructor. Using @p _font, draw a table of chars.
     * @sa setContents
     */
    KCharSelectTable(QWidget *parent, const QFont &_font);

    ~KCharSelectTable() override;

    void resizeEvent(QResizeEvent *) override;

    /** Set the font to be displayed to @p _font . */
    void setFont(const QFont &_font);

    /** Set the highlighted character to @p c . */
    void setChar(uint c);
    /** Set the contents of the table to @p chars . */
    void setContents(const QList<uint> &chars);

    /** @return Currently highlighted character. */
    uint chr();

    /**
     * Returns the currently displayed font.
     */
    QFont font() const;

    /**
     * Returns a list of currently displayed characters.
     */
    QList<uint> displayedChars() const;

    /**
     * Reimplemented.
     */
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

protected:
    /**
     * Reimplemented.
     */
    void keyPressEvent(QKeyEvent *e) override;

Q_SIGNALS:
    /** Emitted to indicate that character @p c is activated (such as by double-clicking it). */
    void activated(uint c);
    void focusItemChanged(uint c);
    void showCharRequested(uint c);

private:
    friend class KCharSelectTablePrivate;
    std::unique_ptr<KCharSelectTablePrivate> const d;

    Q_DISABLE_COPY(KCharSelectTable)
};

// NO D-Pointer needed, private internal class, no public API

class KCharSelectItemModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    KCharSelectItemModel(const QList<uint> &chars, const QFont &font, QObject *parent)
        : QAbstractTableModel(parent)
        , m_chars(chars)
        , m_font(font)
    {
        if (!chars.isEmpty()) {
            m_columns = chars.count();
        } else {
            m_columns = 1;
        }
    }

    enum internalRoles { CharacterRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        if (m_chars.count() % m_columns == 0) {
            return m_chars.count() / m_columns;
        } else {
            return m_chars.count() / m_columns + 1;
        }
    }
    int columnCount(const QModelIndex & = QModelIndex()) const override
    {
        return m_columns;
    }

    void setFont(const QFont &font)
    {
        beginResetModel();
        m_font = font;
        endResetModel();
    }
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        int pos = m_columns * (index.row()) + index.column();
        if (pos >= m_chars.size() || index.row() < 0 || index.column() < 0) {
            return Qt::ItemIsDropEnabled;
        }
        return (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override
    {
        if (indexes.size() != 1) {
            return nullptr;
        }
        QMimeData *mimeData = new QMimeData();
        char32_t character = data(indexes[0], CharacterRole).toUInt();
        mimeData->setText(QString::fromUcs4(&character, 1));
        return mimeData;
    }
    Qt::DropActions supportedDropActions() const override
    {
        return Qt::CopyAction;
    }
    QStringList mimeTypes() const override
    {
        return {QStringLiteral("text/plain")};
    }
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    void setColumnCount(int columns);

    QList<uint> chars() const
    {
        return m_chars;
    }

private:
    QList<uint> m_chars;
    QFont m_font;
    int m_columns;

Q_SIGNALS:
    void showCharRequested(uint c);
};
#endif // KCHARSELECT_P_H
