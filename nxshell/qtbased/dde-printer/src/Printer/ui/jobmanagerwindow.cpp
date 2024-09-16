// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jobmanagerwindow.h"
#include "zjobmanager.h"
#include "qtconvert.h"

#include "common.h"
#include "dprintermanager.h"
#include "dprinter.h"
#include "config.h"

#include <DTitlebar>
#include <DIconButton>
#include <DButtonBox>
#include <DListView>
#include <DDialog>
#include <DPalette>
#include <DApplicationHelper>
#include <DApplication>

#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QDebug>
#include <QDateTime>
#include <QMenu>
#include <QModelIndexList>
#include <QTimer>
#include <QPixmap>
#include <QRect>
#include <QMouseEvent>
#include <QMargins>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolTip>
#include <QCursor>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include <cups/ipp.h>
#include <map>
using namespace std;

#define JOB_ACTION_Cancel 0x01
#define JOB_ACTION_Delete 0x02
#define JOB_ACTION_Hold 0x04
#define JOB_ACTION_Release 0x08
#define JOB_ACTION_Priority 0x10
#define JOB_ACTION_Restart 0x20

#define JOB_ACTION_Count 6
#define ACTION_Column 7

#define JOB_VIEW_WIDTH 880
#define ITEM_Height 36
#define ACTION_BUT_SPACE 15

#define JOB_ITEM_ROLE_ID Qt::UserRole + 1
#define JOB_ITEM_ROLE_STATE Qt::UserRole + 2
#define JOB_ITEM_ROLE_ACTION Qt::UserRole + 3

typedef struct tagHoverAction {
    tagHoverAction()
        : iHoverAction(-1)
        , iLastAction(-1)
    {
    }

    QString toString()
    {
        return QString("last: %1 %2, current: %3 %4").arg(iLastRow).arg(iLastAction).arg(iHoverRow).arg(iHoverAction);
    }

    int iHoverRow;
    int iLastRow;
    int iHoverAction;
    int iLastAction;
} THoverAction;

static THoverAction g_hoverAction;

static QString formatDataTimeString(const QDateTime &dataTime)
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = dataTime.secsTo(now);

    if (secs < 2 * 60) {
        return QObject::tr("1 min ago");
    } else if (secs < 60 * 60) {
        qint64 mins = secs / 60;
        return QObject::tr("%1 mins ago").arg(mins);
    } else if (secs < 24 * 60 * 60) {
        qint64 hours = secs / (60 * 60);
        if (1 == hours) {
            return QObject::tr("1 hr ago");
        } else {
            return QObject::tr("%1 hrs ago").arg(hours);
        }
    } else if (secs < 7 * 24 * 60 * 60) {
        qint64 days = secs / (24 * 60 * 60);
        if (1 == days) {
            return QObject::tr("Yesterday");
        } else {
            return QObject::tr("%1 days ago").arg(days);
        }
    }

    return dataTime.toString("yyyy-MM-dd HH:mm:ss");
}

static QPixmap getActionPixmap(unsigned int iAction, QIcon::Mode mode = QIcon::Normal);
static QPixmap getActionPixmap(unsigned int iAction, QIcon::Mode mode)
{
    QPixmap pixmap;
    QString iconpath;

    switch (iAction) {
    case JOB_ACTION_Cancel:
        iconpath = "dp_job_cancel";
        break;
    case JOB_ACTION_Delete:
        iconpath = "dp_job_delete";
        break;
    case JOB_ACTION_Hold:
        iconpath = "dp_job_pause";
        break;
    case JOB_ACTION_Release:
        iconpath = "dp_job_start";
        break;
    case JOB_ACTION_Restart:
        iconpath = "dp_job_again";
        break;
    case JOB_ACTION_Priority:
        iconpath = "dp_job_priority";
        break;
    default:
        qCWarning(COMMONMOUDLE) << "Unsupport actions: " << iAction;
        break;
    }

    pixmap = QIcon::fromTheme(iconpath).pixmap(QSize(16, 16), mode);
    return pixmap;
}

static QString getActionIconPath(unsigned int iAction);
static QString getActionIconPath(unsigned int iAction)
{
    QString iconpath;

    switch (iAction) {
    case JOB_ACTION_Cancel:
        iconpath = "dp_job_cancel";
        break;
    case JOB_ACTION_Delete:
        iconpath = "dp_job_delete";
        break;
    case JOB_ACTION_Hold:
        iconpath = "dp_job_pause";
        break;
    case JOB_ACTION_Release:
        iconpath = "dp_job_start";
        break;
    case JOB_ACTION_Restart:
        iconpath = "dp_job_again";
        break;
    case JOB_ACTION_Priority:
        iconpath = "dp_job_priority";
        break;
    default:
        qCWarning(COMMONMOUDLE) << "Unsupport actions: " << iAction;
        break;
    }

    return iconpath;
}

static QMap<unsigned int, QRect> getItemActionRect(const QRect &itemRect, const QModelIndex &index)
{
    QMap<unsigned int, QRect> actions;
    unsigned int flags = index.data(JOB_ITEM_ROLE_ACTION).toUInt();
    int butIndex = 0;

    for (int i = 0; i < JOB_ACTION_Count; i++) {
        unsigned int iAction = 1 << i;

        if (flags & iAction) {
            QPixmap pixmap = getActionPixmap(iAction);
            QRect rect(itemRect.left() + ACTION_BUT_SPACE + (pixmap.width() + ACTION_BUT_SPACE) * butIndex++,
                       itemRect.top() + (itemRect.height() - pixmap.height()) / 2,
                       pixmap.width(), pixmap.height());

            actions.insert(iAction, rect);
        }
    }

    return actions;
}

JobListView::JobListView(QWidget *parent)
    : QTableView(parent)
    , m_contextMenu(nullptr)
    , m_itemDelegate(nullptr)
    , m_tipsTimer(nullptr)
{
    setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);

    m_itemDelegate = new JobItemDelegate(this);
    setItemDelegate(m_itemDelegate);
    verticalHeader()->setVisible(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(ITEM_Height);

    DFontSizeManager::instance()->bind(horizontalHeader(), DFontSizeManager::T6, int(QFont::Medium));
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T7, int(QFont::Normal));
    horizontalHeader()->setFixedHeight(ITEM_Height);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_tipsTimer = new QTimer(this);
    m_tipsTimer->setInterval(1000);
    m_tipsTimer->setSingleShot(true);

    connect(m_tipsTimer, &QTimer::timeout, this, &JobListView::slotShowTips);
    connect(this, &QWidget::customContextMenuRequested, this, &JobListView::slotShowContextMenu);

    m_actionNames << tr("Cancel");
    m_actionNames << tr("Delete");
    m_actionNames << tr("Pause");
    m_actionNames << tr("Resume");
    m_actionNames << tr("Print first");
    m_actionNames << tr("Reprint");

    m_label = new QLabel(tr("No print jobs"), this);
    DFontSizeManager::instance()->bind(m_label, DFontSizeManager::T5, int(QFont::DemiBold));
    m_label->setAlignment(Qt::AlignCenter);
    QPalette pa = m_label->palette();
    QStyleOption opt;
    opt.initFrom(m_label);
    QColor color = opt.palette.color(QPalette::Inactive, QPalette::Text);
    pa.setColor(QPalette::WindowText, QColor(color.red(), color.green(), color.blue(), 100));
    m_label->setPalette(pa);
    m_label->setAccessibleName("label_jobsView");
}

QString JobListView::getActionName(unsigned int iAction)
{
    QString strName;

    for (int i = 0; i < JOB_ACTION_Count; i++) {
        if (iAction == static_cast<unsigned int>(1 << i)) {
            strName = m_actionNames[i];
            break;
        }
    }

    return strName;
}

void JobListView::slotShowTips()
{
    QModelIndex index = indexAt(m_tipsPos);
    QRect rect = visualRect(index);
    QMap<unsigned int, QRect> actionRects = getItemActionRect(rect, index);
    QList<unsigned int> flags = actionRects.keys();

    foreach (unsigned int iAction, flags) {
        if (actionRects.value(iAction).contains(m_tipsPos)) {
            QString strTips = getActionName(iAction);

            QToolTip::showText(mapToGlobal(m_tipsPos), strTips, this);
        }
    }
}

void JobListView::mouseMoveEvent(QMouseEvent *event)
{
    QTableView::mouseMoveEvent(event);

    QModelIndex index = indexAt(event->pos());

    if (index.column() != ACTION_Column)
        return;

    QRect rect = visualRect(index);
    QMap<unsigned int, QRect> actionRects = getItemActionRect(rect, index);
    QList<unsigned int> flags = actionRects.keys();
    int iRow = index.row(), action = -1;

    //查找是否有按钮处于hover状态
    foreach (unsigned int iAction, flags) {
        if (actionRects.value(iAction).contains(event->pos())) {
            action = static_cast<int>(iAction);
            m_tipsPos = event->pos();
            m_tipsTimer->start();
        }
    }

    //如果有按钮处于hover状态
    if (-1 != action) {
        //如果还是原来按钮，状态不变
        if (iRow == g_hoverAction.iHoverRow && action == g_hoverAction.iHoverAction) {
            return;
        }

        QModelIndex lastIndex;
        QModelIndex curIndex;
        //如果切换了按钮,刷新两个按钮的状态
        if (-1 != g_hoverAction.iHoverAction) {
            g_hoverAction.iLastRow = g_hoverAction.iHoverRow;
            g_hoverAction.iLastAction = g_hoverAction.iHoverAction;

            lastIndex = model()->index(g_hoverAction.iLastRow, ACTION_Column);

            //如果没有切换按钮，只用刷新一个按钮
        } else {
            lastIndex = model()->index(iRow, ACTION_Column);
        }

        g_hoverAction.iHoverRow = iRow;
        g_hoverAction.iHoverAction = action;
        curIndex = model()->index(g_hoverAction.iHoverRow, ACTION_Column);

        if (lastIndex.row() > curIndex.row())
            dataChanged(curIndex, lastIndex);
        else
            dataChanged(lastIndex, curIndex);

        //如果没有按钮处于hover状态，将之前hover的按钮设置成normal
    } else if (-1 != g_hoverAction.iHoverAction) {
        g_hoverAction.iLastRow = g_hoverAction.iHoverRow;
        g_hoverAction.iLastAction = g_hoverAction.iHoverAction;
        g_hoverAction.iHoverAction = -1;

        QModelIndex lastIndex;
        lastIndex = model()->index(g_hoverAction.iLastRow, ACTION_Column);
        dataChanged(lastIndex, lastIndex);

        m_tipsTimer->stop();
    }
}

void JobListView::mouseReleaseEvent(QMouseEvent *event)
{
    QTableView::mouseReleaseEvent(event);

    QModelIndex index = indexAt(event->pos());

    if (index.column() != ACTION_Column)
        return;

    QRect rect = visualRect(index);
    QMap<unsigned int, QRect> actionRects = getItemActionRect(rect, index);
    QList<unsigned int> flags = actionRects.keys();

    foreach (unsigned int iAction, flags) {
        if (actionRects.value(iAction).contains(event->pos()) && askDeleteJobs(iAction)) {
            JobsDataModel *jobModel = static_cast<JobsDataModel *>(model());
            jobModel->doItemAction(jobModel->getJobId(index.row()), iAction);
        }
    }
}

void JobListView::resizeEvent(QResizeEvent *event)
{
    int headerHeight = horizontalHeader()->height();
    QTableView::resizeEvent(event);

    m_label->setGeometry(0, headerHeight, event->size().width(), event->size().height() - headerHeight);
}

void JobListView::setLabelContentVisable(bool bShow)
{
    m_label->setHidden(!bShow);
}

bool JobListView::askDeleteJobs(unsigned int flag)
{
    int iAccept;
    DDialog dlg("", tr("Are you sure you want to delete this job?"), this);

    if (JOB_ACTION_Delete != flag)
        return true;

    dlg.setIcon(QIcon(":/images/warning_logo.svg"));
    dlg.addButton(tr("Cancel", "button"), true);
    iAccept = dlg.addButton(tr("Delete", "button"), false, DDialog::ButtonWarning);
    dlg.getButton(0)->setFixedWidth(170);
    dlg.getButton(1)->setFixedWidth(170);
    dlg.setModal(true);
    dlg.setFixedSize(372, 162);

    return (iAccept == dlg.exec());
}

void JobListView::slotShowContextMenu(const QPoint &pos)
{
    QModelIndexList selectList = selectionModel()->selectedRows(0);
    unsigned int flag = 0;
    JobsDataModel *jobsModel = static_cast<JobsDataModel *>(model());

    if (selectList.isEmpty())
        return;

    foreach (QModelIndex index, selectList) {
        flag |= jobsModel->getActionStatus(index.row());
    }

    if (!m_contextMenu) {
        m_contextMenu = new QMenu(this);
        for (int i = 0; i < JOB_ACTION_Count; i++) {
            m_atctions.append(m_contextMenu->addAction(m_actionNames[i]));
        }

        connect(m_contextMenu, &QMenu::triggered, this, &JobListView::slotMenuTriggered);
    }

    for (int i = 0; i < m_atctions.count(); i++) {
        m_atctions[i]->setEnabled(flag & (1 << i));
    }

    m_contextMenu->exec(mapToGlobal(pos));
}

void JobListView::processAction(int index)
{
    unsigned int flag = 1 << index;
    QModelIndexList selectList = selectionModel()->selectedRows(0);
    JobsDataModel *jobsModel = static_cast<JobsDataModel *>(model());
    QMap<int, unsigned int> actionMap;

    if (selectList.isEmpty() || !askDeleteJobs(flag))
        return;

    foreach (auto modelindex, selectList) {
        int iRow = modelindex.row();
        unsigned int itemflag = jobsModel->getActionStatus(iRow);
        int jobId = jobsModel->getJobId(iRow);

        actionMap.insert(jobId, itemflag);
    }

    //删除item之后其他item的行号改变，先获取所有选中项的可执行的操作
    QList<int> jobIds = actionMap.keys();
    foreach (int id, jobIds) {
        unsigned int itemflag = actionMap.value(id);

        if (itemflag & flag) {
            jobsModel->doItemAction(id, flag);
        }
    }
}

void JobListView::slotMenuTriggered(QAction *action)
{
    int index = m_atctions.indexOf(action);
    qCDebug(COMMONMOUDLE) << index;
    processAction(index);
}

JobItemDelegate::JobItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

JobItemDelegate::~JobItemDelegate()
{

}

void JobItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    QMap<unsigned int, QRect> actions;
    QList<unsigned int> flags;
    QStyleOptionViewItem newoption = option;
    QString iconPath;
    QStyleOptionButton actionButton;

    if (index.column() == 0) {
        newoption.rect.setLeft(10);
    }

    painter->save();
    painter->setPen(Qt::NoPen);
    QPainterPath canDrawingPathArea; // 裁剪区域
    canDrawingPathArea.setFillRule(Qt::WindingFill); // 多块区域组合填充模式
    canDrawingPathArea.addRoundedRect(newoption.rect, 8, 8);
    int width = newoption.rect.width();
    int height = newoption.rect.height();
    /*第一列和最后一列分别裁剪左边、右边两个圆角*/
    if (index.column() == 0) {
        painter->setClipping(true);
        canDrawingPathArea.addRect(newoption.rect.center().x(), newoption.rect.y(), width / 2 + 1, height);
        painter->setClipPath(canDrawingPathArea);
    } else if (index.column() == 7) {
        painter->setClipping(true);
        canDrawingPathArea.addRect(newoption.rect.x(), newoption.rect.y(), width / 2, height);
        painter->setClipPath(canDrawingPathArea);
    }

    if (option.state & QStyle::State_Selected) {
        QBrush brush = option.palette.highlight();
        painter->setBrush(brush);
        painter->drawRect(newoption.rect);

    } else {
        if (index.row() % 2) {
            painter->fillRect(newoption.rect, QColor(137, 144, 161, 30));
        }
    }
    painter->restore();

    int iState = index.data(JOB_ITEM_ROLE_STATE).toInt();

    if (IPP_JSTATE_ABORTED == iState || IPP_JSTATE_STOPPED == iState) {
        newoption.palette.setColor(QPalette::Inactive, QPalette::Text, QColor(255, 87, 54, 100));
    }

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(QPalette::Text));
    }

    if (index.column() == ACTION_Column) {
        actions = getItemActionRect(newoption.rect, index);
        flags = actions.keys();
        foreach (unsigned int iAction, flags) {
            iconPath = getActionIconPath(iAction);
            actionButton.icon = QIcon::fromTheme(iconPath);
            actionButton.initFrom(option.widget);
            actionButton.rect = actions.value(iAction);
            actionButton.features = QStyleOptionButton::Flat;
            actionButton.rect.setX(actionButton.rect.x() - ACTION_BUT_SPACE);
            actionButton.rect.setY(actionButton.rect.y() - ACTION_BUT_SPACE);
            /*icon没有应用painter的画笔颜色，应该是fromTheme的问题*/
            DApplication::style()->drawControl(QStyle::CE_PushButton, &actionButton, painter);

        }
    } else {
        /*选中采用HighlightedText，没选中采用TEXT*/
        DApplication::style()->drawItemText(painter, newoption.rect, static_cast<int>(newoption.displayAlignment), newoption.palette, true, index.data(Qt::DisplayRole).toString());

    }
}



QSize JobItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    int iRow = index.row();

    if (0 == iRow)
        return QSize(72, ITEM_Height);
    else if (1 == iRow)
        return QSize(102, ITEM_Height);
    else if (2 == iRow)
        return QSize(137, ITEM_Height);
    else if (3 == iRow)
        return QSize(131, ITEM_Height);
    else if (4 == iRow)
        return QSize(65, ITEM_Height);
    else if (5 == iRow)
        return QSize(124, ITEM_Height);
    else if (6 == iRow)
        return QSize(146, ITEM_Height);

    return option.rect.size();
}

JobsDataModel::JobsDataModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_iHighestPriority(50)
    , m_iWhichJob(WHICH_JOB_RUNING)
{
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(100);
    m_refreshTimer->setSingleShot(true);
    connect(m_refreshTimer, &QTimer::timeout, this, &JobsDataModel::slotRefreshJobItems);
}

void JobsDataModel::deleteJobItem(int jobId)
{
    int index = 0;

    qCInfo(COMMONMOUDLE) << jobId;

    for (; index < m_jobs.count(); index++) {
        if (jobId == m_jobs[index][JOB_ATTR_ID].toInt()) {
            break;
        }
    }

    if (index >= m_jobs.count())
        return;

    m_jobs.removeAt(index);
    m_refreshTimer->start();
}

void JobsDataModel::addJobItem(const QMap<QString, QVariant> &job)
{
    if (job.isEmpty())
        return;

    qCInfo(COMMONMOUDLE) << job[JOB_ATTR_ID].toInt();

    m_jobs.append(job);
    m_refreshTimer->start();
}

void JobsDataModel::setJobAttributes(int index, const QMap<QString, QVariant> &job)
{
    if (index >= m_jobs.count())
        return;

    QMap<QString, QVariant> lastJob = m_jobs[index];
    int lastState = lastJob[JOB_ATTR_STATE].toString().toInt();
    int lastPriority = lastJob[JOB_ATTR_PRIORITY].toString().toInt();
    int state = job[JOB_ATTR_STATE].toString().toInt();
    int jobPriority = job[JOB_ATTR_PRIORITY].toString().toInt();

    qCInfo(COMMONMOUDLE) << QString("(%1, %2) -> (%3, %4)").arg(lastState).arg(lastPriority).arg(state).arg(jobPriority);

    m_jobs[index] = job;

    //状态或者优先级改变都需要进行排序
    if (lastState != state || lastPriority != jobPriority) {
        m_refreshTimer->start();
        return;
    }

    //其他情况只用刷新当前任务
    emit dataChanged(this->index(index, 0), this->index(index, ACTION_Column));
}

void JobsDataModel::doItemAction(int jobId, unsigned int iAction)
{
    int iRet = 0;

    switch (iAction) {
    case JOB_ACTION_Cancel:
        iRet = g_jobManager->cancelJob(jobId);
        break;
    case JOB_ACTION_Delete:
        iRet = g_jobManager->deleteJob(jobId);
        if (0 == iRet)
            deleteJobItem(jobId);
        break;
    case JOB_ACTION_Hold:
        iRet = g_jobManager->holdJob(jobId);
        break;
    case JOB_ACTION_Release:
        iRet = g_jobManager->releaseJob(jobId);
        break;
    case JOB_ACTION_Restart:
        iRet = g_jobManager->restartJob(jobId);
        break;
    case JOB_ACTION_Priority: {
        int iPriority = m_iHighestPriority + 1;

        if (HIGHEST_Priority <= iPriority) {
            iPriority = -1;
        }

        iRet = g_jobManager->priorityJob(jobId, iPriority);
        if (0 == iRet) {
            setHighestPriority(jobId, iPriority);
        }

        break;
    }
    default:
        qCWarning(COMMONMOUDLE) << "Unsupport actions: " << iAction;
        break;
    }

    if (0 != iRet)
        emit signalDoActionFailed(jobId, iAction);
}

QVariant JobsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return tr("Job");
        } else if (section == 1) {
            return tr("User");
        } else if (section == 2) {
            return tr("Document");
        } else if (section == 3) {
            return tr("Printer");
        } else if (section == 4) {
            return tr("Size");
        } else if (section == 5) {
            return tr("Time submitted");
        } else if (section == 6) {
            return tr("Status");
        } else if (section == 7) {
            return tr("Action");
        }
    }
    return QVariant();
}

void JobsDataModel::sortJobs()
{
    QList<QMap<QString, QVariant>> donelist, processinglist, holdlist;
    QMap<QString, QVariant> priorityJob;
    QMap<int, QMap<QString, QVariant>> jobHash;

    foreach (auto job, m_jobs) {
        int iState = job[JOB_ATTR_STATE].toString().toInt();
        int jobPriority = job[JOB_ATTR_PRIORITY].toString().toInt();

        if (IPP_JSTATE_PENDING == iState) {
            //等待中的任务根据优先级排序
            jobHash.insertMulti(jobPriority, job);
        } else if (iState == IPP_JSTATE_PROCESSING) {
            processinglist.append(job);
        } else if (iState == IPP_JSTATE_HELD) {
            holdlist.append(job);
        } else {
            donelist.append(job);
        }
    }

    m_jobs.clear();
    //任务列表排列顺序: 打印中任务->等待中任务(按优先级排序)->暂停任务->已完成任务
    m_jobs = processinglist;
    foreach (auto job, jobHash.values()) {
        m_jobs.insert(processinglist.count(), job);
    }
    m_jobs += holdlist;
    m_jobs += donelist;
}

void JobsDataModel::setWhichJob(int which)
{
    m_iWhichJob = which;

    slotRefreshJobsList();
}

void JobsDataModel::updateJobState(int id, int state, const QString &message)
{
    int index = 0;
    int iState = 0;
    map<string, string> jobinfo;
    QMap<QString, QVariant> job;

    for (; index < m_jobs.count(); index++) {
        if (id == m_jobs[index][JOB_ATTR_ID].toInt()) {
            iState = m_jobs[index][JOB_ATTR_STATE].toInt();
            break;
        }
    }

    qCInfo(COMMONMOUDLE) << iState << m_iWhichJob;

    //如果是因为删除触发的状态改变，则从列表中删除
    QDBusInterface interface(SERVICE_INTERFACE_NAME, SERVICE_INTERFACE_PATH, SERVICE_INTERFACE_NAME, QDBusConnection::sessionBus());
    if (interface.isValid()) {
        QDBusReply<bool> result = interface.call("isJobPurged", id);
        if (result.isValid() && result.value()) {
            if (index < m_jobs.count())
                deleteJobItem(id);
            return;
        }
    }

    if (index >= m_jobs.count()) {
        //如果已完成任务不在任务列表中,而且当前需要显示已完成任务，则将已完成任务添加到列表中
        //如果未完成任务不在任务列表中,而且当前需要显示未完成任务，则将新任务添加到任务列表中
        if (((g_jobManager->isCompletedState(state) && m_iWhichJob != WHICH_JOB_RUNING) || (!g_jobManager->isCompletedState(state) && m_iWhichJob != WHICH_JOB_DONE))) {
            if (0 == g_jobManager->getJobById(jobinfo, id)) {
                map<string, string>::const_iterator itjob;

                job.insert(JOB_ATTR_ID, id);
                for (itjob = jobinfo.begin(); itjob != jobinfo.end(); itjob++) {
                    job.insert(STQ(itjob->first), attrValueToQString(itjob->second));
                }
                addJobItem(job);
            }
        }

        //其他不在任务列表的情况不需要刷新列表
        return;
    }

    //如果是任务完成的信号，刷新耗材信息。
    if (g_jobManager->isCompletedState(state)) {
        g_jobManager->getJobById(jobinfo, id);

        auto iter = jobinfo.find("job-printer-uri");

        if (iter != jobinfo.end()) {
            QString strURI = attrValueToQString(iter->second);
            QString strName = getPrinterNameFromUri(strURI);
            DPrinterManager *pManager = DPrinterManager::getInstance();
            DDestination *pDest = pManager->getDestinationByName(strName);

            if (pDest != nullptr) {
                if (PRINTER == pDest->getType()) {
                    DPrinter *pPrinter = static_cast<DPrinter *>(pDest);

                    if (!pPrinter->isPpdFileBroken()) {
                        pPrinter->disableSupplys();
                        pPrinter->updateSupplys();
                    }
                }
            }
        }
    }

    //如果已完成任务在任务列表中，而且当前只显示未完成任务，则将已完成任务从任务列表中删除
    //如果未完成任务在任务列表中，而且当前只显示已完成任务，则将未完成任务从任务列表中删除(重新打印的情况)
    if ((g_jobManager->isCompletedState(state) && m_iWhichJob == WHICH_JOB_RUNING) || (!g_jobManager->isCompletedState(state) && m_iWhichJob == WHICH_JOB_DONE)) {
        deleteJobItem(id);
        return;
    }

    //其他情况只刷新任务属性
    job = m_jobs[index];
    job.insert(JOB_ATTR_STATE, state);
    job.insert(JOB_ATTR_STATE_MEG, message);
    setJobAttributes(index, job);
}

void JobsDataModel::slotRefreshJobItems()
{
    int jobPriority, iState;
    int priorityCount = 0;

    m_refreshTimer->stop();
    beginResetModel();

    m_iHighestPriority = 50;
    //获取最高优先级的值，用于列表显示任务是否能优先打印
    for (int i = 0; i < m_jobs.count(); i++) {
        jobPriority = m_jobs[i][JOB_ATTR_PRIORITY].toString().toInt();
        iState = m_jobs[i][JOB_ATTR_STATE].toString().toInt();

        if (jobPriority > m_iHighestPriority && iState == IPP_JSTATE_PENDING) {
            m_iHighestPriority = jobPriority;
            priorityCount = 1;
        } else if (jobPriority == m_iHighestPriority && iState == IPP_JSTATE_PENDING) {
            priorityCount++;
        }
    }
    //如果多个任务都是优先打印，将最高优先级加1保证都可以再次设置优先打印
    if (priorityCount > 1)
        m_iHighestPriority++;

    sortJobs();

    endResetModel();
    emit signalJobsCountChanged(m_jobs.count());
    qCInfo(COMMONMOUDLE) << "Current highest priorty" << m_iHighestPriority;
}

void JobsDataModel::slotRefreshJobsList()
{
    map<int, map<string, string>> jobsmap;
    map<int, map<string, string>>::const_iterator itmaps;

    qCInfo(COMMONMOUDLE) << m_iWhichJob;

    if (0 != g_jobManager->getJobs(jobsmap, m_iWhichJob, 1))
        return;

    m_jobs.clear();
    for (itmaps = jobsmap.begin(); itmaps != jobsmap.end(); itmaps++) {
        QMap<QString, QVariant> job;
        map<string, string> jobinfo = itmaps->second;
        map<string, string>::const_iterator itjob;

        job.insert(JOB_ATTR_ID, itmaps->first);
        for (itjob = jobinfo.begin(); itjob != jobinfo.end(); itjob++) {
            job.insert(STQ(itjob->first), attrValueToQString(itjob->second));
        }
        m_jobs.prepend(job);
    }

    slotRefreshJobItems();
}

int JobsDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_jobs.size();
}

int JobsDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 8;
}

QVariant JobsDataModel::data(const QModelIndex &index, int role) const
{
    int iRow = index.row();
    if (iRow >= m_jobs.count())
        return QVariant();

    QMap<QString, QVariant> job = m_jobs[iRow];

    if (!index.isValid() || role != Qt::DisplayRole) {
        if (JOB_ITEM_ROLE_ID == role) {
            return job[JOB_ATTR_ID];
        } else if (JOB_ITEM_ROLE_STATE == role) {
            return job[JOB_ATTR_STATE];
        } else if (JOB_ITEM_ROLE_ACTION == role) {
            return getActionStatus(iRow);
        }
        return QVariant();
    }

    if (index.column() == 0) {
        return job[JOB_ATTR_ID].toInt();
    } else if (index.column() == 1) {
        return job[JOB_ATTR_USER].toString();
    } else if (index.column() == 2) {
        QStringList list = job[JOB_ATTR_NAME].toString().split("/", QString::SkipEmptyParts);
        return list.isEmpty() ? "" : list.last();
    } else if (index.column() == 3) {
        QString uri = job[JOB_ATTR_URI].toString();
        return getPrinterNameFromUri(uri);
    } else if (index.column() == 4) {
        QString strSize = job[JOB_ATTR_SIZE].toString();
        int iSize = strSize.toInt();

        if (iSize > 1024) {
            iSize /= 1024;
            strSize = QString::number(iSize) + "M";
        } else {
            strSize += "K";
        }

        return strSize;
    } else if (index.column() == 5) {
        QString strTime = job[JOB_ATTR_TIME_ADD].toString();
        unsigned int iTime = strTime.toUInt();
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(iTime);

        return formatDataTimeString(dateTime);
    } else if (index.column() == 6) {
        QDBusInterface interface(SERVICE_INTERFACE_NAME, SERVICE_INTERFACE_PATH, SERVICE_INTERFACE_NAME, QDBusConnection::sessionBus());
        if (interface.isValid()) {
            QDBusReply<QString> result = interface.call("getJobNotify", job);
            if (result.isValid()) {
                return result.value();
            }
        }
    }
    return QVariant();
}

int JobsDataModel::getHighestPriority()
{
    return m_iHighestPriority;
}

void JobsDataModel::setHighestPriority(int id, int iPriority)
{
    int index = 0;

    for (; index < m_jobs.count(); index++) {
        if (id == m_jobs[index][JOB_ATTR_ID].toInt()) {
            break;
        }
    }

    if (index >= m_jobs.count())
        return;

    QMap<QString, QVariant> job = m_jobs[index];
    m_iHighestPriority = iPriority;
    job.insert(JOB_ATTR_PRIORITY, iPriority);
    setJobAttributes(index, job);
}

unsigned int JobsDataModel::getActionStatus(int iRow) const
{
    unsigned int flag = 0;

    if (iRow >= m_jobs.count())
        return flag;

    QMap<QString, QVariant> job = m_jobs[iRow];
    int iState = job[JOB_ATTR_STATE].toString().toInt();
    int docNum = job[JOB_ATTR_DOC_NUM].toString().toInt();

    switch (iState) {
    case IPP_JSTATE_PENDING: {
        int iPriority = job[JOB_ATTR_PRIORITY].toString().toInt();

        if (iPriority < m_iHighestPriority) {
            flag = JOB_ACTION_Priority;
        }

        flag |= JOB_ACTION_Hold;
        Q_FALLTHROUGH();
    }
    case IPP_JSTATE_PROCESSING:
        flag |= JOB_ACTION_Cancel;
        break;
    case IPP_JSTATE_HELD:
        flag = JOB_ACTION_Cancel | JOB_ACTION_Release;
        break;
    case IPP_JSTATE_STOPPED:
        flag = JOB_ACTION_Cancel | JOB_ACTION_Restart;
        break;
    case IPP_JSTATE_CANCELED:
    case IPP_JSTATE_ABORTED:
    case IPP_JSTATE_COMPLETED:
        flag = JOB_ACTION_Delete;
        if (docNum > 0)
            flag |= JOB_ACTION_Restart;
        break;
    default:
        flag = JOB_ACTION_Delete;
        break;
    }

    return flag;
}

int JobsDataModel::getJobId(int iRow)
{
    if (iRow >= m_jobs.count())
        return 0;

    return m_jobs[iRow][JOB_ATTR_ID].toInt();
}

JobManagerWindow::JobManagerWindow(QWidget *parent)
    : DMainWindow(parent)
    , m_jobsView(nullptr)
    , m_jobsModel(nullptr)
    , m_refreshBut(nullptr)
    , m_whichButBox(nullptr)
    , m_jobCountLabel(nullptr)
{
    createUi();
    initUi();
    initConnect();
    m_jobsModel->setWhichJob(WHICH_JOB_RUNING);
    setAttribute(Qt::WA_DeleteOnClose);
}

void JobManagerWindow::createUi()
{
    m_refreshBut = new DIconButton(this);
    m_whichButBox = new DButtonBox(this);
    m_whichList.append(new DButtonBoxButton(QIcon::fromTheme("dp_print_all")));
    m_whichList.append(new DButtonBoxButton(QIcon::fromTheme("dp_print_wait")));
    m_whichList.append(new DButtonBoxButton(QIcon::fromTheme("dp_print_done")));

    m_jobsView = new JobListView(this);
    m_jobsModel = new JobsDataModel(m_jobsView);

    m_jobCountLabel = new QLabel(this);
}

void JobManagerWindow::initUi()
{
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    m_refreshBut->setIcon(QIcon::fromTheme("dp_refresh"));
    m_refreshBut->setToolTip(tr("Refresh"));
    m_refreshBut->setAccessibleName("refreshBut_jobsWindow");

    m_whichButBox->setButtonList(m_whichList, true);
    m_whichButBox->setAccessibleName("whichButBox_jobsWindow");
    m_whichList[WHICH_JOB_RUNING]->setChecked(true);
    m_whichList[WHICH_JOB_ALL]->setToolTip(tr("All"));
    m_whichList[WHICH_JOB_RUNING]->setToolTip(tr("Print Queue"));
    m_whichList[WHICH_JOB_DONE]->setToolTip(tr("Completed"));
    foreach (DButtonBoxButton *but, m_whichList) {
        but->setFocusPolicy(Qt::NoFocus);
    }

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_whichButBox, Qt::AlignLeft | Qt::AlignVCenter);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_refreshBut, Qt::AlignLeft | Qt::AlignVCenter);
    QWidget *pWidget = new QWidget();
    pWidget->setLayout(hLayout);
    titlebar()->addWidget(pWidget, Qt::AlignLeft | Qt::AlignVCenter);
    titlebar()->setIcon(QIcon(":/images/dde-printer.svg"));
    titlebar()->setTitle("");
    titlebar()->setMenuVisible(false);

    m_jobsView->setModel(m_jobsModel);
    m_jobsView->setAccessibleName("jobsView_jobsWindow");
    m_jobsView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_jobsView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    m_jobsView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_jobsView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    m_jobsView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    m_jobsView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
    m_jobsView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
    m_jobsView->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Interactive);
    m_jobsView->setColumnWidth(0, 72);
    m_jobsView->setColumnWidth(1, 82);
    m_jobsView->setColumnWidth(4, 65);
    m_jobsView->setColumnWidth(5, 166);
    m_jobsView->setColumnWidth(6, 146);

    m_jobCountLabel->setAlignment(Qt::AlignCenter);
    m_jobCountLabel->setFixedHeight(30);
    m_jobCountLabel->setAccessibleName("jobCountLabel_jobsWindow");
    QPalette pa = m_jobCountLabel->palette();
    QStyleOption opt;
    opt.initFrom(m_jobCountLabel);
    QColor color = opt.palette.color(QPalette::Inactive, QPalette::Text);
    pa.setColor(QPalette::WindowText, color);
    m_jobCountLabel->setPalette(pa);

    setFixedSize(JOB_VIEW_WIDTH, 546);
    takeCentralWidget();

    QWidget *centWidget = new QWidget(this);
    QVBoxLayout *lay = new QVBoxLayout();
    lay->setSpacing(0);
    lay->addWidget(m_jobsView);
    lay->addWidget(m_jobCountLabel);
    lay->setContentsMargins(10, 10, 10, 0);
    centWidget->setLayout(lay);
    setCentralWidget(centWidget);
    centWidget->setAccessibleName("centWidget_jobsWindow");
}

void JobManagerWindow::initConnect()
{
    connect(m_refreshBut, &QAbstractButton::clicked, m_jobsModel, &JobsDataModel::slotRefreshJobsList);
    connect(m_whichButBox, &DButtonBox::buttonClicked, this, &JobManagerWindow::slotWhichBoxClicked);

    if (!QDBusConnection::sessionBus().connect(SERVICE_INTERFACE_NAME, SERVICE_INTERFACE_PATH, SERVICE_INTERFACE_NAME,
                                               "signalJobStateChanged", this, SLOT(slotJobStateChanged(QDBusMessage)))) {
        qCWarning(COMMONMOUDLE) << "connect to dbus signal(signalJobStateChanged) failed";
    }

    connect(m_jobsModel, &JobsDataModel::signalJobsCountChanged, this, &JobManagerWindow::slotJobsCountChanged);
    connect(m_jobsModel, &JobsDataModel::signalDoActionFailed, this, &JobManagerWindow::slotDoActionFailed);
}

void JobManagerWindow::slotDoActionFailed(int jobId, unsigned int iAction)
{
    Q_UNUSED(jobId);

    QString strTips = m_jobsView->getActionName(iAction) + tr(" failed");
    QPoint tipsPos = QCursor::pos();

    //右键菜单可能关闭导致tooltips没有显示，延时处理
    QTimer::singleShot(100, this, [ = ]() {
        QToolTip::showText(tipsPos, strTips, this);
    });
}

void JobManagerWindow::slotWhichBoxClicked(QAbstractButton *whichbut)
{
    if (whichbut) {
        int iWhichJob = m_whichList.indexOf(static_cast<DButtonBoxButton *>(whichbut));

        m_jobsModel->setWhichJob(iWhichJob);
    }
}

void JobManagerWindow::slotJobsCountChanged(int count)
{
    qCInfo(COMMONMOUDLE) << count;
    m_jobCountLabel->setText(tr("%1 jobs").arg(count));
    m_jobsView->setLabelContentVisable(0 == count);
}

void JobManagerWindow::slotJobStateChanged(const QDBusMessage &msg)
{
    if (msg.arguments().count() != 3) {
        qCWarning(COMMONMOUDLE) << "JobStateChanged dbus arguments error";
        return;
    }
    m_jobsModel->updateJobState(msg.arguments().at(0).toInt(), msg.arguments().at(1).toInt(), msg.arguments().at(2).toString());
}
