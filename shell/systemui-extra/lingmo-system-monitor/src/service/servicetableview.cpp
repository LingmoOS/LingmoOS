/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "servicetableview.h"

#include "../macro.h"
#include "../util.h"
#include "systemd1serviceinterface.h"
#include "../dialog/customservicenamedlg.h"
#include "../krightwidget.h"

#include <QApplication>
#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QMenu>
#include <QPalette>
#include <QSlider>
#include <QToolTip>
#include <QWidget>
#include <QHeaderView>

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QtConcurrent>

ServiceTableView::ServiceTableView(QSettings* svcSettings, QWidget *parent)
    : KTableView(parent), m_svcSettings(svcSettings)
{
    // 安装按键事件过滤器
    installEventFilter(this);
    this->setProperty("useIconHighlightEffect", 0x2);

    // 模型和排序过滤模型初始化实例
    m_model = new ServiceTableModel(this);
    m_proxyModel = new ServiceSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    setModel(m_proxyModel);

    // 加载服务列表UI配置
    bool settingsLoaded = loadSettings();

    // 初始化ui控件和信号槽连接
    initUI(settingsLoaded);
    initConnections(settingsLoaded);
    mApplyShortcut = new QShortcut(QKeySequence("Shift+F10"), this, nullptr, nullptr, Qt::WidgetWithChildrenShortcut);
    connect(mApplyShortcut, SIGNAL(activated()), this, SLOT(showMenusByShortCut()));
}

void ServiceTableView::showMenusByShortCut()
{
    if (m_headerContextMenu->isVisible()) {
        m_headerContextMenu->hide();
    } else {
        if (this->header()->hasFocus()) {
            displayServiceTableHeaderContextMenu(this->getHeaderMousePressPoint());
        }
    }

    if (m_contextMenu->isVisible()) {
        m_contextMenu->hide();
    } else {
        if (this->hasFocus()) {
            m_contextMenu->close();
            displayServiceTableContextMenu(p);
        }
    }
}

void ServiceTableView::showWarningMsg(const KError& ke, const QString &sname)
{
    QString errorMsg = ke.getErrorMessage();
    if (errorMsg.contains("is masked")) {
        errorMsg = tr("Unit %1 is masked.").arg(sname);
    }
    if (errorMsg.contains("not found")) {
        errorMsg = tr("Unit %1 not found.").arg(sname);
    }
    if (ke && ke.getErrorMessage() != "Permission denied") {
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, ke.getErrorName(),
                                              errorMsg, QMessageBox::NoButton, this);
        msgBox->addButton(tr("OK"), QMessageBox::AcceptRole);
        msgBox->exec();
    }
}

ServiceTableView::~ServiceTableView()
{
}

void ServiceTableView::onWndClose()
{
    saveSettings();
}

// 过滤被设置事件监听的对象的指定事件
bool ServiceTableView::eventFilter(QObject *obj, QEvent *event)
{
    return KTableView::eventFilter(obj, event);
}

// 搜索服务
void ServiceTableView::onSearch(const QString text)
{
    if (!isVisible())
        return;
    m_proxyModel->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive));
    if (text == "") {
        adjustInfoLabelVisibility(true);        
    } else {
        adjustInfoLabelVisibility();
    }
    
}

// 初始化UI控件
void ServiceTableView::initUI(bool settingsLoaded)
{
    setAccessibleName("ServiceTableView");

    // 初始化没有搜索结果控件
    m_notFoundPixLabel = new QLabel(this);
    m_notFoundPixLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_notFoundPixLabel->setVisible(false);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("lingmo-dark" == currentThemeMode || "lingmo-black" == currentThemeMode){
        m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_black.png")).scaled(96,96));
    } else {
        m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_white.png")).scaled(96,96));
    }
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(key).toString();
            if ("lingmo-black" == currentThemeMode || "lingmo-dark" == currentThemeMode) {
                m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_black.png")).scaled(96,96));
            } else if("lingmo-white" == currentThemeMode || "lingmo-default" == currentThemeMode) {
                m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_white.png")).scaled(96,96));
            }
        }
    });

    m_notFoundTextLabel = new QLabel(tr("No search result"), this);
    m_notFoundTextLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_notFoundTextLabel->setVisible(false);

    QVBoxLayout *notFoundVLayout = new QVBoxLayout(this);
    notFoundVLayout->setContentsMargins(0,0,0,0);
    notFoundVLayout->addWidget(m_notFoundPixLabel);
    notFoundVLayout->addWidget(m_notFoundTextLabel);

    // 设置表头属性
    header()->setSectionsMovable(true); // 表头项可移动
    header()->setSectionsClickable(true); // 表头项可点击
    header()->setSectionResizeMode(QHeaderView::Interactive); // 表头项可伸缩
    header()->setStretchLastSection(true); // 表头项最后一一项拉伸填充
    header()->setSortIndicatorShown(true); // 表头排序项上显示排序指向
    header()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter); // 表头项默认对齐方式
    header()->setContextMenuPolicy(Qt::CustomContextMenu); // 表头支持菜单协议

    // 表格配置
    setSortingEnabled(true);    // 使能排序
    setSelectionMode(QAbstractItemView::SingleSelection); // 单选模式
    setSelectionBehavior(QAbstractItemView::SelectRows); // 选择整行
    setContextMenuPolicy(Qt::CustomContextMenu); //表格支持菜单协议

    // 表格菜单和表头菜单实例
    m_contextMenu = new QMenu(this);
    m_contextMenu->setObjectName("ServiceMenu");
    // 添加表格菜单项
    m_actStartService = m_contextMenu->addAction(tr("Start"));
    m_actStopService = m_contextMenu->addAction(tr("Stop"));
    m_actRestartService = m_contextMenu->addAction(tr("Restart"));
    m_menuSevieStartupMode = m_contextMenu->addMenu(tr("Startup type"));
    m_actAutoStart = m_menuSevieStartupMode->addAction(tr("Auto"));
    m_actManualStart = m_menuSevieStartupMode->addAction(tr("Manual"));
    m_actRefresh = m_contextMenu->addAction(tr("Refresh"));

    m_headerContextMenu = new QMenu(this);
    m_headerContextMenu->setObjectName("ServiceHeaderMenu");
    // 添加表头菜单项
    m_actColumeLoadState = m_headerContextMenu->addAction(tr("Load"));
    m_actColumeLoadState->setCheckable(true);
    m_actColumeActiveState = m_headerContextMenu->addAction(tr("Active"));
    m_actColumeActiveState->setCheckable(true);
    m_actColumeSubState = m_headerContextMenu->addAction(tr("Sub"));
    m_actColumeSubState->setCheckable(true);
    m_actColumeState = m_headerContextMenu->addAction(tr("State"));
    m_actColumeState->setCheckable(true);
    m_actColumeStartupMode = m_headerContextMenu->addAction(tr("Startup Type"));
    m_actColumeStartupMode->setCheckable(true);
    m_actColumeDescription = m_headerContextMenu->addAction(tr("Description"));
    m_actColumeDescription->setCheckable(true);
    m_actColumeMainPID = m_headerContextMenu->addAction(tr("PID"));
    m_actColumeMainPID->setCheckable(true);

    // 设置默认的菜单选中项
    if (!settingsLoaded) {
        m_actColumeLoadState->setChecked(false);
        m_actColumeActiveState->setChecked(true);
        m_actColumeSubState->setChecked(true);
        m_actColumeState->setChecked(true);
        m_actColumeStartupMode->setChecked(false);
        m_actColumeDescription->setChecked(true);
        m_actColumeMainPID->setChecked(false);

        // 名称
        setColumnWidth(ServiceTableModel::ServiceNameColumn, servicenamepadding);
        // 加载状态
        setColumnWidth(ServiceTableModel::ServiceLoadStateColumn, serviceloadstatepadding);
        // 活动状态
        setColumnWidth(ServiceTableModel::ServiceActiveStateColumn, serviceactivestatepadding);
        // 子状态
        setColumnWidth(ServiceTableModel::ServiceSubStateColumn, servicesubstatepadding);
        // 状态
        setColumnWidth(ServiceTableModel::ServiceStateColumn, servicestatepadding);
        // 描述
        setColumnWidth(ServiceTableModel::ServiceDescriptionColumn, servicedescpadding);
        // 进程id
        setColumnWidth(ServiceTableModel::ServiceMainPIDColumn, servicemainpidpadding);
        // 启动模式
        setColumnWidth(ServiceTableModel::ServiceStartupModeColumn, servicestartupmodepadding);

        setColumnHidden(ServiceTableModel::ServiceNameColumn, false);
        setColumnHidden(ServiceTableModel::ServiceLoadStateColumn, true);
        setColumnHidden(ServiceTableModel::ServiceActiveStateColumn, false);
        setColumnHidden(ServiceTableModel::ServiceStateColumn, false);
        setColumnHidden(ServiceTableModel::ServiceDescriptionColumn, false);
        setColumnHidden(ServiceTableModel::ServiceSubStateColumn, false);
        setColumnHidden(ServiceTableModel::ServiceMainPIDColumn, true);
        setColumnHidden(ServiceTableModel::ServiceStartupModeColumn, true);

        //默认排序列
        sortByColumn(ServiceTableModel::ServiceNameColumn, Qt::DescendingOrder);
    }
}

// 初始化信号槽连接
void ServiceTableView::initConnections(bool settingsLoaded)
{
    Q_UNUSED(settingsLoaded);
    // 列表菜单请求
    connect(this, &ServiceTableView::customContextMenuRequested, this,
            &ServiceTableView::displayServiceTableContextMenu);

    QHeaderView *header = this->header();
    // 表头菜单请求
    connect(header, &QHeaderView::customContextMenuRequested, this,
            &ServiceTableView::displayServiceTableHeaderContextMenu);

    // 启动服务 菜单事件
    connect(m_actStartService, &QAction::triggered, this, &ServiceTableView::startService);
    // 停止服务 菜单事件
    connect(m_actStopService, &QAction::triggered, this, &ServiceTableView::stopService);
    // 重启服务 菜单事件
    connect(m_actRestartService, &QAction::triggered, this, &ServiceTableView::restartService);
    // 自动启动 菜单事件
    connect(m_actAutoStart, &QAction::triggered, this, [ = ]() { setServiceStartupMode(true); });
    // 自动启动 菜单事件
    connect(m_actManualStart, &QAction::triggered, this, [ = ]() { setServiceStartupMode(false); });
    // 刷新列表 菜单事件
    connect(m_actRefresh, &QAction::triggered, this, &ServiceTableView::refreshList);

    // 根据服务状态修改菜单选项状态
    connect(m_contextMenu, &QMenu::aboutToShow, this, [ = ]() {
        if (selectionModel()->selectedRows().size() > 0) {
            // 选中的模型索引
            auto index = selectionModel()->selectedRows()[0];
            if (index.isValid()) {
                const QModelIndex &sourceIndex = m_proxyModel->mapToSource(index);
                // 获取服务状态和名称
                auto state = m_model->getServiceState(sourceIndex);
                auto sname = m_model->getServiceName(sourceIndex);
                // 当服务名称以@结尾或者不可操作时，禁用修改启动模式
                if (sname.endsWith("@") || ServiceManager::isServiceNoOp(state)) {
                    m_menuSevieStartupMode->setEnabled(false);
                } else {
                    m_menuSevieStartupMode->setEnabled(true);
                }

                auto activeState = m_model->getServiceActiveState(sourceIndex);
                if (ServiceManager::isActiveState(activeState.toLocal8Bit())) {
                    m_actStartService->setEnabled(false);
                    m_actStopService->setEnabled(true);
                    m_actRestartService->setEnabled(true);
                } else {
                    m_actStartService->setEnabled(true);
                    m_actStopService->setEnabled(false);
                    m_actRestartService->setEnabled(false);
                }

                // 根据服务当前启动模式互斥使能启动模式选项
                if (ServiceManager::isServiceAutoStartup(sname, state)) {
                    m_actAutoStart->setEnabled(false);
                    m_actManualStart->setEnabled(true);
                } else {
                    m_actAutoStart->setEnabled(true);
                    m_actManualStart->setEnabled(false);
                }
            }
        }
    });

    // 显隐表格列
    connect(m_actColumeLoadState, &QAction::triggered, this, [ = ](bool b) {
        header->setSectionHidden(ServiceTableModel::ServiceLoadStateColumn, !b);
    });
    connect(m_actColumeActiveState, &QAction::triggered, this, [ = ](bool b) {
        header->setSectionHidden(ServiceTableModel::ServiceActiveStateColumn, !b);
    });
    connect(m_actColumeSubState, &QAction::triggered, this, [ = ](bool b) {
        header->setSectionHidden(ServiceTableModel::ServiceSubStateColumn, !b);
    });
    connect(m_actColumeState, &QAction::triggered, this, [ = ](bool b) {
        header->setSectionHidden(ServiceTableModel::ServiceStateColumn, !b);
    });
    connect(m_actColumeDescription, &QAction::triggered, this, [ = ](bool b) {
        header->setSectionHidden(ServiceTableModel::ServiceDescriptionColumn, !b);
    });
    connect(m_actColumeStartupMode, &QAction::triggered, this, [ = ](bool b) {
        header->setSectionHidden(ServiceTableModel::ServiceStartupModeColumn, !b);
    });
    connect(m_actColumeMainPID, &QAction::triggered, this, [ = ](bool b) {
        header->setSectionHidden(ServiceTableModel::ServiceMainPIDColumn, !b);
    });

    // 更新表头菜单状态
    connect(m_headerContextMenu, &QMenu::aboutToShow, this, [ = ]() {
        bool b;
        QHeaderView *header = this->header();
        b = header->isSectionHidden(ServiceTableModel::ServiceLoadStateColumn);
        m_actColumeLoadState->setChecked(!b);
        b = header->isSectionHidden(ServiceTableModel::ServiceActiveStateColumn);
        m_actColumeActiveState->setChecked(!b);
        b = header->isSectionHidden(ServiceTableModel::ServiceSubStateColumn);
        m_actColumeSubState->setChecked(!b);
        b = header->isSectionHidden(ServiceTableModel::ServiceStateColumn);
        m_actColumeState->setChecked(!b);
        b = header->isSectionHidden(ServiceTableModel::ServiceStartupModeColumn);
        m_actColumeStartupMode->setChecked(!b);
        b = header->isSectionHidden(ServiceTableModel::ServiceDescriptionColumn);
        m_actColumeDescription->setChecked(!b);
        b = header->isSectionHidden(ServiceTableModel::ServiceMainPIDColumn);
        m_actColumeMainPID->setChecked(!b);
    });

    connect(this, &ServiceTableView::errorHappened, this, &ServiceTableView::showWarningMsg);

    auto *manager = ServiceManager::instance();
    Q_ASSERT(manager != nullptr);
    connect(manager, &ServiceManager::updateList, this, [ = ]() {
        m_isDataLoading = true;
        // 禁用界面

        m_notFoundPixLabel->hide();
        m_notFoundTextLabel->hide();
    });
    // 列表刷新完成
    connect(m_model, &ServiceTableModel::modelReset, this, [ = ]() {
        // 使能界面

        m_notFoundPixLabel->hide();
        m_notFoundTextLabel->hide();

        m_isDataLoading = false;
    });

    // 处理模型动态增长显示
    connect(m_model, &ServiceTableModel::itemRowChanged, this,
    [ = ](int row) {
        m_proxyModel->fetchMore({});
        auto *selmo = selectionModel();
        selmo->select(m_proxyModel->mapFromSource(m_model->index(row, 0)),
                      QItemSelectionModel::SelectCurrent |
                      QItemSelectionModel::Rows |
                      QItemSelectionModel::Clear);
    });

    // 模型动态变化时刷新搜索结果提示
    connect(m_proxyModel, &ServiceTableModel::rowsInserted, this,
    [ = ]() {
        adjustInfoLabelVisibility();
    });
    connect(m_proxyModel, &ServiceTableModel::rowsRemoved, this,
    [ = ]() {
        adjustInfoLabelVisibility();
    });
}

// 显示服务表内容菜单
void ServiceTableView::displayServiceTableContextMenu(const QPoint &p)
{
    if (selectedIndexes().size() == 0)
        return;

    QPoint point = mapToGlobal(p);
    // 弹出菜单时不要挡住表头
    point.setY(point.y() + header()->sizeHint().height());
    m_contextMenu->popup(point);
}

// 显示服务表头菜单
void ServiceTableView::displayServiceTableHeaderContextMenu(const QPoint &p)
{
    m_headerContextMenu->popup(mapToGlobal(p));
}

// 列表大小改变事件
void ServiceTableView::resizeEvent(QResizeEvent *event)
{
    adjustInfoLabelVisibility();

    KTableView::resizeEvent(event);
}

// 列表显示事件
void ServiceTableView::showEvent(QShowEvent *)
{
    // 隐藏无结果提示
    if (m_notFoundTextLabel) {
        m_notFoundTextLabel->hide();
    }
    if (m_notFoundPixLabel) {
        m_notFoundPixLabel->hide();
    }
}

// 从其他页面切换显示当前页面
void ServiceTableView::onSwitchOnView()
{
    if (m_needFirstRefresh) {
        auto *manager = ServiceManager::instance();
        manager->updateServiceList();
        m_needFirstRefresh = false;
    }
}

// 启动服务
void ServiceTableView::startService()
{
    if (!m_selectedSName.isValid())
        return;

    auto sname = m_selectedSName.toString();

    // 当服务后缀为@时，可手动输入子服务名称
    if (sname.endsWith('@')) {
        if (selectionModel()->selectedRows().size() > 0) {
            // 选中的模型索引
            auto index = selectionModel()->selectedRows()[0];
            if (index.isValid()) {
                // 获取服务状态和名称
                QString desc = m_model->getServiceDescription(m_proxyModel->mapToSource(index));
                CustomServiceNameDlg dialog(tr("Service instance name"), desc, this);
                dialog.exec();
                if (dialog.getResult() == QMessageBox::Ok) {
                    auto subName = dialog.getCustomServiceName();
                    if (subName.isEmpty())
                        return;
                    sname = sname.append(subName);
                } else {  // cancel clicked
                    return;
                }
            } else {
                return;
            }
        } else {
            return;
        }
    }

    auto *manager = ServiceManager::instance();
    Q_ASSERT(manager != nullptr);

    // 异步启动服务
    auto *watcher  = new QFutureWatcher<KError>();
    connect(watcher, &QFutureWatcher<KError>::finished, this, [this, manager, watcher, sname]() {
        refreshServiceInfo(sname);
        // 有错误发生，弹出错误提示窗口
        if (watcher->result()) {
            Q_EMIT this->errorHappened(watcher->result(), sname);
            KError ke = watcher->result();
            qWarning()<<ke.getErrorName()<<":"<<ke.getErrorMessage();
        }
        watcher->deleteLater();
    });
    QFuture<KError> fu = QtConcurrent::run([manager, sname]() {
        auto ec = manager->startService(sname);
        return ec;
    });
    // 运行异步任务
    watcher->setFuture(fu);
}

// 停止服务
void ServiceTableView::stopService()
{
    if (!m_selectedSName.isValid()) {
        return;
    }
    auto sname = m_selectedSName.toString();

    // 当服务后缀为@时，可手动输入子服务名称
    if (sname.endsWith('@')) {
        if (selectionModel()->selectedRows().size() > 0) {
            // 选中的模型索引
            auto index = selectionModel()->selectedRows()[0];
            if (index.isValid()) {
                // 获取服务状态和名称
                QString desc = m_model->getServiceDescription(m_proxyModel->mapToSource(index));
                CustomServiceNameDlg dialog(tr("Service instance name"), desc, this);
                dialog.exec();
                if (dialog.getResult() == QMessageBox::Ok) {
                    auto subName = dialog.getCustomServiceName();
                    if (subName.isEmpty())
                        return;
                    sname = sname.append(subName);
                } else {  // cancel clicked
                    return;
                }
            } else {
                return;
            }
        } else {
            return;
        }
    }

    auto *mgr = ServiceManager::instance();
    Q_ASSERT(mgr != nullptr);

    // 异步方式停止服务
    auto *watcher  = new QFutureWatcher<KError>();
    connect(watcher, &QFutureWatcher<KError>::finished, this, [this, mgr, watcher, sname]() {
        refreshServiceInfo(sname);
        if (watcher->result()) {
            Q_EMIT this->errorHappened(watcher->result(), sname);
            KError ke = watcher->result();
            qWarning()<<ke.getErrorName()<<":"<<ke.getErrorMessage();
        }
        watcher->deleteLater();
    });
    QFuture<KError> fu = QtConcurrent::run([mgr, sname]() {
        auto ec = mgr->stopService(sname);
        return ec;
    });
    // 运行异步任务
    watcher->setFuture(fu);
}

// 重启服务
void ServiceTableView::restartService()
{
    if (!m_selectedSName.isValid()) {
        return;
    }
    auto sname = m_selectedSName.toString();
    // 当服务后缀为@时，可手动输入子服务名称
    if (sname.endsWith('@')) {
        if (selectionModel()->selectedRows().size() > 0) {
            // 选中的模型索引
            auto index = selectionModel()->selectedRows()[0];
            if (index.isValid()) {
                // 获取服务状态和名称
                QString desc = m_model->getServiceDescription(m_proxyModel->mapToSource(index));
                CustomServiceNameDlg dialog(tr("Service instance name"), desc, this);
                dialog.exec();
                if (dialog.getResult() == QMessageBox::Ok) {
                    auto subName = dialog.getCustomServiceName();
                    if (subName.isEmpty())
                        return;
                    sname = sname.append(subName);
                } else {  // cancel clicked
                    return;
                }
            } else {
                return;
            }
        } else {
            return;
        }
    }

    auto *manager = ServiceManager::instance();
    Q_ASSERT(manager != nullptr);

    // 异步方式重启服务
    auto *watcher  = new QFutureWatcher<KError>();
    connect(watcher, &QFutureWatcher<KError>::finished, this, [this, manager, watcher, sname]() {
        refreshServiceInfo(sname);
        if (watcher->result()) {
            Q_EMIT this->errorHappened(watcher->result(), sname);
            KError ke = watcher->result();
            qWarning()<<ke.getErrorName()<<":"<<ke.getErrorMessage();
        }
        watcher->deleteLater();
    });
    QFuture<KError> fu = QtConcurrent::run([manager, sname]() {
        auto ec = manager->restartService(sname);
        return ec;
    });
    // 运行异步任务
    watcher->setFuture(fu);
}

// 设置服务启动模式
void ServiceTableView::setServiceStartupMode(bool autoStart)
{
    if (!m_selectedSName.isValid()) {
        return;
    }

    auto sname = m_selectedSName.toString();

    auto *manager = ServiceManager::instance();
    Q_ASSERT(manager != nullptr);

    // 异步方式设置服务启动模式
    auto *watcher  = new QFutureWatcher<KError>();
    connect(watcher, &QFutureWatcher<KError>::finished, this, [this, manager, watcher, sname]() {
        refreshServiceInfo(sname);
        if (watcher->result()) {
            Q_EMIT this->errorHappened(watcher->result(), sname);
            KError ke = watcher->result();
            qWarning()<<ke.getErrorName()<<":"<<ke.getErrorMessage();
        }
        watcher->deleteLater();
    });
    QFuture<KError> fu = QtConcurrent::run([manager, sname, autoStart]() {
        auto ec = manager->setServiceStartupMode(sname, autoStart);
        return ec;
    });
    // 异步运行任务
    watcher->setFuture(fu);
}

// 选中条目改变
void ServiceTableView::selectionChanged(const QItemSelection &selected,
                                        const QItemSelection &deselected)
{
    if (selected.size() <= 0) {
        return;
    }

    m_selectedSName = selected.indexes().value(ServiceTableModel::ServiceNameColumn).data();

    QTreeView::selectionChanged(selected, deselected);
}

// 指点列的默认宽度
int ServiceTableView::sizeHintForColumn(int column) const
{
    int margin = 10;
    return std::max(header()->sizeHintForColumn(column) + margin * 2,
                    QTreeView::sizeHintForColumn(column) + margin * 2);
}

void ServiceTableView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        p = e->pos();
    }
    QTreeView::mouseMoveEvent(e);
}

// 调整提示信息显示
void ServiceTableView::adjustInfoLabelVisibility(bool force)
{
    setUpdatesEnabled(false);
    // 当模型索引行为空时显示没有搜索结果
    m_notFoundPixLabel->setVisible(m_proxyModel->rowCount() == 0);
    m_notFoundTextLabel->setVisible(m_proxyModel->rowCount() == 0);
    if (force || KRightWidget::searchText.isEmpty()) {
        m_notFoundPixLabel->setVisible(false);
        m_notFoundTextLabel->setVisible(false);
    }

    setUpdatesEnabled(true);
}

// 如果服务状态更新，刷新
void ServiceTableView::refreshServiceInfo(const QString sname)
{
    auto *manager = ServiceManager::instance();
    Q_ASSERT(manager != nullptr);

    auto sn = manager->normalizeServiceId(sname);
    auto o = Systemd1ServiceInterface::normalizeUnitPath(sn);
    // 获取更新后的serviceinfo
    auto service = manager->updateServiceInfo(o.path());

    // 如果服务状态不是最终状态，尝试多次获取最终状态
    if (!manager->isFinalState(service.getActiveState().toLocal8Bit())) {
        auto *timer = new DelayUpdateTimer(manager, this);
        timer->start(o.path());
    }
}

// 刷新服务列表
void ServiceTableView::refreshList()
{
    if (m_isDataLoading)
        return;

    // 重置模型和选中项服务名称
    m_model->reset();
    m_selectedSName.clear();

    ServiceManager::instance()->updateServiceList();
}

// 列表隐藏事件
void ServiceTableView::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

// 加载界面配置信息
bool ServiceTableView::loadSettings()
{
    if (m_svcSettings) {
        m_svcSettings->beginGroup("SERVICE");
        QVariant opt = m_svcSettings->value(SETTINGSOPTION_SERVICETABLEHEADERSTATE);
        m_svcSettings->endGroup();
        if (opt.isValid()) {
            QByteArray buf = QByteArray::fromBase64(opt.toByteArray());
            header()->restoreState(buf);
            return true;
        }
    }
    return false;
}

// 备份界面配置
void ServiceTableView::saveSettings()
{
    if (m_svcSettings) {
        QByteArray buf = header()->saveState();
        m_svcSettings->beginGroup("SERVICE");
        m_svcSettings->setValue(SETTINGSOPTION_SERVICETABLEHEADERSTATE, buf.toBase64());
        m_svcSettings->endGroup();
    }
}

void ServiceTableView::adjustColumnsSize()
{
    if (!model())
        return;

    if (model()->columnCount() == 0)
        return;

    // 名称
    setColumnWidth(ServiceTableModel::ServiceNameColumn, servicenamepadding);
    // 加载状态
    setColumnWidth(ServiceTableModel::ServiceLoadStateColumn, serviceloadstatepadding);
    // 活动状态
    setColumnWidth(ServiceTableModel::ServiceActiveStateColumn, serviceactivestatepadding);
    // 子状态
    setColumnWidth(ServiceTableModel::ServiceSubStateColumn, servicesubstatepadding);
    // 状态
    setColumnWidth(ServiceTableModel::ServiceStateColumn, servicestatepadding);
    // 描述
    setColumnWidth(ServiceTableModel::ServiceDescriptionColumn, servicedescpadding);
    // 进程id
    setColumnWidth(ServiceTableModel::ServiceMainPIDColumn, servicemainpidpadding);
    // 启动模式
    setColumnWidth(ServiceTableModel::ServiceStartupModeColumn, servicestartupmodepadding);

    int rightPartsSize = SLIDER_WIDTH;
    for (int n = ServiceTableModel::ServiceNameColumn; n < ServiceTableModel::ServiceColumnCount; n ++) {
        if (!header()->isSectionHidden(n)) {
            int nSectionWidth = 0;
            switch(n) {
            case ServiceTableModel::ServiceLoadStateColumn:
                nSectionWidth = servicenamepadding;
                break;
            case ServiceTableModel::ServiceActiveStateColumn:
                nSectionWidth = serviceactivestatepadding;
                break;
            case ServiceTableModel::ServiceSubStateColumn:
                nSectionWidth = servicesubstatepadding;
                break;
            case ServiceTableModel::ServiceStateColumn:
                nSectionWidth = servicestatepadding;
                break;
            case ServiceTableModel::ServiceDescriptionColumn:
                nSectionWidth = servicedescpadding;
                break;
            case ServiceTableModel::ServiceMainPIDColumn:
                nSectionWidth = servicemainpidpadding;
                break;
            case ServiceTableModel::ServiceStartupModeColumn:
                nSectionWidth = servicestartupmodepadding;
                break;
            default:
                break;
            }
            rightPartsSize += nSectionWidth;
        }
    }

    if(columnWidth(0) < columnWidth(1))
        setColumnWidth(0, columnWidth(1));

    if (this->width() - rightPartsSize < servicenamepadding) {
        int size = width() - servicenamepadding - SLIDER_WIDTH;
        size /= header()->count() - 1;
        setColumnWidth(0, servicenamepadding);
        for (int column = 1; column < model()->columnCount(); column++) {
            setColumnWidth(column, size);
        }
        return;
    }
    header()->resizeSection(ServiceTableModel::ServiceNameColumn, this->viewport()->width() - rightPartsSize);
}
