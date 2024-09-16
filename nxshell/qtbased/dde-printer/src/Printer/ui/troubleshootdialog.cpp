// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "troubleshootdialog.h"
#include "ztroubleshoot.h"
#include "zjobmanager.h"
#include "ztroubleshoot_p.h"
#include "qtconvert.h"

#include <DPalette>
#include <DApplicationHelper>
#include <DFrame>
#include <DWidget>
#include <DListView>
#include <DMainWindow>
#include <DFontSizeManager>
#include <DTitlebar>

#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>

TroubleShootItem::TroubleShootItem(TroubleShootJob *job, int index, QWidget *parent)
    : QFrame(parent)
    , m_job(job)
    , m_index(index)
{
    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(17, 17);
    m_iconLabel->setPixmap(QIcon(":/images/warning_logo.svg").pixmap(17, 17));
    m_titleLabel = new QLabel(job->getJobName(), this);
    m_titleLabel->setFont(DFontSizeManager::instance()->t5());
    m_messageLabel = new QLabel(this);
    m_messageLabel->setFont(DFontSizeManager::instance()->t7());

    m_iconLabel->setAccessibleName("iconLabel_shootItem");
    m_titleLabel->setAccessibleName("titleLabel_shootItem");
    m_messageLabel->setAccessibleName("msgLabel_shootItem");

    QGridLayout *lay = new QGridLayout(this);
    lay->setContentsMargins(10, 5, 10, 5);
    lay->setColumnStretch(1, 100);
    lay->addWidget(m_iconLabel, 0, 0, Qt::AlignCenter);
    lay->addWidget(m_titleLabel, 0, 1, Qt::AlignLeft | Qt::AlignHCenter);
    lay->addWidget(m_messageLabel, 1, 1, Qt::AlignLeft | Qt::AlignHCenter);
    setLayout(lay);
    setMaximumHeight(125);
    this->setAccessibleName("shootItem_troubleShoot");

    connect(job, &TroubleShootJob::signalStateChanged, this, &TroubleShootItem::slotStateChanged);
}

void TroubleShootItem::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    if (0 == (m_index % 2)) {
        DListView listView;
        DPalette pl(DApplicationHelper::instance()->palette(&listView));
        QPainter painter(this);
        painter.setBrush(pl.brush(QPalette::AlternateBase));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(event->rect(), 10, 10);
    }
}

void TroubleShootItem::slotStateChanged(int state, const QString &message)
{
    setHidden(false);

    if (TStat_Suc == state)
        m_iconLabel->setPixmap(QIcon(":/images/success.svg").pixmap(17, 17));

    QString messageInfo = message;
    geteElidedText(m_messageLabel->font(), messageInfo, 600);
    m_messageLabel->setToolTip(message);
    m_messageLabel->setText(messageInfo);
}

TroubleShootDialog::TroubleShootDialog(const QString &printerName, QWidget *parent)
    : DAbstractDialog(false, parent)
{
    m_printerName = printerName;
    DTitlebar *titleBar = new DTitlebar();
    titleBar->setIcon(QIcon(":/images/dde-printer.svg"));
    titleBar->setMenuVisible(false);
    titleBar->setTitle("");
    titleBar->setAccessibleName("titleBar_troubleShoot");

    m_trobleShoot = new TroubleShoot(printerName, this);
    m_trobleShoot->addJob(new CheckConnected(printerName, m_trobleShoot));

    setAttribute(Qt::WA_TranslucentBackground, false);
    QWidget *contentWidget = new QWidget(this);

    QLabel *title = new QLabel(tr("Troubleshoot: "), contentWidget);
    title->setMaximumHeight(45);
    title->setAccessibleName("title_contentWidget");
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T5, QFont::DemiBold);
    QHBoxLayout *pHLayout = new QHBoxLayout();

    m_helpButton = new QPushButton(this);
    m_helpButton->setContentsMargins(0, 0, 0, 0);
    DFontSizeManager::instance()->bind(m_helpButton, DFontSizeManager::T7, QFont::Light);
    m_helpButton->setText(tr("Can't print? Learn how to resolve"));
    QIcon icon = QIcon::fromTheme("icon_tips");
    m_helpButton->setIcon(icon);
    m_helpButton->setIconSize(QSize(22, 22));
    m_helpButton->setLayoutDirection(Qt::RightToLeft);
    QPalette palette;
    palette.setColor(QPalette::ButtonText, "#0081FF");
    m_helpButton->setPalette(palette);
    m_helpButton->setFlat(true);
    m_helpButton->setMinimumWidth(180);
    m_helpButton->setMaximumWidth(400);

    QHBoxLayout *helpLayout = new QHBoxLayout;
    helpLayout->addStretch();
    helpLayout->addWidget(m_helpButton);
    pHLayout->addWidget(title);
    pHLayout->addLayout(helpLayout);
    pHLayout->setContentsMargins(28, 10, 0, 0);

    DFrame *frame = new DFrame(contentWidget);
    QVBoxLayout *itemlays = new QVBoxLayout(frame);
    itemlays->addLayout(pHLayout);
    itemlays->setContentsMargins(0, 0, 0, 0);
    itemlays->setSpacing(0);
    QVBoxLayout *itemlay = new QVBoxLayout();
    itemlay->setContentsMargins(10, 0, 10, 10);
    itemlay->setSpacing(0);
    itemlay->addSpacing(10);
    QList<TroubleShootJob *> jobs = m_trobleShoot->getJobs();
    for (int i = 0; i < jobs.count(); i++) {
        TroubleShootItem *item = new TroubleShootItem(jobs[i], i, this);
        item->hide();
        itemlay->addWidget(item);
    }
    itemlay->addStretch();
    itemlays->addLayout(itemlay);
    frame->setLayout(itemlays);
    frame->setAccessibleName("frame_contentWidget");

    m_button = new QPushButton(contentWidget);
    m_button->setFixedWidth(200);
    m_button->setText(tr("Cancel", "button"));
    m_button->setFocusPolicy(Qt::NoFocus);
    m_button->setAccessibleName("button_contentWidget");
    QVBoxLayout *lay = new QVBoxLayout(contentWidget);
    lay->setSpacing(0);
    lay->setContentsMargins(10, 10, 10, 10);
    lay->addWidget(frame);
    lay->addSpacing(10);
    lay->addWidget(m_button, 0, Qt::AlignCenter);

    contentWidget->setLayout(lay);
    contentWidget->setAccessibleName("contentWidget_troubleShoot");

    QVBoxLayout *mainlay = new QVBoxLayout(this);
    mainlay->setContentsMargins(0, 0, 0, 0);
    mainlay->addWidget(titleBar);
    mainlay->addWidget(contentWidget);
    setLayout(mainlay);

    this->setFixedSize(692, 590);
    this->setAccessibleName("troubleShoot_mainWindow");

    connect(m_trobleShoot, &TroubleShoot::signalStatus, this, &TroubleShootDialog::slotTroubleShootStatus);
    connect(m_button, &QPushButton::clicked, this, &DDialog::close);
    connect(m_helpButton, &QPushButton::clicked, [this]() {
        if (!m_pHelpWindow) {
            m_pHelpWindow = new PrinterHelpWindow(this);
        }
        m_pHelpWindow->show();
    });

    m_trobleShoot->start();
}

void TroubleShootDialog::slotTroubleShootStatus(int id, int state)
{
    Q_UNUSED(id);

    if (TStat_Suc <= state) {
        m_button->setText(tr("OK", "button"));
    }
}
