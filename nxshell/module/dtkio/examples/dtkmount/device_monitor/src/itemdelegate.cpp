// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "itemdelegate.h"
#include "devicedef.h"

#include <DApplication>
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QStyledItemDelegate>

QStringList ItemDelegate::m_unit{" B", " KB", " MB", " GB", " TB", " PB", tr("Invalid Size")};

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

ItemDelegate::~ItemDelegate() {}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();

        QVariant variant = index.data(Qt::UserRole);
        DeviceData data = variant.value<DeviceData>();

        QStyleOptionViewItem viewOption(option);

        const QRectF &rect(option.rect);

        const QRect &iconRect = QRect(rect.left() + 10, rect.top() + 10, 80, 80);
        const QRect &nameRect = QRect(rect.left() + 60, rect.top() + 15, rect.width() - 40, 20);
        const QRect &sizeTextRect = QRect(rect.left() + 60, rect.top() + 40, rect.width() - 40, 20);
        const QRect &unmountTextRect = QRect(rect.left() + 60, rect.bottom() - 40, rect.width() - 40, 20);
        const QRect &sizeBarRect = QRect(rect.left() + 100, rect.bottom() - 35, rect.width() - 110, 10);

        painter->drawPixmap(iconRect, QIcon::fromTheme(QString::fromUtf8("drive-removable-media")).pixmap(iconRect.size()));

        painter->setPen(QPen(Qt::black));
        qDebug() << data.mountPoint;
        if (data.mountPoint.size() == 1 && data.mountPoint.at(0) == '/') {
            data.deviceName = tr("System Device");
        }

        int sizeUnit = 0;                                    // 总容量单位
        quint64 usedSize = data.usedSize, size = data.size;  // 用于临时容量换算

        while ((size >>= 10) >= 1024)
            ++sizeUnit;
        sizeUnit > 0 ? ++sizeUnit : sizeUnit;  // 计算单位

        if (sizeUnit > 5) {
            qDebug() << "Invalid Size";
            sizeUnit = 6;
        }
        if (data.deviceName.isEmpty()) {
            data.deviceName = QString::number(size) + m_unit[sizeUnit] + tr(" Volume");
        }
        painter->drawText(nameRect, Qt::AlignHCenter, data.deviceName);

        if (data.isMount) {
            int usedSizeUnit = 0;  // 已使用容量单位

            while ((usedSize >>= 10) >= 1024)
                ++usedSizeUnit;
            usedSizeUnit > 0 ? ++usedSizeUnit : usedSizeUnit;

            if (usedSizeUnit > 5) {
                qDebug() << "Invalid Size";
                usedSizeUnit = 6;
            }
            painter->drawText(sizeTextRect,
                              Qt::AlignHCenter,
                              QString::number(usedSize) + m_unit[usedSizeUnit] + " / " + QString::number(size) +
                                  m_unit[sizeUnit]);

            QStyleOptionProgressBar bar;

            bar.rect = sizeBarRect;
            bar.progress = data.usedSize * 100.0 / data.size;
            bar.minimum = 0;
            bar.maximum = 100;

            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, painter);
        } else {
            painter->drawText(unmountTextRect, Qt::AlignHCenter, tr("Unmounted"));
        }

        painter->restore();
    }
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(250, 100);
}
