/*
 * SPDX-FileCopyrightText: 2009 Ben Cooksley <bcooksley@kde.org>
 * SPDX-FileCopyrightText: 2022 ivan tkachenko <me@ratijas.tk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "SidebarMode.h"

#include "MenuItem.h"
#include "MenuModel.h"
#include "MenuProxyModel.h"
#include "ModuleView.h"
#include "kcmmetadatahelpers.h"

#include <KAboutData>
#include <KActionCollection>
#include <KCModuleLoader>
#include <KConfigGroup>
#include <KDescendantsProxyModel>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QAction>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
#include <QStandardPaths>

FocusHackWidget::FocusHackWidget(QWidget *parent)
    : QWidget(parent)
{
}

FocusHackWidget::~FocusHackWidget()
{
}

void FocusHackWidget::focusNext()
{
    focusNextChild();
}

void FocusHackWidget::focusPrevious()
{
    focusPreviousChild();
}

SubcategoryModel::SubcategoryModel(QAbstractItemModel *parentModel, SidebarMode *parent)
    : KSelectionProxyModel(nullptr, parent)
    , m_parentModel(parentModel)
    , m_sidebarMode(parent)
{
    setSourceModel(parentModel);
    setSelectionModel(new QItemSelectionModel(parentModel, this));
    setFilterBehavior(SubTreesWithoutRoots);
}

QString SubcategoryModel::title() const
{
    auto const *mi = m_activeModuleIndex.data(MenuModel::MenuItemRole).value<MenuItem *>();

    if (!mi) {
        return {};
    }

    return mi->name();
}

QIcon SubcategoryModel::icon() const
{
    return m_activeModuleIndex.data(Qt::DecorationRole).value<QIcon>();
}

bool SubcategoryModel::categoryOwnedByKCM() const
{
    return m_activeModuleIndex.data(MenuModel::IsKCMRole).toBool();
}

void SubcategoryModel::setParentIndex(const QModelIndex &activeModule)
{
    selectionModel()->select(activeModule, QItemSelectionModel::ClearAndSelect);
    m_activeModuleIndex = QPersistentModelIndex(activeModule);
    Q_EMIT titleChanged();
    Q_EMIT iconChanged();
    Q_EMIT categoryOwnedByKCMChanged();
}

void SubcategoryModel::loadParentCategoryModule()
{
    auto menuItem = m_activeModuleIndex.data(MenuModel::MenuItemRole).value<MenuItem *>();
    if (menuItem->isLibrary()) {
        m_sidebarMode->loadModule(m_activeModuleIndex);
    }
}

class SidebarMode::Private
{
public:
    Private()
        : quickWidget(nullptr)
        , moduleView(nullptr)
        , collection(nullptr)
        , activeCategoryRow(-1)
        , activeSubCategoryRow(-1)
    {
    }

    QQuickWidget *quickWidget = nullptr;
    SubcategoryModel *subCategoryModel = nullptr;
    FocusHackWidget *mainWidget = nullptr;
    QQuickWidget *placeHolderWidget = nullptr;
    QHBoxLayout *mainLayout = nullptr;
    MenuModel *model = nullptr;
    MenuProxyModel *categorizedModel = nullptr;
    MenuProxyModel *searchModel = nullptr;
    KDescendantsProxyModel *flatModel = nullptr;
    ModuleView *moduleView = nullptr;
    KActionCollection *collection = nullptr;
    QPersistentModelIndex activeCategoryIndex;
    std::shared_ptr<QQmlEngine> engine;
    int activeCategoryRow = -1;
    int activeSubCategoryRow = -1;
    int activeSearchRow = -1;
    qreal headerHeight = 0;
    bool actionMenuVisible = false;
    bool m_defaultsIndicatorsVisible = false;
    KConfigGroup config;
    MenuItem *rootItem = nullptr;
    MenuItem *homeItem = nullptr;
    KPluginMetaData metaData;
    QString startupModule;
    QStringList startupModuleArgs;
    bool showToolTips = true;
    ApplicationMode applicationMode = SystemSettings;
};

SidebarMode::SidebarMode(QObject *parent,
                         ApplicationMode mode,
                         const QString &startupModule,
                         const QStringList &startupModuleArgs,
                         KActionCollection *actions,
                         MenuItem *homeItem,
                         MenuItem *rootItem)
    : QObject(parent)
    , d(new Private())
{
    d->applicationMode = mode;
    d->startupModule = startupModule;
    d->startupModuleArgs = startupModuleArgs;
    d->collection = actions;
    d->homeItem = homeItem;
    d->rootItem = rootItem;

    qApp->setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    qmlRegisterAnonymousType<QAction>("", 1);
    qmlRegisterAnonymousType<QAbstractItemModel>("", 1);
    d->config = KSharedConfig::openConfig()->group(QStringLiteral("systemsettings_sidebar_mode"));
    initEvent();
    connect(moduleView(), &ModuleView::moduleChanged, this, &SidebarMode::viewChanged);
}

SidebarMode::~SidebarMode()
{
    d->config.sync();
    delete d;
}

ModuleView *SidebarMode::moduleView() const
{
    return d->moduleView;
}

QWidget *SidebarMode::mainWidget()
{
    if (!d->quickWidget) {
        initWidget();
    }
    return d->mainWidget;
}

QAbstractItemModel *SidebarMode::categoryModel() const
{
    return d->categorizedModel;
}

QAbstractItemModel *SidebarMode::searchModel() const
{
    return d->searchModel;
}

QAbstractItemModel *SidebarMode::subCategoryModel() const
{
    return d->subCategoryModel;
}

void SidebarMode::initEvent()
{
    d->model = new MenuModel(rootItem(), this);
    foreach (MenuItem *child, rootItem()->children()) {
        d->model->addException(child);
    }

    d->categorizedModel = new MenuProxyModel(this);
    d->categorizedModel->setCategorizedModel(true);
    d->categorizedModel->setSourceModel(d->model);
    d->categorizedModel->sort(0);
    d->categorizedModel->setFilterHighlightsEntries(false);

    d->flatModel = new KDescendantsProxyModel(this);
    d->flatModel->setSourceModel(d->model);

    d->searchModel = new MenuProxyModel(this);
    d->searchModel->setCategorizedModel(true);
    d->searchModel->setFilterHighlightsEntries(false);
    d->searchModel->setSourceModel(d->flatModel);

    d->subCategoryModel = new SubcategoryModel(d->categorizedModel, this);
    d->mainWidget = new FocusHackWidget();
    d->mainWidget->installEventFilter(this);
    d->mainLayout = new QHBoxLayout(d->mainWidget);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);
    d->engine = std::make_shared<QQmlEngine>();
    qApp->setProperty("__qmlEngine", QVariant::fromValue(d->engine.get()));
    d->moduleView = new ModuleView(d->engine, d->mainWidget);
    connect(d->moduleView, &ModuleView::moduleChanged, this, &SidebarMode::moduleLoaded);
    connect(d->moduleView, &ModuleView::moduleSaved, this, &SidebarMode::updateDefaults);
    d->quickWidget = nullptr;
    moduleView()->setFaceType(KPageView::Plain);
    if (d->applicationMode == InfoCenter) {
        d->moduleView->setSaveStatistics(false);
        d->moduleView->setApplyVisible(false);
        d->moduleView->setDefaultsVisible(false);
    }

    if (d->config.readEntry("HighlightNonDefaultSettings", false)) {
        toggleDefaultsIndicatorsVisibility();
    }
}

QAction *SidebarMode::action(const QString &name) const
{
    return d->collection->action(name);
}

QString SidebarMode::actionIconName(const QString &name) const
{
    if (QAction *a = action(name)) {
        return a->icon().name();
    }

    return {};
}

void SidebarMode::showActionMenu(const QPoint &position)
{
    auto menu = new QMenu();
    connect(menu, &QMenu::aboutToHide, this, [this]() {
        setActionMenuVisible(false);
    });
    menu->setAttribute(Qt::WA_DeleteOnClose);
    // Ocean and Oxygen have rounded corners on menus. They set this attribute in polish()
    // but at that time the underlying surface has already been created where setting this
    // flag makes no difference anymore (Bug 385311)
    menu->setAttribute(Qt::WA_TranslucentBackground);

    const QStringList actionList{QStringLiteral("highlight_changes"),
                                 QStringLiteral("report_bug_in_current_module"),
                                 QStringLiteral("help_report_bug"),
                                 QStringLiteral("help_contents"),
                                 QStringLiteral("help_about_app"),
                                 QStringLiteral("help_about_kde")};

    for (const QString &actionName : actionList) {
        QAction *action = d->collection->action(actionName);
        if (action) {
            menu->addAction(action);
        }
    }

    menu->winId();
    menu->windowHandle()->setTransientParent(d->quickWidget->window()->windowHandle());
    menu->popup(position);
    setActionMenuVisible(true);
}

void SidebarMode::loadModule(const QModelIndex &activeModule, const QStringList &args)
{
    if (!activeModule.isValid()) {
        return;
    }

    auto mi = activeModule.data(MenuModel::MenuItemRole).value<MenuItem *>();

    if (!mi) {
        return;
    }

    // If we are trying to load a module already open
    if (mi->name() == d->moduleView->activeModuleName()) {
        const QVariantList variantArgs(args.cbegin(), args.cend());
        d->moduleView->requestActivation(variantArgs);
        return;
    }

    if (!d->moduleView->resolveChanges()) {
        return;
    }

    d->moduleView->closeModules();

    d->moduleView->loadModule(activeModule, args);

    if (activeModule.model() == d->categorizedModel) {
        const int newCategoryRow = activeModule.row();

        d->activeCategoryIndex = activeModule;
        d->activeCategoryRow = newCategoryRow;

        d->activeSubCategoryRow = 0;

        d->subCategoryModel->setParentIndex(activeModule);

        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            Q_EMIT activeSearchRowChanged();
        }
        Q_EMIT activeCategoryRowChanged();
        Q_EMIT activeSubCategoryRowChanged();

    } else if (activeModule.model() == d->subCategoryModel) {
        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            Q_EMIT activeSearchRowChanged();
        }
        d->activeSubCategoryRow = activeModule.row();
        Q_EMIT activeSubCategoryRowChanged();

    } else if (activeModule.model() == d->searchModel) {
        QModelIndex originalIndex = d->categorizedModel->mapFromSource(d->flatModel->mapToSource(d->searchModel->mapToSource(activeModule)));

        if (originalIndex.isValid()) {
            // are we in a  subcategory of the top categories?
            if (originalIndex.parent().isValid() && mi->parent()->menu()) {
                d->activeCategoryRow = originalIndex.parent().row();
                d->activeSubCategoryRow = originalIndex.row();

                // Is this kcm directly at the top level without a top category?
            } else {
                d->activeCategoryRow = originalIndex.row();
                d->activeSubCategoryRow = -1;
            }

            d->subCategoryModel->setParentIndex(originalIndex.parent().isValid() ? originalIndex.parent() : originalIndex);
            Q_EMIT activeCategoryRowChanged();
            Q_EMIT activeSubCategoryRowChanged();
        }

        d->activeSearchRow = activeModule.row();
        Q_EMIT activeSearchRowChanged();

    } else {
        if (d->activeSearchRow > -1) {
            d->activeSearchRow = -1;
            Q_EMIT activeSearchRowChanged();
        }

        QModelIndex flatIndex;

        // search the corresponding item on the main model
        for (int i = 0; i < d->flatModel->rowCount(); ++i) {
            QModelIndex idx = d->flatModel->index(i, 0);
            auto otherMi = idx.data(MenuModel::MenuItemRole).value<MenuItem *>();

            if (mi->metaData().isValid() && otherMi->metaData() == mi->metaData()) {
                flatIndex = idx;
                break;
            }
        }

        if (flatIndex.isValid()) {
            QModelIndex idx = d->categorizedModel->mapFromSource(d->flatModel->mapToSource(flatIndex));

            auto parentMi = idx.parent().data(MenuModel::MenuItemRole).value<MenuItem *>();

            if (idx.isValid()) {
                if (parentMi && parentMi->menu()) {
                    d->subCategoryModel->setParentIndex(idx.parent());
                    d->activeCategoryRow = idx.parent().row();
                    d->activeSubCategoryRow = idx.row();
                } else {
                    if (d->categorizedModel->rowCount(idx) > 0) {
                        d->subCategoryModel->setParentIndex(idx);
                    }
                    d->activeCategoryRow = idx.row();
                    d->activeSubCategoryRow = -1;
                }
                Q_EMIT activeCategoryRowChanged();
                Q_EMIT activeSubCategoryRowChanged();
            }
        }
    }
}

void SidebarMode::focusNext()
{
    d->mainWidget->focusNext();
}

void SidebarMode::focusPrevious()
{
    d->mainWidget->focusPrevious();
}

void SidebarMode::moduleLoaded()
{
    if (d->placeHolderWidget) {
        d->placeHolderWidget->hide();
    }
    d->moduleView->show();
    if (d->applicationMode == InfoCenter) {
        d->moduleView->setSaveStatistics(false);
        d->moduleView->setApplyVisible(false);
        d->moduleView->setDefaultsVisible(false);
    }
}

void SidebarMode::updateDefaults()
{
    // When the landing page is loaded, we don't have activeCategoryRow and need to reload everything
    if (d->activeCategoryRow < 0) {
        refreshDefaults();
        return;
    }
    QModelIndex categoryIdx = d->categorizedModel->index(d->activeCategoryRow, 0);
    auto item = categoryIdx.data(Qt::UserRole).value<MenuItem *>();
    Q_ASSERT(item);

    // If subcategory exist update from subcategory, unless this category is owned by a kcm
    if (!item->children().isEmpty() && d->activeSubCategoryRow > -1) {
        auto subCateogryIdx = d->subCategoryModel->index(d->activeSubCategoryRow, 0);
        item = subCateogryIdx.data(Qt::UserRole).value<MenuItem *>();
    }
    // In case we are updating a parent like LnF we refresh all defaults
    if (item->isCategoryOwner() && item->parent() != rootItem()) {
        refreshDefaults();
        return;
    }

    KCModuleData *moduleData = loadModuleData(item->metaData());
    if (moduleData) {
        connect(moduleData, &KCModuleData::loaded, this, [this, item, moduleData, categoryIdx]() {
            item->setDefaultIndicator(!moduleData->isDefaults());
            updateCategoryModel(categoryIdx);
            moduleData->deleteLater();
        });
    }
}

void SidebarMode::updateCategoryModel(const QModelIndex &categoryIdx)
{
    auto sourceIdx = d->categorizedModel->mapToSource(categoryIdx);
    Q_EMIT d->model->dataChanged(sourceIdx, sourceIdx);

    auto subCateogryIdx = d->subCategoryModel->index(d->activeSubCategoryRow, 0);
    auto subCategorySourceIdx = d->categorizedModel->mapToSource(d->subCategoryModel->mapToSource(subCateogryIdx));
    Q_EMIT d->model->dataChanged(subCategorySourceIdx, subCategorySourceIdx);
}

int SidebarMode::activeSearchRow() const
{
    return d->activeSearchRow;
}

int SidebarMode::activeCategoryRow() const
{
    return d->activeCategoryRow;
}

void SidebarMode::setHeaderHeight(qreal height)
{
    if (height == d->moduleView->headerHeight()) {
        return;
    }

    d->moduleView->setHeaderHeight(height);
    Q_EMIT headerHeightChanged();
}

qreal SidebarMode::headerHeight() const
{
    return d->moduleView->headerHeight();
}

void SidebarMode::refreshDefaults()
{
    if (d->m_defaultsIndicatorsVisible) {
        for (int i = 0; i < d->flatModel->rowCount(); ++i) {
            QModelIndex idx = d->flatModel->index(i, 0);
            auto item = idx.data(MenuModel::MenuItemRole).value<MenuItem *>();
            if (item->menu()) {
                continue;
            }

            KCModuleData *moduleData = loadModuleData(item->metaData());
            if (moduleData) {
                connect(moduleData, &KCModuleData::loaded, this, [this, item, moduleData]() {
                    item->setDefaultIndicator(!moduleData->isDefaults());
                    updateModelMenuItem(item);
                    moduleData->deleteLater();
                });
            }
        }
    }
}

void SidebarMode::toggleDefaultsIndicatorsVisibility()
{
    d->m_defaultsIndicatorsVisible = !d->m_defaultsIndicatorsVisible;
    d->moduleView->moduleShowDefaultsIndicators(d->m_defaultsIndicatorsVisible);
    refreshDefaults();
    d->config.writeEntry("HighlightNonDefaultSettings", d->m_defaultsIndicatorsVisible);
    Q_EMIT defaultsIndicatorsVisibleChanged();
}

void SidebarMode::updateModelMenuItem(MenuItem *item)
{
    auto itemIdx = d->model->indexForItem(item);
    Q_EMIT d->model->dataChanged(itemIdx, itemIdx);
    MenuItem *parent = item->parent();
    while (parent) {
        auto parentIdx = d->model->indexForItem(parent);
        if (parentIdx.isValid()) {
            Q_EMIT d->model->dataChanged(parentIdx, parentIdx);
            parent = parent->parent();
        } else {
            parent = nullptr;
        }
    }
}

int SidebarMode::width() const
{
    return d->mainWidget->width();
}

bool SidebarMode::actionMenuVisible() const
{
    return d->actionMenuVisible;
}

void SidebarMode::setActionMenuVisible(bool visible)
{
    if (d->actionMenuVisible == visible) {
        return;
    }
    d->actionMenuVisible = visible;
    Q_EMIT actionMenuVisibleChanged();
}

int SidebarMode::activeSubCategoryRow() const
{
    return d->activeSubCategoryRow;
}

bool SidebarMode::defaultsIndicatorsVisible() const
{
    return d->m_defaultsIndicatorsVisible;
}

void SidebarMode::initWidget()
{
    // Create the widgets
    // SidebarMode and ModuleView have the reference
    Q_ASSERT_X(d->engine.use_count() == 2, Q_FUNC_INFO, qUtf8Printable(QString::number(d->engine.use_count())));
    d->quickWidget = new QQuickWidget(d->engine.get(), d->mainWidget);
    d->quickWidget->quickWindow()->setTitle(i18n("Sidebar"));
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    d->quickWidget->rootContext()->setContextProperty(QStringLiteral("systemsettings"), this);

    d->quickWidget->rootContext()->setContextObject(new KLocalizedContext(d->quickWidget));

    // Breaking change in Qt6: https://github.com/qt/qtdeclarative/commit/0d80dbd8c2cfc2a7d2a4d970b7acfc7fb5fb97a0
    // Use setContent to prevent the root item from being destroyed: https://github.com/qt/qtdeclarative/commit/69d61fecf2deee7510f5f2448614174683744d82
    QQmlComponent component(d->quickWidget->engine(),
                            QUrl(QStringLiteral("qrc:/qt/qml/org/kde/systemsettings/Main.qml")),
                            QQmlComponent::PreferSynchronous,
                            this);
    QQuickItem *item = qobject_cast<QQuickItem *>(component.create(d->quickWidget->rootContext()));
    if (!item) {
        for (const QList<QQmlError> errors = component.errors(); const auto &err : errors) {
            qWarning() << err.toString();
        }
        qFatal("Fatal error while loading the sidebar view qml component");
    }
    const int rootImplicitWidth = item->implicitWidth();
    if (rootImplicitWidth != 0) {
        d->quickWidget->setFixedWidth(rootImplicitWidth);
    } else {
        d->quickWidget->setFixedWidth(240);
    }

    setHeaderHeight(item->property("headerHeight").toReal());

    d->quickWidget->setContent(QUrl(), nullptr, item);
    connect(item, &QQuickItem::implicitWidthChanged, this, [this]() {
        const int rootImplicitWidth = d->quickWidget->rootObject()->property("implicitWidth").toInt();
        if (rootImplicitWidth != 0) {
            d->quickWidget->setFixedWidth(rootImplicitWidth);
        } else {
            d->quickWidget->setFixedWidth(240);
        }
    });

    d->quickWidget->installEventFilter(this);

    d->mainLayout->addWidget(d->quickWidget);
    d->moduleView->hide();
    d->mainLayout->addWidget(d->moduleView);

    if (!startupModule().isEmpty()) {
        initPlaceHolderWidget();
        d->placeHolderWidget->show();
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            loadModule(d->model->indexForItem(item), startupModuleArgs());
        }
    } else if (homeItem()) {
        d->moduleView->show();
        QModelIndex index = d->categorizedModel->mapFromSource(d->model->indexForItem(homeItem()));
        if (index.isValid()) {
            loadModule(index);
        } else {
            d->moduleView->loadModule(d->model->indexForItem(homeItem()), QStringList());
        }
    } else {
        initPlaceHolderWidget();
        d->placeHolderWidget->show();
    }
}

void SidebarMode::initPlaceHolderWidget()
{
    d->placeHolderWidget = new QQuickWidget(d->engine.get(), d->mainWidget);
    d->placeHolderWidget->quickWindow()->setTitle(i18n("Most Used"));
    d->placeHolderWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->placeHolderWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(d->placeHolderWidget));
    d->placeHolderWidget->engine()->rootContext()->setContextProperty(QStringLiteral("systemsettings"), this);
    d->placeHolderWidget->setSource(QUrl(QStringLiteral("qrc:/qt/qml/org/kde/systemsettings/IntroPage.qml")));
    d->placeHolderWidget->installEventFilter(this);

    d->mainLayout->addWidget(d->placeHolderWidget);
}

void SidebarMode::reloadStartupModule()
{
    if (!startupModule().isEmpty()) {
        MenuItem *item = rootItem()->descendantForModule(startupModule());
        if (item) {
            loadModule(d->model->indexForItem(item), startupModuleArgs());
        }
    }
}

bool SidebarMode::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != d->quickWidget && watched != d->mainWidget) {
        return QObject::eventFilter(watched, event);
    }

    // FIXME: those are all workarounds around the QQuickWidget brokenness
    switch (event->type()) {
    case QEvent::KeyPress: {
        // allow tab navigation inside the qquickwidget
        auto ke = static_cast<QKeyEvent *>(event);
        auto qqw = qobject_cast<QQuickWidget *>(watched);
        // Do nothing if qqw is nullptr, otherwise this would crash
        // This can happen if user has pressed ALT or other modifier key
        // to enable the quick shortcuts and there is a Widget on the same page.
        // BUG: 480006
        if (qqw == nullptr) {
            return false;
        }
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            QCoreApplication::sendEvent(qqw->quickWindow(), event);
            return true;
        }
        break;
    }

    case QEvent::Leave: {
        if (watched == d->quickWidget) {
            QCoreApplication::sendEvent(d->quickWidget->quickWindow(), event);
        }
        break;
    }

    case QEvent::Resize: {
        if (watched == d->mainWidget) {
            Q_EMIT widthChanged();
        }
        break;
    }

    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

void SidebarMode::giveFocus()
{
    d->quickWidget->setFocus();
}

MenuItem *SidebarMode::rootItem() const
{
    return d->rootItem;
}

MenuItem *SidebarMode::homeItem() const
{
    return d->homeItem;
}

void SidebarMode::setStartupModule(const QString &startupModule)
{
    d->startupModule = startupModule;
}

QString SidebarMode::startupModule() const
{
    return d->startupModule;
}

void SidebarMode::setStartupModuleArgs(const QStringList &startupModuleArgs)
{
    d->startupModuleArgs = startupModuleArgs;
}

QStringList SidebarMode::startupModuleArgs() const
{
    return d->startupModuleArgs;
}

#include "moc_SidebarMode.cpp"
