

// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "system_service_table_view.h"

#include "application.h"
#include "main_window.h"
#include "dialog/error_dialog.h"
#include "service_name_sub_input_dialog.h"
#include "common/error_context.h"
#include "settings.h"
#include "toolbar.h"

#include "model/system_service_sort_filter_proxy_model.h"
#include "model/system_service_table_model.h"

#include "dbus/systemd1_unit_interface.h"

#include "service/service_manager.h"
#include "service/system_service_entry.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DHeaderView>
#include <DLabel>
#include <DMenu>
#include <DMessageBox>
#include <DSpinner>

#include <QScrollBar>
#include <QtConcurrent>
#include <QKeyEvent>
#include <QShortcut>
#include <QDebug>

DWIDGET_USE_NAMESPACE

// service table view backup setting key
static const char *kSettingsOption_ServiceTableHeaderState = "service_table_header_state";

// multithread unsafe
static bool defer_initialized {false};

// constructor
SystemServiceTableView::SystemServiceTableView(DWidget *parent)
    : BaseTableView(parent)
{
    // install event handler for service table to handle key events
    installEventFilter(this);

    // service model & sort filter proxy model
    m_model = new SystemServiceTableModel(this);
    m_proxyModel = new SystemServiceSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    setModel(m_proxyModel);

    // load backup settings
    bool settingsLoaded = loadSettings();

    // initialize ui components & connections
    initUI(settingsLoaded);
    initConnections();

    QTimer::singleShot(100, this, SLOT(onLoadServiceDataList()));
}

// destructor
SystemServiceTableView::~SystemServiceTableView()
{
    // backup settings when quit
    saveSettings();
}

// backup service table view settings
void SystemServiceTableView::saveSettings()
{
    Settings *s = Settings::instance();
    if (s) {
        QByteArray buf = header()->saveState();
        s->setOption(kSettingsOption_ServiceTableHeaderState, buf.toBase64());
        s->flush();
    }
}

// load service table view settings from backup storage
bool SystemServiceTableView::loadSettings()
{
    Settings *s = Settings::instance();
    if (s) {
        QVariant opt = s->getOption(kSettingsOption_ServiceTableHeaderState);
        if (opt.isValid()) {
            QByteArray buf = QByteArray::fromBase64(opt.toByteArray());
            header()->restoreState(buf);
            return true;
        }
    }
    return false;
}

// show service table header context menu
void SystemServiceTableView::displayHeaderContextMenu(const QPoint &p)
{
    m_headerContextMenu->popup(mapToGlobal(p));
}

// show service table context menu
void SystemServiceTableView::displayTableContextMenu(const QPoint &p)
{
    if (selectedIndexes().size() == 0)
        return;

    QPoint point = mapToGlobal(p);
    // when popup context menu for items, take table header height into consideration
    point.setY(point.y() + header()->sizeHint().height());
    m_contextMenu->popup(point);
}

// start service handler
void SystemServiceTableView::startService()
{
    // no selected item, do nothing
    if (!m_selectedSName.isValid())
        return;

    auto sname = m_selectedSName.toString();

    // request service sub name if origin service name ends  with '@'
    if (sname.endsWith('@')) {
        ServiceNameSubInputDialog dialog(this);
        dialog.setTitle(
            DApplication::translate("Service.Instance.Name.Dialog", "Service instance name"));
        auto desc = selectedIndexes()
                    .value(SystemServiceTableModel::kSystemServiceDescriptionColumn)
                    .data()
                    .toString();
        dialog.setMessage(desc);
        dialog.exec();
        if (dialog.result() == QMessageBox::Ok) {
            // no service sub name given, do nothing
            auto subName = dialog.getServiceSubName();
            if (subName.isEmpty())
                return;
            sname = sname.append(subName);
        } else {  // cancel clicked
            return;
        }
    }

    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr != nullptr);

    // start service asynchronically
    auto *watcher  = new QFutureWatcher<ErrorContext>();
    connect(watcher, &QFutureWatcher<ErrorContext>::finished, this, [this, mgr, watcher, sname]() {
        refreshServiceStatus(sname);
        // show error dialog when error occurred
        if (watcher->result()) {
            Q_EMIT mgr->errorOccurred(watcher->result());
        }
        // reset global background task state
        gApp->backgroundTaskStateChanged(Application::kTaskFinished);
        watcher->deleteLater();
    });
    QFuture<ErrorContext> fu = QtConcurrent::run([mgr, sname]() {
        auto ec = mgr->startService(sname);
        return ec;
    });
    // change global background task state to running state, so we can enable/disable ui components accordingly
    gApp->backgroundTaskStateChanged(Application::kTaskStarted);
    // run async job
    watcher->setFuture(fu);
}

// stop service handler
void SystemServiceTableView::stopService()
{
    // no selected item, do nothing
    if (!m_selectedSName.isValid()) {
        return;
    }

    auto sname = m_selectedSName.toString();

    // service id syntax: xxx@, requires service sub name from user if origin service name ends with '@'
    if (sname.endsWith('@')) {
        ServiceNameSubInputDialog dialog(this);
        dialog.setTitle(
            DApplication::translate("Service.Instance.Name.Dialog", "Service instance name"));
        auto desc = selectedIndexes()
                    .value(SystemServiceTableModel::kSystemServiceDescriptionColumn)
                    .data()
                    .toString();
        dialog.setMessage(desc);
        dialog.exec();
        if (dialog.result() == QMessageBox::Ok) {
            auto subName = dialog.getServiceSubName();
            // no service sub name given, do nothing
            if (subName.isEmpty())
                return;
            sname = sname.append(subName);
        } else {
            return;
        }
    }

    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr != nullptr);

    // stop service in asynchronize way
    auto *watcher  = new QFutureWatcher<ErrorContext>();
    connect(watcher, &QFutureWatcher<ErrorContext>::finished, this, [this, mgr, watcher, sname]() {
        refreshServiceStatus(sname);
        // show error dialog when error occurred
        if (watcher->result()) {
            Q_EMIT mgr->errorOccurred(watcher->result());
        }
        // reset global background task state
        gApp->backgroundTaskStateChanged(Application::kTaskFinished);
        watcher->deleteLater();
    });
    QFuture<ErrorContext> fu = QtConcurrent::run([mgr, sname]() {
        auto ec = mgr->stopService(sname);
        return ec;
    });
    // change global background task state to running state, so we can enable/disable ui components accordingly
    gApp->backgroundTaskStateChanged(Application::kTaskStarted);
    // run async job
    watcher->setFuture(fu);
}

// restart service handler
void SystemServiceTableView::restartService()
{
    // no selected item, do nothing
    if (!m_selectedSName.isValid()) {
        return;
    }

    auto sname = m_selectedSName.toString();

    // service id syntax: xxx@, requires service sub name from user if origin service name ends with '@'
    if (sname.endsWith('@')) {
        ServiceNameSubInputDialog dialog(this);
        dialog.setTitle(
            DApplication::translate("Service.Instance.Name.Dialog", "Service instance name"));
        auto desc = selectedIndexes()
                    .value(SystemServiceTableModel::kSystemServiceDescriptionColumn)
                    .data()
                    .toString();
        dialog.setMessage(desc);
        dialog.exec();
        if (dialog.result() == QMessageBox::Ok) {
            auto subName = dialog.getServiceSubName();
            // no service sub name given, do nothing
            if (subName.isEmpty())
                return;
            sname = sname.append(subName);
        } else {
            return;
        }
    }

    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr != nullptr);

    // restart service in asynchronize way
    auto *watcher  = new QFutureWatcher<ErrorContext>();
    connect(watcher, &QFutureWatcher<ErrorContext>::finished, this, [this, mgr, watcher, sname]() {
        refreshServiceStatus(sname);
        // show error dialog when error occurred
        if (watcher->result()) {
            Q_EMIT mgr->errorOccurred(watcher->result());
        }
        // reset global background task state
        gApp->backgroundTaskStateChanged(Application::kTaskFinished);
        watcher->deleteLater();
    });
    QFuture<ErrorContext> fu = QtConcurrent::run([mgr, sname]() {
        auto ec = mgr->restartService(sname);
        return ec;
    });
    // change global background task state to running state, so we can enable/disable ui components accordingly
    gApp->backgroundTaskStateChanged(Application::kTaskStarted);
    // run async job
    watcher->setFuture(fu);
}

// set service startup mode handler
void SystemServiceTableView::setServiceStartupMode(bool autoStart)
{
    // no selected item, do nothing
    if (!m_selectedSName.isValid()) {
        return;
    }

    auto sname = m_selectedSName.toString();

    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr != nullptr);

    // set service startup mode in asynchronize way
    auto *watcher  = new QFutureWatcher<ErrorContext>();
    connect(watcher, &QFutureWatcher<ErrorContext>::finished, this, [this, mgr, watcher, sname]() {
        refreshServiceStatus(sname);
        // show error dialog when error occurred
        if (watcher->result()) {
            Q_EMIT mgr->errorOccurred(watcher->result());
        }
        // reset global background task state
        gApp->backgroundTaskStateChanged(Application::kTaskFinished);
        watcher->deleteLater();
    });
    QFuture<ErrorContext> fu = QtConcurrent::run([mgr, sname, autoStart]() {
        auto ec = mgr->setServiceStartupMode(sname, autoStart);
        return ec;
    });
    // change global background task state to running state, so we can enable/disable ui components accordingly
    gApp->backgroundTaskStateChanged(Application::kTaskStarted);
    // run async job
    watcher->setFuture(fu);
}

// adjust search result tip label's visibility & positon
void SystemServiceTableView::adjustInfoLabelVisibility()
{
    setUpdatesEnabled(false);
    // show label only when proxy model is empty, and spinner is not running
    m_noMatchingResultLabel->setVisible(!m_proxyModel->rowCount() &&
                                        m_spinner &&
                                        !m_spinner->isPlaying());
    // move tip label to center of the service table view
    if (m_noMatchingResultLabel->isVisible())
        m_noMatchingResultLabel->move(
            rect().center() - m_noMatchingResultLabel->rect().center());
    setUpdatesEnabled(true);
}

// refresh servcie status if service gets updated
void SystemServiceTableView::refreshServiceStatus(const QString sname)
{
    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr != nullptr);

    auto sn = mgr->normalizeServiceId(sname);
    auto o = Systemd1UnitInterface::normalizeUnitPath(sn);
    // get refreshed service entry
    auto entry = mgr->updateServiceEntry(o.path());

    // if service's active state is in none final state, start a timer to pull new state Periodically
    if (!isFinalState(entry.getActiveState().toLocal8Bit())) {
        auto *timer = new CustomTimer(mgr, this);
        timer->start(o.path());
    }
}

// filter service on specific pattern
void SystemServiceTableView::search(const QString &pattern)
{
    m_proxyModel->setFilterRegExp(QRegExp(pattern, Qt::CaseInsensitive));

    // adjust search result tip label's position & visibility
    adjustInfoLabelVisibility();
}

// resize event handler
void SystemServiceTableView::resizeEvent(QResizeEvent *event)
{
    // move spinner to center while resizing
    if (m_spinner) {
        m_spinner->move(rect().center() - m_spinner->rect().center());
    }
    // adjust search result tip label's position & visibility
    adjustInfoLabelVisibility();

    BaseTableView::resizeEvent(event);
}

// selection changed handler
void SystemServiceTableView::selectionChanged(const QItemSelection &selected,
                                              const QItemSelection &deselected)
{
    if (selected.size() <= 0) {
        return;
    }

    // backup currently selected item's service name
    m_selectedSName = selected.indexes()
                      .value(SystemServiceTableModel::kSystemServiceNameColumn)
                      .data();

    BaseTableView::selectionChanged(selected, deselected);
}

// initialize ui components
void SystemServiceTableView::initUI(bool settingsLoaded)
{
    setAccessibleName("SystemServiceTableView");

    // search result tip label instance
    m_noMatchingResultLabel =
        new DLabel(DApplication::translate("Common.Search", "No search results"), this);
    DFontSizeManager::instance()->bind(m_noMatchingResultLabel, DFontSizeManager::T4);
    // set text color
    auto palette = DApplicationHelper::instance()->palette(m_noMatchingResultLabel);
    QColor labelColor = palette.color(DPalette::PlaceholderText);
    palette.setColor(DPalette::Text, labelColor);
    m_noMatchingResultLabel->setPalette(palette);
    m_noMatchingResultLabel->setVisible(false);
    // header view instance
    auto *hdr = header();
    // header section movable
    hdr->setSectionsMovable(true);
    // header section clickable
    hdr->setSectionsClickable(true);
    // header section resizable
    hdr->setSectionResizeMode(DHeaderView::Interactive);
    // stretch last section
    hdr->setStretchLastSection(true);
    // show sort indicator on sort column
    hdr->setSortIndicatorShown(true);
    // section default aligment
    hdr->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // header view context menu policy
    hdr->setContextMenuPolicy(Qt::CustomContextMenu);

    // table options
    // enable sorting
    setSortingEnabled(true);
    // only single row selection allowed
    setSelectionMode(QAbstractItemView::SingleSelection);
    // select whole row
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // service table view context menu policy
    setContextMenuPolicy(Qt::CustomContextMenu);

    // set service table default style when backup settings can not be loaded
    if (!settingsLoaded)
    {
        setColumnWidth(SystemServiceTableModel::kSystemServiceNameColumn, 200);
        setColumnHidden(SystemServiceTableModel::kSystemServiceNameColumn, false);
        setColumnWidth(SystemServiceTableModel::kSystemServiceLoadStateColumn, 100);
        setColumnHidden(SystemServiceTableModel::kSystemServiceLoadStateColumn, true);
        setColumnWidth(SystemServiceTableModel::kSystemServiceActiveStateColumn, 100);
        setColumnHidden(SystemServiceTableModel::kSystemServiceActiveStateColumn, false);
        setColumnWidth(SystemServiceTableModel::kSystemServiceSubStateColumn, 100);
        setColumnHidden(SystemServiceTableModel::kSystemServiceSubStateColumn, false);
        setColumnWidth(SystemServiceTableModel::kSystemServiceStateColumn, 100);
        setColumnHidden(SystemServiceTableModel::kSystemServiceStateColumn, false);
        setColumnWidth(SystemServiceTableModel::kSystemServiceStartupModeColumn, 80);
        setColumnHidden(SystemServiceTableModel::kSystemServiceStartupModeColumn, true);
        setColumnWidth(SystemServiceTableModel::kSystemServiceDescriptionColumn, 320);
        setColumnHidden(SystemServiceTableModel::kSystemServiceDescriptionColumn, false);
        setColumnWidth(SystemServiceTableModel::kSystemServicePIDColumn, 100);
        setColumnHidden(SystemServiceTableModel::kSystemServicePIDColumn, true);
        sortByColumn(SystemServiceTableModel::kSystemServiceNameColumn, Qt::AscendingOrder);
    }

    // service table view context menu instance
    m_contextMenu = new DMenu(this);
    // start service action
    m_startServiceAction =
        m_contextMenu->addAction(DApplication::translate("Service.Table.Context.Menu", "Start"));
    // ALT + S
    m_startServiceAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
    // stop service action
    m_stopServiceAction =
        m_contextMenu->addAction(DApplication::translate("Service.Table.Context.Menu", "Stop"));
    // ALT + T
    m_stopServiceAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_T));
    // restart service action
    m_restartServiceAction =
        m_contextMenu->addAction(DApplication::translate("Service.Table.Context.Menu", "Restart"));
    // ALT + R
    m_restartServiceAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_R));
    // service startup mode action
    m_setServiceStartupModeMenu =
        m_contextMenu->addMenu(DApplication::translate("Service.Table.Context.Menu", "Startup type"));
    m_setAutoStartAction = m_setServiceStartupModeMenu->addAction(DApplication::translate("Service.Table.Context.Menu", "Auto"));
    m_setManualStartAction = m_setServiceStartupModeMenu->addAction(DApplication::translate("Service.Table.Context.Menu", "Manual"));
    // refresh context menu item
    m_refreshAction =
        m_contextMenu->addAction(DApplication::translate("Service.Table.Context.Menu", "Refresh"));
    // F5
    m_refreshAction->setShortcut(QKeySequence(QKeySequence::Refresh));

    // >>> header context menu instance
    m_headerContextMenu = new DMenu(this);
    // load state column action
    m_loadStateHeaderAction = m_headerContextMenu->addAction(
                                    DApplication::translate("Service.Table.Header", kSystemServiceLoadState));
    m_loadStateHeaderAction->setCheckable(true);
    // active state column action
    m_activeStateHeaderAction = m_headerContextMenu->addAction(
                                    DApplication::translate("Service.Table.Header", kSystemServiceActiveState));
    m_activeStateHeaderAction->setCheckable(true);
    // sub state column action
    m_subStateHeaderAction = m_headerContextMenu->addAction(
                                    DApplication::translate("Service.Table.Header", kSystemServiceSubState));
    m_subStateHeaderAction->setCheckable(true);
    // state column action
    m_stateHeaderAction = m_headerContextMenu->addAction(
                                    DApplication::translate("Service.Table.Header", kSystemServiceState));
    m_stateHeaderAction->setCheckable(true);
    // description column action
    m_descriptionHeaderAction = m_headerContextMenu->addAction(
                                    DApplication::translate("Service.Table.Header", kSystemServiceDescription));
    m_descriptionHeaderAction->setCheckable(true);
    // pid column
    m_pidHeaderAction = m_headerContextMenu->addAction(
                                    DApplication::translate("Service.Table.Header", kSystemServicePID));
    m_pidHeaderAction->setCheckable(true);
    // startup mode column
    m_startupModeHeaderAction = m_headerContextMenu->addAction(
                                    DApplication::translate("Service.Table.Header", kSystemServiceStartupMode));
    m_startupModeHeaderAction->setCheckable(true);

    // set default checkable state when backup settings load without success
    if (!settingsLoaded) {
        m_loadStateHeaderAction->setChecked(false);
        m_activeStateHeaderAction->setChecked(true);
        m_subStateHeaderAction->setChecked(true);
        m_stateHeaderAction->setChecked(true);
        m_descriptionHeaderAction->setChecked(true);
        m_pidHeaderAction->setChecked(false);
    }

    // refresh service table shortcut
    m_refreshKP = new QShortcut(QKeySequence(QKeySequence::Refresh), this);
    // start service shortcut
    m_startKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_S), this);
    // stop service shortcut
    m_stopKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_T), this);
    // restart service shortcut
    m_restartKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_R), this);

    // spinner instance
    m_spinner = new DSpinner(this);
    auto pa = DApplicationHelper::instance()->applicationPalette();
    // set spinner color
    QBrush hlBrush = pa.color(DPalette::Active, DPalette::Highlight);
    pa.setColor(DPalette::Highlight, hlBrush.color());
    m_spinner->setPalette(pa);
    // move spinner to center of the viewport
    m_spinner->move(rect().center() - m_spinner->rect().center());
}

// initialize connections
void SystemServiceTableView::initConnections()
{
    auto *dAppHelper = DApplicationHelper::instance();
    Q_ASSERT(dAppHelper != nullptr);
    // change search result tip label text color when theme type changed
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this, [ = ]() {
        auto palette = DApplicationHelper::instance()->applicationPalette();
        if (m_noMatchingResultLabel) {
            QColor labelColor = palette.color(DPalette::PlaceholderText);
            palette.setColor(DPalette::Text, labelColor);
            m_noMatchingResultLabel->setPalette(palette);
        }
        auto pa = DApplicationHelper::instance()->applicationPalette();
        // set spinner color
        QBrush hlBrush = pa.color(DPalette::Active, DPalette::Highlight);
        pa.setColor(DPalette::Highlight, hlBrush.color());
        m_spinner->setPalette(pa);
    });

    // show context menu when custom context menu requested
    connect(this, &SystemServiceTableView::customContextMenuRequested, this,
            &SystemServiceTableView::displayTableContextMenu);

    // header view instance
    auto *hdr = header();
    // backup settings when any of the following events happened
    connect(hdr, &QHeaderView::sectionResized, this, [ = ]() { saveSettings(); });
    connect(hdr, &QHeaderView::sectionMoved, this, [ = ]() { saveSettings(); });
    connect(hdr, &QHeaderView::sortIndicatorChanged, this, [ = ]() { saveSettings(); });
    // show header context menu
    connect(hdr, &QHeaderView::customContextMenuRequested, this,
            &SystemServiceTableView::displayHeaderContextMenu);

    connect(gApp->mainWindow()->toolbar(), &Toolbar::search, this, &SystemServiceTableView::search);

    // call start service handler when start service menu item triggered
    connect(m_startServiceAction, &QAction::triggered, this, &SystemServiceTableView::startService);
    // call stop service handler when stop service menu item triggered
    connect(m_stopServiceAction, &QAction::triggered, this, &SystemServiceTableView::stopService);
    // call restart service handler when restart service menu item triggered
    connect(m_restartServiceAction, &QAction::triggered, this, &SystemServiceTableView::restartService);
    // call set service startup mode handler when set auto startup menu item triggered
    connect(m_setAutoStartAction, &QAction::triggered, this, [ = ]() { setServiceStartupMode(true); });
    // call set service startup mode handler when set manual startup menu item triggered
    connect(m_setManualStartAction, &QAction::triggered, this, [ = ]() { setServiceStartupMode(false); });
    // call refresh handler when refresh menu item triggered
    connect(m_refreshAction, &QAction::triggered, this, &SystemServiceTableView::refresh);

    // change context menu item usable state based on service's current state when popup
    connect(m_contextMenu, &QMenu::aboutToShow, this, [ = ]() {
        if (selectionModel()->selectedRows().size() > 0) {
            // proxy index
            auto index = selectionModel()->selectedRows()[0];
            if (index.isValid()) {
                // unit file state
                const QModelIndex &sourceIndex = m_proxyModel->mapToSource(index);
                auto state = m_model->getUnitFileState(sourceIndex);
                // service name
                auto sname = m_model->getUnitFileName(sourceIndex);
                // disable set startup mode menu when service ends with '@' or noop
                if (sname.endsWith("@") || isUnitNoOp(state)) {
                    m_setServiceStartupModeMenu->setEnabled(false);
                } else {
                    m_setServiceStartupModeMenu->setEnabled(true);
                }

                auto activeState = m_model->getUnitActiveState(sourceIndex);
                if (isActiveState(activeState.toLocal8Bit())) {
                    m_startServiceAction->setEnabled(false);
                    m_stopServiceAction->setEnabled(true);
                    m_restartServiceAction->setEnabled(true);
                } else {
                    m_startServiceAction->setEnabled(true);
                    m_stopServiceAction->setEnabled(false);
                    m_restartServiceAction->setEnabled(false);
                }

                // change auto/manual startup menu item's usable state based on current service's startup mode
                if (isServiceAutoStartup(sname, state)) {
                    m_setAutoStartAction->setEnabled(false);
                    m_setManualStartAction->setEnabled(true);
                } else {
                    m_setAutoStartAction->setEnabled(true);
                    m_setManualStartAction->setEnabled(false);
                }
            }
        }
    });

    // swap load state header section visible state, then backup setting
    connect(m_loadStateHeaderAction, &QAction::triggered, this, [ = ](bool b) {
        hdr->setSectionHidden(SystemServiceTableModel::kSystemServiceLoadStateColumn, !b);
        saveSettings();
    });
    // swap active state header section visible state, then backup setting
    connect(m_activeStateHeaderAction, &QAction::triggered, this, [ = ](bool b) {
        hdr->setSectionHidden(SystemServiceTableModel::kSystemServiceActiveStateColumn, !b);
        saveSettings();
    });
    // swap sub state header section visible state, then backup setting
    connect(m_subStateHeaderAction, &QAction::triggered, this, [ = ](bool b) {
        hdr->setSectionHidden(SystemServiceTableModel::kSystemServiceSubStateColumn, !b);
        saveSettings();
    });
    // swap state header section visible state, then backup setting
    connect(m_stateHeaderAction, &QAction::triggered, this, [ = ](bool b) {
        hdr->setSectionHidden(SystemServiceTableModel::kSystemServiceStateColumn, !b);
        saveSettings();
    });
    // swap description header section visible state, then backup setting
    connect(m_descriptionHeaderAction, &QAction::triggered, this, [ = ](bool b) {
        hdr->setSectionHidden(SystemServiceTableModel::kSystemServiceDescriptionColumn, !b);
        saveSettings();
    });
    // swap pid header section visible state, then backup setting
    connect(m_pidHeaderAction, &QAction::triggered, this, [ = ](bool b) {
        hdr->setSectionHidden(SystemServiceTableModel::kSystemServicePIDColumn, !b);
        saveSettings();
    });
    // swap startup mode header section visible state, then backup setting
    connect(m_startupModeHeaderAction, &QAction::triggered, this, [ = ](bool b) {
        hdr->setSectionHidden(SystemServiceTableModel::kSystemServiceStartupModeColumn, !b);
        saveSettings();
    });

    // change header context menu item's checkable state based on current section's visible state
    connect(m_headerContextMenu, &QMenu::aboutToShow, this, [ = ]() {
        bool b;
        b = hdr->isSectionHidden(SystemServiceTableModel::kSystemServiceLoadStateColumn);
        m_loadStateHeaderAction->setChecked(!b);
        b = hdr->isSectionHidden(SystemServiceTableModel::kSystemServiceActiveStateColumn);
        m_activeStateHeaderAction->setChecked(!b);
        b = hdr->isSectionHidden(SystemServiceTableModel::kSystemServiceSubStateColumn);
        m_subStateHeaderAction->setChecked(!b);
        b = hdr->isSectionHidden(SystemServiceTableModel::kSystemServiceStateColumn);
        m_stateHeaderAction->setChecked(!b);
        b = hdr->isSectionHidden(SystemServiceTableModel::kSystemServiceStartupModeColumn);
        m_startupModeHeaderAction->setChecked(!b);
        b = hdr->isSectionHidden(SystemServiceTableModel::kSystemServiceDescriptionColumn);
        m_descriptionHeaderAction->setChecked(!b);
        b = hdr->isSectionHidden(SystemServiceTableModel::kSystemServicePIDColumn);
        m_pidHeaderAction->setChecked(!b);
    });

    // connect refresh handler to refresh shortcut's activated signal
    connect(m_refreshKP, &QShortcut::activated, this, &SystemServiceTableView::refresh);
    // connect start service handler to start shortcut's activated signal
    connect(m_startKP, &QShortcut::activated, this, &SystemServiceTableView::startService);
    // connect stop service handler to stop shortcut's activated signal
    connect(m_stopKP, &QShortcut::activated, this, &SystemServiceTableView::stopService);
    // connect restart service handler to restart shortcut's activated signal
    connect(m_restartKP, &QShortcut::activated, this, &SystemServiceTableView::restartService);

    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr != nullptr);
    // show error dialog when error occurred
    connect(mgr, &ServiceManager::errorOccurred, this, [ = ](const ErrorContext & ec) {
        if (ec) {
            if (ec.getErrorMessage() != "Permission denied")
                ErrorDialog::show(this, ec.getErrorName(), ec.getErrorMessage());
        }
    });
    // change loading state & hide tip label & show spinner before updating service list
    connect(mgr, &ServiceManager::beginUpdateList, this, [ = ]() {
        m_loading = true;

        header()->setEnabled(false);

        m_noMatchingResultLabel->hide();
        m_spinner->start();
        m_spinner->show();
    });
    // hide tip label & spinner & reset loading state after service list updated
    connect(m_model, &SystemServiceTableModel::modelReset, this, [ = ]() {
        header()->setEnabled(true);

        m_noMatchingResultLabel->hide();
        m_spinner->hide();
        m_spinner->stop();

        m_loading = false;
    });

    // we need override currentRowChanged method to overcome incremental service list fetch glitch, if user use [down] key
    // to move current select item and next item is right out side of tableview's viewport region, then we need call fetch
    // more on proxy model
    connect(m_model, &SystemServiceTableModel::currentRowChanged, this,
    [ = ](int row) {
        m_proxyModel->fetchMore({});
        auto *selmo = selectionModel();
        selmo->select(m_proxyModel->mapFromSource(m_model->index(row, 0)),
                      QItemSelectionModel::SelectCurrent |
                      QItemSelectionModel::Rows |
                      QItemSelectionModel::Clear);
    });

    // need update search result tip label while rows got removed or inserted
    connect(m_proxyModel, &SystemServiceTableModel::rowsInserted, this,
    [ = ]() {
        adjustInfoLabelVisibility();
    });
    connect(m_proxyModel, &SystemServiceTableModel::rowsRemoved, this,
    [ = ]() {
        adjustInfoLabelVisibility();
    });
}

void SystemServiceTableView::onLoadServiceDataList()
{
    if (!defer_initialized) {
        ServiceManager::instance()->updateServiceList();
        defer_initialized = true;
    }
}

// size hint for column to help calculate prefered section width while user double clicked section's gripper
int SystemServiceTableView::sizeHintForColumn(int column) const
{
    int margin = 10;
    return std::max(header()->sizeHintForColumn(column) + margin * 2,
                    BaseTableView::sizeHintForColumn(column) + margin * 2);
}

// refresh service list handler
void SystemServiceTableView::refresh()
{
    // while already in processing state, do nothing
    if (m_loading)
        return;

    // reset model & select service's name
    m_model->reset();
    m_selectedSName.clear();

    ServiceManager::instance()->updateServiceList();
}

// event filter
bool SystemServiceTableView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        // handle key press event for service table view
        if (event->type() == QEvent::KeyPress) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            // ALT + M show context menu based on focused widget
            if (kev->modifiers() == Qt::ALT && kev->key() == Qt::Key_M) {
                if (this->hasFocus()) {
                    // show context menu for table view if table view itself has focus
                    if (selectedIndexes().size() > 0) {
                        auto index = selectedIndexes()[0];
                        auto rect = visualRect(index);
                        displayTableContextMenu({rect.x() + rect.width() / 2,
                                                 rect.y() + rect.height() / 2});
                        return true;
                    }
                } else if (header()->hasFocus()) {
                    // show header context menu if header has focus
                    displayHeaderContextMenu({
                        header()->sectionSize(header()->logicalIndexAt(0)) / 2,
                        header()->height() / 2});
                    return true;
                }
            }
        }
    }
    return BaseTableView::eventFilter(obj, event);
}
