/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEWIDGET_H
#define KPAGEWIDGET_H

#include <kpagewidgetmodel.h>

#include "kpageview.h"

class KPageWidgetPrivate;
/**
 * @class KPageWidget kpagewidget.h KPageWidget
 *
 * @short Page widget with many layouts (faces).
 * A KPageView with hierarchical page model.
 *
 * @author Tobias Koenig (tokoe@kde.org)
 */
class KWIDGETSADDONS_EXPORT KPageWidget : public KPageView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KPageWidget)

public:
    /**
     * Creates a new page widget.
     *
     * @param parent The parent widget.
     */
    explicit KPageWidget(QWidget *parent = nullptr);

    /**
     * Destroys the page widget.
     */
    ~KPageWidget() override;

    /**
     * Adds a new top level page to the widget.
     *
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated KPageWidgetItem.
     */
    KPageWidgetItem *addPage(QWidget *widget, const QString &name);

    /**
     * Adds a new top level page to the widget.
     *
     * @param item The KPageWidgetItem which describes the page.
     */
    void addPage(KPageWidgetItem *item);

    /**
     * Inserts a new page in the widget.
     *
     * @param before The new page will be insert before this KPageWidgetItem
     *               on the same level in hierarchy.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated KPageWidgetItem.
     */
    KPageWidgetItem *insertPage(KPageWidgetItem *before, QWidget *widget, const QString &name);

    /**
     * Inserts a new page in the widget.
     *
     * @param before The new page will be insert before this KPageWidgetItem
     *               on the same level in hierarchy.
     *
     * @param item The KPageWidgetItem which describes the page.
     */
    void insertPage(KPageWidgetItem *before, KPageWidgetItem *item);

    /**
     * Inserts a new sub page in the widget.
     *
     * @param parent The new page will be insert as child of this KPageWidgetItem.
     * @param widget The widget of the page.
     * @param name The name which is displayed in the navigation view.
     *
     * @returns The associated KPageWidgetItem.
     */
    KPageWidgetItem *addSubPage(KPageWidgetItem *parent, QWidget *widget, const QString &name);

    /**
     * Inserts a new sub page in the widget.
     *
     * @param parent The new page will be insert as child of this KPageWidgetItem.
     *
     * @param item The KPageWidgetItem which describes the page.
     */
    void addSubPage(KPageWidgetItem *parent, KPageWidgetItem *item);

    /**
     * Removes the page associated with the given KPageWidgetItem.
     */
    void removePage(KPageWidgetItem *item);

    /**
     * Sets the page which is associated with the given KPageWidgetItem to
     * be the current page and emits the currentPageChanged() signal.
     */
    void setCurrentPage(KPageWidgetItem *item);

    /**
     * Returns the KPageWidgetItem for the current page or a null pointer if there is no
     * current page.
     */
    KPageWidgetItem *currentPage() const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the current page has changed.
     *
     * @param current The new current page or a null pointer if no current page is available.
     * @param before The page that was current before the new current page has changed.
     */
    void currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *before);

    /**
     * This signal is emitted whenever a checkable page changes its state. @param checked is true
     * when the @p page is checked, or false if the @p page is unchecked.
     */
    void pageToggled(KPageWidgetItem *page, bool checked);

    /**
     * This signal is emitted when a page is removed.
     * @param page The page which is removed
     * */
    void pageRemoved(KPageWidgetItem *page);

protected:
    KWIDGETSADDONS_NO_EXPORT KPageWidget(KPageWidgetPrivate &dd, QWidget *parent);
};

#endif
