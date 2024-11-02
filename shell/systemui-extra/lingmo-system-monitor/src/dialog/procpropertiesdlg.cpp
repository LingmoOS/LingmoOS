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

#include "procpropertiesdlg.h"
#include "../process/process_list.h"
#include "../process/process_monitor.h"
#include "../util.h"
#include "../xatom-helper.h"
#include "../macro.h"
#include "../desktopfileinfo.h"

#include <QApplication>
#include <QDateTime>
#include <QPushButton>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QScreen>
#include <QWidget>
#include <QFileInfo>
#include <QIcon>
#include <QLineEdit>
#include <QPainterPath>

#define PROC_GRIDITEM_SPACING   6
#define PROC_GRIDLAYOUT_WIDTH   332
#define PROC_GRIDITEM_BOARD     2
#define PROC_GRIDITEM_TITLE_WIDTH  116
#define PROC_GRIDITEM_CONTENT_WIDTH  210
#define PROC_DIALOGTITLE_WIDTH  240

using namespace sysmonitor::process;

ProcPropertiesDlg::ProcPropertiesDlg(pid_t processId, QWidget *parent) : QDialog(parent)
  , mousePressed(false)
  , fontSettings(nullptr)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_Resized, false);

    this->setFixedSize(380, 300);

    const QByteArray id(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(id))
    {
        fontSettings = new QGSettings(id);
    }

    initFontSize();

    pid = processId;

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

//    QLabel *topSplit = new QLabel();
//    topSplit->setFixedSize(320, 1);

//    QLabel *bottomSplit = new QLabel();
//    bottomSplit->setFixedSize(320, 1);

    m_layoutFrame = new QVBoxLayout();
    m_layoutFrame->setContentsMargins(24,0,24,0);
    m_layoutFrame->setSpacing(10);

    m_bottomLayout = new QHBoxLayout;
    m_bottomLayout->setContentsMargins(24,24,24,24);
    m_bottomLayout->setSpacing(0);
    m_okBtn = new QPushButton;
    m_okBtn->setFixedWidth(96);
    m_okBtn->setObjectName("blackButton");
    m_okBtn->setFocusPolicy(Qt::NoFocus);
    m_okBtn->setText(tr("OK"));
    connect(m_okBtn, &QPushButton::clicked, this, [=] {
        this->close();
    });
    m_bottomLayout->addWidget(m_okBtn, 0, Qt::AlignBottom | Qt::AlignRight);

    m_layout->addSpacing(10);
    m_layout->addLayout(m_layoutFrame);
    m_layout->addLayout(m_bottomLayout);

    m_listTitleValue << QObject::tr("User name:") << QObject::tr("Process name:") << QObject::tr("Command line:") << QObject::tr("Started Time:") << QObject::tr("CPU Time:");
    for (int i = 0; i < m_listTitleValue.length(); ++i) {
        QLabel *titleLabel = new QLabel();
        titleLabel->setMinimumHeight(36);
        titleLabel->setFixedWidth(PROC_GRIDITEM_TITLE_WIDTH);
        titleLabel->setAlignment(Qt::AlignLeft);
        QString ShowValue = getElidedText(titleLabel->font(), m_listTitleValue.value(i), PROC_GRIDITEM_TITLE_WIDTH-PROC_GRIDITEM_BOARD);
        titleLabel->setText(ShowValue);
        if (ShowValue != m_listTitleValue.value(i))
            titleLabel->setToolTip(m_listTitleValue.value(i));
        m_labelTitleList << titleLabel;

        QLabel *infoLabel = new QLabel();
        infoLabel->setMinimumHeight(36);
        infoLabel->setFixedWidth(PROC_GRIDITEM_CONTENT_WIDTH);
        infoLabel->setAlignment(Qt::AlignLeft);
        m_labelList << infoLabel;
        QHBoxLayout *layoutItemLine = new QHBoxLayout();
        layoutItemLine->setContentsMargins(0,0,0,0);
        layoutItemLine->setSpacing(10);
        layoutItemLine->addWidget(titleLabel, 0, Qt::AlignLeft);
        layoutItemLine->addWidget(infoLabel, 0, Qt::AlignLeft);
        m_layoutFrame->addLayout(layoutItemLine);
    }

    this->initProcproperties();

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(refreshProcproperties()));
    timer->start(3000);
}

ProcPropertiesDlg::~ProcPropertiesDlg()
{
    if (timer != NULL) {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(refreshProcproperties()));
        if(timer->isActive()) {
            timer->stop();
        }
    }

    m_labelList.clear();
    m_labelTitleList.clear();

    if (fontSettings) {
        delete fontSettings;
        fontSettings = nullptr;
    }
}

void ProcPropertiesDlg::initFontSize()
{
    if (!fontSettings) {
        fontSize = DEFAULT_FONT_SIZE;
        return;
    }
    connect(fontSettings,&QGSettings::changed,[=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
            this->onThemeFontChange(fontSize);
        }
    });
    fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
}

void ProcPropertiesDlg::onThemeFontChange(unsigned uFontSize)
{
    Q_UNUSED(uFontSize);
    for (int i = 0; i < this->m_listTitleValue.length(); ++i) {
        QString ShowValue = getElidedText(m_labelTitleList.value(i)->font(), m_listTitleValue.value(i), PROC_GRIDITEM_TITLE_WIDTH-PROC_GRIDITEM_BOARD);
        this->m_labelTitleList.value(i)->setText(ShowValue);
        if (ShowValue != m_listTitleValue.value(i)) {
            this->m_labelTitleList.value(i)->setToolTip(m_listTitleValue.value(i));
        } else {
            this->m_labelTitleList.value(i)->setToolTip("");
        }
    }
    for (int i = 0; i < this->m_labelList.length(); ++i) {
        QString ShowValue = getElidedText(m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
        this->m_labelList.value(i)->setText(ShowValue);
        if (ShowValue != m_listValue.value(i)) {
            this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
        } else {
            this->m_labelList.value(i)->setToolTip("");
        }
    }
}

void ProcPropertiesDlg::updateLabelFrameHeight()
{
    foreach (QLabel *label, m_labelList) {
        label->adjustSize();
    }
    this->adjustSize();
}

void ProcPropertiesDlg::initProcproperties()
{
    sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(pid);
    if (info.isValid()) {
        QString username = info.getProcUser();
        QString name = info.getProcName();

        QString iconPath;
        QPixmap icon_pixmap = ProcessMonitor::instance()->processList()->getProcIcon(pid, iconPath, 24);

        m_strTitleName = DesktopFileInfo::instance()->getNameByExec(name);
        if (m_strTitleName.isEmpty()) {
            m_strTitleName = DesktopFileInfo::instance()->getAndroidAppNameByCmd(info.getProcArgments());
            if (m_strTitleName.isEmpty()) {
                m_strTitleName = ProcessMonitor::instance()->processList()->getWndNameByPid(pid);
                if (m_strTitleName.isEmpty()) {
                    m_strTitleName = name;
                }
            }
        }

//        this->setWindowIcon(QIcon(icon_pixmap));
        this->setWindowTitle(m_strTitleName);

        m_listValue << username << QString(info.getProcName()) << QString(info.getProcArgments())
                   << QFileInfo(QString("/proc/%1").arg(pid)).created().toString("yyyy-MM-dd hh:mm:ss")
                   << formatDurationForDisplay(100 * info.getProcCpuTime() / info.getFrequency());
        

        for (int i = 0; i < this->m_labelList.length(); ++i) {
            QString ShowValue = getElidedText(this->m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
            this->m_labelList.value(i)->setText(ShowValue);
            if (ShowValue != m_listValue.value(i)) {
                this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
            } else {
                this->m_labelList.value(i)->setToolTip("");
            }
        }
    }
    this->updateLabelFrameHeight();
}

void ProcPropertiesDlg::refreshProcproperties()
{
    sysmonitor::process::Process info = ProcessMonitor::instance()->processList()->getProcessById(pid);
    if (info.isValid()) {
        for (int i = 0; i < this->m_labelList.length(); ++i) {
            if (i == 3) {
                this->m_labelList.value(i)->setText(QFileInfo(QString("/proc/%1").arg(pid)).created().toString("yyyy-MM-dd hh:mm:ss"));
                m_listValue[i] = this->m_labelList.value(i)->text();
                QString ShowValue = getElidedText(this->m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
                this->m_labelList.value(i)->setText(ShowValue);
                if (ShowValue != m_listValue.value(i)) {
                    this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
                } else {
                    this->m_labelList.value(i)->setToolTip("");
                }                    
            } else if (i == 4) {
                this->m_labelList.value(i)->setText(formatDurationForDisplay(100 * info.getProcCpuTime() / info.getFrequency()));
                m_listValue[i] = this->m_labelList.value(i)->text();
                QString ShowValue = getElidedText(this->m_labelList.value(i)->font(), m_listValue.value(i), PROC_GRIDITEM_CONTENT_WIDTH-PROC_GRIDITEM_BOARD);
                this->m_labelList.value(i)->setText(ShowValue);
                if (ShowValue != m_listValue.value(i)) {
                    this->m_labelList.value(i)->setToolTip(m_listValue.value(i));
                } else {
                    this->m_labelList.value(i)->setToolTip("");
                }
            }
        }
    }
}

pid_t ProcPropertiesDlg::getPid()
{
    return pid;
}

QRect ProcPropertiesDlg::getParentGeometry() const
{
    if (this->parentWidget()) {
        return this->parentWidget()->window()->geometry();
    } else {
        QPoint pos = QCursor::pos();

        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                return screen->geometry();
            }
        }
    }

    return qApp->primaryScreen()->geometry();
}

void ProcPropertiesDlg::moveToCenter()
{
    QRect qr = geometry();
    qr.moveCenter(this->getParentGeometry().center());
    move(qr.topLeft());
}

void ProcPropertiesDlg::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->dragPosition = event->globalPos() - frameGeometry().topLeft();
        this->mousePressed = true;
    }

    QDialog::mousePressEvent(event);
}

void ProcPropertiesDlg::mouseReleaseEvent(QMouseEvent *event)
{
    this->mousePressed = false;

    QDialog::mouseReleaseEvent(event);
}

void ProcPropertiesDlg::mouseMoveEvent(QMouseEvent *event)
{
    QDialog::mouseMoveEvent(event);
}

void ProcPropertiesDlg::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QDialog::paintEvent(event);
}
