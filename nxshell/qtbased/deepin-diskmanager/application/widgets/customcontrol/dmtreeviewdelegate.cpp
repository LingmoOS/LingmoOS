// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "dmtreeviewdelegate.h"
#include "common.h"
#include "dmtreeview.h"

#include <DFontSizeManager>

#include <QDebug>
#include <QTextOption>
#include <QApplication>

DmTreeviewDelegate::DmTreeviewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
    m_parentPb = DApplicationHelper::instance()->palette(m_parentView);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this,
            &DmTreeviewDelegate::onHandleChangeTheme);
    onHandleChangeTheme();
}

QSize DmTreeviewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option);

    DiskInfoData infoData = index.data(Qt::UserRole + 1).value<DiskInfoData>();
    int height = 55;
    if (infoData.m_level == DMDbusHandler::DISK || infoData.m_level == DMDbusHandler::VOLUMEGROUP) {
        height = 72 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
        return QSize(180, height);
    }

    if (infoData.m_level == DMDbusHandler::OTHER) {
        height = 30 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
        return QSize(180, height);
    }

    height = 55 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
    return QSize(180, height);
}

void DmTreeviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QVariant varData = index.data(Qt::UserRole + 1);
    DiskInfoData data = varData.value<DiskInfoData>();
    QRect rect;
    rect.setX(option.rect.x() + 10);
    if (data.m_level == DMDbusHandler::DISK || data.m_level == DMDbusHandler::OTHER || data.m_level == DMDbusHandler::VOLUMEGROUP) {
        rect.setY(option.rect.y() + 10);
    } else {
        rect.setY(option.rect.y());
    }

    rect.setWidth(option.rect.width());
    //    rect.setHeight(option.rect.height()); // 分区节点间有间隔
    // 去掉分区节点间隔
    if (data.m_level == DMDbusHandler::DISK || data.m_level == DMDbusHandler::VOLUMEGROUP) {
        rect.setHeight(option.rect.height() - 9);
    } else {
        rect.setHeight(option.rect.height() + 1);
    }

    painter->setRenderHints(QPainter::SmoothPixmapTransform);
    QRect paintRect = QRect(rect.left(), rect.top(), rect.width() - 19, rect.height());
    if (data.m_level == DMDbusHandler::OTHER) {
        paintRect = QRect(rect.left(), rect.top(), rect.width() - 10, rect.height());
    }

    QPainterPath path;
    const int radius = 8;
    path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
    path.lineTo(paintRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                     QSize(radius * 2, radius * 2)),
               0, 90);
    path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                     QSize(radius * 2, radius * 2)),
               180, 90);
    path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                     QSize(radius * 2, radius * 2)),
               270, 90);
    //    设置分区背景色
    //            if (data.level == 1) {
    //        QBrush brush = m_parentPb.itemBackground();
    //        painter->setBrush(brush);
    //        painter->fillPath(path, brush);
    //    }

    // 设置分区选中时文本颜色
    if ((option.state & QStyle::State_Selected) && (data.m_level != DMDbusHandler::OTHER)) {
        QColor fillColor = m_parentPb.color(DPalette::Normal, DPalette::Highlight);
        painter->setBrush(QBrush(fillColor));
        painter->fillPath(path, painter->brush());
        painter->setPen(QPen(Qt::white));
    }

    DmTreeview *treeView = qobject_cast<DmTreeview *>(m_parentView);
    //    if (treeView == nullptr) {
    //    }

    QIcon directionIcon;
    int pixmapWidth = 8; // 伸缩按钮宽
    int pixmapHeight = 8; // 伸缩按钮高
    if (treeView->getItemByIndex(index) && treeView->getItemByIndex(index)->hasChildren()) {
        if ((option.state & QStyle::State_Selected) && (data.m_level == DMDbusHandler::DISK || data.m_level == DMDbusHandler::VOLUMEGROUP)) {
            if (treeView->isExpanded(index)) {
                directionIcon = Common::getIcon("arrow_check");
                pixmapWidth = 10;
                pixmapHeight = 11;
            } else {
                directionIcon = Common::getIcon("arrow_right_check");
                pixmapWidth = 12;
                pixmapHeight = 11;
            }
        } else if (data.m_level == DMDbusHandler::OTHER) {
            if (treeView->isExpanded(index)) {
                directionIcon = Common::getIcon("smallarrow");
                pixmapWidth = 9;
                pixmapHeight = 10;
            } else {
                directionIcon = Common::getIcon("smallarrow_right");
                pixmapWidth = 8;
                pixmapHeight = 10;
            }
        } else {
            if (treeView->isExpanded(index)) {
                directionIcon = Common::getIcon("arrow");
                pixmapWidth = 10;
                pixmapHeight = 7;
            } else {
                directionIcon = Common::getIcon("arrow_right");
                pixmapWidth = 8;
                pixmapHeight = 11;
            }
        }
    }

    QRect lefticon1Rect;
    QRect lefticonRect2;
    QRect textRect;
    QRect textRect1;
    QString text = data.m_diskPath;
    QString text1 = data.m_diskSize;
    QString text2 = data.m_partitionPath;
    QString text3 = data.m_partitionSize;

    if (data.m_level == DMDbusHandler::LOGICALVOLUME) {
        text2 = data.m_sysLabel;
    }

    if (data.m_level == DMDbusHandler::OTHER) {
        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T8, QFont::Medium);
        DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
        if (themeType == DGuiApplicationHelper::LightType) {
            QColor color("#000000");
            color.setAlphaF(0.5);
            painter->setPen(color);
        } else if (themeType == DGuiApplicationHelper::DarkType) {
            QColor color("#ffffff");
            color.setAlphaF(0.5);
            painter->setPen(color);
        }

        painter->setFont(font);
        textRect.setRect(paintRect.left() + 10, paintRect.top(), 130, 100);
        QString path = painter->fontMetrics().elidedText(text, Qt::ElideMiddle, 120);
        painter->drawText(textRect, path);

        lefticon1Rect.setRect(paintRect.left() + 160, paintRect.top() + 6, pixmapWidth, pixmapHeight);
        painter->drawPixmap(lefticon1Rect, directionIcon.pixmap(17, 17));

    } else if (data.m_level == DMDbusHandler::DISK || data.m_level == DMDbusHandler::VOLUMEGROUP) {
        int height = 24 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
        lefticon1Rect.setRect(paintRect.left() + 8, paintRect.top() + height, pixmapWidth, pixmapHeight);
        painter->drawPixmap(lefticon1Rect, directionIcon.pixmap(17, 17));
        height = 9 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
        lefticonRect2.setRect(paintRect.left() + 20, paintRect.top() + height, 40, 40);

        QIcon icon = Common::getIcon("treedisk");
        if (data.m_level == DMDbusHandler::DISK) {
            if (DMDbusHandler::instance()->getIsJoinAllVG().value(text) == "true") {
                icon = Common::getIcon("treevg");
            } else if (DMDbusHandler::instance()->getIsAllEncryption().value(text) == "true") {
                icon = Common::getIcon("treedisklock");
            }
        } else if (data.m_level == DMDbusHandler::VOLUMEGROUP) {
            if (DMDbusHandler::instance()->getIsAllEncryption().value(text) == "true") {
                icon = Common::getIcon("treevglock");
            } else {
                icon = Common::getIcon("treevg");
            }
        }
        painter->drawPixmap(lefticonRect2, icon.pixmap(38, 38));

        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
        if ((option.state & QStyle::State_Selected) && (data.m_level == DMDbusHandler::DISK || data.m_level == DMDbusHandler::VOLUMEGROUP)) {
            QColor textcolor = m_parentPb.color(DPalette::Normal, DPalette::HighlightedText);
            painter->setPen(textcolor);
            painter->setFont(font);
            textRect.setRect(paintRect.left() + 65, paintRect.top() + 9, 110, 100);
            QString devName = painter->fontMetrics().elidedText(text, Qt::ElideMiddle, 108);
            painter->drawText(textRect, devName);
            QColor text1color = m_parentPb.color(DPalette::Normal, DPalette::HighlightedText);
            painter->setPen(text1color);
            font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
            painter->setFont(font);
            height = 32 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            textRect1.setRect(paintRect.left() + 65, paintRect.top() + height, 110, 100);
            QString textSize = painter->fontMetrics().elidedText(text1, Qt::ElideMiddle, 108);
            painter->drawText(textRect1, textSize);
        } else {
            QColor textcolor = m_parentPb.color(DPalette::Normal, DPalette::Text);
            painter->setPen(textcolor);
            painter->setFont(font);
            textRect.setRect(paintRect.left() + 65, paintRect.top() + 9, 110, 100);
            QString devName = painter->fontMetrics().elidedText(text, Qt::ElideMiddle, 108);
            painter->drawText(textRect, devName);
            QColor text1color = m_parentPb.color(DPalette::Normal, DPalette::TextTips);
            painter->setPen(text1color);
            font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
            painter->setFont(font);
            height = 32 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            textRect1.setRect(paintRect.left() + 65, paintRect.top() + height, 110, 100);
            QString textSize = painter->fontMetrics().elidedText(text1, Qt::ElideMiddle, 108);
            painter->drawText(textRect1, textSize);
        }
    } else {
        int height = 10 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
        lefticon1Rect.setRect(paintRect.left() + 30, paintRect.top() + height, 35, 35);
        QIcon icon = Common::getIcon("harddisk");
        if (data.m_level == DMDbusHandler::LOGICALVOLUME) {
            if (data.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
                icon = Common::getIcon("treelvlock");
            } else {
                icon = Common::getIcon("treelv");
            }
        } else {
            if (data.m_vgFlag != LVMFlag::LVM_FLAG_NOT_PV) {
                icon = Common::getIcon("treelv");
            } else if (data.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
                icon = Common::getIcon("treepartitionlock");
            }
        }
        painter->drawPixmap(lefticon1Rect, icon.pixmap(35, 35));

        QIcon icon1 = Common::getIcon("mounticon");
        QIcon icon2 = Common::getIcon("uninstallicon");
        QIcon icon3 = Common::getIcon("hidden");
        height = 32 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
        QRect mounticonRect = QRect(paintRect.left() + 52, paintRect.top() + height, 10, 10);

        //        // 获取分区是否隐藏
        //        int hide = 0;
        //        if (data.mountpoints.isEmpty() || data.mountpoints == "/recovery") {
        //            int result = DMDbusHandler::instance()->getPartitionHiddenFlag(data.diskpath, data.partitonpath);
        //            if (1 == result) {
        //                hide = 1;
        //            }
        //        }

        if (data.m_fstype == "unallocated") {
            painter->drawPixmap(mounticonRect, icon2.pixmap(10, 10));
        } else if (1 == data.m_flag) {
            painter->drawPixmap(mounticonRect, icon3.pixmap(10, 10));
        } else {
            painter->drawPixmap(mounticonRect, icon1.pixmap(10, 10));
        }

        QFont font = DFontSizeManager::instance()->get(DFontSizeManager::T6);
        if ((option.state & QStyle::State_Selected) && (data.m_level == DMDbusHandler::PARTITION || data.m_level == DMDbusHandler::LOGICALVOLUME)) {
            QColor textColor = m_parentPb.color(DPalette::Normal, DPalette::HighlightedText);
            painter->setPen(textColor);
            painter->setFont(font);
            textRect.setRect(paintRect.left() + 70, paintRect.top() + 5, 106, 100);
            QString text = painter->fontMetrics().elidedText(text2, Qt::ElideMiddle, 104);
            painter->drawText(textRect, text);
            QColor text1Color = m_parentPb.color(DPalette::Normal, DPalette::HighlightedText);
            painter->setPen(text1Color);
            font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
            painter->setFont(font);
            height = 25 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            textRect1.setRect(paintRect.left() + 70, paintRect.top() + height, 106, 100);
            QString textSize = painter->fontMetrics().elidedText(text3, Qt::ElideMiddle, 104);
            painter->drawText(textRect1, textSize);
        } else {
            QColor textColor = m_parentPb.color(DPalette::Normal, DPalette::Text);
            painter->setPen(textColor);
            painter->setFont(font);
            textRect.setRect(paintRect.left() + 70, paintRect.top() + 5, 106, 100);
            QString text = painter->fontMetrics().elidedText(text2, Qt::ElideMiddle, 104);
            painter->drawText(textRect, text);
            QColor text1Color = m_parentPb.color(DPalette::Normal, DPalette::TextTips);
            painter->setPen(text1Color);
            font = DFontSizeManager::instance()->get(DFontSizeManager::T8);
            painter->setFont(font);
            height = 25 + static_cast<int>((QApplication::font().pointSizeF() / 0.75 - 14) * 1);
            textRect1.setRect(paintRect.left() + 70, paintRect.top() + height, 106, 100);
            QString textSize = painter->fontMetrics().elidedText(text3, Qt::ElideMiddle, 104);
            painter->drawText(textRect1, textSize);
        }
    }

    painter->restore();
}
void DmTreeviewDelegate::onHandleChangeTheme()
{
    m_parentPb = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    m_parentView->update(m_parentView->currentIndex());
}
