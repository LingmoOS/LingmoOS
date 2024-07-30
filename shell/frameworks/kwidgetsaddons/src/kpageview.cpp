/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpageview.h"
#include "kpageview_p.h"

#include "kpagemodel.h"
#include "kpagewidgetmodel.h"
#include "loggingcategory.h"

#include <ktitlewidget.h>

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QSize>
#include <QTimer>

// Helper class that draws a rect over a matched widget
class SearchMatchOverlay : public QWidget
{
public:
    SearchMatchOverlay(QWidget *parent, int tabIdx = -1)
        : QWidget(parent)
        , m_tabIdx(tabIdx)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        resize_impl();
        parent->installEventFilter(this);

        show();
        raise();
    }

    int tabIndex() const
    {
        return m_tabIdx;
    }

private:
    void doResize()
    {
        QMetaObject::invokeMethod(this, &SearchMatchOverlay::resize_impl, Qt::QueuedConnection);
    }

    void resize_impl()
    {
        if (m_tabIdx >= 0) {
            auto tabBar = qobject_cast<QTabBar *>(parentWidget());
            if (!tabBar) {
                setVisible(false);
                return;
            }
            const QRect r = tabBar->tabRect(m_tabIdx);
            if (geometry() != r) {
                setGeometry(r);
            }
            return;
        }

        if (parentWidget() && size() != parentWidget()->size()) {
            resize(parentWidget()->size());
        }
    }

    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (parentWidget() && o == parentWidget() && (e->type() == QEvent::Resize || e->type() == QEvent::Show)) {
            doResize();
        }
        return QWidget::eventFilter(o, e);
    }

    void paintEvent(QPaintEvent *e) override
    {
        QPainter p(this);
        p.setClipRegion(e->region());
        QColor c = palette().brush(QPalette::Active, QPalette::Highlight).color();
        c.setAlpha(110);
        p.fillRect(rect(), c);
    }

    int m_tabIdx = -1;
};

void KPageViewPrivate::rebuildGui()
{
    // clean up old view
    Q_Q(KPageView);

    QModelIndex currentLastIndex;
    if (view && view->selectionModel()) {
        QObject::disconnect(m_selectionChangedConnection);
        currentLastIndex = view->selectionModel()->currentIndex();
    }

    delete view;
    view = q->createView();

    Q_ASSERT(view);

    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setSelectionMode(QAbstractItemView::SingleSelection);

    if (model) {
        view->setModel(model);
    }

    // setup new view
    if (view->selectionModel()) {
        m_selectionChangedConnection = QObject::connect(view->selectionModel(),
                                                        &QItemSelectionModel::selectionChanged,
                                                        q,
                                                        [this](const QItemSelection &selected, const QItemSelection &deselected) {
                                                            pageSelected(selected, deselected);
                                                        });

        if (currentLastIndex.isValid()) {
            view->selectionModel()->setCurrentIndex(currentLastIndex, QItemSelectionModel::Select);
        } else if (model) {
            view->selectionModel()->setCurrentIndex(model->index(0, 0), QItemSelectionModel::Select);
        }
    }

    if (faceType == KPageView::Tabbed) {
        stack->setVisible(false);
        layout->removeWidget(stack);
    } else {
        layout->addWidget(stack, 3, 1);
        stack->setVisible(true);
    }

    titleWidget->setPalette(qApp->palette(titleWidget));

    if (!hasSearchableView()) {
        layout->removeWidget(searchLineEditContainer);
        searchLineEditContainer->setVisible(false);
        titleWidget->setAutoFillBackground(false);
        layout->setSpacing(0);
        separatorLine->setVisible(false);
        titleWidget->setObjectName("KPageView::TitleWidgetNonSearchable");
        titleWidget->setContentsMargins(q_ptr->style()->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                        q_ptr->style()->pixelMetric(QStyle::PM_LayoutTopMargin),
                                        q_ptr->style()->pixelMetric(QStyle::PM_LayoutRightMargin),
                                        q_ptr->style()->pixelMetric(QStyle::PM_LayoutBottomMargin));
    } else {
        titleWidget->setObjectName("KPageView::TitleWidget");
        searchLineEditContainer->setVisible(true);
        searchLineEditContainer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        separatorLine->setVisible(true);

        // Adjust margins for a better alignment
        searchLineEditContainer->setContentsMargins(4, 3, 4, 3);
        titleWidget->setContentsMargins(5, 4, 4, 2);

        // Adjust the search + title background so that it merges into the titlebar
        layout->setSpacing(0);
        layout->setContentsMargins({});
    }

    layout->removeWidget(titleWidget);

    if (pageHeader) {
        layout->removeWidget(pageHeader);
        pageHeader->setVisible(q->showPageHeader());
        titleWidget->setVisible(false);

        if (faceType == KPageView::Tabbed) {
            layout->addWidget(pageHeader, 1, 1);
        } else {
            layout->addWidget(pageHeader, 1, 1, 1, 2);
        }
    } else {
        titleWidget->setVisible(q->showPageHeader());
        if (faceType == KPageView::Tabbed) {
            layout->addWidget(titleWidget, 1, 1);
        } else {
            layout->addWidget(titleWidget, 1, 1, 1, 2);
        }
    }

    Qt::Alignment alignment = q->viewPosition();
    if (alignment & Qt::AlignTop) {
        layout->addWidget(view, 2, 1);
    } else if (alignment & Qt::AlignRight) {
        // search line
        layout->addWidget(searchLineEditContainer, 1, 2, Qt::AlignVCenter);
        // item view below the search line
        layout->addWidget(view, 3, 2, 3, 1);
    } else if (alignment & Qt::AlignBottom) {
        layout->addWidget(view, 4, 1);
    } else if (alignment & Qt::AlignLeft) {
        // search line
        layout->addWidget(searchLineEditContainer, 1, 0, Qt::AlignVCenter);
        // item view below the search line
        layout->addWidget(view, 3, 0, 3, 1);
    }
}

void KPageViewPrivate::updateSelection()
{
    // Select the first item in the view if not done yet.

    if (!model) {
        return;
    }

    if (!view || !view->selectionModel()) {
        return;
    }

    const QModelIndex index = view->selectionModel()->currentIndex();
    if (!index.isValid()) {
        view->selectionModel()->setCurrentIndex(model->index(0, 0), QItemSelectionModel::Select);
    }
}

void KPageViewPrivate::cleanupPages()
{
    // Remove all orphan pages from the stacked widget.

    const QList<QWidget *> widgets = collectPages();

    for (int i = 0; i < stack->count(); ++i) {
        QWidget *page = stack->widget(i);

        bool found = false;
        for (int j = 0; j < widgets.count(); ++j) {
            if (widgets[j] == page) {
                found = true;
            }
        }

        if (!found) {
            stack->removeWidget(page);
        }
    }
}

QList<QWidget *> KPageViewPrivate::collectPages(const QModelIndex &parentIndex)
{
    // Traverse through the model recursive and collect all widgets in
    // a list.
    QList<QWidget *> retval;

    int rows = model->rowCount(parentIndex);
    for (int j = 0; j < rows; ++j) {
        const QModelIndex index = model->index(j, 0, parentIndex);
        retval.append(qvariant_cast<QWidget *>(model->data(index, KPageModel::WidgetRole)));

        if (model->rowCount(index) > 0) {
            retval += collectPages(index);
        }
    }

    return retval;
}

KPageView::FaceType KPageViewPrivate::effectiveFaceType() const
{
    if (faceType == KPageView::Auto) {
        return detectAutoFace();
    }

    return faceType;
}

KPageView::FaceType KPageViewPrivate::detectAutoFace() const
{
    if (!model) {
        return KPageView::Plain;
    }

    // Check whether the model has sub pages.
    bool hasSubPages = false;
    const int count = model->rowCount();
    for (int i = 0; i < count; ++i) {
        if (model->rowCount(model->index(i, 0)) > 0) {
            hasSubPages = true;
            break;
        }
    }

    if (hasSubPages) {
        return KPageView::Tree;
    }

    if (model->rowCount() > 1) {
        return KPageView::List;
    }

    return KPageView::Plain;
}

void KPageViewPrivate::modelChanged()
{
    if (!model) {
        return;
    }

    // If the face type is Auto, we rebuild the GUI whenever the layout
    // of the model changes.
    if (faceType == KPageView::Auto) {
        rebuildGui();
        // If you discover some crashes use the line below instead...
        // QTimer::singleShot(0, q, SLOT(rebuildGui()));
    }

    // Set the stack to the minimum size of the largest widget.
    QSize size = stack->size();
    const QList<QWidget *> widgets = collectPages();
    for (int i = 0; i < widgets.count(); ++i) {
        const QWidget *widget = widgets[i];
        if (widget) {
            size = size.expandedTo(widget->minimumSizeHint());
        }
    }
    stack->setMinimumSize(size);

    updateSelection();
}

void KPageViewPrivate::pageSelected(const QItemSelection &index, const QItemSelection &previous)
{
    if (!model) {
        return;
    }

    // Return if the current Index is not valid
    if (index.indexes().size() != 1) {
        return;
    }
    QModelIndex currentIndex = index.indexes().first();

    QModelIndex previousIndex;
    // The previous index can be invalid
    if (previous.indexes().size() == 1) {
        previousIndex = previous.indexes().first();
    }

    if (faceType != KPageView::Tabbed) {
        QWidget *widget = qvariant_cast<QWidget *>(model->data(currentIndex, KPageModel::WidgetRole));

        if (widget) {
            if (stack->indexOf(widget) == -1) { // not included yet
                stack->addWidget(widget);
            }

            stack->setCurrentWidget(widget);
        } else {
            stack->setCurrentWidget(defaultWidget);
        }

        updateTitleWidget(currentIndex);
    }

    Q_Q(KPageView);
    Q_EMIT q->currentPageChanged(currentIndex, previousIndex);
}

void KPageViewPrivate::updateTitleWidget(const QModelIndex &index)
{
    Q_Q(KPageView);

    const bool headerVisible = model->data(index, KPageModel::HeaderVisibleRole).toBool();
    if (!headerVisible) {
        titleWidget->setVisible(false);
        return;
    }
    QString header = model->data(index, KPageModel::HeaderRole).toString();
    if (header.isEmpty()) {
        header = model->data(index, Qt::DisplayRole).toString();
    }

    titleWidget->setText(header);

    titleWidget->setVisible(q->showPageHeader());
}

void KPageViewPrivate::dataChanged(const QModelIndex &, const QModelIndex &)
{
    // When data has changed we update the header and icon for the currently selected
    // page.
    if (!view) {
        return;
    }

    QModelIndex index = view->selectionModel()->currentIndex();
    if (!index.isValid()) {
        return;
    }

    updateTitleWidget(index);
}

KPageViewPrivate::KPageViewPrivate(KPageView *_parent)
    : q_ptr(_parent)
    , model(nullptr)
    , faceType(KPageView::Auto)
    , layout(nullptr)
    , stack(nullptr)
    , titleWidget(nullptr)
    , searchLineEditContainer(new QWidget())
    , searchLineEdit(new QLineEdit())
    , view(nullptr)
{
}

void KPageViewPrivate::init()
{
    Q_Q(KPageView);
    layout = new QGridLayout(q);
    stack = new KPageStackedWidget(q);
    titleWidget = new KTitleWidget(q);
    titleWidget->setObjectName("KPageView::TitleWidget");

    separatorLine = new QFrame(q);
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFixedHeight(1);
    separatorLine->setFrameShadow(QFrame::Sunken);

    // list view under it to the left
    layout->addWidget(titleWidget, 1, 1, 1, 2);
    // separator
    layout->addWidget(separatorLine, 2, 0, 1, 3);
    // and then the actual page on the right
    layout->addWidget(stack, 3, 1);

    defaultWidget = new QWidget(q);
    stack->addWidget(defaultWidget);

    // stack should use most space
    layout->setColumnStretch(1, 1);
    layout->setRowStretch(3, 1);

    searchTimer.setInterval(400);
    searchTimer.setSingleShot(true);
    searchTimer.callOnTimeout(q, [this] {
        onSearchTextChanged();
    });
    q->setFocusProxy(searchLineEdit);
    searchLineEdit->setPlaceholderText(KPageView::tr("Search…", "@info:placeholder"));
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setParent(defaultWidget);
    auto a = new QAction(q);
    a->setIcon(QIcon::fromTheme(QStringLiteral("search")));
    searchLineEdit->addAction(a, QLineEdit::LeadingPosition);
    q->connect(searchLineEdit, &QLineEdit::textChanged, &searchTimer, QOverload<>::of(&QTimer::start));
    auto containerLayout = new QVBoxLayout(searchLineEditContainer);
    containerLayout->setContentsMargins({});
    containerLayout->setSpacing(0);
    containerLayout->addWidget(searchLineEdit);
    searchLineEditContainer->setObjectName("KPageView::Search");
}

static QList<KPageWidgetItem *> getAllPages(KPageWidgetModel *model, const QModelIndex &parent)
{
    const int rc = model->rowCount(parent);
    QList<KPageWidgetItem *> ret;
    for (int i = 0; i < rc; ++i) {
        auto child = model->index(i, 0, parent);
        auto item = model->item(child);
        if (child.isValid() && item) {
            ret << item;
            ret << getAllPages(model, child);
        }
    }
    return ret;
}

template<typename WidgetType>
static QList<QWidget *> hasMatchingText(const QString &text, QWidget *page)
{
    QList<QWidget *> ret;
    const auto widgets = page->findChildren<WidgetType *>();
    for (auto label : widgets) {
        if (label->text().contains(text, Qt::CaseInsensitive)) {
            ret << label;
        }
    }
    return ret;
}

template<>
QList<QWidget *> hasMatchingText<QComboBox>(const QString &text, QWidget *page)
{
    QList<QWidget *> ret;
    const auto comboxBoxes = page->findChildren<QComboBox *>();
    for (auto cb : comboxBoxes) {
        if (cb->findText(text, Qt::MatchFlag::MatchContains) != -1) {
            ret << cb;
        }
    }
    return ret;
}

template<typename...>
struct FindChildrenHelper {
    static QList<QWidget *> hasMatchingTextForTypes(const QString &, QWidget *)
    {
        return {};
    }
};

template<typename First, typename... Rest>
struct FindChildrenHelper<First, Rest...> {
    static QList<QWidget *> hasMatchingTextForTypes(const QString &text, QWidget *page)
    {
        return hasMatchingText<First>(text, page) << FindChildrenHelper<Rest...>::hasMatchingTextForTypes(text, page);
    }
};

static QModelIndex walkTreeAndHideItems(QTreeView *tree, const QString &searchText, const QSet<QString> &pagesToHide, const QModelIndex &parent)
{
    QModelIndex current;
    auto model = tree->model();
    const int rows = model->rowCount(parent);
    for (int i = 0; i < rows; ++i) {
        const auto index = model->index(i, 0, parent);
        const auto itemName = index.data().toString();
        tree->setRowHidden(i, parent, pagesToHide.contains(itemName) && !itemName.contains(searchText, Qt::CaseInsensitive));
        if (!searchText.isEmpty() && !tree->isRowHidden(i, parent) && !current.isValid()) {
            current = model->index(i, 0, parent);
        }
        auto curr = walkTreeAndHideItems(tree, searchText, pagesToHide, index);
        if (!current.isValid()) {
            current = curr;
        }
    }
    return current;
}

bool KPageViewPrivate::hasSearchableView() const
{
    // We support search for only these two types as they can hide rows
    return qobject_cast<KDEPrivate::KPageListView *>(view) || qobject_cast<KDEPrivate::KPageTreeView *>(view);
}

void KPageViewPrivate::onSearchTextChanged()
{
    if (!hasSearchableView()) {
        return;
    }

    const QString text = searchLineEdit->text();
    QSet<QString> pagesToHide;
    std::vector<QWidget *> matchedWidgets;
    if (!text.isEmpty()) {
        const auto pages = getAllPages(static_cast<KPageWidgetModel *>(model), {});
        for (auto item : pages) {
            const auto matchingWidgets = FindChildrenHelper<QLabel, QAbstractButton, QComboBox>::hasMatchingTextForTypes(text, item->widget());
            if (matchingWidgets.isEmpty()) {
                pagesToHide << item->name();
            }
            matchedWidgets.insert(matchedWidgets.end(), matchingWidgets.begin(), matchingWidgets.end());
        }
    }

    if (model) {
        QModelIndex current;
        if (auto list = qobject_cast<QListView *>(view)) {
            for (int i = 0; i < model->rowCount(); ++i) {
                const auto itemName = model->index(i, 0).data().toString();
                list->setRowHidden(i, pagesToHide.contains(itemName) && !itemName.contains(text, Qt::CaseInsensitive));
                if (!text.isEmpty() && !list->isRowHidden(i) && !current.isValid()) {
                    current = model->index(i, 0);
                }
            }
        } else if (auto tree = qobject_cast<QTreeView *>(view)) {
            current = walkTreeAndHideItems(tree, text, pagesToHide, {});
            auto parent = current.parent();
            while (parent.isValid()) {
                tree->setRowHidden(parent.row(), parent.parent(), false);
                parent = parent.parent();
            }
        } else {
            qWarning() << "Unreacheable, unknown view:" << view;
            Q_UNREACHABLE();
        }

        if (current.isValid()) {
            view->setCurrentIndex(current);
        }
    }

    qDeleteAll(m_searchMatchOverlays);
    m_searchMatchOverlays.clear();

    using TabWidgetAndPage = QPair<QTabWidget *, QWidget *>;
    auto tabWidgetParent = [](QWidget *w) {
        // Finds if @p w is in a QTabWidget and returns
        // The QTabWidget + the widget in the stack where
        // @p w lives
        auto parent = w->parentWidget();
        TabWidgetAndPage p = {nullptr, nullptr};
        QVarLengthArray<QWidget *, 8> parentChain;
        parentChain << parent;
        while (parent) {
            if (auto tw = qobject_cast<QTabWidget *>(parent)) {
                if (parentChain.size() >= 3) {
                    // last == QTabWidget
                    // second last == QStackedWidget of QTabWidget
                    // third last => the widget we want
                    p.second = parentChain.value((parentChain.size() - 1) - 2);
                }
                p.first = tw;
                break;
            }
            parent = parent->parentWidget();
            parentChain << parent;
        }
        return p;
    };

    for (auto w : matchedWidgets) {
        if (w) {
            m_searchMatchOverlays << new SearchMatchOverlay(w);
            if (!w->isVisible()) {
                const auto [tabWidget, page] = tabWidgetParent(w);
                if (!tabWidget && !page) {
                    continue;
                }
                const int idx = tabWidget->indexOf(page);
                if (idx < 0) {
                    // qDebug() << page << tabWidget << "not found" << w;
                    continue;
                }

                const bool alreadyOverlayed =
                    std::any_of(m_searchMatchOverlays.cbegin(), m_searchMatchOverlays.cend(), [tabbar = tabWidget->tabBar(), idx](SearchMatchOverlay *overlay) {
                        return idx == overlay->tabIndex() && tabbar == overlay->parentWidget();
                    });
                if (!alreadyOverlayed) {
                    m_searchMatchOverlays << new SearchMatchOverlay(tabWidget->tabBar(), idx);
                }
            }
        }
    }
}

// KPageView Implementation
KPageView::KPageView(QWidget *parent)
    : KPageView(*new KPageViewPrivate(this), parent)
{
}

KPageView::KPageView(KPageViewPrivate &dd, QWidget *parent)
    : QWidget(parent)
    , d_ptr(&dd)
{
    d_ptr->init();
}

KPageView::~KPageView() = default;

void KPageView::setModel(QAbstractItemModel *model)
{
    Q_D(KPageView);
    // clean up old model
    if (d->model) {
        disconnect(d->m_layoutChangedConnection);
        disconnect(d->m_dataChangedConnection);
    }

    d->model = model;

    if (d->model) {
        d->m_layoutChangedConnection = connect(d->model, &QAbstractItemModel::layoutChanged, this, [d]() {
            d->modelChanged();
        });
        d->m_dataChangedConnection = connect(d->model, &QAbstractItemModel::dataChanged, this, [d](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
            d->dataChanged(topLeft, bottomRight);
        });

        // set new model in navigation view
        if (d->view) {
            d->view->setModel(model);
        }
    }

    d->rebuildGui();
}

QAbstractItemModel *KPageView::model() const
{
    Q_D(const KPageView);
    return d->model;
}

void KPageView::setFaceType(FaceType faceType)
{
    Q_D(KPageView);
    d->faceType = faceType;

    d->rebuildGui();
}

KPageView::FaceType KPageView::faceType() const
{
    Q_D(const KPageView);
    return d->faceType;
}

void KPageView::setCurrentPage(const QModelIndex &index)
{
    Q_D(KPageView);
    if (!d->view || !d->view->selectionModel()) {
        return;
    }

    d->view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

QModelIndex KPageView::currentPage() const
{
    Q_D(const KPageView);
    if (!d->view || !d->view->selectionModel()) {
        return QModelIndex();
    }

    return d->view->selectionModel()->currentIndex();
}

void KPageView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    Q_D(KPageView);
    if (d->view) {
        d->view->setItemDelegate(delegate);
    }
}

QAbstractItemDelegate *KPageView::itemDelegate() const
{
    Q_D(const KPageView);
    if (d->view) {
        return d->view->itemDelegate();
    } else {
        return nullptr;
    }
}

void KPageView::setDefaultWidget(QWidget *widget)
{
    Q_D(KPageView);

    Q_ASSERT(widget);

    bool isCurrent = (d->stack->currentIndex() == d->stack->indexOf(d->defaultWidget));

    // remove old default widget
    d->stack->removeWidget(d->defaultWidget);
    delete d->defaultWidget;

    // add new default widget
    d->defaultWidget = widget;
    d->stack->addWidget(d->defaultWidget);

    if (isCurrent) {
        d->stack->setCurrentWidget(d->defaultWidget);
    }
}

void KPageView::setPageHeader(QWidget *header)
{
    Q_D(KPageView);
    if (d->pageHeader == header) {
        return;
    }

    if (d->pageHeader) {
        d->layout->removeWidget(d->pageHeader);
    }
    d->layout->removeWidget(d->titleWidget);

    d->pageHeader = header;

    // Give it a colSpan of 2 to add a margin to the right
    if (d->pageHeader) {
        d->layout->addWidget(d->pageHeader, 1, 1, 1, 2);
        d->pageHeader->setVisible(showPageHeader());
    } else {
        d->layout->addWidget(d->titleWidget, 1, 1, 1, 2);
        d->titleWidget->setVisible(showPageHeader());
    }
}

QWidget *KPageView::pageHeader() const
{
    Q_D(const KPageView);
    if (!d->pageHeader) {
        return d->titleWidget;
    }
    return d->pageHeader;
}

void KPageView::setPageFooter(QWidget *footer)
{
    Q_D(KPageView);
    if (d->pageFooter == footer) {
        return;
    }

    if (d->pageFooter) {
        d->layout->removeWidget(d->pageFooter);
    }

    d->pageFooter = footer;

    if (footer) {
        d->pageFooter->setContentsMargins(4, 4, 4, 4);
        d->layout->addWidget(d->pageFooter, 4, 1);
    }
}

QWidget *KPageView::pageFooter() const
{
    Q_D(const KPageView);
    return d->pageFooter;
}

QAbstractItemView *KPageView::createView()
{
    Q_D(KPageView);
    const FaceType faceType = d->effectiveFaceType();

    if (faceType == Plain) {
        return new KDEPrivate::KPagePlainView(this);
    }
    if (faceType == FlatList) {
        return new KDEPrivate::KPageListView(this);
    }
    if (faceType == List) {
        auto view = new KDEPrivate::KPageListView(this);
        view->setItemDelegate(new KDEPrivate::KPageListViewDelegate(this));
        view->setFlexibleWidth(true);
        return view;
    }
    if (faceType == Tree) {
        return new KDEPrivate::KPageTreeView(this);
    }
    if (faceType == Tabbed) {
        return new KDEPrivate::KPageTabbedView(this);
    }

    return nullptr;
}

bool KPageView::showPageHeader() const
{
    Q_D(const KPageView);
    const FaceType faceType = d->effectiveFaceType();

    if (faceType == Tabbed) {
        return false;
    } else {
        return d->pageHeader || !d->titleWidget->text().isEmpty();
    }
}

Qt::Alignment KPageView::viewPosition() const
{
    Q_D(const KPageView);
    const FaceType faceType = d->effectiveFaceType();

    if (faceType == Plain || faceType == Tabbed) {
        return Qt::AlignTop;
    } else {
        return Qt::AlignLeft;
    }
}

#include "moc_kpageview.cpp"
