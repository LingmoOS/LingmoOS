// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanitem.h"

#include "widgets/cleanerresultitemwidget.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QThread>

#define KILOBYTE 1024
#define SHOWDELAY 5

TrashCleanItem::TrashCleanItem(TrashCleanItem *parentItem)
    : QObject(parentItem)
    , m_isToStop(false)
    , m_isShown(false)
    , m_totalSize(0)
    , m_totalAmount(0)
    , m_itemsize(0)
    , m_itemAmount(0)
    , m_cleanedSize(0)
    , m_cleanedAmount(0)
    , m_parent(parentItem)
    , m_itemWidget(nullptr)
    , m_root(nullptr)
    , m_isChecked(true)
    , m_isAnyChildSelected(false)
    , m_unCheckable(false)
{
    initItemWidget();
}

TrashCleanItem::~TrashCleanItem() { }

// 将文件大小由字节数转换为对应单位
// 最大单位在需求文档上没有明确，暂时定义为GB
// 保留两位小数
QString TrashCleanItem::fileSizeToString(quint64 filesize)
{
    QStringList unit;
    unit << "Byte"
         << "KB"
         << "MB"
         << "GB";

    double quotient = 0;

    // 以1024为单位换算
    int i = 0;
    while (KILOBYTE <= filesize && i < unit.size() - 1) {
        quotient = filesize % KILOBYTE;
        filesize /= KILOBYTE;
        i++;
    }

    double size = quotient / KILOBYTE + filesize;
    // 输出时，如果有小数，保留两位
    return QString("%1 %2").arg(QString::number(size, 'f', 2)).arg(unit[i]);
}

// 作为根项目被选中时，将所有子项目设为选中状态
void TrashCleanItem::setChildSelected(bool status)
{
    if (m_unCheckable) {
        return;
    }

    m_isChecked = status;
    // 向下递归, 将选中状态与根项目同步
    foreach (auto item, m_childItems) {
        if (!item) {
            continue;
        }
        item->setSelected(status);
        item->setChildSelected(status);
    }

    findRootItem()->recount();
}

// 获取根节点位置
TrashCleanItem *TrashCleanItem::findRootItem()
{
    if (m_root != nullptr) {
        return m_root;
    }

    // 自身根结点
    if (m_parent == nullptr) {
        m_root = this;
        return m_root;
    }

    // 递归向上查找
    m_root = m_parent;
    while (m_root->m_parent != nullptr) {
        m_root = m_root->m_parent;
    }

    return m_root;
}

// 依赖窗口上的checkbox产生的勾选或取消勾选，在此处响应
void TrashCleanItem::stageChanged(bool isItemSelected)
{
    if (m_unCheckable) {
        return;
    }

    m_isChecked = isItemSelected;
    // 子项被选中，将所有父项设为选中
    if (isItemSelected) {
        TrashCleanItem *tmp = m_parent;
        while (tmp != nullptr) {
            tmp->setSelected(true);
            tmp = tmp->m_parent;
        }
    } else {
        // bug
        // https://pms.uniontech.com/zentao/bug-view-60687.html,如果所有子项目被取消，父项目也应取消
        // 子项被取消时，在此同步父项目的标志
        TrashCleanItem *tmp = m_parent;
        // 更新父结点的状态，需要遍历父结点下所有结点
        // 存在选中的子项时置为true
        // 否则置为false
        while (tmp) {
            tmp->m_isAnyChildSelected = false;
            foreach (auto item, tmp->m_childItems) {
                if (!item) {
                    continue;
                }
                if (item->isSelected()) {
                    tmp->m_isAnyChildSelected = true;
                    break;
                }
            }
            if (!m_isAnyChildSelected) {
                tmp->setSelected(tmp->m_isAnyChildSelected);
            }

            tmp = tmp->m_parent;
        }
    }

    foreach (auto child, m_childItems) {
        if (!child) {
            continue;
        }
        child->setSelected(isItemSelected);
    }

    // 由于已经产生了勾选行为，因此需要重新统计总大小并显示
    findRootItem()->recount();
}

// 作为子结点，由根项或者父项设置选中状态
void TrashCleanItem::setSelected(bool status)
{
    if (m_unCheckable) {
        return;
    }

    m_isChecked = status;
    Q_EMIT setWidgetItemSelected(status);
}

void TrashCleanItem::setConfigToScan()
{
    if (nullptr == m_parent) {
        m_isChecked = true;
        Q_EMIT setWidgetItemSelected(true);
    }
}

// title 作为本项说明，只在必要的项目上设置
void TrashCleanItem::setTitle(const QString &title)
{
    m_itemWidget->setHeadText(title);
    m_title = title;
}

// tip 作为本项说明，只在必要的项目上设置
void TrashCleanItem::setTip(const QString &tip)
{
    m_itemWidget->setHeadTipText(tip);
}

// 扫描文件的路径列表，由主界面通过后台服务接口产生，并设置
void TrashCleanItem::setFilePaths(const QList<QString> &paths)
{
    m_scanPaths = paths;
    // 路径内容去掉重复项
    QSet<QString> set(m_scanPaths.begin(), m_scanPaths.end());
    m_scanPaths = set.values();
}

void TrashCleanItem::setPkgName(const QString &pkgName)
{
    m_pkgName = pkgName;
}

// 有子项检查添加
void TrashCleanItem::addChild(TrashCleanItem *child)
{
    m_childItems.push_back(child);
}

void TrashCleanItem::removeChildren()
{
    foreach (auto child, m_childItems) {
        if (!child) {
            continue;
        }
        child->removeChildren();
        // 可能有消息在处理，不要直接delete
        removeChild(child);
    }

    resetData();
}

void TrashCleanItem::removeChild(TrashCleanItem *child)
{
    // 检查是否存在,removeat需要一个有效参数
    int index = m_childItems.indexOf(child);
    if (-1 == index) {
        return;
    }
    // 删除该子结点
    m_childItems.removeAt(index);
    child->itemWidget()->deleteLater();
    child->deleteLater();
}

void TrashCleanItem::scan()
{
    // 深度递归
    // 对于根结点，widget设为扫描状态
    if (m_isToStop || !m_isChecked) {
        return;
    }

    resetData();

    foreach (auto item, m_childItems) {
        if (!item) {
            continue;
        }
        item->scan();
        m_totalSize += item->totalFileSize();
        m_totalAmount += item->totalFileAmount();
    }

    foreach (const auto &path, m_scanPaths) {
        if (m_isToStop || !m_isChecked) {
            break;
        }
        scanFile(path);
    }

    m_totalSize += m_itemsize;
    m_totalAmount += m_itemAmount;
}

// 重新扫描时，清空内容，避免控件上有残留的文字
void TrashCleanItem::resetData()
{
    m_totalSize = 0;
    m_totalAmount = 0;
    m_itemsize = 0;
    m_itemAmount = 0;
    m_isShown = false;
    m_isToStop = false;
    m_fileList.clear();
    m_deletePaths.clear();
}

void TrashCleanItem::increaseCleanedItemInfo(quint64 cleanedSize, int cleanedAmount)
{
    m_cleanedSize += cleanedSize;
    m_cleanedAmount += cleanedAmount;
}

// 获取单个文件的大小，同时记录文件总数
void TrashCleanItem::scanFile(const QString &path)
{
    QFileInfo info(path);
    if (info.exists()) {
        if (info.isFile()) {
            m_itemAmount++; // 记录文件数
            m_itemsize += quint64(info.size());
            // 用户显示在主界面的标题内容，由于文件路径是同检查项层次决定的，所以不需要该文件的路径信息
            m_fileList.append(info.fileName());
            m_deletePaths.append(info.absoluteFilePath());
        } else if (info.isDir()) {
            QDir dir(path);
            for (const QFileInfo &i :
                 dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden
                                   | QDir::NoSymLinks)) {
                // 递归扫描目录内容
                scanFile(i.absoluteFilePath());
            }
        }
    }
}

// 清理的临时实现，后续优化中，将此逻辑移入后台服务
void TrashCleanItem::clean()
{
    // 仅删除选中项
    if (!m_isChecked || !m_isShown) {
        // 根结点可能被选中，但是没有被show，此时也应当发送信号
        // 对应的，未被选中的根结点不需要发送信号
        if (nullptr == m_parent && m_isChecked) {
            Q_EMIT cleanFinished();
        }
        return;
    }

    foreach (const auto child, m_childItems) {
        if (!child) {
            continue;
        }
        child->clean();
    }

    quint64 cleanndSize = m_itemsize;
    // 目前不再验证是否有残留
    // 直接将值置为0
    m_totalSize = 0;
    m_totalAmount = 0;
    m_itemsize = 0;
    m_itemAmount = 0;
    m_isShown = true;

    // 只对底层项目设置清理信息
    // 删除操作移到后台，不再显示未成功情形
    if (!m_childItems.size() && m_parent != nullptr) {
        setSelected(false);
        Q_EMIT setCleanDone();
    }

    // 根项目通知主界面清理完成
    if (m_parent == nullptr) {
        recount();
        Q_EMIT cleanFinished();
    } else {
        findRootItem()->increaseCleanedItemInfo(cleanndSize, m_deletePaths.size());
    }

    if (!m_deletePaths.empty()) {
        findRootItem()->NoticeUIRemoveFiles(m_deletePaths);
        findRootItem()->NoticeUICleanedApp(m_pkgName);
        m_deletePaths.clear();
    }

    // 如果存在子项目,统计所有子项大小
    // 如果所有大小均为0,将父项或根项设置为不可选中
    quint64 remainSize = 0;
    uint remainAmount = 0;
    if (m_childItems.size()) {
        recountRemainItems(&remainSize, &remainAmount);
        if (!remainSize) {
            m_unCheckable = true;
            Q_EMIT setUncheckable();
        }
    }
}

// 注意，重算大小和数目时不要重新扫描
// 目录如果有变化应体现在“重新扫描”功能中
void TrashCleanItem::recountItems(quint64 *size, uint *amount)
{
    if (m_isChecked && m_isShown) {
        findRootItem()->m_isAnyChildSelected = true;
        findRootItem()->m_isShown = true;
        foreach (auto item, m_childItems) {
            if (!item) {
                continue;
            }
            item->recountItems(size, amount);
        }
        *size += m_itemsize;
        *amount += m_itemAmount;
    }
}

// 根结点对于重新计算选中项目累计大小的响应
void TrashCleanItem::recount()
{
    // 重新统计根节点
    if (m_parent == nullptr) {
        quint64 size = 0;
        uint amount = 0;
        m_isAnyChildSelected = false;

        // 仅计算被选中且已经被展示的项目
        foreach (auto item, m_childItems) {
            if (!item) {
                continue;
            }
            item->recountItems(&size, &amount);
        }

        // 将结果写入控件并记录总大小
        QString filesize = fileSizeToString(size);
        m_totalSize = size;
        m_totalAmount = amount;

        Q_EMIT setScanResult(m_totalAmount, filesize);

        // 通知主界面，本项目计数完成
        Q_EMIT recounted();
    }
}

void TrashCleanItem::recountRemainItems(quint64 *size, uint *amount)
{
    // 仅处理根项与父项,不处理子项或无子项的父项
    if (0 == m_childItems.size()) {
        return;
    }

    // 统计项目下所有项目大小,如果为0,将本项目设置为 已选中-且不可勾选
    // 无论是否选中,都加入统计
    foreach (auto const child, m_childItems) {
        if (!child) {
            continue;
        }
        *size += child->totalFileSize();
        *amount += child->totalFileAmount();
    }
}

// 由根结点通知主界面，更新主界面上，当前正在扫描的文件名
// 触发主界面进度条重新设值
void TrashCleanItem::noticeUIFileName(const QString &filePath)
{
    if (m_parent == nullptr) {
        Q_EMIT noticeFileName(filePath);
    }
}

void TrashCleanItem::NoticeUICleanedApp(const QString &pkgName)
{
    if (m_parent == nullptr && !pkgName.isEmpty()) {
        Q_EMIT noticeAppName(pkgName);
    }
}

void TrashCleanItem::NoticeUIRemoveFiles(const QStringList &paths)
{
    // 只在根项目上进行信号发送
    if (m_parent == nullptr) {
        Q_EMIT deleteUserFiles(paths);
    }
}

void TrashCleanItem::setZeroSizeForStoppedItem()
{
    m_totalSize = 0;
    m_totalAmount = 0;
    m_itemsize = 0;
    m_itemAmount = 0;
    Q_EMIT setScanResult(0, fileSizeToString(0));
    foreach (auto &child, m_childItems) {
        if (child) {
            child->setZeroSizeForStoppedItem();
        }
    }
}

void TrashCleanItem::setItemUncheckable()
{
    setSelected(false);
    m_unCheckable = true;
    Q_EMIT setUncheckable();

    foreach (auto item, m_childItems) {
        if (item) {
            item->setSelected(false);
            item->setItemUncheckable();
        }
    }
}

// 主界面通知，将扫描的内容展示在主界面上
void TrashCleanItem::showScanResult()
{
    if (m_isToStop || !m_isChecked) {
        if (m_isToStop) {
            // 被中止的项目现在给一个"0"大小
            setZeroSizeForStoppedItem();
            setItemUncheckable();
        }

        resetData();
        return;
    }

    // 通知检查项界面开始显示检查结果
    Q_EMIT scanStarted(true);

    // 该项目已经展示在主界面上，将被计入累计文件大小
    m_isShown = true;
    m_totalSize = 0;
    m_totalAmount = 0;
    m_itemsize = 0;
    m_itemAmount = 0;

    m_cleanedSize = 0;
    m_cleanedAmount = 0;
    // 取得检查项路径名称
    // 递归到根结点，取得完整检查路径
    QString path = m_title;
    TrashCleanItem *parent = m_parent;
    while (parent != nullptr) {
        path = parent->title() + "/" + path;
        parent = parent->m_parent;
    }

    foreach (const auto &file, m_deletePaths) {
        // 通知界面当前扫描文件的名称
        // 没有必要频繁刷新，降低标题内容刷新频率
        // 控制结果展示时长
        m_itemAmount++;
        // 重新统计文件大小，在扫描中止时提供实时的大小
        QFileInfo info(file);
        if (info.exists()) {
            if (info.isFile()) {
                m_itemsize += static_cast<quint64>(info.size());
            }
        }

        if (m_itemAmount % PROGRESS_NOTICE_INTERVAL == 0) {
            // 用户体验相关，延长进度条显示时间
            // 如果有性能优化需要，可以减少此处延时或取消
            // 延时值为5时，展示9000个文件花费约2秒
            QThread::msleep(SHOWDELAY);
            QApplication::processEvents();
            if (m_isToStop || !m_isChecked) {
                // 检查项扫描中被结束,不做特别处理
                break;
            }
            findRootItem()->noticeUIFileName(path + QString("/") + info.fileName());
        }
    }

    // 控制每一个子项展示结果
    foreach (auto item, m_childItems) {
        QApplication::processEvents();
        if (!item) {
            continue;
        }

        if (m_isToStop) {
            Q_EMIT scanStarted(false);
            item->setZeroSizeForStoppedItem();
            item->setItemUncheckable();
            continue;
        }

        item->showScanResult();
        m_totalSize += item->itemFileSize();
        m_totalAmount += item->itemFileAmount();
    }

    // 展示完成，通知界面隐藏spinner，改变说明标签内容
    Q_EMIT scanStarted(false);

    m_totalSize += m_itemsize;
    m_totalAmount += m_itemAmount;

    // 这里重设本次清理删除的文件列表,仅被扫描的文件可以清理
    m_deletePaths = m_deletePaths.mid(0, static_cast<int>(m_itemAmount));
    findRootItem()->recount();

    QString filesize = fileSizeToString(m_totalSize);
    if (0 == m_totalSize) {
        setItemUncheckable();
    }

    // 根结点的信息在recount时设置
    if (nullptr != m_parent) {
        Q_EMIT setScanResult(m_totalAmount, filesize);
    }

    // 根结点,且被选中时，发送展示完成信号
    if (nullptr == m_parent) {
        if (m_isChecked || m_unCheckable) {
            Q_EMIT showScanResultFinished();
        }
    }
}

// 扫描指定的文件列表，获取总数量与大小
// 此处的统计将提供总文件数以设置主界面进度条
void TrashCleanItem::startScan()
{
    if (!m_isChecked) {
        return;
    }

    foreach (auto item, m_childItems) {
        if (!item) {
            continue;
        }
        item->startScan();
    }

    if (m_parent == nullptr) {
        m_unCheckable = false;
        scan();
        Q_EMIT scanFinished();
    }
}

// 主界面要求停止展示扫描结果
void TrashCleanItem::stopShowResult()
{
    m_isToStop = true;
    foreach (auto item, m_childItems) {
        if (!item) {
            continue;
        }
        item->stopShowResult();
    }
}

// 准备开始扫描，清空上一次扫描结果
// 重置标志位
void TrashCleanItem::prepare()
{
    // 避免上一次扫描数据的遗留
    resetData();

    foreach (auto item, m_childItems) {
        if (!item) {
            continue;
        }
        item->prepare();
    }

    Q_EMIT prepareWidget();
}

// 实例化本结点依赖的窗口
void TrashCleanItem::initItemWidget()
{
    m_itemWidget = new CleanerResultItemWidget();
    if (m_parent != nullptr) {
        // 跟随根项目的窗口变化选中或未选中状态
        m_itemWidget->setRoot(findRootItem()->itemWidget());
        m_parent->addChild(this);
    } else {
        // 如果父窗口为空，设置本窗口为根检查项的样式
        m_itemWidget->setAsRoot();
    }

    // UI整改,右侧对齐,子项目减少长度
    if (m_parent != nullptr && m_parent->m_parent != nullptr) {
        m_itemWidget->setAsChild();
    }

    // 通知窗口清空内容，隐藏控件以准备新的扫描
    connect(this, &TrashCleanItem::prepareWidget, m_itemWidget, &CleanerResultItemWidget::prepare);
    // 通知窗口开始或停止展示
    connect(this,
            &TrashCleanItem::scanStarted,
            m_itemWidget,
            &CleanerResultItemWidget::setWorkStarted);
    // 将结果显示在依赖窗口下
    connect(this,
            &TrashCleanItem::setScanResult,
            m_itemWidget,
            &CleanerResultItemWidget::setScanResult);
    // 同步是否选中
    connect(this,
            &TrashCleanItem::setWidgetItemSelected,
            m_itemWidget,
            &CleanerResultItemWidget::setCheckBoxStatus);
    // 设置检查项已经被清理
    connect(this,
            &TrashCleanItem::setCleanDone,
            m_itemWidget,
            &CleanerResultItemWidget::setCleanDone);
    // 子项已完成清理或没有被扫描时,设置为不可选中的状态
    connect(this,
            &TrashCleanItem::setUncheckable,
            m_itemWidget,
            &CleanerResultItemWidget::setUncheckable);

    // 涉及到根选项的状态变化
    // 子项被选中时，让根项被选中
    // 根项被点击时，让所有子项选中、反选
    connect(m_itemWidget,
            &CleanerResultItemWidget::rootClicked,
            this,
            &TrashCleanItem::setChildSelected);
    connect(m_itemWidget,
            &CleanerResultItemWidget::childClicked,
            this,
            &TrashCleanItem::stageChanged);
}
