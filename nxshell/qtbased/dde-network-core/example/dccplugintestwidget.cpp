// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dccplugintestwidget.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <DGuiApplicationHelper>
#include <DPalette>
#include <QPainter>
#include <DApplicationHelper>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QScrollArea>
#include <DTitlebar>
#include <DPaletteHelper>

#include <QLineEdit>
#include <QScroller>
#include <QObject>
#include <QTimer>

#include "dccnetworkmodule.h"
#include "moduledatamodel.h"
#include <DBackgroundGroup>
#include <DListView>
#include <DStyledItemDelegate>
#include <DFrame>
#include <QScreen>
#include <QScrollBar>
#include <QLabel>

typedef DListView ListView;
typedef DStyledItemDelegate ListItemDelegate;

const QSize MainWindowMininumSize(QSize(800, 600));
const QMargins ZeroMargins(0, 0, 0, 0);
const int NavViewMaximumWidth = QWIDGETSIZE_MAX;
const int NavViewMinimumWidth = 188;
const QSize ListViweItemIconSize_IconMode(84, 84);
const QSize ListViweItemSize_IconMode(280, 84);
const QSize ListViweItemIconSize_ListMode(32, 32);
const QSize ListViweItemSize_ListMode(168, 48);

static int WidgetMinimumWidth = 820;
static int WidgetMinimumHeight = 634;

const int second_widget_min_width = 230;
const int third_widget_min_width = 340;
const int widget_total_min_width = 820;
const int four_widget_min_widget = widget_total_min_width + third_widget_min_width + 40;
const int first_widget_min_width = 188;
const qint32 ActionIconSize = 30; //大图标角标大小

const QSize ListViweItemIconSize(84, 84);
const QSize ListViweItemSize(170, 168);

Q_DECLARE_METATYPE(QMargins)
const QMargins navItemMargin(5, 3, 5, 3);
const QVariant NavItemMargin = QVariant::fromValue(navItemMargin);

//////////////////////////////////////////
namespace DCC_NAMESPACE {

class MainModulePrivate
{
public:
    explicit MainModulePrivate(MainModule *parent = nullptr)
        : q_ptr(parent)
        , m_view(nullptr)
        , m_layout(nullptr)
    {
    }

    void onCurrentModuleChanged(ModuleObject *child)
    {
        if (!m_layout)
            return;
        if (m_layout->count() > 1) {
            QLayoutItem *item = m_layout->takeAt(1);
            delete item->widget();
            delete item;
        }
        if (child) {
            m_view->setViewMode(QListView::ListMode);
            m_view->setIconSize(ListViweItemIconSize_ListMode);
            m_view->setGridSize(ListViweItemSize_ListMode);
            m_view->setSpacing(0);
            m_layout->addWidget(child->activePage(), 5);
            ModuleDataModel *model = static_cast<ModuleDataModel *>(m_view->model());
        } else {
            m_view->setViewMode(QListView::IconMode);
            m_view->setIconSize(ListViweItemIconSize_IconMode);
            m_view->setGridSize(ListViweItemSize_IconMode);
            m_view->setSpacing(20);
        }
    }

    QWidget *page()
    {
        Q_Q(MainModule);
        QWidget *parentWidget = new QWidget();
        m_layout = new QHBoxLayout(parentWidget);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        parentWidget->setLayout(m_layout);

        m_view = new ListView(parentWidget);
        ListItemDelegate *delegate = new ListItemDelegate(m_view);
        m_view->setItemDelegate(delegate);
        ModuleDataModel *model = new ModuleDataModel(m_view);
        model->setModuleObject(q);
        QObject::connect(q, &MainModule::currentModuleChanged, m_layout, [this](ModuleObject *child) {
            onCurrentModuleChanged(child);
        });
        m_view->setModel(model);
        m_view->setFrameShape(QFrame::Shape::NoFrame);
        m_view->setAutoScroll(true);
        m_view->setDragEnabled(false);
        m_view->setMaximumWidth(NavViewMaximumWidth);
        m_view->setMinimumWidth(NavViewMinimumWidth);
        m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_view->setSelectionMode(QAbstractItemView::SingleSelection);
        m_view->setAcceptDrops(false);

        auto onClicked = [](const QModelIndex &index) {
            ModuleObject *obj = static_cast<ModuleObject *>(index.internalPointer());
            if (obj)
                obj->trigger();
        };

        QObject::connect(m_view, &ListView::activated, m_view, &ListView::clicked);
        QObject::connect(m_view, &ListView::clicked, m_view, onClicked);
        m_layout->addWidget(m_view, 1);

        onCurrentModuleChanged(q->currentModule());
        return parentWidget;
    }

private:
    MainModule *q_ptr;
    Q_DECLARE_PUBLIC(MainModule)

    DListView *m_view;
    QHBoxLayout *m_layout;
};

}

MainModule::MainModule(QObject *parent)
    : ModuleObject(parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QString &displayName, QObject *parent)
    : ModuleObject(name, displayName, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QStringList &contentText, QObject *parent)
    : ModuleObject(name, contentText, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QString &displayName, const QStringList &contentText, QObject *parent)
    : ModuleObject(name, displayName, contentText, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QString &displayName, const QVariant &icon, QObject *parent)
    : ModuleObject(name, displayName, icon, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QString &displayName, const QString &description, QObject *parent)
    : ModuleObject(name, displayName, description, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QString &displayName, const QString &description, const QVariant &icon, QObject *parent)
    : ModuleObject(name, displayName, description, icon, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QString &displayName, const QString &description, const QIcon &icon, QObject *parent)
    : ModuleObject(name, displayName, description, icon, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::MainModule(const QString &name, const QString &displayName, const QString &description, const QStringList &contentText, const QVariant &icon, QObject *parent)
    : ModuleObject(name, displayName, description, contentText, icon, parent)
    , d_ptr(new MainModulePrivate(this))
{
}

MainModule::~MainModule()
{
}

void MainModule::appendChild(ModuleObject *const module)
{
    ModuleObject::appendChild(module);
}

QWidget *MainModule::page()
{
    Q_D(MainModule);
    return d->page();
}

ModuleObject *MainModule::defultModule()
{
    return nullptr;
}
//////////////////////////////////////////

DccPluginTestWidget::DccPluginTestWidget(QWidget *parent)
    : DMainWindow(parent)
    , m_backwardBtn(new DIconButton(QStyle::SP_ArrowBack, this))
    , m_rootModule(new MainModule())
    , m_mainView(nullptr)
{
    qRegisterMetaType<ModuleObject *>("ModuleObject *");

    initUI();
    initConfig();
    loadModules();
}

DccPluginTestWidget::~DccPluginTestWidget()
{
}

int DccPluginTestWidget::getScrollPos(const int index)
{
    int pos = 0;
    for (int i = 0; i < qMin<int>(m_pages.count(), index); i++) {
        pos += m_pages[i]->height();
    }
    return pos;
}

void DccPluginTestWidget::showPage(const QString &url, const UrlType &uType)
{
    qInfo() << "show page url:" << url;
    if (url.isEmpty() || url == "/") {
        toHome();
    }
    if (!m_rootModule) {
        QTimer::singleShot(10, this, [=] {
            showPage(url, uType);
        });
        return;
    }
    showPage(m_rootModule, url, uType);
}

void DccPluginTestWidget::showPage(ModuleObject *module, const QString &url, const UrlType &uType)
{
    QStringList names = url.split('/');
    const QString &name = names.takeFirst();
    int index = -1;
    QString childName;
    for (auto child : module->childrens()) {
        if (uType == UrlType::Name)
            childName = child->name();
        if (uType == UrlType::DisplayName)
            childName = child->displayName();
        if (childName == name || child->contentText().contains(name)) {
            index = module->childrens().indexOf(child);
            return showPage(child, names.join('/'), uType);
        }
    }
}

void DccPluginTestWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        updateMainView();
    }
    return DMainWindow::changeEvent(event);
}

void DccPluginTestWidget::initUI()
{
    setMinimumSize(MainWindowMininumSize);
    setCentralWidget(m_rootModule->activePage());

    layout()->setMargin(0);
    layout()->setSpacing(0);
    layout()->setContentsMargins(ZeroMargins);

    auto menu = titlebar()->menu();
    if (!menu) {
        qDebug() << "menu is nullptr, create menu!";
        menu = new QMenu;
    }
    menu->setAccessibleName("titlebarmenu");
    titlebar()->setMenu(menu);

    auto action = new QAction(tr("Help"), menu);
    menu->addAction(action);
    connect(action, &QAction::triggered, this, [this] {
        openManual();
    });

    m_backwardBtn->setAccessibleName("backwardbtn");

    titlebar()->addWidget(m_backwardBtn, Qt::AlignLeft | Qt::AlignVCenter);
    titlebar()->setIcon(QIcon::fromTheme("preferences-system"));

    connect(m_backwardBtn, &DIconButton::clicked, this, &DccPluginTestWidget::toHome);
}

void DccPluginTestWidget::initConfig()
{
}

void DccPluginTestWidget::loadModules()
{
    QFutureWatcher<ModuleObject *> *watcher = new QFutureWatcher<ModuleObject *>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher] {
        onAddModule(m_rootModule);
        m_rootModule->appendChild(watcher->result());
        showModule(m_rootModule);
    });

    QFuture<ModuleObject *> future = QtConcurrent::run([this] {
        DccNetworkPlugin *net = new DccNetworkPlugin();
        ModuleObject *module = net->module();
        module->moveToThread(qApp->thread());
        return module;
    });
    watcher->setFuture(future);
}

void DccPluginTestWidget::toHome()
{
    showModule(m_rootModule);
}

void DccPluginTestWidget::updateMainView()
{
    if (!m_mainView)
        return;
    // set background
    DPalette pa = DPaletteHelper::instance()->palette(m_mainView);
    QColor baseColor = palette().base().color();
    DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
    if (ct == DGuiApplicationHelper::LightType) {
        pa.setBrush(DPalette::ItemBackground, palette().base());
    } else {
        baseColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, +5, 0, 0, 0, 0);
        pa.setColor(DPalette::ItemBackground, baseColor);
    }
    DPaletteHelper::instance()->setPalette(m_mainView, pa);
}

void DccPluginTestWidget::clearPage(QWidget *const widget)
{
    QLayout *layout = widget->layout();
    QScrollArea *area = qobject_cast<QScrollArea *>(widget);
    if (area)
        area->widget()->deleteLater();
    if (layout) {
        while (QLayoutItem *child = layout->takeAt(0)) {
            layout->removeWidget(child->widget());
            child->widget()->deleteLater();
            layout->removeItem(child);
            delete child;
        }
        delete layout;
    }
}

void DccPluginTestWidget::configLayout(QBoxLayout *const layout)
{
    layout->setMargin(0);
    layout->setSpacing(0);
}

void DccPluginTestWidget::showModule(ModuleObject *module, const int index)
{
    if (m_currentModule.contains(module) && module->defultModule())
        return;

    m_backwardBtn->setEnabled(module != m_rootModule);
    QList<ModuleObject *> modules;
    ModuleObject *child = module;
    while (child) {
        child->setCurrentModule(child->defultModule());
        modules.append(child);
        child = child->currentModule();
    }
    child = module;
    ModuleObject *parent = module->getParent();
    while (parent) {
        parent->setCurrentModule(child);
        modules.prepend(parent);
        child = parent;
        parent = parent->getParent();
    }
    m_currentModule = modules;
}

QWidget *DccPluginTestWidget::getPage(QWidget *const widget, const QString &title)
{
    if (!widget)
        return nullptr;
    QLabel *titleLbl = new QLabel(title, this);
    QWidget *page = new QWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(page);
    page->setLayout(vLayout);
    vLayout->addWidget(titleLbl, 0, Qt::AlignTop);
    vLayout->addWidget(widget, 1, Qt::AlignTop);
    if (title.isEmpty()) {
        titleLbl->setVisible(false);
    }
    return page;
}

QWidget *DccPluginTestWidget::getExtraPage(QWidget *const widget)
{
    QWidget *tmpWidget = new QWidget(this);
    QVBoxLayout *vLayout = new QVBoxLayout(tmpWidget);
    tmpWidget->setLayout(vLayout);
    vLayout->addWidget(widget);
    vLayout->setSpacing(0);
    return tmpWidget;
}

void DccPluginTestWidget::onAddModule(ModuleObject *const module)
{
    QString url;

    QList<QPair<QString, ModuleObject *>> modules = { { url, module } };
    while (!modules.isEmpty()) {
        auto it = modules.takeFirst();
        ModuleObject *moduleObject = it.second;
        connect(moduleObject, &ModuleObject::insertedChild, this, &DccPluginTestWidget::onAddModule);
        connect(moduleObject, &ModuleObject::removedChild, this, &DccPluginTestWidget::onRemoveModule);
        connect(moduleObject, &ModuleObject::childStateChanged, this, &DccPluginTestWidget::onChildStateChanged);
        connect(moduleObject, &ModuleObject::triggered, this, &DccPluginTestWidget::onTriggered, Qt::QueuedConnection);
        for (auto &&tmpObj : moduleObject->childrens()) {
            modules.append({ it.first + "/" + tmpObj->name(), tmpObj });
        }
    }
}

void DccPluginTestWidget::onRemoveModule(ModuleObject *module)
{
    QList<ModuleObject *> modules;
    modules.append(module);
    while (!modules.isEmpty()) {
        ModuleObject *moduleObject = modules.takeFirst();
        disconnect(moduleObject, nullptr, this, nullptr);
        modules.append(moduleObject->childrens());
    }
    // 最后一个是滚动到，不参与比较
    if (!m_currentModule.isEmpty() && std::any_of(m_currentModule.cbegin(), m_currentModule.cend() - 1, [module](ModuleObject *data) { return data == module; }))
        toHome();
}

void DccPluginTestWidget::onTriggered()
{
    ModuleObject *module = qobject_cast<ModuleObject *>(sender());
    if (module)
        showModule(module);
}

void DccPluginTestWidget::onChildStateChanged(ModuleObject *child, uint32_t flag, bool state)
{
    if (ModuleObject::IsHiddenFlag(flag)) {
        if (state)
            onRemoveModule(child);
        else
            onAddModule(child);
    }
}

void DccPluginTestWidget::openManual()
{
}
