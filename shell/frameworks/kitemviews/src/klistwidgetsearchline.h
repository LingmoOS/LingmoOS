/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Scott Wheeler <wheeler@kde.org>
    SPDX-FileCopyrightText: 2004 Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLISTWIDGETSEARCHLINE_H
#define KLISTWIDGETSEARCHLINE_H

#include <QLineEdit>
#include <memory>

#include <kitemviews_export.h>

class QListWidget;
class QListWidgetItem;
class QModelIndex;

/**
 * @class KListWidgetSearchLine klistwidgetsearchline.h KListWidgetSearchLine
 *
 * This class makes it easy to add a search line for filtering the items in a
 * listwidget based on a simple text search.
 *
 * No changes to the application other than instantiating this class with an
 * appropriate QListWidget should be needed.
 */
class KITEMVIEWS_EXPORT KListWidgetSearchLine : public QLineEdit
{
    Q_OBJECT

public:
    /**
     * Constructs a KListWidgetSearchLine with \a listWidget being the QListWidget to
     * be filtered.
     *
     * If \a listWidget is null then the widget will be disabled until a listWidget
     * is set with setListWidget().
     */
    explicit KListWidgetSearchLine(QWidget *parent = nullptr, QListWidget *listWidget = nullptr);

    /**
     * Destroys the KListWidgetSearchLine.
     */
    ~KListWidgetSearchLine() override;

    /**
     * Returns if the search is case sensitive.  This defaults to Qt::CaseInsensitive.
     *
     * @see setCaseSensitive()
     */
    Qt::CaseSensitivity caseSensitive() const;

    /**
     * Returns the listWidget that is currently filtered by the search.
     *
     * @see setListWidget()
     */
    QListWidget *listWidget() const;

public Q_SLOTS:
    /**
     * Updates search to only make visible the items that match \a s.  If
     * \a s is null then the line edit's text will be used.
     */
    virtual void updateSearch(const QString &s = QString());

    /**
     * Make the search case sensitive or case insensitive.
     *
     * @see caseSenstive()
     */
    void setCaseSensitivity(Qt::CaseSensitivity cs);

    /**
     * Sets the QListWidget that is filtered by this search line.  If \a lv is null
     * then the widget will be disabled.
     *
     * @see listWidget()
     */
    void setListWidget(QListWidget *lv);

    /**
     * Clear line edit and empty hiddenItems, returning elements to listWidget.
     */
    void clear();

protected:
    /**
     * Returns true if \a item matches the search \a s.  This will be evaluated
     * based on the value of caseSensitive().  This can be overridden in
     * subclasses to implement more complicated matching schemes.
     */
    virtual bool itemMatches(const QListWidgetItem *item, const QString &s) const;
    /**
     * Re-implemented for internal reasons.  API not affected.
     */
    bool event(QEvent *event) override;

private:
    friend class KListWidgetSearchLinePrivate;
    std::unique_ptr<class KListWidgetSearchLinePrivate> const d;

    Q_PRIVATE_SLOT(d, void _k_listWidgetDeleted())
    Q_PRIVATE_SLOT(d, void _k_queueSearch(const QString &))
    Q_PRIVATE_SLOT(d, void _k_activateSearch())
    Q_PRIVATE_SLOT(d, void _k_rowsInserted(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d, void _k_dataChanged(const QModelIndex &, const QModelIndex &))
};

#endif /* KLISTWIDGETSEARCHLINE_H */
