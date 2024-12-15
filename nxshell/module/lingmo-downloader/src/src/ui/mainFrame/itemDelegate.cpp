// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file itemDelegate.cpp
 *
 * @brief 列表代理类
 *
 * @date 2020-06-09 10:00
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "itemDelegate.h"

#include <DCheckBox>
#include <DLineEdit>
#include <DApplication>

#include <QStandardItemModel>
#include <QStyleOptionProgressBar>
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QLineEdit>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QIcon>
#include <QProgressBar>
#include <QMouseEvent>
#include <QItemEditorFactory>
#include <QMimeType>
#include <QMimeDatabase>

#include "tableView.h"
#include "tableModel.h"
#include "global.h"
#include "../database/dbinstance.h"

ItemDelegate::ItemDelegate(QObject *parent, int Flag)
    : QStyledItemDelegate(parent)
    , m_hoverRow(-1)
{
    m_tableFlag = Flag;
    m_isFirstInside = true;
    // progressbar = new QProgressBar;
    m_bgImage = new QPixmap(":/icons/icon/bar-bg.png");
    m_frontImage = new QPixmap(":/icons/icon/bar-front.png");

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
            this, &ItemDelegate::onPalettetypechanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ItemDelegate::onPalettetypechanged);
}

ItemDelegate::~ItemDelegate()
{
    delete (m_bgImage);
    delete (m_frontImage);
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() == m_hoverRow) {
        painter->fillRect(option.rect, Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().frameBorder());
            // QColor(0,0,0,13)QColor(255,255,255,26)
    }
    if (index.row() % 2 != 0) {
        painter->fillRect(option.rect, QBrush(QColor(0, 0, 0, 8)));
    } else {
        if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType){
            painter->fillRect(option.rect, QBrush(QColor(255, 255, 255, 150)));
        } else {
            painter->fillRect(option.rect, QBrush(QColor(255, 255, 255, 10)));
        }
    }
    const QRect rect(option.rect);
    const int column(index.column());
    const bool isSelected = index.data(TableModel::Ischecked).toBool(); //option.state & QStyle::State_Selected;

    QFont font;
    painter->setFont(font);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QColor("#414D68"));
    if (Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        painter->setPen(QColor("#C0C6D4"));
    } else if (Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        painter->setPen(QColor("#414D68"));
    }
    const QRect textRect = rect.marginsRemoved(QMargins(10, 2, 0, 0));
    if (column == 0) {
        QStyleOptionButton checkBoxStyle;
        checkBoxStyle.state = index.data(TableModel::Ischecked).toBool() ? QStyle::State_On : QStyle::State_Off;
        checkBoxStyle.state |= QStyle::State_Enabled;
        checkBoxStyle.rect = option.rect;
        checkBoxStyle.rect.setX(option.rect.x() + 5);
        checkBoxStyle.rect.setWidth(20);
        QSharedPointer<DCheckBox> checkBtn = QSharedPointer<DCheckBox>(new DCheckBox, &QObject::deleteLater);
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxStyle, painter, checkBtn.data());
    } else if (column == 1) {
        if (isSelected) {
            painter->setPen(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
            painter->setBrush(QBrush(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight()));
            painter->drawRoundRect(rect.x(), rect.y(), 25, rect.height(), 25, 25);
            painter->drawRect(rect.x() + 15, rect.y(), rect.width() - 15, rect.height());
            painter->setPen(QColor("#FFFFFF"));
            QItemEditorFactory::setDefaultFactory(new QItemEditorFactory);
        }

        QRect localRect = textRect;
        QString filetype = index.data(TableModel::FileName).toString();
        QString fileSavePath = index.data(TableModel::SavePath).toString();
        QFileInfo fileInfo(fileSavePath);
        QPixmap pic;
        if (fileSavePath.contains(filetype) && fileInfo.isDir()) {
            QIcon icon = QIcon::fromTheme("folder");
            pic = icon.pixmap(20, 20);
        } else {
            QFileIconProvider iconProvider;
            QFileInfo filenameInfo(fileSavePath + filetype);
            QIcon icon = iconProvider.icon(filenameInfo);
            if (icon.isNull()) {
                icon = iconProvider.icon(QFileIconProvider::IconType::File);
            }
            pic = icon.pixmap(20, 20);
        }

        painter->drawPixmap(option.rect.x(), option.rect.y() + (option.rect.height() - 20)/2, pic);

        const QString path = index.data(TableModel::SavePath).toString();
        int status = index.data(TableModel::Status).toInt();
        if ((!QFileInfo::exists(path)) && (status == Global::DownloadTaskStatus::Complete || status == Global::DownloadTaskStatus::Removed)) { //文件不存在的任务，添加提示
            QPixmap errorPic = QIcon(":icons/icon/error.svg").pixmap(12, 12);
            painter->drawPixmap(option.rect.x() + 10, option.rect.y() + (option.rect.height() - 20)/2 + 8, errorPic);
        }

        const QRect rectText = localRect.marginsRemoved(QMargins(25, 2, 0, 5));
        QString name = painter->fontMetrics().elidedText(index.data(TableModel::FileName).toString(),
                                                         Qt::ElideMiddle,
                                                         textRect.width() - 20);
        painter->drawText(rectText, Qt::AlignVCenter | Qt::AlignLeft, name);
    } else if (column == 2) {
        if (isSelected) {
            painter->setPen(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
            painter->setBrush(QBrush(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight()));
            painter->drawRect(rect);
            painter->setPen(QColor("#FFFFFF"));
        }
        const QString size = index.data(TableModel::Size).toString();
        painter->drawText(rect.marginsRemoved(QMargins(5, 2, 0, 2)), Qt::AlignVCenter | Qt::AlignLeft, size);
    } else if (column == 3) {
        if (m_tableFlag == 0) {
            if (isSelected) {
                painter->setPen(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
                painter->setBrush(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight());
                painter->drawRoundRect(rect.x() + rect.width() - 25, rect.y(), 25, rect.height(), 25, 25);
                painter->drawRect(rect.x(), rect.y(), rect.width() - 15, rect.height());
                painter->setPen(QColor("#FFFFFF"));

                m_bgImage->load(":/icons/icon/progressbar_bg.png");
                m_frontImage->load(":/icons/icon/progressbar_fg.png");
            } else {
                if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType){
                    m_bgImage->load(":/icons/icon/bar-bg.png");
                    m_frontImage->load(":/icons/icon/bar-front.png");
                } else {
                    m_bgImage->load(":/icons/icon/progressbar_bg_dark.png");
                    m_frontImage->load(":/icons/icon/progressbar_fg_dark.png");
                }
            }

            QFont font;
            painter->setFont(font);

            QRect barRect = textRect;
            barRect.setTop(textRect.top() + textRect.height() / 2);
            barRect.setWidth(barRect.width() - 10);
            barRect.setHeight(textRect.height() / 2);
            QRect sizeRect = barRect;
            sizeRect.setTop(barRect.top() - 10);
            sizeRect.setHeight(10);

            QStyleOptionViewItem viewOption(option);
            initStyleOption(&viewOption, index);
            if ((index.data(TableModel::Status) == Global::Paused) ||
                    (index.data(TableModel::Status) == Global::Lastincomplete)) {
                const QString pauseText = painter->fontMetrics().elidedText(tr("Paused"),
                                                                            Qt::ElideRight, textRect.width() - 10);
                painter->drawText(barRect, Qt::AlignVCenter | Qt::AlignLeft, pauseText);
            } else if (index.data(TableModel::Status) == Global::Error) {
                QFont font;
                font.setPointSize(10);
                painter->setFont(font);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(QColor("#9a2f2f"));
                const QRect rectText = textRect.marginsRemoved(QMargins(5, 2, 0, 5));
                const QString errorText = painter->fontMetrics().elidedText(tr("Failed"),
                                                                            Qt::ElideRight, rectText.width() - 10);
                painter->drawText(rectText, Qt::AlignVCenter | Qt::AlignLeft, errorText);
                return;
            } else if (index.data(TableModel::Status) == Global::Waiting) {
                QFont font;
                font.setPointSize(10);
                painter->setFont(font);
                painter->setRenderHint(QPainter::Antialiasing);
                painter->setPen(QColor("#8AA1B4"));
                const QRect rectText = textRect.marginsRemoved(QMargins(5, 2, 0, 5));
                const QString wattingText = painter->fontMetrics().elidedText(tr("Waiting"),
                                                                              Qt::ElideRight, rectText.width() - 10);
                painter->drawText(rectText, Qt::AlignVCenter | Qt::AlignLeft, wattingText);
                return;
            } else {
                QString speed = index.data(TableModel::Speed).toString();
                if (speed.left(2).toInt() < 0) {
                    speed = "0KB/s";
                }
                QString str = " " + index.data(TableModel::Percent).toString()
                              + "%    " + speed;
                if (index.data(TableModel::announceList).toInt()) {
                    str += "  " + tr("Resources:") + " " + QString("%1/%2")
                            .arg(index.data(TableModel::connection).toInt())
                            .arg(index.data(TableModel::announceList).toInt());
                }
                str += "   " + tr("Time left ") + index.data(TableModel::Time).toString();

                const QString sizeText = painter->fontMetrics().elidedText(str,
                                                                           Qt::ElideRight, textRect.width() - 10);
                painter->drawText(barRect, Qt::AlignVCenter | Qt::AlignLeft, sizeText);
            }

            QRect s1(0, 0, 3, m_bgImage->height());
            QRect t1(sizeRect.x(), sizeRect.y(), 3, m_bgImage->height());
            painter->drawPixmap(t1, *m_bgImage, s1);

            // bg m
            QRect s2(m_bgImage->width() - 3, 0, 3, m_bgImage->height());
            QRect t2(sizeRect.x() + sizeRect.width() - 16, sizeRect.y(), 3, m_bgImage->height());
            painter->drawPixmap(t2, *m_bgImage, s2);

            // bg t
            QRect s3(3, 0, m_bgImage->width() - 6, m_bgImage->height());
            QRect t3(sizeRect.x() + 3, sizeRect.y(), sizeRect.width() - 19, m_bgImage->height());
            painter->drawPixmap(t3, *m_bgImage, s3);

            float p = index.data(TableModel::Percent).toFloat() / 100.0f;
            int w = static_cast<int>((sizeRect.width() - 16) * p);

            if (w <= 3) {
                QRect s(sizeRect.x(), sizeRect.y(), w, m_frontImage->height());
                QRect f(0, 0, 3, m_frontImage->height());
                painter->drawPixmap(s, *m_frontImage, f);
            } else if ((w > 3) && (w <= sizeRect.width() - 10)) {
                // front h
                QRect s(sizeRect.x(), sizeRect.y(), 3, m_frontImage->height());
                QRect f(0, 0, 3, m_frontImage->height());
                painter->drawPixmap(s, *m_frontImage, f);

                // front m
                QRect fs3(sizeRect.x() + 3, sizeRect.y(), w - 3, m_frontImage->height());
                QRect ft3(3, 0, m_frontImage->width() - 6, m_frontImage->height());
                painter->drawPixmap(fs3, *m_frontImage, ft3);

                // front td
                QRect s4(sizeRect.x() + w, sizeRect.y(), 3, m_frontImage->height());
                QRect f4(m_frontImage->width() - 3, 0, 3, m_frontImage->height());
                painter->drawPixmap(s4, *m_frontImage, f4);
            }
        }
    } else if (column == 4) {
        if (isSelected) {
            painter->setPen(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
            painter->setBrush(QBrush(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight()));
            painter->drawRoundRect(rect.x() + rect.width() - 25, rect.y(), 25, rect.height(), 25, 25);
            painter->drawRect(rect.x(), rect.y(), rect.width() - 15, rect.height());
            painter->setPen(QColor("#FFFFFF"));
        }
        const QString time = index.data(TableModel::Time).toString();
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, time);

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    switch (index.column()) {
    case 0:
        return QSize(40, 50);
    case 1:
        return QSize(248, 50);
    case 2:
        return QSize(114, 50);
    case 3:
        return QSize(304, 50);
    case 4:
        return QSize(304, 50);
    }
    return QSize(-1, 50);
}

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                               const QStyleOptionViewItem &option, const QModelIndex &index)
{
    bool ret = true;
    const int column(index.column());

    if (column == 0) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        QRect rect(option.rect);
        rect.setWidth(25);
        rect.setX(rect.x() + 10);
        if ((event->type() == QEvent::MouseButtonPress) && rect.contains(mouseEvent->pos())) {
            QVariant value;
            value = model->data(index, TableModel::Ischecked);
            Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                                        ? Qt::Unchecked
                                        : Qt::Checked);
            ret = model->setData(index, state, TableModel::Ischecked); // 取反后存入模型
        } else {
            ret = QStyledItemDelegate::editorEvent(event, model, option, index);
        }
    } else {
        ret = QStyledItemDelegate::editorEvent(event, model, option, index);
    }
    return ret;
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    Q_UNUSED(option);
    DLineEdit *pEdit = new DLineEdit(parent);
    QRegExp regx("[^\\\\/\':\\*\\?\"<>|#%？]+"); //屏蔽特殊字符
    QValidator *validator = new QRegExpValidator(regx, pEdit);
    pEdit->lineEdit()->setValidator(validator);
    pEdit->lineEdit()->setMaxLength(83);
    connect(pEdit, &DLineEdit::textChanged, this, [=](QString filename) {
        DLineEdit *pEdit = qobject_cast<DLineEdit *>(sender());
        if(pEdit == nullptr) {
            return ;
        }
        QString str = index.data(TableModel::FileName).toString();
        QMimeDatabase db;
        QString mime = db.suffixForFileName(str);
        QString filePath = index.data(TableModel::SavePath).toString();
        filePath = filePath.left(filePath.lastIndexOf("/") + 1);
        filePath = filePath + filename + "." + mime;
        QFileInfo file(filePath);

        int hasSameFile = false;
        for (int i = 0; i < index.model()->rowCount(); i++) {
            QModelIndex idx = index.sibling(i, index.column());
            QString path = idx.data(TableModel::SavePath).toString();
            if (filePath == path && (filePath != index.data(TableModel::SavePath).toString())) {
                hasSameFile = true;
                break;
            }
        }

        if ((file.isFile() && (filePath != index.data(TableModel::SavePath).toString())) || (hasSameFile)) {
            pEdit->showAlertMessage(tr("Duplicate name"), -1);
        } else {
            pEdit->hideAlertMessage();
        }
    });
    pEdit->resize(parent->size());
    return pEdit;
}

void ItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    DLineEdit *pEdit = qobject_cast<DLineEdit *>(editor);
    QString str = index.data(TableModel::FileName).toString();

    QMimeDatabase db;
    QString mime = db.suffixForFileName(str);
    str = str.left(str.size() - mime.size() - 1);
    pEdit->setText(str);
}

void ItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    DLineEdit *pEdit = qobject_cast<DLineEdit *>(editor);
    QString str = index.data(TableModel::FileName).toString();
    QMimeDatabase db;
    QString mime = db.suffixForFileName(str);
    QString fileName = pEdit->text() + "." + mime;

    QString filePath = index.data(TableModel::SavePath).toString();
    filePath = filePath.left(filePath.lastIndexOf("/") + 1);
    filePath = filePath + fileName;
    if (!QFileInfo::exists(filePath) && !pEdit->text().isEmpty()) {
        for (int i = 0; i < index.model()->rowCount(); i++) {
            QModelIndex idx = index.sibling(i, index.column());
            QString path = idx.data(TableModel::SavePath).toString();
            if (filePath == path && (filePath != index.data(TableModel::SavePath).toString())) {
                return;
            }
        }

        QFile::rename(index.data(TableModel::SavePath).toString(), filePath);
        model->setData(index, fileName, TableModel::FileName);
        model->setData(index, filePath, TableModel::SavePath);
        TaskInfo task;
        DBInstance::getTaskByID(index.data(TableModel::taskId).toString(), task);
        task.downloadPath = filePath;
        task.downloadFilename = fileName;
        DBInstance::updateTaskInfoByID(task);
    }
}

void ItemDelegate::onHoverchanged(const QModelIndex &index)
{
    m_hoverRow = index.row();
}

void ItemDelegate::onPalettetypechanged(DGuiApplicationHelper::ColorType type)
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        m_frontImage->load(":/icons/icon/bar-bg.png");
        m_bgImage->load(":/icons/icon/bar-front.png");
    } else if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        m_bgImage->load(":/icons/icon/bar-bg.png");
        m_frontImage->load(":/icons/icon/bar-front.png");
    }
}
