/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEMODEL_H
#define KPAGEMODEL_H

#include <kwidgetsaddons_export.h>

#include <QAbstractItemModel>
#include <memory>

class KPageModelPrivate;

/**
 *  @class KPageModel kpagemodel.h KPageModel
 *
 *  @short A base class for a model used by KPageView.
 *
 *  This class is an abstract base class which must be used to
 *  implement custom models for KPageView. Additional to the standard
 *  Qt::ItemDataRoles it provides the two roles
 *
 *    @li HeaderRole
 *    @li HeaderVisibleRole
 *    @li WidgetRole
 *
 *  which are used to return a header string for a page and a QWidget
 *  pointer to the page itself.
 *
 *  <b>Example:</b>\n
 *
 *  \code
 *    KPageView *view = new KPageView( this );
 *    KPageModel *model = new MyPageModel( this );
 *
 *    view->setModel( model );
 *  \endcode
 *
 *  @see KPageView
 *  @author Tobias Koenig <tokoe@kde.org>
 */
class KWIDGETSADDONS_EXPORT KPageModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KPageModel)

public:
    /**
     * Additional roles that KPageView uses.
     */
    enum Role {
        /**
         * A string to be rendered as page header.
         */
        HeaderRole = Qt::UserRole + 1,
        /**
         * A pointer to the page widget. This is the widget that is shown when the item is
         * selected.
         *
         * You can make QVariant take a QWidget using
         * \code
         * QWidget *myWidget = new QWidget;
         * QVariant v = QVariant::fromValue(myWidget);
         * \endcode
         */
        WidgetRole,
        /**
         * when true, show the page header, if false don't
         * @since 5.52
         */
        HeaderVisibleRole,
    };

    /**
     * Constructs a page model with the given parent.
     */
    explicit KPageModel(QObject *parent = nullptr);

    /**
     * Destroys the page model.
     */
    ~KPageModel() override;

protected:
    KWIDGETSADDONS_NO_EXPORT KPageModel(KPageModelPrivate &dd, QObject *parent);

    std::unique_ptr<class KPageModelPrivate> const d_ptr;
};

#endif
