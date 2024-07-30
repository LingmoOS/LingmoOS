/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEVIEW_P_H
#define KPAGEVIEW_P_H

#include "kpageview.h"

#include <QAbstractItemDelegate>
#include <QAbstractProxyModel>
#include <QGridLayout>
#include <QLineEdit>
#include <QListView>
#include <QPointer>
#include <QStackedWidget>
#include <QTimer>
#include <QTreeView>
#include <ktitlewidget.h>

class KPageStackedWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit KPageStackedWidget(QWidget *parent = nullptr)
        : QStackedWidget(parent)
    {
    }

    void setMinimumSize(const QSize &size)
    {
        mMinimumSize = size;
    }

    QSize minimumSizeHint() const override
    {
        return mMinimumSize.expandedTo(QStackedWidget::minimumSizeHint());
    }

private:
    QSize mMinimumSize;
};

class KPageViewPrivate
{
    Q_DECLARE_PUBLIC(KPageView)

public:
    virtual ~KPageViewPrivate() = default;

protected:
    KPageViewPrivate(KPageView *);

    KPageView *q_ptr;

    // data
    QAbstractItemModel *model;
    KPageView::FaceType faceType;

    // gui
    QGridLayout *layout;
    KPageStackedWidget *stack;
    KTitleWidget *titleWidget;
    QWidget *defaultWidget;
    QWidget *searchLineEditContainer;
    QLineEdit *searchLineEdit;
    QFrame *separatorLine;
    QTimer searchTimer;

    QAbstractItemView *view;

    QPointer<QWidget> pageHeader;
    QPointer<QWidget> pageFooter;

    void updateTitleWidget(const QModelIndex &index);

    void updateSelection();
    void cleanupPages();
    QList<QWidget *> collectPages(const QModelIndex &parent = QModelIndex());
    KPageView::FaceType detectAutoFace() const;
    KPageView::FaceType effectiveFaceType() const;

    // private slots
    void rebuildGui();
    void modelChanged();
    void dataChanged(const QModelIndex &, const QModelIndex &);
    void pageSelected(const QItemSelection &, const QItemSelection &);
    bool hasSearchableView() const;

private:
    void onSearchTextChanged();
    void init();

    QMetaObject::Connection m_dataChangedConnection;
    QMetaObject::Connection m_layoutChangedConnection;
    QMetaObject::Connection m_selectionChangedConnection;
    QList<class SearchMatchOverlay *> m_searchMatchOverlays;
};

namespace KDEPrivate
{
class KPageListViewDelegate;
class KPageListViewProxy;

class KPagePlainView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit KPagePlainView(QWidget *parent = nullptr);

    QModelIndex indexAt(const QPoint &point) const override;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
    QRect visualRect(const QModelIndex &index) const override;

protected:
    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) override;
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex &) const override;
    void setSelection(const QRect &, QFlags<QItemSelectionModel::SelectionFlag>) override;
    QRegion visualRegionForSelection(const QItemSelection &) const override;
};

class KPageListView : public QListView
{
    Q_OBJECT

public:
    explicit KPageListView(QWidget *parent = nullptr);
    ~KPageListView() override;

    void setModel(QAbstractItemModel *model) override;
    void setFlexibleWidth(bool flexibleWidth);

protected:
    void changeEvent(QEvent *event) override;

private Q_SLOTS:
    void updateWidth();

private:
    bool m_flexibleWidth = false;
};

class KPageTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit KPageTreeView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model) override;

private Q_SLOTS:
    void updateWidth();

private:
    void expandItems(const QModelIndex &index = QModelIndex());
};

class KPageTabbedView : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit KPageTabbedView(QWidget *parent = nullptr);
    ~KPageTabbedView() override;

    void setModel(QAbstractItemModel *model) override;

    QModelIndex indexAt(const QPoint &point) const override;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
    QRect visualRect(const QModelIndex &index) const override;

    QSize minimumSizeHint() const override;

protected:
    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) override;
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex &) const override;
    void setSelection(const QRect &, QFlags<QItemSelectionModel::SelectionFlag>) override;
    QRegion visualRegionForSelection(const QItemSelection &) const override;

private Q_SLOTS:
    void currentPageChanged(int);
    void layoutChanged();
    void dataChanged(const QModelIndex &, const QModelIndex &, const QList<int> &roles) override;

private:
    QTabWidget *mTabWidget;
};

class KPageListViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit KPageListViewDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void drawFocus(QPainter *, const QStyleOptionViewItem &, const QRect &) const;
};

/**
 * We need this proxy model to map the leaves of a tree-like model
 * to a one-level list model.
 */
class KPageListViewProxy : public QAbstractProxyModel
{
    Q_OBJECT

public:
    explicit KPageListViewProxy(QObject *parent = nullptr);
    ~KPageListViewProxy() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex mapFromSource(const QModelIndex &index) const override;
    QModelIndex mapToSource(const QModelIndex &index) const override;

public Q_SLOTS:
    void rebuildMap();

private:
    void addMapEntry(const QModelIndex &);

    QList<QModelIndex> mList;
};

class SelectionModel : public QItemSelectionModel
{
    Q_OBJECT

public:
    SelectionModel(QAbstractItemModel *model, QObject *parent);

public Q_SLOTS:
    void clear() override;
    void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
};

}

#endif
