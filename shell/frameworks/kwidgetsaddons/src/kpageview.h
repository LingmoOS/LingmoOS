/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEVIEW_H
#define KPAGEVIEW_H

#include <kwidgetsaddons_export.h>

#include <QWidget>
#include <memory>

class KPageModel;

class QAbstractItemDelegate;
class QAbstractItemView;
class QModelIndex;
class KPageViewPrivate;
class QAbstractItemModel;

/**
 * @class KPageView kpageview.h KPageView
 *
 * @short A base class which can handle multiple pages.
 *
 * This class provides a widget base class which handles multiple
 * pages and allows the user to switch between these pages in
 * different ways.
 *
 * Currently, @p Auto, @p Plain, @p List, @p Tree, @p FlatList and
 * @p Tabbed face types are available (cmp. KPageWidget).
 *
 * <b>Example:</b>\n
 *
 * \code
 *  KPageModel *model = new MyPageModel();
 *
 *  auto view = new KPageView(this);
 *  view->setModel(model);
 *
 *  view->setFaceType(KPageView::List);
 * \endcode
 *
 * @author Tobias Koenig (tokoe@kde.org)
 */
class KWIDGETSADDONS_EXPORT KPageView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QWidget *pageHeader READ pageHeader WRITE setPageHeader)
    Q_PROPERTY(QWidget *pageFooter READ pageFooter WRITE setPageFooter)
    Q_PROPERTY(FaceType faceType READ faceType WRITE setFaceType)
    Q_DECLARE_PRIVATE(KPageView)

public:
    /**
     * This enum is used to decide which type of navigation view
     * shall be used in the page view.
     */
    enum FaceType {
        /**
         * Depending on the number of pages in the model,
         * the @c Plain (one page), the @c List (several pages)
         * or the @c Tree face (nested pages) will be used.
         * This is the default face type.
         */
        Auto,
        /**
         * No navigation view will be visible and only the
         * first page of the model will be shown.
         */
        Plain,
        /**
         * An icon list is used as navigation view
         */
        List,
        /**
         * A tree list is used as navigation view
         */
        Tree,
        /**
         * A tab widget is used as navigation view
         */
        Tabbed,
        /**
         * A flat list with small icons is used as navigation view
         */
        FlatList,
    };
    Q_ENUM(FaceType)

    /**
     * Creates a page view with given parent.
     */
    explicit KPageView(QWidget *parent = nullptr);

    /**
     * Destroys the page view.
     */
    ~KPageView() override;

    /**
     * Sets the @p model of the page view.
     *
     * The model has to provide data for the roles defined in KPageModel::Role.
     */
    void setModel(QAbstractItemModel *model);

    /**
     * Returns the model of the page view.
     */
    QAbstractItemModel *model() const;

    /**
     * Sets the face type of the page view.
     */
    void setFaceType(FaceType faceType);

    /**
     * Returns the face type of the page view.
     */
    FaceType faceType() const;

    /**
     * Sets the page with @param index to be the current page and emits
     * the signal currentPageChanged.
     */
    void setCurrentPage(const QModelIndex &index);

    /**
     * Returns the index for the current page or an invalid index
     * if no current page exists.
     */
    QModelIndex currentPage() const;

    /**
     * Sets the item @param delegate which can be used customize
     * the page view.
     */
    void setItemDelegate(QAbstractItemDelegate *delegate);

    /**
     * Returns the item delegate of the page view.
     */
    QAbstractItemDelegate *itemDelegate() const;

    /**
     * Sets the @p widget which will be shown when a page is selected
     * that has no own widget set.
     */
    void setDefaultWidget(QWidget *widget);

    /**
     * Set a widget as the header for this Page view
     * It will replace the standard page title
     * @since 5.61
     */
    void setPageHeader(QWidget *header);

    /**
     * Widget of the header for this page view
     * @since 5.61
     */
    QWidget *pageHeader() const;

    /**
     * Set a widget as the footer for this Page view
     * @since 5.61
     */
    void setPageFooter(QWidget *footer);

    /**
     * Widget of the footer for this page view
     * @since 5.61
     */
    QWidget *pageFooter() const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the current page changes.
     * The previous page index is replaced by the current index.
     */
    void currentPageChanged(const QModelIndex &current, const QModelIndex &previous);

protected:
    /**
     * Returns the navigation view, depending on the current
     * face type.
     *
     * This method can be reimplemented to provide custom
     * navigation views.
     */
    virtual QAbstractItemView *createView();

    /**
     * Returns whether the page header should be visible.
     *
     * This method can be reimplemented for adapting custom
     * views.
     */
    virtual bool showPageHeader() const;

    /**
     * Returns the position where the navigation view should be
     * located according to the page stack.
     *
     * This method can be reimplemented for adapting custom
     * views.
     */
    virtual Qt::Alignment viewPosition() const;

    KWIDGETSADDONS_NO_EXPORT KPageView(KPageViewPrivate &dd, QWidget *parent);

protected:
    std::unique_ptr<class KPageViewPrivate> const d_ptr;
};

#endif
