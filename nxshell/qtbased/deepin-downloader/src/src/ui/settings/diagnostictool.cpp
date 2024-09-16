// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file Diagnostictool.cpp
 *
 * @brief 下载诊断工具
 *
 * @date 2020-08-24 17:10
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

#include "diagnostictool.h"

#include <DLabel>
#include <DGuiApplicationHelper>
#include <QLayout>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QTimer>
#include <QScrollBar>
#include <QPaintEvent>
#include <QPainterPath>
#include <QScrollArea>
#include "global.h"
#include "func.h"
#include "aria2rpcinterface.h"

DiagnosticTool::DiagnosticTool(QWidget *parent)
    : DDialog(parent)
    , m_Tableview(new QTableView)
    , m_Model(new DiagnosticModel)
{
    initUI();
    QTimer::singleShot(500, this, SLOT(startDiagnostic()));
    setAccessibleName("DiagnosticTool");
    m_Tableview->setAccessibleName("DiagnosticTableView");
    m_Tableview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_Tableview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

DiagnosticTool::~DiagnosticTool()
{
    delete m_Model;
    delete m_Tableview;
    delete m_delegate;
}

void DiagnosticTool::initUI()
{
    QIcon tryIcon = QIcon(QIcon::fromTheme(":/icons/icon/downloader2.svg"));
    tryIcon.pixmap(QSize(30, 30));
    setIcon(tryIcon);
    QLabel *mainLabel = new QLabel(this);
    mainLabel->setMinimumSize(440, 380);
    BaseWidget *pWidget = new BaseWidget("");
    pWidget->setMinimumSize(420, 380);
//    QPalette p;
//    p.setColor(QPalette::Background, QColor(255, 255, 255));
//    pWidget->setPalette(p);
    //pWidget->setStyleSheet("QLabel{background-color: rgb(255, 255, 255);}");
    //setPalette(p);

    QFont f;
    f.setBold(true);
    QLabel *pLabel = new QLabel(tr("Result:"));
    pLabel->setFont(f);
    m_Button = new QPushButton(tr("Diagnose Again"), this);
    m_Button->setAccessibleName("diagnoseBtn");
    m_Button->setFixedWidth(202);
    connect(m_Button, &QPushButton::clicked, this, [=]() {
        m_Model->clearData();
        QTimer::singleShot(500, this, [=]() {
            startDiagnostic();
        });
    });

    QVBoxLayout *pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(10, 10, 10, 10);
    pLayout->addSpacing(10);
    pLayout->addWidget(pLabel);
    pLayout->addSpacing(10);
    pLayout->addWidget(m_Tableview);
    pLayout->addStretch();
    pWidget->setLayout(pLayout);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(pWidget);
    mainLayout->addStretch();
    mainLabel->setLayout(mainLayout);
    addContent(mainLabel, Qt::AlignHCenter |  Qt::AlignTop);
    addSpacing(10);
    addContent(m_Button, Qt::AlignHCenter);

    m_delegate = new DiagnosticDelegate(this);
    m_Tableview->setModel(m_Model);
    m_Tableview->setItemDelegate(m_delegate);
    m_Tableview->horizontalHeader()->hide();
    m_Tableview->verticalHeader()->hide();
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        m_Tableview->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
        m_Tableview->setMinimumHeight(Global::tableView_NormalMode_Width * 6);
        pWidget->setMinimumHeight(Global::tableView_NormalMode_Width * 6 + 80);
        mainLabel->setMinimumHeight(Global::tableView_NormalMode_Width * 6 + 100);
    } else {
        m_Tableview->verticalHeader()->setDefaultSectionSize(Global::tableView_CompactMode_Width);
        m_Tableview->setMinimumHeight(Global::tableView_CompactMode_Width * 6);
        pWidget->setMinimumHeight(Global::tableView_CompactMode_Width * 6 + 80);
        mainLabel->setMinimumHeight(Global::tableView_CompactMode_Width * 6 + 100);
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            m_Tableview->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
            m_Tableview->setMinimumHeight(Global::tableView_NormalMode_Width * 6);
            pWidget->setMinimumHeight(Global::tableView_NormalMode_Width * 6 + 80);
            mainLabel->setMinimumHeight(Global::tableView_NormalMode_Width * 6 + 100);
        } else {
            m_Tableview->verticalHeader()->setDefaultSectionSize(Global::tableView_CompactMode_Width);
            m_Tableview->setMinimumHeight(Global::tableView_CompactMode_Width * 6);
            pWidget->setMinimumHeight(Global::tableView_CompactMode_Width * 6 + 80);
            mainLabel->setMinimumHeight(Global::tableView_CompactMode_Width * 6 + 100);
        }
    });
#else
    m_Tableview->verticalHeader()->setDefaultSectionSize(Global::tableView_NormalMode_Width);
    m_Tableview->setMinimumHeight(Global::tableView_NormalMode_Width * 6);
    pWidget->setMinimumHeight(Global::tableView_NormalMode_Width * 6 + 80);
    mainLabel->setMinimumHeight(Global::tableView_NormalMode_Width * 6 + 100);
#endif
    m_Tableview->setFixedWidth(404);
    m_Tableview->setShowGrid(false);
    m_Tableview->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_Tableview->setColumnWidth(0, 42);
    m_Tableview->setColumnWidth(1, 290);
    m_Tableview->setColumnWidth(2, 72);
    m_Tableview->setAlternatingRowColors(true);
    m_Tableview->setEnabled(false);
    m_Tableview->verticalScrollBar()->setHidden(true);

}

void DiagnosticTool::onAriaOption(bool isHasTracks, bool isHasDHT)
{
    m_IsHasTracks = isHasTracks;
    m_IsHasDHT = isHasDHT;
    //m_Model->setData(isHasTracks);
}

void DiagnosticTool::startDiagnostic()
{
    Aria2RPCInterface::instance()->getGlobalOption();
    m_Button->setEnabled(false);
    //m_Model->appendData(Func::isIpv6Connect());
    m_Tableview->update();
    QTimer::singleShot(200, this, [=]() {
        m_Model->appendData(Func::isIPV6Connect());
    });

    QTimer::singleShot(qrand() % (800) + 200, this, [=]() {
        m_Model->appendData(m_IsHasDHT & Func::isNetConnect());
    });

    QTimer::singleShot(qrand() % (800) + 800, this, [=]() {
        m_Model->appendData(Func::isHTTPConnect());
    });

    QTimer::singleShot(qrand() % (800) + 1400, this, [=]() {
        m_Model->appendData((m_IsHasTracks | m_IsHasDHT) & Func::isNetConnect());
    });
    QTimer::singleShot(qrand() % (800) + 2000, this, [=]() {
        m_Model->appendData((m_IsHasTracks | m_IsHasDHT) & Func::isNetConnect());
    });

    QTimer::singleShot(qrand() % (800) + 2500, this, [=]() {
        m_Model->appendData(Func::isNetConnect());
        m_Button->setEnabled(true);
    });
}

DiagnosticModel::DiagnosticModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

DiagnosticModel::~DiagnosticModel()
{
}

void DiagnosticModel::appendData(bool b)
{
    const int row = m_DiagnosticStatusList.size();
    beginInsertRows(QModelIndex(), row, row);
    m_DiagnosticStatusList.append(b);
    endInsertRows();
}

void DiagnosticModel::clearData()
{
    m_DiagnosticStatusList.clear();
}

bool DiagnosticModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    switch (role) {
    case Qt::DisplayRole:
        m_DiagnosticStatusList.replace(index.row(), value.toBool());
        break;
    default:
        break;
    }

    return true;
}

int DiagnosticModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 6;
}

int DiagnosticModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant DiagnosticModel::data(const QModelIndex &index, int role) const
{
    if (m_DiagnosticStatusList.count() <= index.row()) {
        return QVariant();
    }
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0) {
            return m_DiagnosticStatusList.at(index.row()) ? (":icons/icon/normal.svg") : (":icons/icon/defeat.svg");
        } else if (index.column() == 1) {
            switch (index.row()) {
            case 0:
                return tr("IPv6 support");
            case 1:
                return tr("DHT status");
            case 2:
                return tr("HTTP task");
            case 3:
                return tr("BT task");
            case 4:
                return tr("Magnet task");
            case 5:
                return tr("Network detection");
            default:
                break;
            }
        } else if (index.column() == 2) {
            return m_DiagnosticStatusList.at(index.row()) ? tr("Pass") : tr("Failed");
        }
        break;
    case Qt::TextAlignmentRole:
        if (index.column() == 2) {
            return Qt::AlignLeft;
        }
        break;
    case Qt::TextColorRole:
        return m_DiagnosticStatusList.at(index.row()) ? ("#00c77d") : ("#ff5736");
    case Qt::AccessibleTextRole:
    case Qt::AccessibleDescriptionRole: {
        if (index.column() == 1) {
            switch (index.row()) {
            case 0:
                return ("IPv6support");
            case 1:
                return ("DHTstatus");
            case 2:
                return ("HTTPtask");
            case 3:
                return ("BTtask");
            case 4:
                return ("Magnettask");
            case 5:
                return ("Networkdetection");
            default:
                break;
            }
        } else if (index.column() == 2) {
            return m_DiagnosticStatusList.at(index.row()) ? "Pass" : "Failed";
        }
    }
    }
    return QVariant();
}

DiagnosticDelegate::DiagnosticDelegate(QObject *parent, int Flag)
{
    Q_UNUSED(parent);
    Q_UNUSED(Flag);
}

void DiagnosticDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int radius = 8;
    QRect paintRect = QRect(option.rect.left(), option.rect.top(), option.rect.width(), option.rect.height());
    QPainterPath path;
    path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
    path.lineTo(paintRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                     QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                     QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                     QSize(radius * 2, radius * 2)), 270, 90);


    painter->setPen(QColor(index.data(Qt::TextColorRole).toString()));
    if (index.row() % 2 != 0) {
        painter->fillRect(option.rect, QBrush(QColor(0, 0, 0, 8))); //
    } else {
        if(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType){
            painter->fillRect(option.rect, QBrush(QColor(255, 255, 255, 150)));
        } else {
            painter->fillRect(option.rect, QBrush(QColor(255, 255, 255, 10)));
        }
    }

    switch (index.column()) {
    case 0: {
        QPixmap pic = index.data(Qt::DisplayRole).toString();
        QRect rect = QRect(option.rect.x() + 10, option.rect.y() + 2 + (option.rect.height() - pic.height())/2.f, pic.width(), pic.height());
        painter->drawPixmap(rect, pic);
    } break;
    case 1: {
        QRect rect = option.rect;
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, index.data(Qt::DisplayRole).toString());
    } break;
    case 2: {
        QRect rect = option.rect;
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, index.data(Qt::DisplayRole).toString());
    } break;
    default:
        break;
    }
}

BaseWidget::BaseWidget(const QString &text, QWidget *parent, Qt::WindowFlags f)
    :QLabel (text, parent, f)
{

}

void BaseWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter( this);
    const int radius = 8;
    QRect paintRect = e->rect();
    QPainterPath path;
    path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
    path.lineTo(paintRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)),
                     QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)),
                     QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)),
                     QSize(radius * 2, radius * 2)), 270, 90);
    painter.fillPath(path, DGuiApplicationHelper::instance()->applicationPalette().base());
}
