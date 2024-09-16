// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DocTabBar.h"
#include "DocSheet.h"
#include "Application.h"
#include "Global.h"

#include <DPlatformWindowHandle>

#include <QMimeData>
#include <QDir>
#include <QTimer>
#include <QUuid>
#include <QWindow>
#include <QLayout>

DocTabBar::DocTabBar(QWidget *parent)
    : DTabBar(parent)
{
#if (DTK_VERSION_MAJOR > 5 || (DTK_VERSION_MAJOR >= 5 && DTK_VERSION_MINOR >= 2 ))
    this->setEnabledEmbedStyle(true);//设置直角样式
    this->setExpanding(true);//设置平铺窗口模式
#endif

    this->setTabsClosable(true);

    this->setMovable(true);

    this->setElideMode(Qt::ElideMiddle);

    this->setVisibleAddButton(true);

    this->setDragable(true);

    this->setFocusPolicy(Qt::NoFocus);

    connect(this, SIGNAL(tabCloseRequested(int)), SLOT(onTabCloseRequested(int)));

    connect(this, SIGNAL(tabAddRequested()), SIGNAL(sigNeedOpenFilesExec()));

    connect(this, SIGNAL(currentChanged(int)), SLOT(onTabChanged(int)));

    connect(this, &DTabBar::tabReleaseRequested, this, &DocTabBar::onTabReleased);

    connect(this, &DTabBar::tabDroped, this, &DocTabBar::onTabDroped);

    connect(this, &DTabBar::dragActionChanged, this, &DocTabBar::onDragActionChanged);

    m_intervalTimer = new QTimer(this);

    m_intervalTimer->setSingleShot(true);
}

int DocTabBar::indexOfFilePath(const QString &filePath)
{
    //修改成根据文件的绝对路径查重
    for (int i = 0; i < count(); ++i) {
        DocSheet *docSheet = DocSheet::getSheet(this->tabData(i).toString());
        if (docSheet && (docSheet->filePath() == filePath)) {
            return i;
        }
    }
    return -1;
}

void DocTabBar::insertSheet(DocSheet *sheet, int index)
{
    if (sheet == nullptr)
        return;

    QString fileName = QFileInfo(sheet->filePath()).fileName();

    if (-1 == index)
        index = addTab(fileName);
    else
        index = insertTab(index, fileName);

    this->setTabToolTip(index, fileName);

    this->setTabData(index, DocSheet::getUuid(sheet));

    updateTabWidth();

    m_delayIndex = index;

    QTimer::singleShot(1, this, SLOT(onSetCurrentIndex()));
}

void DocTabBar::removeSheet(DocSheet *sheet)
{
    for (int i = 0; i < count(); ++i) {
        if (DocSheet::getSheet(this->tabData(i).toString()) == sheet) {
            removeTab(i);
            updateTabWidth();
            return;
        }
    }
}

void DocTabBar::showSheet(DocSheet *sheet)
{
    for (int i = 0; i < count(); ++i) {
        if (DocSheet::getSheet(this->tabData(i).toString()) == sheet) {
            this->setCurrentIndex(i);
            return;
        }
    }
}

QList<DocSheet *> DocTabBar::getSheets()
{
    QList<DocSheet *> sheets;

    for (int i = 0; i < count(); ++i) {
        DocSheet *sheet = DocSheet::getSheet(this->tabData(i).toString());
        if (nullptr != sheet)
            sheets.append(sheet);
    }

    if (sheets.isEmpty())
        return sheets;

    //需要按倒序排列
    QList<DocSheet *> orderSheets;
    QList<DocSheet *> allSheets = DocSheet::getSheets();
    for (int i = allSheets.count() - 1; i >= 0; --i) {
        if (sheets.contains(allSheets[i]))
            orderSheets.append(allSheets[i]);
    }

    return orderSheets;
}

void DocTabBar::updateTabWidth()
{
    int tabCount = count();
    if (tabCount != 0) {
        int tabWidth = (this->width() - 40) / tabCount - (tabCount - 1) * 10;
        for (int i = 0; i < count(); i++) {
            if (tabWidth <= 140) {
                setUsesScrollButtons(true);
                setTabMinimumSize(i, QSize(140, 37));
                setTabMaximumSize(i, QSize(140, 37));
            } else {
                setUsesScrollButtons(false);
                setTabMinimumSize(i, QSize(tabWidth, 37));
                setTabMaximumSize(i, QSize(tabWidth, 37));
            }
        }
    }
}

QMimeData *DocTabBar::createMimeDataFromTab(int index, const QStyleOptionTab &) const
{
    const QString tabName = tabText(index);

    QMimeData *mimeData = new QMimeData;

    mimeData->setData("deepin_reader/tabbar", tabName.toUtf8());

    mimeData->setData("deepin_reader/uuid", this->tabData(index).toByteArray());

    return mimeData;
}

void DocTabBar::insertFromMimeDataOnDragEnter(int index, const QMimeData *source)
{
    const QString tabName = QString::fromUtf8(source->data("deepin_reader/tabbar"));

    if (tabName.isEmpty())
        return;

    emit sigNeedActivateWindow();

    insertTab(index, tabName);

    this->setTabToolTip(index, tabName);

    updateTabWidth();
}

void DocTabBar::insertFromMimeData(int index, const QMimeData *source)
{
    QString id = source->data("deepin_reader/uuid");

    if (id.isEmpty())
        return;

    DocSheet *sheet = DocSheet::getSheet(id);

    if (nullptr != sheet) {
        insertSheet(sheet, index);
        sigTabMoveIn(sheet);
    }
}

bool DocTabBar::canInsertFromMimeData(int, const QMimeData *source) const
{
    return source->hasFormat("deepin_reader/tabbar");
}

void DocTabBar::dragEnterEvent(QDragEnterEvent *event)
{
    DTabBar::dragEnterEvent(event);
    if (event->mimeData()->hasFormat("deepin_reader/tabbar")) {
        QTimer::singleShot(1, [this]() {
            DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), false);
            QGuiApplication::changeOverrideCursor(Qt::DragCopyCursor);
            DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), true);
        });
    }
}

void DocTabBar::resizeEvent(QResizeEvent *e)
{
    DTabBar::resizeEvent(e);
    updateTabWidth();
}

void DocTabBar::onDragActionChanged(Qt::DropAction action)
{
    if (nullptr == dragIconWindow())    //不存在拖拽窗口执行setDisableWindowOverrideCursor会导致崩溃
        return;

    if (action == Qt::IgnoreAction) {
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), false);
        if (count() <= 1)
            QGuiApplication::changeOverrideCursor(Qt::ForbiddenCursor);
        else
            QGuiApplication::changeOverrideCursor(Qt::DragCopyCursor);
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), true);
    } else if (action == Qt::CopyAction) {
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), false);
        QGuiApplication::changeOverrideCursor(Qt::ArrowCursor);
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), true);
    } else if (dragIconWindow()) {
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), false);
        if (QGuiApplication::overrideCursor())
            QGuiApplication::changeOverrideCursor(QGuiApplication::overrideCursor()->shape());
    }
}

void DocTabBar::onTabChanged(int index)
{
    QString id = tabData(index).toString();

    sigTabChanged(DocSheet::getSheet(id));

}

void DocTabBar::onTabReleased(int)
{
    if (count() <= 1)
        return;

    int dropIndex = currentIndex();     //使用dropIndex替代index ,因为index是记录刚drag的index，当拖拽的时候几个item被移动了就会出错

    DocSheet *sheet = DocSheet::getSheet(this->tabData(dropIndex).toString());

    if (nullptr == sheet)
        return;

    removeTab(dropIndex);

    emit sigTabNewWindow(sheet);
}

void DocTabBar::onTabDroped(int, Qt::DropAction da, QObject *target)
{
    Q_UNUSED(da)    //同程序da可以根据目标传回，跨程序全是copyAction

    int dropIndex = currentIndex();     //使用dropIndex替代index ,因为index是记录刚drag的index，当拖拽的时候几个item被移动了就会出错

    DocSheet *sheet = DocSheet::getSheet(this->tabData(dropIndex).toString());

    if (nullptr == sheet)
        return;

    if (nullptr == target) {
        //如果是空则为新建窗口
        if (count() <= 1) {//如果是最后一个，不允许
            return;
        }
        removeTab(dropIndex);
        emit sigTabNewWindow(sheet);
    } else if (Qt::MoveAction == da) {
        //如果是移动
        removeTab(dropIndex);
        emit sigTabMoveOut(sheet);
    }
}

void DocTabBar::onSetCurrentIndex()
{
    setCurrentIndex(m_delayIndex);
}

void DocTabBar::onTabCloseRequested(int index)
{
    if (m_intervalTimer->isActive())
        return;

    DocSheet *sheet = DocSheet::getSheet(this->tabData(index).toString());

    if (nullptr == sheet)
        return;

    emit sigTabClosed(sheet);

    m_intervalTimer->start(100);    //100ms内的重复点击将被过滤
}

