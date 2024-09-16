// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "selectactionpage.h"
#include "iconlabel.h"
#include "utilitylist.h"
#include "utilitylistitem.h"

#include <QListView>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QLabel>
#include <QEvent>

SelectActionPage::SelectActionPage(QWidget *parent)
    : ContentPairPage(parent)
{
    init();
    setMode(Disk);
}

SelectActionPage::Mode SelectActionPage::mode() const
{
    return m_mode;
}

SelectActionPage::Action SelectActionPage::action() const
{
    return m_action;
}

QString SelectActionPage::selectedItemTitle() const
{
    if (m_rightContent->selectedItems().isEmpty())
        return QString();

    if (UtilityListItem *item = qobject_cast<UtilityListItem*>(m_rightContent->itemWidget(m_rightContent->selectedItems().first())))
        return item->title();

    return QString();
}

QIcon SelectActionPage::selectedItemIcon() const
{
    if (m_rightContent->selectedItems().isEmpty())
        return QIcon();

    if (UtilityListItem *item = qobject_cast<UtilityListItem*>(m_rightContent->itemWidget(m_rightContent->selectedItems().first())))
        return item->icon();

    return QIcon();
}

void SelectActionPage::init()
{
    setLeftTitle(tr("Select media"));
    setRightTitle(tr("Select operation for media"));

    QWidget *left_content = new QWidget(this);
    QHBoxLayout *left_layout = new QHBoxLayout(left_content);

    left_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    IconLabel *disk_item = new IconLabel(left_content);

    disk_item->setTitle(tr("Disk"));
    disk_item->setIcon(QIcon(":/icons/disk_96.svg"), 80);
    disk_item->setDirection(QBoxLayout::TopToBottom);
    disk_item->setFixedSize(140, 140);
    disk_item->installEventFilter(this);
    disk_item->setHoverBackground(QColor(0, 0, 0, 0.05 * 255));

    IconLabel *partition_item = new IconLabel(left_content);

    partition_item->setTitle(tr("Partition"));
    partition_item->setIcon(QIcon(":/icons/partition_96.svg"), 80);
    partition_item->setDirection(QBoxLayout::TopToBottom);
    partition_item->setFixedSize(140, 140);
    partition_item->installEventFilter(this);
    partition_item->setHoverBackground(QColor(0, 0, 0, 0.05 * 255));

    left_layout->setContentsMargins(20, 20 ,20, 20);
    left_layout->addWidget(disk_item, 0, Qt::AlignHCenter | Qt::AlignTop);
    left_layout->addWidget(partition_item, 0, Qt::AlignHCenter | Qt::AlignTop);

    setLeftContent(left_content, true);

    m_diskItem = disk_item;
    m_partItem = partition_item;
}

void SelectActionPage::setMode(SelectActionPage::Mode mode)
{
    m_mode = mode;
    m_rightContent = new UtilityList(this);
    m_rightContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (mode == Disk) {
        m_rightContent->addItem(QIcon(":/icons/clone_disk_to_disk.svg"),
                               tr("Clone Disk"), tr("Clone source disk to target disk"),
                               QSize(120, 49));
        m_rightContent->addItem(QIcon(":/icons/clone_disk_to_image.svg"),
                               tr("Disk to Image"), tr("Backup disk data to an image file"),
                               QSize(120, 49));
        m_rightContent->addItem(QIcon(":/icons/restore_image_to_disk.svg"),
                               tr("Image to Disk"), tr("Restore image file to disk"),
                               QSize(120, 49));
        m_partItem->setChecked(false);
        m_diskItem->setChecked(true);
    } else {
        m_rightContent->addItem(QIcon(":/icons/clone_partition_to_partition.svg"),
                               tr("Clone Partition"), tr("Clone source partition to target partition"),
                               QSize(120, 49));
        m_rightContent->addItem(QIcon(":/icons/clone_partition_to_image.svg"),
                               tr("Partition to Image"), tr("Backup partition data to an image file"),
                               QSize(120, 49));
        m_rightContent->addItem(QIcon(":/icons/restore_image_to_partition.svg"),
                               tr("Image to Partition"), tr("Restore image file to partition"),
                               QSize(120, 49));
        m_partItem->setChecked(true);
        m_diskItem->setChecked(false);
    }

    setRightContent(m_rightContent, true);
    setAction(Clone);

    connect(m_rightContent, &UtilityList::currentRowChanged, this, [this] (int row) {
        setAction(Action(row));
    });
}

void SelectActionPage::setAction(SelectActionPage::Action action)
{
    m_action = action;
    m_rightContent->item(m_action)->setSelected(true);
}

bool SelectActionPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::MouseButtonPress) {
        return ContentPairPage::eventFilter(watched, event);
    }

    if (watched == m_diskItem) {
        setMode(Disk);
    } else if (watched == m_partItem) {
        setMode(Partition);
    }

    return ContentPairPage::eventFilter(watched, event);
}
