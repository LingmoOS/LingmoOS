/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Scott Wheeler <wheeler@kde.org>
    SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTREEWIDGETSEARCHLINEWIDGET_H
#define KTREEWIDGETSEARCHLINEWIDGET_H

#include <QWidget>
#include <kitemviews_export.h>
#include <memory>

class QModelIndex;
class QTreeWidget;
class KTreeWidgetSearchLine;

/**
 * @class KTreeWidgetSearchLineWidget ktreewidgetsearchlinewidget.h KTreeWidgetSearchLineWidget
 *
 * Creates a widget featuring a KTreeWidgetSearchLine, a label with the text
 * "Search" and a button to clear the search.
 */
class KITEMVIEWS_EXPORT KTreeWidgetSearchLineWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Creates a KTreeWidgetSearchLineWidget for \a treeWidget with \a parent as the
     * parent.
     */
    explicit KTreeWidgetSearchLineWidget(QWidget *parent = nullptr, QTreeWidget *treeWidget = nullptr);

    /**
     * Destroys the KTreeWidgetSearchLineWidget
     */
    ~KTreeWidgetSearchLineWidget() override;

    /**
     * Returns a pointer to the search line.
     */
    KTreeWidgetSearchLine *searchLine() const;

protected Q_SLOTS:
    /**
     * Creates the widgets inside of the widget.  This is called from the
     * constructor via a single shot timer so that it it guaranteed to run
     * after construction is complete.  This makes it suitable for overriding in
     * subclasses.
     */
    virtual void createWidgets();

protected:
    /**
     * Creates the search line.  This can be useful to reimplement in cases where
     * a KTreeWidgetSearchLine subclass is used.
     *
     * It is const because it is be called from searchLine(), which to the user
     * doesn't conceptually alter the widget.
     */
    virtual KTreeWidgetSearchLine *createSearchLine(QTreeWidget *treeWidget) const;

private:
    std::unique_ptr<class KTreeWidgetSearchLineWidgetPrivate> const d;
};

#endif
