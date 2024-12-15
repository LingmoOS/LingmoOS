// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loglistview.h"
#include "logapplicationhelper.h"
#include "dbusmanager.h"
#include "DebugTimeManager.h"
#include "logapplicationhelper.h"
#include "dbusproxy/dldbushandler.h"
#include "utils.h"

#include <DDesktopServices>
#include <DDialog>
#include <DDialogButtonBox>
#include <DInputDialog>
#include <DApplication>
#include <DStyle>
#include <DApplication>
#include <DSysInfo>

#include <QItemSelectionModel>
#include <QMargins>
#include <QPaintEvent>
#include <QPainter>
#include <QProcess> //add by Airy
#include <QDebug>
#include <QHeaderView>
#include <QToolTip>
#include <QDir>
#include <QDir>
#include <QMenu>
#include <QShortcut>
#include <QAbstractButton>
#define ITEM_HEIGHT 40
#define ITEM_HEIGHT_COMPACT 24
#define ITEM_WIDTH 108
#define ICON_SIZE 16

#define ICON_DATA (Qt::UserRole + 99)

Q_DECLARE_METATYPE(QMargins)

DWIDGET_USE_NAMESPACE

const QVariant VListViewItemMargin = QVariant::fromValue(QMargins(15, 0, 5, 0));

LogListDelegate::LogListDelegate(LogListView *parent)
    : DStyledItemDelegate(parent)
{
}

void LogListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DStyledItemDelegate::paint(painter, option, index);
    LogListView *parentView = qobject_cast<LogListView *>(this->parent());
    if ((option.state & QStyle::State_HasFocus) && parentView && (parentView->focusReson() == Qt::TabFocusReason || parentView->focusReson() == Qt::BacktabFocusReason) && (parentView->hasFocus())) {
        // draw focus
        auto *style = dynamic_cast<DStyle *>(DApplication::style());
        QRect rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());

        if (index.isValid()) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(option);
            o.state |= QStyle::State_KeyboardFocusChange | QStyle::State_HasFocus;
            o.rect = style->visualRect(Qt::LeftToRight, option.rect, option.rect);
            style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, painter);
        }
    }
}

/**
 * @brief LogListDelegate::helpEvent 重写helpEvent以让tooltip能在鼠标移出item项目后正确消失
 * @param event
 * @param view
 * @param option
 * @param index
 * @return
 */
bool LogListDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QToolTip::hideText();
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(Qt::DisplayRole).toString();
        //如果tooltip不为空且合法则显示，否则直接关闭所有tooltip
        if (tooltip.isEmpty() || tooltip == "_split_") {
            hideTooltipImmediately();
        } else {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
            QToolTip::showText(event->globalPos(), strtooltip, view);
        }
        return false;
    }
    return DStyledItemDelegate::helpEvent(event, view, option, index);
}

/**
 * @brief LogListDelegate::hideTooltipImmediately 隐藏所有tooltip
 */
void LogListDelegate::hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

LogListView::LogListView(QWidget *parent)
    : DListView(parent)
{
    initUI();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &LogListView::customContextMenuRequested, this, &LogListView::requestshowRightMenu);
    //DGuiApplicationHelper::ColorType ct = DApplicationHelper::instance()->themeType();

    m_rightClickTriggerShortCut = new QShortcut(this);
    m_rightClickTriggerShortCut->setKey(Qt::ALT + Qt::Key_M);
    m_rightClickTriggerShortCut->setContext(Qt::WidgetShortcut);
    m_rightClickTriggerShortCut->setAutoRepeat(false);
    connect(m_rightClickTriggerShortCut, &QShortcut::activated, this, [this] {
        QRect r = rectForIndex(this->currentIndex());
        showRightMenu(QPoint(r.x() + r.width() / 2, r.y() + r.height() / 2), true);
    });

    connect(LogApplicationHelper::instance(), &LogApplicationHelper::sigValueChanged, this, &LogListView::slot_valueChanged_dConfig_or_gSetting);

#ifdef DTKWIDGET_CLASS_DSizeMode
    // 紧凑模式信号处理
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &LogListView::updateSizeMode);
    updateSizeMode();
#endif
}

/**
 * @brief LogListView::initUI 设置基本属性，且本listview为固定的种类，所以在此初始化函数中根据日志文件是否存在动态显示日志种类
 */
void LogListView::initUI()
{
    this->setMinimumWidth(150);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setItemDelegate(new LogListDelegate(this));
    this->setItemSpacing(0);
    this->setViewportMargins(10, 10, 10, 0);
    Dtk::Core::DSysInfo::UosEdition edition = Dtk::Core::DSysInfo::uosEditionType();
    //等于服务器行业版或欧拉版(centos)
    bool isCentos = Dtk::Core::DSysInfo::UosEuler == edition || Dtk::Core::DSysInfo::UosEnterpriseC == edition || Dtk::Core::DSysInfo::UosMilitaryS == edition;
    m_pModel = new QStandardItemModel(this);
    QStandardItem *item = nullptr;
    if (isFileExist("/var/log/journal") || isCentos) {
        item = new QStandardItem(QIcon::fromTheme("dp_system"), DApplication::translate("Tree", "System Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(DApplication::translate("Tree", "System Log")); // add by Airy for bug 16245
        item->setData(JOUR_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        m_logTypes.push_back(JOUR_TREE_DATA);
    }

    if (isCentos) {
        item = new QStandardItem(QIcon::fromTheme("dp_core"), DApplication::translate("Tree", "Kernel Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(DApplication::translate("Tree", "Kernel Log")); // add by Airy for bug 16245
        item->setData(DMESG_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        m_logTypes.push_back(DMESG_TREE_DATA);
    } else {
        if (isFileExist("/var/log/kern.log")) {
            item = new QStandardItem(QIcon::fromTheme("dp_core"), DApplication::translate("Tree", "Kernel Log"));
            setIconSize(QSize(ICON_SIZE, ICON_SIZE));
            item->setToolTip(DApplication::translate("Tree", "Kernel Log")); // add by Airy for bug 16245
            item->setData(KERN_TREE_DATA, ITEM_DATE_ROLE);
            item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
            item->setData(VListViewItemMargin, Dtk::MarginsRole);
            m_pModel->appendRow(item);
            m_logTypes.push_back(KERN_TREE_DATA);
        }
    }
    if (Utils::isWayland()) {
        item = new QStandardItem(QIcon::fromTheme("dp_start"), DApplication::translate("Tree", "Boot Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(DApplication::translate("Tree", "Boot Log")); // add by Airy for bug 16245
        item->setData(BOOT_KLU_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        m_logTypes.push_back(BOOT_KLU_TREE_DATA);
    } else {
        item = new QStandardItem(QIcon::fromTheme("dp_start"), DApplication::translate("Tree", "Boot Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(DApplication::translate("Tree", "Boot Log")); // add by Airy for bug 16245
        item->setData(BOOT_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        m_logTypes.push_back(BOOT_TREE_DATA);
    }
    if (isCentos) {
        item = new QStandardItem(QIcon::fromTheme("dp_d"), DApplication::translate("Tree", "dnf Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(DApplication::translate("Tree", "dnf Log"));
        item->setData(DNF_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        m_logTypes.push_back(DNF_TREE_DATA);
    } else {
        if (isFileExist("/var/log/dpkg.log")) {
            item = new QStandardItem(QIcon::fromTheme("dp_d"), DApplication::translate("Tree", "dpkg Log"));
            setIconSize(QSize(ICON_SIZE, ICON_SIZE));
            item->setToolTip(DApplication::translate("Tree", "dpkg Log")); // add by Airy for bug 16245
            item->setData(DPKG_TREE_DATA, ITEM_DATE_ROLE);
            item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
            item->setData(VListViewItemMargin, Dtk::MarginsRole);
            m_pModel->appendRow(item);
            m_logTypes.push_back(DPKG_TREE_DATA);
        }
    }
    //wayland环境才有kwin日志
    if (Utils::isWayland()) {
        item = new QStandardItem(QIcon::fromTheme("dp_kwin"), DApplication::translate("Tree", "Kwin Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(DApplication::translate("Tree", "Kwin Log"));
        item->setData(KWIN_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        m_logTypes.push_back(KWIN_TREE_DATA);
    } else {
        item = new QStandardItem(QIcon::fromTheme("dp_x"), DApplication::translate("Tree", "Xorg Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(DApplication::translate("Tree", "Xorg Log")); // add by Airy for bug 16245
        item->setData(XORG_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        m_logTypes.push_back(XORG_TREE_DATA);
    }
    auto *appHelper = LogApplicationHelper::instance();
    QMap<QString, QString> appMap = appHelper->getMap();
    if (!appMap.isEmpty()) {
        item = new QStandardItem(QIcon::fromTheme("dp_application"), DApplication::translate("Tree", "Application Log"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setToolTip(
            DApplication::translate("Tree", "Application Log")); // add by Airy for bug 16245
        item->setData(APP_TREE_DATA, ITEM_DATE_ROLE);
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        this->setModel(m_pModel);
        m_logTypes.push_back(APP_TREE_DATA);  
    }

    // coredump log
    item = new QStandardItem(QIcon::fromTheme("dp_customlog"), DApplication::translate("Tree", "Coredump Log"));
    setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    item->setToolTip(DApplication::translate("Tree", "Coredump Log"));
    item->setData(COREDUMP_TREE_DATA, ITEM_DATE_ROLE);
    item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    item->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pModel->appendRow(item);
    m_logTypes.push_back(COREDUMP_TREE_DATA);


    // add by Airy
    if (isFileExist("/var/log/wtmp")) {
        item = new QStandardItem(QIcon::fromTheme("dp_onoff"), DApplication::translate("Tree", "Boot-Shutdown Event"));
        setIconSize(QSize(ICON_SIZE, ICON_SIZE));
        item->setData(LAST_TREE_DATA, ITEM_DATE_ROLE);
        item->setToolTip(
            DApplication::translate("Tree", "Boot-Shutdown Event")); // add by Airy for bug 16245
        item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
        item->setData(VListViewItemMargin, Dtk::MarginsRole);
        m_pModel->appendRow(item);
        this->setModel(m_pModel);
        m_logTypes.push_back(LAST_TREE_DATA);
    }

    //other
    item = new QStandardItem(QIcon::fromTheme("dp_customlog", QIcon(":/customlog.svg")), DApplication::translate("Tree", "Other Log"));
    setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    item->setToolTip(DApplication::translate("Tree", "Other Log"));
    item->setData(OTHER_TREE_DATA, ITEM_DATE_ROLE);
    item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    item->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pModel->appendRow(item);
    m_logTypes.push_back(OTHER_TREE_DATA);

    //custom
    if (LogApplicationHelper::instance()->getCustomLogList().size()) {
        m_logTypes.push_back(CUSTOM_TREE_DATA);
        initCustomLogItem();
    }

    // 审计日志 （因dbus安全整改要求，isFileExist接口需要进行polkit鉴权，因此调整为程序启动时默认显示审计日志）
    item = new QStandardItem(QIcon::fromTheme("dp_customlog"), DApplication::translate("Tree", "Audit Log"));
    setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    item->setToolTip(DApplication::translate("Tree", "Audit Log"));
    item->setData(AUDIT_TREE_DATA, ITEM_DATE_ROLE);
    item->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    item->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pModel->appendRow(item);
    m_logTypes.push_back(AUDIT_TREE_DATA);

    DLDBusHandler::instance(this)->whiteListOutPaths();

    // set first item is select when app start
    if (m_pModel->rowCount() > 0) {
        this->setCurrentIndex(m_pModel->index(0, 0));
    }  
}

void LogListView::initCustomLogItem()
{
    if (!m_customLogItem) {
        m_customLogItem = new QStandardItem(QIcon::fromTheme("dp_customlog", QIcon(":/customlog.svg")), DApplication::translate("Tree", "Custom Log"));
    }

    setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_customLogItem->setToolTip(DApplication::translate("Tree", "Custom Log"));
    m_customLogItem->setData(CUSTOM_TREE_DATA, ITEM_DATE_ROLE);
    m_customLogItem->setSizeHint(QSize(ITEM_WIDTH, ITEM_HEIGHT));
    m_customLogItem->setData(VListViewItemMargin, Dtk::MarginsRole);
    m_pModel->appendRow(m_customLogItem);
}

QStringList LogListView::getAllFiles(const QString &file)
{
    QStringList files;
    if (file.contains("*")) {
        QString path = file.left(file.lastIndexOf('/'));
        QDir dir(path);
        QStringList filters;
        filters << file.split('/').last();
        files = dir.entryList(filters, QDir::Files);

        for (auto &file : files) {
            file = path + "/" + file;
        }
    } else {
        files << file;
    }
    return files;
}

void LogListView::setDefaultSelect()
{
    setCurrentIndex(currentIndex());
    itemChanged(currentIndex());
}

/**
 * @brief LogListView::isFileExist 判断文件路径是否存在
 * @param iFile 要判断的文件路径字符串
 * @return
 */
bool LogListView::isFileExist(const QString &iFile)
{
    QFile file(iFile);
    return file.exists();
}

/**
 * @brief LogListView::paintEvent 绘制背景颜色为全是base角色
 * @param event
 */
void LogListView::paintEvent(QPaintEvent *event)
{
    DPalette pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.color(DPalette::Base));
    pa.setBrush(DPalette::Background, pa.color(DPalette::Base));
    this->setPalette(pa);
    DApplicationHelper::instance()->setPalette(this, pa);

    this->setAutoFillBackground(true);

    DListView::paintEvent(event);
}

/**
 * @brief LogListView::currentChanged 虚函数中变换图标的选中状态
 * @param current
 * @param previous
 */
void LogListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    PERF_PRINT_BEGIN("POINT-03", "");
    if (current.row() < 0) {
        return;
    }

    emit itemChanged(current);
    DListView::currentChanged(current, previous);
}

/**
 * @author Airy
 * @brief LogListView::truncateFile 清空日志文件内容
 * @param path_
 */
void LogListView::truncateFile(QString path_)
{
    QProcess prc;
    if (path_ == KERN_TREE_DATA || path_ == BOOT_TREE_DATA || path_ == DPKG_TREE_DATA || path_ == KWIN_TREE_DATA) {
        QStringList files = getAllFiles(path_.append("*"));
        prc.start("pkexec", QStringList() << "logViewerTruncate" << files.join(' '));
    } else if (path_ == XORG_TREE_DATA) {
        path_ = "/var/log/Xorg*.log*";
        QStringList files = getAllFiles(path_);
        prc.start("pkexec", QStringList() << "logViewerTruncate" << files.join(' '));
    } else if (path_ == DNF_TREE_DATA) {
        path_ = "/var/log/dnf*.log*";
        QStringList files = getAllFiles(path_);
        prc.start("pkexec", QStringList() << "logViewerTruncate" << files.join(' '));
    } else {
        QStringList files = getAllFiles(path_.append("*"));
        prc.start("pkexec", QStringList() << "logViewerTruncate" << files.join(' '));
    }

    prc.waitForFinished();
}

/**
 * @author Airy
 * @brief LogListView::slot_getAppPath  清空应用日志时，当前显示应用日志的目录由外部提供并赋予成员变量
 * @param app 要设置的当前应用项目名称
 */
void LogListView::slot_getAppPath(int id, const QString &app)
{
    Q_UNUSED(id);

    AppLogConfig appConfig = LogApplicationHelper::instance()->appLogConfig(app);
    if (!appConfig.isValid())
        g_path = "";
    else {
        std::vector<SubModuleConfig> vSubmodules = appConfig.subModules.toVector().toStdVector();
        auto it = std::find_if(vSubmodules.begin(), vSubmodules.end(), [=](SubModuleConfig submodule) {
            return submodule.logType == "file" && submodule.name == appConfig.name;
        });

        if (it != vSubmodules.end()) {
            g_path = it->logPath;
            return;
        }

        g_path = "";
    }
}

Qt::FocusReason LogListView::focusReson()
{
    return m_reson;
}

void LogListView::showRightMenu(const QPoint &pos, bool isUsePoint)
{
    QModelIndex idx = this->currentIndex();
    QString pathData = idx.data(ITEM_DATE_ROLE).toString();
    if (!this->selectionModel()->selectedIndexes().empty()) {
        g_context = new QMenu(this);
        g_openForder = new QAction(/*tr("在文件管理器中显示")*/ DApplication::translate("Action", "Display in file manager"), this);
        g_clear = new QAction(/*tr("清除日志内容")*/ DApplication::translate("Action", "Clear log"), this);
        g_refresh = new QAction(/*tr("刷新")*/ DApplication::translate("Action", "Refresh"), this);

        g_context->addAction(g_openForder);
        g_context->addAction(g_clear);
        g_context->addAction(g_refresh);

        if (pathData == JOUR_TREE_DATA || pathData == LAST_TREE_DATA || pathData == BOOT_KLU_TREE_DATA
                || pathData == OTHER_TREE_DATA || pathData == CUSTOM_TREE_DATA || pathData == AUDIT_TREE_DATA
                || pathData == COREDUMP_TREE_DATA) {
            g_clear->setEnabled(false);
            g_openForder->setEnabled(false);
        }
        if (pathData == DMESG_TREE_DATA) {
            g_clear->setEnabled(false);
            g_openForder->setEnabled(false);
        }

        QString dirPath = Utils::homePath;
        QString _path_ = g_path; //get app path
        QString path = "";

        if (pathData == KERN_TREE_DATA || pathData == BOOT_TREE_DATA || pathData == DPKG_TREE_DATA || pathData == XORG_TREE_DATA || pathData == KWIN_TREE_DATA || pathData == DNF_TREE_DATA || pathData == DMESG_TREE_DATA) {
            path = pathData;
        } else if (pathData == APP_TREE_DATA) {
            path = _path_;
            g_clear->setEnabled(!path.isEmpty());
            g_openForder->setEnabled(!path.isEmpty());
        }

        //显示当前日志目录
        connect(g_openForder, &QAction::triggered, this, [=] {
            if (pathData != COREDUMP_TREE_DATA) {
                DDesktopServices::showFileItem(path);
            } else {
                DDesktopServices::showFolder(path);
            }
        });

        QModelIndex index = idx;
        //刷新逻辑
        connect(g_refresh, &QAction::triggered, this, [=]() {
            emit sigRefresh(index);
        });

        //清除日志逻辑
        connect(g_clear, &QAction::triggered, this, [=]() {
            DDialog *dialog = new DDialog(this);
            dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowStaysOnTopHint);
            dialog->setIcon(QIcon::fromTheme("dialog-warning"));
            dialog->setMessage(/*"清除日志内容"*/ DApplication::translate("Action", "Are you sure you want to clear the log?"));
            dialog->addButton(QString(/*tr("取消")*/ DApplication::translate("Action", "Cancel")), false, DDialog::ButtonNormal);
            dialog->addButton(QString(/*tr("确定")*/ DApplication::translate("Action", "Confirm")), true, DDialog::ButtonRecommend);
            int Ok = dialog->exec();
            if (Ok == DDialog::Accepted) {
                truncateFile(path);
                emit sigRefresh(index);
            }
        });

        this->setContextMenuPolicy(Qt::CustomContextMenu);
        QPoint p;
        if (isUsePoint) {
            p = mapToGlobal(pos);
        } else {
            p = QCursor::pos();
        }
        g_context->exec(p);
    }
}

void LogListView::requestshowRightMenu(const QPoint &pos)
{
    if (this->indexAt(pos).isValid()) {
        showRightMenu(pos, false);
    }
}

void LogListView::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (QToolTip::isVisible()) {
        QToolTip::hideText();
    }
    return;
}

void LogListView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        if (currentIndex().row() == 0) {
            QModelIndex modelIndex = model()->index(model()->rowCount() - 1, 0);
            setCurrentIndex(modelIndex);
        } else {
            DListView::keyPressEvent(event);
        }
    } else if (event->key() == Qt::Key_Down) {
        if (currentIndex().row() == model()->rowCount() - 1) {
            QModelIndex modelIndex = model()->index(0, 0);
            setCurrentIndex(modelIndex);
        } else {
            DListView::keyPressEvent(event);
        }
    } else {
        DListView::keyPressEvent(event);
    }
}

void LogListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit clicked(indexAt(event->pos()));
    }
    DListView::mousePressEvent(event);
}

void LogListView::focusInEvent(QFocusEvent *event)
{
    if ((event->reason() != Qt::PopupFocusReason) && (event->reason() != Qt::ActiveWindowFocusReason)) {
        m_reson = event->reason();
    }
    DListView::focusInEvent(event);
}

void LogListView::focusOutEvent(QFocusEvent *event)
{
    DListView::focusOutEvent(event);
}

void LogListView::slot_valueChanged_dConfig_or_gSetting(const QString &key)
{
    if (key == "customLogFiles" || key == "customlogfiles") {
        int size = LogApplicationHelper::instance()->getCustomLogList().size();
        if (size > 0 && (!m_customLogItem || !m_pModel->indexFromItem(m_customLogItem).isValid())) {
            initCustomLogItem();
        } else if(size <= 0 && m_customLogItem && m_pModel->indexFromItem(m_customLogItem).isValid()) {
            // set first item is select
            if (currentIndex() == m_customLogItem->index()) {
               this->setCurrentIndex(m_pModel->index(0, 0));
            }
            m_pModel->removeRow(m_customLogItem->row());
            m_customLogItem = nullptr;
        }
    }
}

void LogListView::updateSizeMode()
{
    int nItemHeight = ITEM_HEIGHT;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::isCompactMode())
        nItemHeight = ITEM_HEIGHT_COMPACT;
    else
        nItemHeight = ITEM_HEIGHT;
#else
    nItemHeight = ITEM_HEIGHT;
#endif

    if (m_pModel) {
        int nCount = m_pModel->rowCount();
        for (int i = 0; i < nCount; i++) {
            QStandardItem* item = m_pModel->item(i);
            item->setSizeHint(QSize(ITEM_WIDTH, nItemHeight));
        }
    }
}
