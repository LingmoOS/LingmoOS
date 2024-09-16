// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ddlog.h"
#include "process_table_view.h"

#include "application.h"
#include "main_window.h"
#include "kill_process_confirm_dialog.h"
#include "priority_slider.h"
#include "process_attribute_dialog.h"
#include "dialog/error_dialog.h"
#include "settings.h"
#include "toolbar.h"
#include "ui_common.h"
#include "common/perf.h"
#include "common/common.h"
#include "common/error_context.h"
#include "model/process_sort_filter_proxy_model.h"
#include "model/process_table_model.h"
#include "process/process_db.h"
#include "common/eventlogutils.h"
#include "helper.hpp"

#include <DApplication>
#include <DApplicationHelper>
#include <DDialog>
#include <DErrorMessage>
#include <DFontSizeManager>
#include <DFrame>
#include <DLabel>
#include <DMenu>
#include <DPalette>
#include <DSlider>
#include <DTipLabel>
#include <DTitlebar>
#include <DToolTip>
#include <DWidget>
#include <DHeaderView>

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QKeyEvent>
#include <QShortcut>

using namespace DDLog;
using namespace common::init;

// process table view backup setting key
const QByteArray header_version = "_1.0.0";
static const char *kSettingsOption_ProcessTableHeaderState = "process_table_header_state";
static const char *kSettingsOption_ProcessTableHeaderStateOfUserMode = "process_table_header_state_user";
/**
 * @brief m_pControlConnection 单例对象的信号连接之后，需要保持只连接一次，避免多次触发信号槽
 */
static QMetaObject::Connection m_pControlConnection = QMetaObject::Connection();
ProcessTableView::ProcessTableView(DWidget *parent, QString userName)
    : BaseTableView(parent), m_useModeName(userName)
{
    // install event filter for table view to handle key events
    installEventFilter(this);

    // model & sort filter proxy model instance
    m_model = new ProcessTableModel(this, userName);
    m_proxyModel = new ProcessSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    // setModel must be called before calling loadSettings();
    setModel(m_proxyModel);
    // load process table view backup settings
    bool settingsLoaded = userName.isNull() ? loadSettings(kSettingsOption_ProcessTableHeaderState) : loadSettings(kSettingsOption_ProcessTableHeaderStateOfUserMode);
    // initialize ui components & connections
    initUI(settingsLoaded);
    initConnections(settingsLoaded);
    // adjust search result tip label text color dynamically on theme type change
    onThemeTypeChanged();
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &ProcessTableView::onThemeTypeChanged);
    if (!userName.isNull()) {
        m_cpuUsage = m_model->getTotalCPUUsage();
        m_memUsage = m_model->getTotalMemoryUsage();
        m_download = m_model->getTotalDownload();
        m_upload = m_model->getTotalUpload();
        m_smemUsage = m_model->getTotalSharedMemoryUsage();
        m_vmemUsage = m_model->getTotalVirtualMemoryUsage();
        m_diskread = m_model->getTotalDiskRead();
        m_diskwrite = m_model->getTotalDiskWrite();
    }
}

// destructor
ProcessTableView::~ProcessTableView()
{
    // backup table view settings
    saveSettings();
}

void ProcessTableView::onThemeTypeChanged()
{
    auto palette = DApplicationHelper::instance()->applicationPalette();
    palette.setColor(DPalette::Text, palette.color(DPalette::PlaceholderText));
    m_notFoundLabel->setPalette(palette);
}

QString ProcessTableView::getProcessName(int pid)
{
    return m_model->getProcess(qvariant_cast<pid_t>(pid)).name();
}

// event filter
bool ProcessTableView::eventFilter(QObject *obj, QEvent *event)
{
    // handle key press events for process table view
    if (obj == this) {
        if (event->type() == QEvent::KeyPress) {
            auto *kev = dynamic_cast<QKeyEvent *>(event);
            // ALT+M show context menu
            if (kev->modifiers() == Qt::ALT && kev->key() == Qt::Key_M) {
                // if table view itself has focus, then show context menu for selected item
                if (this->hasFocus()) {
                    if (selectedIndexes().size() > 0) {
                        auto index = selectedIndexes()[0];
                        auto rect = visualRect(index);
                        displayProcessTableContextMenu({ rect.x() + rect.width() / 2, rect.y() + rect.height() / 2 });
                        return true;
                    }
                } else if (header()->hasFocus()) {
                    // if header view has focus, then show context menu for header view
                    displayProcessTableHeaderContextMenu({ header()->sectionSize(header()->logicalIndexAt(0)) / 2, header()->height() / 2 });
                    return true;
                }
            }
        }
    }
    return BaseTableView::eventFilter(obj, event);
}

// end process handler
void ProcessTableView::endProcess()
{
    // no selected item, do nothing
    if (m_selectedPID.isNull()) {
        return;
    }

    // kill confirm dialog title & description
    QString title = DApplication::translate("Kill.Process.Dialog", "End process");
    QString description = DApplication::translate("Kill.Process.Dialog",
                                                  "Ending this process may cause data "
                                                  "loss.\nAre you sure you want to continue?");

    KillProcessConfirmDialog dialog(this);
    //    dialog.setTitle(title);
    dialog.setMessage(description);
    dialog.addButton(DApplication::translate("Kill.Process.Dialog", "Cancel", "button"), false);
    dialog.addButton(DApplication::translate("Kill.Process.Dialog", "End", "button"), true,
                     DDialog::ButtonWarning);
    dialog.exec();
    if (dialog.result() == QMessageBox::Ok) {
        Process proc = m_model->getProcess(qvariant_cast<pid_t>(m_selectedPID));
        QJsonObject obj {
            { "tid", EventLogUtils::ProcessKilled },
            { "version", QCoreApplication::applicationVersion() },
            { "process_name", proc.name() }
        };
        EventLogUtils::get().writeLogs(obj);

        ProcessDB::instance()->endProcess(qvariant_cast<pid_t>(m_selectedPID));
    } else {
        return;
    }
}

// pause process handler
void ProcessTableView::pauseProcess()
{
    auto pid = qvariant_cast<pid_t>(m_selectedPID);
    // no selected item or app self been selected, then do nothing
    if (m_selectedPID.isNull() || ProcessDB::instance()->isCurrentProcess(pid)) {
        return;
    }
    ProcessDB::instance()->pauseProcess(pid);
}

// resume process handler
void ProcessTableView::resumeProcess()
{
    auto pid = qvariant_cast<pid_t>(m_selectedPID);
    //no selected item or app self been selected, then do nothing
    if (m_selectedPID.isNull() || ProcessDB::instance()->isCurrentProcess(pid)) {
        return;
    }

    ProcessDB::instance()->resumeProcess(pid);
}

// open process bin path in file manager
void ProcessTableView::openExecDirWithFM()
{
    // selection check needed
    if (m_selectedPID.isValid()) {
        pid_t pid = qvariant_cast<pid_t>(m_selectedPID);
        const Process &proc = ProcessDB::instance()->processSet()->getProcessById(pid);
        QString cmdline = proc.cmdlineString();

        if (cmdline.size() > 0) {
            // Found wine program location if cmdline starts with c://.
            if (cmdline.startsWith("c:")) {
                QString winePrefix = proc.environ().value("WINEPREFIX");
                cmdline = cmdline.replace("\\", "/").replace("c:/", "/drive_c/");

                const QString &path = QString(winePrefix + cmdline).trimmed();
                common::openFilePathItem(path);
            } else {
                QString flatpakAppidEnv = proc.environ().value("FLATPAK_APPID");
                // Else find program location through 'which' command.
                if (flatpakAppidEnv == "") {
                    QProcess whichProcess;
                    QString exec = "which";
                    QStringList params;
                    params << cmdline.split(" ");
                    whichProcess.start(exec, params);
                    whichProcess.waitForFinished();
                    QString output(whichProcess.readAllStandardOutput());

                    QString path;
                    if (!common::systemInfo().isOldVersion()) {
                        path = QString(output.split("\n")[0]).trimmed();
                        // 读取persistent目录
                        if (path.isEmpty()) {
                            char nsPath[PATH_MAX] = { 0 }, nsSelfPath[PATH_MAX] = { 0 };
                            auto nsSize = readlink(QString("/proc/%1/ns/pid").arg(pid).toStdString().c_str(), nsPath, PATH_MAX);
                            auto nsSelfSize = readlink(QString("/proc/self/ns/pid").toStdString().c_str(), nsSelfPath, PATH_MAX);
                            if (nsSize > 0 && nsSelfSize > 0) {
                                QString nsPathStr(nsPath), nsSelfPathStr(nsSelfPath);
                                if (nsPathStr != nsSelfPathStr) {
                                    Process preProc = proc, curProc = proc;
                                    int count = 0;
                                    // 100次循环
                                    while (curProc.name() != "ll-box" && curProc.pid() != 0 && count != 100) {
                                        preProc = curProc;
                                        curProc = ProcessDB::instance()->processSet()->getProcessById(preProc.ppid());
                                        count++;
                                    }
                                    if (curProc.pid() != 0 || count != 100) {
                                        pid = preProc.pid();
                                    }
                                    char exePath[PATH_MAX] = { 0 };
                                    auto exeSize = readlink(QString("/proc/%1/exe").arg(pid).toStdString().c_str(), exePath, PATH_MAX);
                                    if (exeSize > 0) {
                                        QString exeStr(exePath);
                                        QFile file(QString("/proc/%1/mountinfo").arg(pid));
                                        if (file.open(QIODevice::ReadOnly)) {
                                            QString devStr;
                                            do {
                                                QString line = file.readLine();
                                                auto works = line.split(" ", QString::SkipEmptyParts);
                                                if (works.size() > 9 && exeStr.startsWith(works[4]) && works[9].startsWith("/dev")) {
                                                    devStr = works[9];
                                                    if (!devStr.isEmpty()) {
                                                        path = "/persistent" + works[3] + exeStr.right(exeStr.size() - works[4].size());
                                                    }
                                                    break;
                                                }
                                            } while (!file.atEnd());
                                            file.close();
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        path = QString(output.split("\n")[0]).trimmed();
                    }
                    common::openFilePathItem(path);
                }
                // Find flatpak application location.
                else {
                    QProcess whichProcess;
                    QString exec = "flatpak";
                    QStringList params;
                    params << "info";
                    params << flatpakAppidEnv;
                    whichProcess.start(exec, params);
                    whichProcess.waitForFinished();
                    QString output(whichProcess.readAllStandardOutput());

                    QDir flatpakRootDir(output.split("Location:")[1].split("\n")[0].simplified());
                    if (flatpakRootDir.cd("files") && flatpakRootDir.cd("bin")) {
                        // Need split full path to get last filename.
                        const QString &path = QString(flatpakRootDir.absoluteFilePath(cmdline.split("/").last())).trimmed();
                        common::openFilePathItem(path);
                    }
                }
            }
        }   // ::if(cmdline)
    }   // ::if(selectedPID)
}

// show process attribute dialog
void ProcessTableView::showProperties()
{
    // selection valid check
    if (m_selectedPID.isValid()) {
        pid_t pid = qvariant_cast<pid_t>(m_selectedPID);
        // get process entry item from model
        auto proc = m_model->getProcess(pid);
        auto *attr = new ProcessAttributeDialog(pid,
                                                proc.name(),
                                                proc.displayName(),
                                                proc.cmdlineString(),
                                                proc.icon(),
                                                proc.startTime(),
                                                this);
        attr->show();
    }
}

// kill process handler
void ProcessTableView::killProcess()
{
    // no selected item, do nothing
    if (m_selectedPID.isNull()) {
        return;
    }

    // dialog
    QString title = DApplication::translate("Kill.Process.Dialog", "End process");
    QString description = DApplication::translate("Kill.Process.Dialog",
                                                  "Force ending this process may cause data "
                                                  "loss.\nAre you sure you want to continue?");

    // show confirm dialog
    KillProcessConfirmDialog dialog(this);
    dialog.setMessage(description);
    dialog.addButton(DApplication::translate("Kill.Process.Dialog", "Cancel", "button"), false);
    dialog.addButton(DApplication::translate("Kill.Process.Dialog", "Force End", "button"), true,
                     DDialog::ButtonWarning);
    dialog.exec();
    if (dialog.result() == QMessageBox::Ok) {
        Process proc = m_model->getProcess(qvariant_cast<pid_t>(m_selectedPID));
        QJsonObject obj {
            { "tid", EventLogUtils::ProcessKilled },
            { "version", QCoreApplication::applicationVersion() },
            { "process_name", proc.name() }
        };
        EventLogUtils::get().writeLogs(obj);
        ProcessDB::instance()->killProcess(qvariant_cast<pid_t>(m_selectedPID));
    } else {
        return;
    }
}

// filter process table based on searched text
void ProcessTableView::search(const QString &text)
{
    m_proxyModel->setSortFilterString(text);
    // adjust search result tip label's visibility & position if needed
    adjustInfoLabelVisibility();
    PERF_PRINT_END("POINT-03");
}

// switch process table view display mode
void ProcessTableView::switchDisplayMode(FilterType type)
{

    m_proxyModel->setFilterType(type);
}

// change process priority
void ProcessTableView::changeProcessPriority(int priority)
{
    // check selection first
    if (m_selectedPID.isValid()) {
        pid_t pid = qvariant_cast<pid_t>(m_selectedPID);

        // if no changes in priority value, then do nothing
        auto prio = m_model->getProcessPriority(pid);
        if (prio == priority)
            return;

        ProcessDB::instance()->setProcessPriority(pid, priority);
    }
}

// load & restore table view settings from backup storage
bool ProcessTableView::loadSettings(const QString &flag)
{
    Settings *s = Settings::instance();
    if (s) {
        const QVariant &opt = s->getOption(flag);

        if (opt.isValid()) {
            const QByteArray &buf = QByteArray::fromBase64(opt.toByteArray());
            if (buf.endsWith(header_version)) {
                header()->restoreState(buf);
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

// save table view settings to backup storage
void ProcessTableView::saveSettings()
{
    Settings *s = Settings::instance();
    if (s) {
        QByteArray buf = header()->saveState();
        buf += header_version;

        if (m_useModeName.isNull()) {
            s->setOption(kSettingsOption_ProcessTableHeaderState, buf.toBase64());
        } else {
            s->setOption(kSettingsOption_ProcessTableHeaderStateOfUserMode, buf.toBase64());
        }

        s->flush();
    }
}

// initialize ui components
void ProcessTableView::initUI(bool settingsLoaded)
{

    setAccessibleName("ProcessTableView");

    // search result not found tip label instance
    m_notFoundLabel = new DLabel(DApplication::translate("Common.Search", "No search results"), this);
    DFontSizeManager::instance()->bind(m_notFoundLabel, DFontSizeManager::T4);
    // change text color
    auto palette = DApplicationHelper::instance()->palette(m_notFoundLabel);
    QColor labelColor = palette.color(DPalette::PlaceholderText);
    palette.setColor(DPalette::Text, labelColor);
    m_notFoundLabel->setPalette(palette);
    m_notFoundLabel->setVisible(false);
    // header view options
    // header view instance
    auto *hdr = header();
    // header section movable
    hdr->setSectionsMovable(true);
    // header section clickable
    hdr->setSectionsClickable(true);
    // header section resizable
    hdr->setSectionResizeMode(DHeaderView::Interactive);
    // stretch last header section
    hdr->setStretchLastSection(true);
    // show sort indicator on sort column
    hdr->setSortIndicatorShown(true);
    // header section default alignment
    hdr->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // header section context menu policy
    hdr->setContextMenuPolicy(Qt::CustomContextMenu);
    // table options
    setSortingEnabled(true);
    // only single row selection allowed
    setSelectionMode(QAbstractItemView::SingleSelection);
    // can only select whole row
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // table view context menu policy
    setContextMenuPolicy(Qt::CustomContextMenu);

    // context menu & header context menu instance
    m_contextMenu = new DMenu(this);
    m_headerContextMenu = new DMenu(this);

    // if no backup settings loaded, show default style
    if (!settingsLoaded) {
        // proc name
        setColumnWidth(ProcessTableModel::kProcessNameColumn, 300);
        setColumnHidden(ProcessTableModel::kProcessNameColumn, false);

        // cpu
        setColumnWidth(ProcessTableModel::kProcessCPUColumn, 100);
        setColumnHidden(ProcessTableModel::kProcessCPUColumn, false);

        // account
        setColumnWidth(ProcessTableModel::kProcessUserColumn, 80);
        setColumnHidden(ProcessTableModel::kProcessUserColumn, true);

        // memory
        setColumnWidth(ProcessTableModel::kProcessMemoryColumn, 70);
        setColumnHidden(ProcessTableModel::kProcessMemoryColumn, false);

        // share memory
        setColumnWidth(ProcessTableModel::kProcessShareMemoryColumn, 80);
        setColumnHidden(ProcessTableModel::kProcessShareMemoryColumn, true);

        // vtr memory
        setColumnWidth(ProcessTableModel::kProcessVTRMemoryColumn, 80);
        setColumnHidden(ProcessTableModel::kProcessVTRMemoryColumn, true);

        // download
        setColumnWidth(ProcessTableModel::kProcessDownloadColumn, 70);
        setColumnHidden(ProcessTableModel::kProcessDownloadColumn, false);

        // upload
        setColumnWidth(ProcessTableModel::kProcessUploadColumn, 70);
        setColumnHidden(ProcessTableModel::kProcessUploadColumn, false);

        // disk read
        setColumnWidth(ProcessTableModel::kProcessDiskReadColumn, 80);
        setColumnHidden(ProcessTableModel::kProcessDiskReadColumn, true);

        // disk write
        setColumnWidth(ProcessTableModel::kProcessDiskWriteColumn, 80);
        setColumnHidden(ProcessTableModel::kProcessDiskWriteColumn, true);

        // pid
        setColumnWidth(ProcessTableModel::kProcessPIDColumn, 70);
        setColumnHidden(ProcessTableModel::kProcessPIDColumn, false);

        // nice
        setColumnWidth(ProcessTableModel::kProcessNiceColumn, 100);
        setColumnHidden(ProcessTableModel::kProcessNiceColumn, true);

        // priority
        setColumnWidth(ProcessTableModel::kProcessPriorityColumn, 100);
        setColumnHidden(ProcessTableModel::kProcessPriorityColumn, true);

        //sort
        sortByColumn(ProcessTableModel::kProcessCPUColumn, Qt::DescendingOrder);
    }
}

// initialize connections
void ProcessTableView::initConnections(bool settingsLoaded)
{
    auto *mainWindow = gApp->mainWindow();
    // connect search slot to toolbar's search signal
    connect(mainWindow->toolbar(), &Toolbar::search, this, &ProcessTableView::search);

    // table context menu
    connect(this, &ProcessTableView::customContextMenuRequested, this,
            &ProcessTableView::displayProcessTableContextMenu);
    // end process
    auto *endProcAction = m_contextMenu->addAction(
            DApplication::translate("Process.Table.Context.Menu", "End process"));
    // ALT + E
    endProcAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_E));
    connect(endProcAction, &QAction::triggered, this, &ProcessTableView::endProcess);
    // pause process
    auto *pauseProcAction = m_contextMenu->addAction(
            DApplication::translate("Process.Table.Context.Menu", "Suspend process"));
    // ALT + P
    pauseProcAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_P));
    connect(pauseProcAction, &QAction::triggered, this, &ProcessTableView::pauseProcess);
    // resume process
    auto *resumeProcAction = m_contextMenu->addAction(
            DApplication::translate("Process.Table.Context.Menu", "Resume process"));
    // ALT + C
    resumeProcAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_C));
    connect(resumeProcAction, &QAction::triggered, this, &ProcessTableView::resumeProcess);

    // change priority dialog
    auto *chgProcPrioMenu = m_contextMenu->addMenu(
            DApplication::translate("Process.Table.Context.Menu", "Change priority"));
    QActionGroup *prioGroup = new QActionGroup(chgProcPrioMenu);
    prioGroup->setExclusive(true);

    // very high priority action
    auto *setVeryHighPrioAction =
            chgProcPrioMenu->addAction(DApplication::translate("Process.Priority", "Very high"));
    setVeryHighPrioAction->setCheckable(true);
    setVeryHighPrioAction->setActionGroup(prioGroup);
    connect(setVeryHighPrioAction, &QAction::triggered,
            [=]() { changeProcessPriority(kVeryHighPriority); });

    // high priority action
    auto *setHighPrioAction =
            chgProcPrioMenu->addAction(DApplication::translate("Process.Priority", "High"));
    setHighPrioAction->setCheckable(true);
    setHighPrioAction->setActionGroup(prioGroup);
    connect(setHighPrioAction, &QAction::triggered,
            [=]() { changeProcessPriority(kHighPriority); });

    // normal priority action
    auto *setNormalPrioAction =
            chgProcPrioMenu->addAction(DApplication::translate("Process.Priority", "Normal"));
    setNormalPrioAction->setCheckable(true);
    setNormalPrioAction->setActionGroup(prioGroup);
    connect(setNormalPrioAction, &QAction::triggered,
            [=]() { changeProcessPriority(kNormalPriority); });

    // low priority action
    auto *setLowPrioAction =
            chgProcPrioMenu->addAction(DApplication::translate("Process.Priority", "Low"));
    setLowPrioAction->setCheckable(true);
    setLowPrioAction->setActionGroup(prioGroup);
    connect(setLowPrioAction, &QAction::triggered,
            [=]() { changeProcessPriority(kLowPriority); });

    // very low priority action
    auto *setVeryLowPrioAction =
            chgProcPrioMenu->addAction(DApplication::translate("Process.Priority", "Very low"));
    setVeryLowPrioAction->setCheckable(true);
    setVeryLowPrioAction->setActionGroup(prioGroup);
    connect(setVeryLowPrioAction, &QAction::triggered,
            [=]() { changeProcessPriority(kVeryLowPriority); });

    // custom priority action
    auto *setCustomPrioAction =
            chgProcPrioMenu->addAction(DApplication::translate("Process.Priority", "Custom"));
    setCustomPrioAction->setCheckable(true);
    setCustomPrioAction->setActionGroup(prioGroup);
    connect(setCustomPrioAction, &QAction::triggered, [=]() { customizeProcessPriority(); });

    // show exec location action
    auto *openExecDirAction = m_contextMenu->addAction(
            DApplication::translate("Process.Table.Context.Menu", "View command location"));
    connect(openExecDirAction, &QAction::triggered, this, &ProcessTableView::openExecDirWithFM);
    // show property action
    auto *showAttrAction = m_contextMenu->addAction(
            DApplication::translate("Process.Table.Context.Menu", "Properties"));
    // ALt + ENTER
    showAttrAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Enter));
    connect(showAttrAction, &QAction::triggered, this, &ProcessTableView::showProperties);
    m_contextMenu->addSeparator();
    // kill process
    auto *killProcAction = m_contextMenu->addAction(
            DApplication::translate("Process.Table.Context.Menu", "Kill process"));
    // ALT + K
    killProcAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_K));
    connect(killProcAction, &QAction::triggered, this, &ProcessTableView::killProcess);

    // change menu item checkable state before context menu popup
    connect(m_contextMenu, &DMenu::aboutToShow, this, [=]() {
        // process running or not flag

        if (m_selectedPID.isValid()) {
            bool running = true;
            pid_t pid = qvariant_cast<pid_t>(m_selectedPID);
            char state = m_model->getProcessState(pid);
            if (state == 'T') {
                // process in stopped state
                running = false;
            } else if (state == 'R') {
                // process in running state
                running = true;
            }
            // monitor process itself cant be stopped, so we need a second modified running flag
            bool modRunning = running;
            pauseProcAction->setEnabled(modRunning);
            resumeProcAction->setEnabled(!running);

            // change priority menu item checkable state
            auto prio = m_model->getProcessPriority(pid);
            switch (prio) {
            case kVeryHighPriority: {
                setVeryHighPrioAction->setChecked(true);
            } break;
            case kHighPriority: {
                setHighPrioAction->setChecked(true);
            } break;
            case kNormalPriority: {
                setNormalPrioAction->setChecked(true);
            } break;
            case kLowPriority: {
                setLowPrioAction->setChecked(true);
            } break;
            case kVeryLowPriority: {
                setVeryLowPrioAction->setChecked(true);
            } break;
            case kCustomPriority: {
                setCustomPrioAction->setChecked(true);
            } break;
            default: {
                // unexpected, do nothing
            }
            }
        }
    });

    // backup table view settings if any of the header view state changes
    auto *h = header();
    connect(h, &QHeaderView::sectionResized, this, [=]() { saveSettings(); });
    connect(h, &QHeaderView::sectionMoved, this, [=]() { saveSettings(); });
    connect(h, &QHeaderView::sortIndicatorChanged, this, [=]() { saveSettings(); });
    connect(h, &QHeaderView::customContextMenuRequested, this,
            &ProcessTableView::displayProcessTableHeaderContextMenu);

    // header context menu
    // cpu action
    auto *cpuHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessCPU));
    cpuHeaderAction->setCheckable(true);
    connect(cpuHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessCPUColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    // process user action
    auto *userHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessUser));
    userHeaderAction->setCheckable(true);
    connect(userHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessUserColumn, !b);
        saveSettings();
    });
    // memory action
    auto *memHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessMemory));
    memHeaderAction->setCheckable(true);
    connect(memHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessMemoryColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    // share memory action
    auto *sharememHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessShareMemory));
    sharememHeaderAction->setCheckable(true);
    connect(sharememHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessShareMemoryColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    //vtr memory action
    auto *vtrmemHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessVtrMemory));
    vtrmemHeaderAction->setCheckable(true);
    connect(vtrmemHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessVTRMemoryColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    // upload rate action
    auto *uploadHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessUpload));
    uploadHeaderAction->setCheckable(true);
    connect(uploadHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessUploadColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    // download rate action
    auto *downloadHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessDownload));
    downloadHeaderAction->setCheckable(true);
    connect(downloadHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessDownloadColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    // disk read rate action
    auto *dreadHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessDiskRead));
    dreadHeaderAction->setCheckable(true);
    connect(dreadHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessDiskReadColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    // disk write rate action
    auto *dwriteHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessDiskWrite));
    dwriteHeaderAction->setCheckable(true);
    connect(dwriteHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessDiskWriteColumn, !b);
        saveSettings();
        Q_EMIT signalHeadchanged();
    });
    // pid action
    auto *pidHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessPID));
    pidHeaderAction->setCheckable(true);
    connect(pidHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessPIDColumn, !b);
        saveSettings();
    });
    // nice value action
    auto *niceHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessNice));
    niceHeaderAction->setCheckable(true);
    connect(niceHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessNiceColumn, !b);
        saveSettings();
    });
    // priority value action
    auto *priorityHeaderAction = m_headerContextMenu->addAction(
            DApplication::translate("Process.Table.Header", kProcessPriority));
    priorityHeaderAction->setCheckable(true);
    connect(priorityHeaderAction, &QAction::triggered, this, [this](bool b) {
        header()->setSectionHidden(ProcessTableModel::kProcessPriorityColumn, !b);
        saveSettings();
    });

    // set default header context menu checkable state when settings load without success
    if (!settingsLoaded) {
        cpuHeaderAction->setChecked(true);
        memHeaderAction->setChecked(true);
        sharememHeaderAction->setChecked(false);
        vtrmemHeaderAction->setChecked(false);
        uploadHeaderAction->setChecked(true);
        downloadHeaderAction->setChecked(true);
        dreadHeaderAction->setChecked(false);
        dwriteHeaderAction->setChecked(false);
        pidHeaderAction->setChecked(true);
        niceHeaderAction->setChecked(true);
        priorityHeaderAction->setChecked(true);
    }
    // set header context menu checkable state based on current header section's visible state before popup
    connect(m_headerContextMenu, &QMenu::aboutToShow, this, [=]() {
        bool b;
        b = header()->isSectionHidden(ProcessTableModel::kProcessCPUColumn);
        cpuHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessMemoryColumn);
        memHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessShareMemoryColumn);
        sharememHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessVTRMemoryColumn);
        vtrmemHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessUploadColumn);
        uploadHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessDownloadColumn);
        downloadHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessDiskReadColumn);
        dreadHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessDiskWriteColumn);
        dwriteHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessPIDColumn);
        pidHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessNiceColumn);
        niceHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessPriorityColumn);
        priorityHeaderAction->setChecked(!b);
        b = header()->isSectionHidden(ProcessTableModel::kProcessUserColumn);
        userHeaderAction->setChecked(!b);
    });

    // on each model update, we restore settings, adjust search result tip lable's visibility & positon, select the same process item before update if any
    connect(m_model, &ProcessTableModel::modelUpdated, this, [&]() {
        adjustInfoLabelVisibility();
        if (m_selectedPID.isValid()) {
            for (int i = 0; i < m_proxyModel->rowCount(); i++) {
                if (m_proxyModel->data(m_proxyModel->index(i, ProcessTableModel::kProcessPIDColumn),
                                       Qt::UserRole)
                    == m_selectedPID)
                    this->setCurrentIndex(m_proxyModel->index(i, 0));
            }
        }
        if (!m_useModeName.isNull()) {
            m_cpuUsage = m_model->getTotalCPUUsage();
            m_memUsage = m_model->getTotalMemoryUsage();
            m_download = m_model->getTotalDownload();
            m_upload = m_model->getTotalUpload();
            m_smemUsage = m_model->getTotalSharedMemoryUsage();
            m_vmemUsage = m_model->getTotalVirtualMemoryUsage();
            m_diskread = m_model->getTotalDiskRead();
            m_diskwrite = m_model->getTotalDiskWrite();
        }
        Q_EMIT signalModelUpdated();
    });

    // end process shortcut
    m_endProcKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_E), this);
    connect(m_endProcKP, &QShortcut::activated, this, &ProcessTableView::endProcess);
    // pause process shortcut
    m_pauseProcKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_P), this);
    connect(m_pauseProcKP, &QShortcut::activated, this, &ProcessTableView::pauseProcess);
    // resume process shortcut
    m_resumeProcKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_C), this);
    connect(m_resumeProcKP, &QShortcut::activated, this, &ProcessTableView::resumeProcess);
    // view process attribute shortcut
    m_viewPropKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return), this);
    connect(m_viewPropKP, &QShortcut::activated, this, &ProcessTableView::showProperties);
    // kill process shortcut
    m_killProcKP = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_K), this);
    connect(m_killProcKP, &QShortcut::activated, this, &ProcessTableView::killProcess);

    //show error dialog if change priority failed
    connect(ProcessDB::instance(), &ProcessDB::priorityPromoteResultReady, this,
            [=](const ErrorContext &ec) {
                if (ec) {
                    ErrorDialog::show(this, ec.getErrorName(), ec.getErrorMessage());
                }
            });
    qCInfo(app) << "'processControlResultReady' signal is connect?" << m_pControlConnection;
    //The singleton object only needs to connect to the signal slot once.
    //show error dialog if sending signals to process failed
    if (!m_pControlConnection)
        m_pControlConnection = connect(ProcessDB::instance(), &ProcessDB::processControlResultReady, this,
                                       [=](const ErrorContext &ec) {
                                           if (ec) {
                                               ErrorDialog::show(this, ec.getErrorName(), ec.getErrorMessage());
                                               qCWarning(app) << "ErrorName: " << ec.getErrorName() << ",ErrorMessage: " << ec.getErrorMessage();
                                           }
                                       });
}

// show process table view context menu on specified positon
void ProcessTableView::displayProcessTableContextMenu(const QPoint &p)
{
    if (selectedIndexes().size() == 0)
        return;

    QPoint point = mapToGlobal(p);
    point.setY(point.y() + header()->sizeHint().height());
    m_contextMenu->popup(point);
}

// show process header view context menu on specified position
void ProcessTableView::displayProcessTableHeaderContextMenu(const QPoint &p)
{
    m_headerContextMenu->popup(mapToGlobal(p));
}

// resize event handler
void ProcessTableView::resizeEvent(QResizeEvent *event)
{
    // adjust search result tip label's visibility & position when resizing
    adjustInfoLabelVisibility();

    DTreeView::resizeEvent(event);
}

// show event handler
void ProcessTableView::showEvent(QShowEvent *)
{
    // hide search result not found on initial show
    if (m_notFoundLabel) {
        m_notFoundLabel->hide();
    }
}

// backup current selected item's pid when selection changed
void ProcessTableView::selectionChanged(const QItemSelection &selected,
                                        const QItemSelection &deselected)
{
    // if no selection, do nothing
    if (selected.size() <= 0) {
        return;
    }

    m_selectedPID = selected.indexes().value(ProcessTableModel::kProcessPIDColumn).data();

    DTreeView::selectionChanged(selected, deselected);
}

// return hinted size for specified column, so column can be resized to a prefered width when double clicked
int ProcessTableView::sizeHintForColumn(int column) const
{
    int margin = 10;
    return std::max(header()->sizeHintForColumn(column) + margin * 2,
                    DTreeView::sizeHintForColumn(column) + margin * 2);
}

// adjust search result tip label's visibility & position
void ProcessTableView::adjustInfoLabelVisibility()
{
    setUpdatesEnabled(false);
    // show search not found label only when proxy model is empty & search text input is none empty
    m_notFoundLabel->setVisible(m_proxyModel->rowCount() == 0
                                && gApp->mainWindow()->toolbar()->isSearchContentEmpty());
    // move label to center of the process table view
    if (m_notFoundLabel->isVisible())
        m_notFoundLabel->move(rect().center() - m_notFoundLabel->rect().center());
    setUpdatesEnabled(true);
}

// show customize process priority dialog
void ProcessTableView::customizeProcessPriority()
{
    // priority dialog instance
    DDialog *prioDialog = new DDialog(this);
    prioDialog->setIcon(QIcon::fromTheme("dialog-warning"));
    prioDialog->setAttribute(Qt::WA_DeleteOnClose);
    prioDialog->setTitle(
            DApplication::translate("Process.Table.Custom.Priority.Dialog", "Custom Priority"));
    prioDialog->addSpacing(20);
    PrioritySlider *slider = new PrioritySlider(Qt::Horizontal, prioDialog);
    slider->slider()->setInvertedAppearance(true);
    // set slider range based on minimum & maximum priority value
    slider->setMinimum(kVeryHighPriorityMax);
    slider->setMaximum(kVeryLowPriorityMin);
    slider->setPageStep(1);
    slider->slider()->setTracking(true);
    slider->setMouseWheelEnabled(true);
    slider->setBelowTicks({ QString("%1").arg(kVeryLowPriorityMin),
                            QString("%1").arg(kVeryHighPriorityMax) });
    // change tip value dynamically when slider value changed
    connect(slider, &DSlider::valueChanged,
            [=](int value) { slider->setTipValue(QString("%1").arg(value)); });
    // set initial slider & tip value based on current process priority
    QString prio { "0" };
    if (m_selectedPID.isValid()) {
        pid_t pid = qvariant_cast<pid_t>(m_selectedPID);
        slider->setValue(m_model->getProcessPriorityValue(pid));
        Process selectedProcess = m_model->getProcess(pid);
        int selectedPriority = kNormalPriority;
        if (selectedProcess.isValid()) {
            selectedPriority = ProcessDB::instance()->processSet()->getProcessById(pid).priority();
        }
        slider->setValue(selectedPriority);
        prio = QString("%1").arg(slider->value());
        slider->setTipValue(prio);
    }
    prioDialog->addContent(slider);
    prioDialog->addSpacing(16);
    prioDialog->addButton(DApplication::translate("Process.Table.Custom.Priority.Dialog", "Cancel", "button"),
                          false, DDialog::ButtonNormal);
    prioDialog->addButton(DApplication::translate("Process.Table.Custom.Priority.Dialog", "Change", "button"),
                          true, DDialog::ButtonRecommend);
    // clear focus first, otherwise we wont get the tab order we want
    prioDialog->clearFocus();
    connect(prioDialog, &DDialog::buttonClicked, this, [=](int index, QString text) {
        Q_UNUSED(text);
        if (index == 1) {
            changeProcessPriority(slider->value());
            prioDialog->setResult(QMessageBox::Ok);
        } else {
            prioDialog->setResult(QMessageBox::Cancel);
        }
    });
    prioDialog->exec();
}

void ProcessTableView::setUserModeName(const QString &userName)
{
    if (userName != m_useModeName) {
        m_useModeName = userName;
        m_model->setUserModeName(m_useModeName);
    }
}
