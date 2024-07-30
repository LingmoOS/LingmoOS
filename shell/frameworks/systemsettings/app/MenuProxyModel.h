/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 * SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MENUPROXYMODEL_H
#define MENUPROXYMODEL_H

#include <KCategorizedSortFilterProxyModel>

/**
 * @brief Provides a filter model for MenuModel
 *
 * Provides a standardised model to be used with views to filter a MenuModel.\n
 * It automatically sorts the items appropriately depending on if it is categorised
 * or not.
 * Call setFilterRegExp(QString) with the desired text to filter to perform searching.
 * Items that do not match the search parameters will be disabled, not hidden.
 *
 * @author Will Stephenson <wstephenson@kde.org>
 * @author Ben Cooksley <bcooksley@kde.org>
 */
class MenuProxyModel : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QString filterRegExp READ filterRegularExpression WRITE setFilterRegularExpression NOTIFY filterRegularExpressionChanged)

public:
    /**
     * Constructs a MenuProxyModel with the specified parent.
     *
     * @param parent The QObject to use as a parent.
     */
    MenuProxyModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    /**
     * Please see the Qt QSortFilterProxyModel documentation for further information.\n
     * Provides information on whether or not the QModelIndex specified by left is below right.
     *
     * @param left the QModelIndex that is being used for comparing.
     * @param right the QModelIndex to compare against.
     * @returns true if the left is below the right.
     */
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    /**
     * Please see the KDE KCategorizedSortFilterProxyModel documentation for further information.\n
     * Provides information on whether or not the QModelIndex specified by left is below right.
     *
     * @param left the QModelIndex that is being used for comparing.
     * @param right the QModelIndex to compare against.
     * @returns true if the left is below the right.
     */
    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override;

    /**
     * Please see the Qt QSortFilterProxyModel documentation for further information.\n
     * Provides additional filtering of the MenuModel to only show categories which contain modules.
     *
     * @param source_column Please see QSortFilterProxyModel documentation.
     * @param source_parent Please see QSortFilterProxyModel documentation.
     * @returns true if the row should be displayed, false if it should not.
     */
    bool filterAcceptsRow(int source_column, const QModelIndex &source_parent) const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Provides the status flags for the QModelIndex specified.
     * The item will be selectable and enabled for its status unless invalid or filtered by search terms.
     *
     * @returns The flags for the QModelIndex provided.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Reimplemented for internal reasons.
     */
    void setFilterRegularExpression(const QRegularExpression &regExp);

    /**
     * Please see Qt QAbstractItemModel documentation for more details.\n
     * Reimplemented for internal reasons.
     */
    void setFilterRegularExpression(const QString &pattern);

    QString filterRegularExpression() const;

    /**
     * makes the filter highlight matching entries instead of hiding them
     */
    void setFilterHighlightsEntries(bool highlight);

    /**
     * @returns the filter highlight matching entries instead of hiding them, default true
     */
    bool filterHighlightsEntries() const;

Q_SIGNALS:
    void filterRegularExpressionChanged();

private:
    bool m_filterHighlightsEntries : 1;
};

#endif
